#include "FontRenderBatch.h"
#include "CommandFactory.h"
#include "RenderPass.h"
#include "ShaderTags.h"
#include "GLM/glm.hpp"
#include <string>
#include <vector>

bool FontRenderBatch::s_bIsInitialized(false);
FT_Library FontRenderBatch::s_oFt;

FontRenderBatch::FontRenderBatch(Desc& oDesc)
{
	m_pRenderpass = oDesc.pRenderpass;
	m_pPipeline = oDesc.pPipeline;
	m_pFactory = oDesc.pFactory;
	m_pPool = oDesc.pPool;
	m_pWrapper = oDesc.pWrapper;
	m_bEnabled = true;

	if (!s_bIsInitialized)
	{
		Init();
	}

	if (FT_New_Face(s_oFt, oDesc.sFontName.c_str(), 0, &m_oFace))
	{
		std::cout << "Failed to load font" << std::endl;
	}

	FT_Set_Pixel_Sizes(m_oFace, 0, 48);

	for (unsigned char c = 0; c < 128; c++)
	{
		if (FT_Load_Char(m_oFace, c, FT_LOAD_RENDER))
		{
			std::cout << "Failed to load glyph" << std::endl;
		}

		if (m_oFace->glyph->bitmap.width == 0)
			continue;

		Image::FromBufferDesc oBufferCreateDesc;
		oBufferCreateDesc.eAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		oBufferCreateDesc.eTiling = VK_IMAGE_TILING_LINEAR;
		oBufferCreateDesc.bEnableMip = false;
		oBufferCreateDesc.eFormat = VK_FORMAT_R8_SRGB;
		oBufferCreateDesc.eSampleFlag = VK_SAMPLE_COUNT_1_BIT;
		oBufferCreateDesc.iHeight = m_oFace->glyph->bitmap.rows;
		oBufferCreateDesc.iWidth = m_oFace->glyph->bitmap.width;
		oBufferCreateDesc.pBuffer = m_oFace->glyph->bitmap.buffer;
		oBufferCreateDesc.pWrapper = oDesc.pWrapper;
		oBufferCreateDesc.pFactory = oDesc.pFactory;


		CharacterUnit oChar;
		oChar.pImage = Image::CreateFromBuffer(oBufferCreateDesc);
		oChar.pImage->TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_pFactory, 1);
		oChar.pDescriptorSet = m_pPipeline->GetDescriptorSetLayout()->InstantiateDescriptorSet(*m_pPool, *m_pWrapper->GetDevice());
		oChar.pDescriptorSet->FillSlotAtTag(oChar.pImage, TAG_COLORMAP);
		
		BasicBuffer::Desc oBufferDesc;
		oBufferDesc.eUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		oBufferDesc.iUnitCount = 3;
		oBufferDesc.iUnitSize = sizeof(glm::mat4);
		oBufferDesc.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		oBufferDesc.pWrapper = oDesc.pWrapper;
		BasicBuffer* pProjectionBuffer = new BasicBuffer(oBufferDesc);

		int iWidth, iHeight;
		oDesc.pWrapper->GetModifiableDevice()->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);
		std::vector<glm::mat4> oMats = { glm::mat4(1.0f) , glm::mat4(1.0f), glm::ortho(0.0f, (float)iWidth, 0.0f, (float)iHeight) };
		pProjectionBuffer->CopyFromMemory(oMats.data(), m_pWrapper->GetDevice());
		oChar.pDescriptorSet->FillSlot(1, pProjectionBuffer);

		oChar.pDescriptorSet->CommitSlots(m_pPool);

		oChar.vSize = glm::ivec2(m_oFace->glyph->bitmap.width, m_oFace->glyph->bitmap.rows);
		oChar.vBearing = glm::ivec2(m_oFace->glyph->bitmap_left, m_oFace->glyph->bitmap_top);
		oChar.iAdvance = m_oFace->glyph->advance.x;

		m_oCacheTextures[c] = oChar;
	}

	BasicBuffer::Desc oBufferDesc;
	oBufferDesc.eUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	oBufferDesc.iUnitCount = 6;
	oBufferDesc.iUnitSize = sizeof(glm::vec2) * 2;
	oBufferDesc.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	oBufferDesc.pWrapper = oDesc.pWrapper;
	for (int i = 0; i < 50; i++)
	{
		m_oBuffers.push_back(new BasicBuffer(oBufferDesc));
	}

	FT_Done_Face(m_oFace);
}

FontRenderBatch::TextInstance* FontRenderBatch::AddText(std::string sText, glm::vec4 vColor, glm::mat4 vTransform)
{
	TextInstance oInstance;
	oInstance.bDirty = false;
	oInstance.vColor = vColor;
	oInstance.sText = sText;
	oInstance.vTransform = vTransform;
	
	TextInstance* pInstance = new TextInstance(oInstance);
	m_oAllInstances.push_back(pInstance);

	return pInstance;
}

VkCommandBuffer* FontRenderBatch::GetDrawCommand(Framebuffer* pFramebuffer)
{
	VkCommandBufferBeginInfo oCommandBeginInfo{};
	oCommandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkCommandBuffer* pBuffer = m_pFactory->CreateCommand();
	if (vkBeginCommandBuffer(*pBuffer, &oCommandBeginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo oBeginInfo{};
	oBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	oBeginInfo.renderPass = *m_pRenderpass->GetRenderPass();
	oBeginInfo.framebuffer = *pFramebuffer->GetFramebuffer();

	int iWidth, iHeight;
	m_pWrapper->GetModifiableDevice()->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);

	VkExtent2D oExtent;
	oExtent.height = iHeight;
	oExtent.width = iWidth;

	oBeginInfo.renderArea.offset = { 0, 0 };
	oBeginInfo.renderArea.extent = oExtent;

	std::vector<VkClearValue> oClear(1);
	oClear[0].color = { 0.1f, 0.3f,0.8f, 1.0f };

	if ((Framebuffer::E_DEPTH & pFramebuffer->GetAttachmentFlags()) != 0)
	{
		VkClearValue oClearDepth;
		oClearDepth.depthStencil = { 1.0f, 0 };
		oClear.push_back(oClearDepth);
	}

	oBeginInfo.clearValueCount = static_cast<uint32_t>(oClear.size());
	oBeginInfo.pClearValues = oClear.data();

	vkCmdBeginRenderPass(*pBuffer, &oBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	ChainSubpass(pBuffer);

	vkCmdEndRenderPass(*pBuffer);
	if (vkEndCommandBuffer(*pBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command");
	}

	return pBuffer;
}

void FontRenderBatch::ChainSubpass(VkCommandBuffer* pCommand)
{
	if (m_bEnabled)
	{
		vkCmdBindPipeline(*pCommand, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pPipeline->GetPipeline());

		for (TextInstance* pInstance : m_oAllInstances)
		{
			int x = pInstance->vTransform[3][0];
			int y = pInstance->vTransform[3][1];
			int i = 0;
			for (char sCharacter : pInstance->sText)
			{
				if (sCharacter == ' ')
				{
					x += 2000 >> 6;
					continue;
				}

				CharacterUnit oCh = m_oCacheTextures[sCharacter];

				float xPos = x + (oCh.vBearing.x * pInstance->vTransform[0][0]);
				float yPos = y - (oCh.vSize.y * pInstance->vTransform[1][1]);
				yPos -= (oCh.vBearing.y - oCh.vSize.y) * pInstance->vTransform[1][1];

				float w = oCh.vSize.x * pInstance->vTransform[0][0];
				float h = oCh.vSize.y * pInstance->vTransform[1][1];

				float vPositions[] =
				{
					xPos, yPos + h, 0,0,
					xPos, yPos, 0,1,
					xPos + w, yPos, 1,1,
					xPos, yPos + h, 0,0,
					xPos + w, yPos, 1,1,
					xPos + w, yPos + h, 1,0
				};
				m_oBuffers[i]->CopyFromMemory(vPositions, m_pWrapper->GetDevice());

				VkDeviceSize oOffsets[] = { 0 };
				vkCmdBindVertexBuffers(*pCommand, 0, 1,m_oBuffers[i]->GetBuffer(), oOffsets);
				vkCmdBindDescriptorSets(*pCommand, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pPipeline->GetPipelineLayout(), 0, 1, m_oCacheTextures[sCharacter].pDescriptorSet->GetDescriptorSet(), 0, nullptr);
				vkCmdDraw(*pCommand, 6, 1, 0, 0);

				x += (oCh.iAdvance >> 6);
				i++;
			}
		}
	}

	if (m_pNext != nullptr)
	{
		vkCmdNextSubpass(*pCommand, VK_SUBPASS_CONTENTS_INLINE);
		m_pNext->ChainSubpass(pCommand);
	}

}

size_t FontRenderBatch::GetInstancesCount()
{
	int iSum = 0;
	for (TextInstance* pInstance : m_oAllInstances)
	{
		iSum += pInstance->sText.size();
	}
	return iSum;
}


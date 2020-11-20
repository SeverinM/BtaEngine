#include "Font.h"
#include <iostream>
#include "CommandFactory.h"
#include "RenderPass.h"
#include "ShaderTags.h"
#include "GLM/glm.hpp"

bool Font::s_bIsInitialized(false);
FT_Library Font::s_oFt;
BasicBuffer* Font::s_pProjectionBuffer(nullptr);

Font::Font(Desc& oDesc)
{
	m_pWrapper = oDesc.pWrapper;
	m_pRenderpass = oDesc.pRenderpass;
	m_pPipeline = oDesc.pPipeline;
	m_pFactory = oDesc.pFactory;
	m_pPool = oDesc.pPool;

	if (!s_bIsInitialized)
	{
		Init();

		BasicBuffer::Desc oDesc;
		oDesc.iUnitCount = 1;
		oDesc.iUnitSize = sizeof(glm::mat4);
		oDesc.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		oDesc.pWrapper = m_pWrapper;
		oDesc.eUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		
		Font::s_pProjectionBuffer = new BasicBuffer(oDesc);

		glm::mat4 projection = glm::ortho(0.0f, 1000.0f, 0.0f, 1000.0f);
		s_pProjectionBuffer->CopyFromMemory(&projection, m_pWrapper->GetDevice());
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

		Character oChar;
		oChar.pImage = Image::CreateFromBuffer(oBufferCreateDesc);
		oChar.pImage->TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_pFactory, 1);
		oChar.vSize = glm::ivec2(m_oFace->glyph->bitmap.width, m_oFace->glyph->bitmap.rows);
		oChar.vBearing = glm::ivec2(m_oFace->glyph->bitmap_left, m_oFace->glyph->bitmap_top);
		oChar.iAdvance = m_oFace->glyph->advance.x;
		oChar.pDescriptorSet = m_pPipeline->GetDescriptorSetLayout()->InstantiateDescriptorSet(*m_pPool, *m_pWrapper->GetDevice());
		oChar.pDescriptorSet->FillSlotAtTag(oChar.pImage, TAG_COLORMAP);
		oChar.pDescriptorSet->FillSlot(1, s_pProjectionBuffer);
		oChar.pDescriptorSet->CommitSlots(m_pPool);

		m_oCacheTextures[c] = oChar;
	}

	FT_Done_Face(m_oFace);

	//No longer string than 20
	for (int i = 0; i < 20; i++)
	{
		BasicBuffer::Desc oBufferDesc;
		oBufferDesc.eUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		oBufferDesc.iUnitCount = 6;
		oBufferDesc.iUnitSize = sizeof(glm::vec2) * 2;
		oBufferDesc.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		oBufferDesc.pWrapper = m_pWrapper;
		m_oVerticesBuffer.push_back( new BasicBuffer(oBufferDesc) );
	}
}

VkCommandBuffer Font::GetDrawCommand(std::string sText,Framebuffer* pFramebuffer,float x, float y, float fScale)
{
	if (m_oCacheCommand.count(pFramebuffer) > 0)
	{
		return *m_oCacheCommand[pFramebuffer];
	}

	VkCommandBuffer oBuffer = *m_pFactory->CreateCommand();

	VkCommandBufferBeginInfo oCommandBeginInfo{};
	oCommandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(oBuffer, &oCommandBeginInfo);
	
	VkRenderPassBeginInfo oBegin{};
	oBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	oBegin.renderPass = *m_pRenderpass->GetRenderPass();
	oBegin.renderArea.offset = { 0, 0 };
	oBegin.framebuffer = *pFramebuffer->GetFramebuffer();

	int iWidth, iHeight;
	m_pWrapper->GetModifiableDevice()->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);
	oBegin.renderArea.extent.height = (uint32_t)iHeight;
	oBegin.renderArea.extent.width = (uint32_t)iWidth;

	vkCmdBeginRenderPass(oBuffer, &oBegin, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(oBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pPipeline->GetPipeline());

	int i = 0;
	for (char sCharacter : sText)
	{
		if (sCharacter == ' ')
		{
			x += 2000 >> 6;
			continue;
		}

		Character oCh = m_oCacheTextures[sCharacter];

		float xPos = x + ( oCh.vBearing.x * fScale );
		float yPos = y - ( oCh.vSize.y * fScale);
		yPos -= (oCh.vBearing.y - oCh.vSize.y) * fScale;

		float w = oCh.vSize.x * fScale;
		float h = oCh.vSize.y * fScale; 

		float vPositions[] =
		{
			xPos, yPos +h, 0,0,
			xPos, yPos, 0,1,
			xPos + w, yPos, 1,1,
			xPos, yPos + h, 0,0,
			xPos + w, yPos, 1,1,
			xPos + w, yPos + h, 1,0
		};
		m_oVerticesBuffer[i]->CopyFromMemory(vPositions, m_pWrapper->GetDevice());

		VkDeviceSize oOffsets[] = { 0 };
		vkCmdBindVertexBuffers(oBuffer, 0, 1, m_oVerticesBuffer[i]->GetBuffer(),oOffsets);
		vkCmdBindDescriptorSets(oBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pPipeline->GetPipelineLayout(), 0, 1, oCh.pDescriptorSet->GetDescriptorSet(), 0, nullptr);
		
		vkCmdDraw(oBuffer, 6, 1, 0, 0);

		x += (oCh.iAdvance >> 6);
		i++;
	}

	vkCmdEndRenderPass(oBuffer);
	vkEndCommandBuffer(oBuffer);
	m_oCacheCommand[pFramebuffer] = new VkCommandBuffer( oBuffer );
	return oBuffer;
}

void Font::Init()
{
	if (FT_Init_FreeType(&s_oFt))
	{
		std::cout << "Error initializing free type" << std::endl;
	}
	s_bIsInitialized = true;
}


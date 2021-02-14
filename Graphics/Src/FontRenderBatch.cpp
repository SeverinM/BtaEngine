#include "FontRenderBatch.h"
#include "CommandFactory.h"
#include "RenderPass.h"
#include "ShaderTags.h"
#include "GLM/glm.hpp"
#include <string>
#include <vector>

namespace Bta
{
	namespace Graphic
	{
		bool FontRenderBatch::s_bIsInitialized(false);
		FT_Library FontRenderBatch::s_oFt;

		FontRenderBatch::FontRenderBatch(Desc& oDesc)
		{
			m_xCam = nullptr;
			m_pRenderpass = oDesc.pRenderpass;
			m_pPipeline = oDesc.pPipeline;
			m_pPool = oDesc.pPool;
			m_bEnabled = true;
			m_xVP = oDesc.xVPBuffer;

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

				CharacterUnit oChar;
				oChar.pImage = Image::CreateFromBuffer(oBufferCreateDesc);
				oChar.pImage->TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
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
			for (int i = 0; i < 50; i++)
			{
				m_oBuffers.push_back(GPUMemory::GetInstance()->AllocateMemory(oBufferDesc));
			}

			FT_Done_Face(m_oFace);
		}

		FontRenderBatch::TextInstance* FontRenderBatch::AddText(std::string sText, glm::vec4 vColor, glm::mat4 mTransform)
		{
			TextInstance oInstance;
			oInstance.vColor = vColor;
			oInstance.sText = sText;
			oInstance.m_pBatch = this;

			BasicBuffer::Desc oBufferDesc;
			oBufferDesc.eUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			oBufferDesc.iUnitCount = 1;
			oBufferDesc.iUnitSize = sizeof(glm::mat4);
			oBufferDesc.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			BasicBuffer* pModelBuffer = GPUMemory::GetInstance()->AllocateMemory(oBufferDesc);

			for (int i = 0; i < sText.size(); i++)
			{
				DescriptorSetWrapper* pDescriptor = m_pPipeline->GetDescriptorSetLayout()->InstantiateDescriptorSet(*m_pPool, *Bta::Graphic::Globals::g_pDevice);
				if (!(pDescriptor->FillSlot(0, m_oCacheTextures[sText[i]].pImage) && pDescriptor->FillSlot(1, m_xVP.get())))
				{
					throw std::runtime_error("Error");
				}

				if (i == 0)
				{
					glm::mat4 mInitial = glm::mat4(1.0f);
					oInstance.pBufferedTransform = new BufferedTransform(mInitial, 0, std::shared_ptr<Buffer>(pModelBuffer), Bta::Graphic::Globals::g_pDevice);
					oInstance.pBufferedTransform->ForceMatrix(mTransform);
				}

				if (!pDescriptor->FillSlot(2, pModelBuffer))
				{
					throw std::runtime_error("Error");
				}
				pDescriptor->CommitSlots(m_pPool);

				oInstance.oDescriptorSet.push_back(pDescriptor);
			}
			m_oAllInstances.push_back(new TextInstance(oInstance));

			return m_oAllInstances[m_oAllInstances.size() - 1];
		}

		VkCommandBuffer* FontRenderBatch::GetDrawCommand(Framebuffer* pFramebuffer)
		{
			VkCommandBufferBeginInfo oCommandBeginInfo{};
			oCommandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			VkCommandBuffer* pBuffer = Bta::Graphic::Globals::g_pFactory->CreateCommand();
			if (vkBeginCommandBuffer(*pBuffer, &oCommandBeginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo oBeginInfo{};
			oBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			oBeginInfo.renderPass = *m_pRenderpass->GetRenderPass();
			oBeginInfo.framebuffer = *pFramebuffer->GetFramebuffer();

			int iWidth, iHeight;
			Bta::Graphic::Globals::g_pDevice->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);

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
					int x = 0;
					int y = 0;
					int i = 0;
					for (char sCharacter : pInstance->sText)
					{
						if (sCharacter == ' ')
						{
							x += 2000 >> 6;
							continue;
						}

						CharacterUnit oCh = m_oCacheTextures[sCharacter];

						float xPos = x + (oCh.vBearing.x);
						float yPos = y - (oCh.vSize.y);
						yPos -= (oCh.vBearing.y - oCh.vSize.y);

						float w = oCh.vSize.x;
						float h = oCh.vSize.y;

						float vPositions[] =
						{
							xPos, yPos + h, 0,0,
							xPos, yPos, 0,1,
							xPos + w, yPos, 1,1,
							xPos, yPos + h, 0,0,
							xPos + w, yPos, 1,1,
							xPos + w, yPos + h, 1,0
						};
						m_oBuffers[i]->CopyFromMemory(vPositions, Bta::Graphic::Globals::g_pDevice);

						VkDeviceSize oOffsets[] = { 0 };
						vkCmdBindVertexBuffers(*pCommand, 0, 1, m_oBuffers[i]->GetBuffer(), oOffsets);
						vkCmdBindDescriptorSets(*pCommand, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pPipeline->GetPipelineLayout(), 0, 1, pInstance->oDescriptorSet[i]->GetDescriptorSet(), 0, nullptr);
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

		void FontRenderBatch::SetCamera(std::shared_ptr<Camera> xCam)
		{
		}
	}
}


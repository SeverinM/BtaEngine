#include "Output.h"
#include <algorithm>
#include "Globals.h"
#include "RenderBatch.h"
#include "Texture.h"
#include "CommandFactory.h"
#include "ImGuiWrapper.h"

namespace Bta
{
	namespace Graphic
	{
		Output::Output(Desc oDesc) : m_iCurrentFrame(0)
		{
			m_bNeedResize = false;
			m_oRecreateDesc = oDesc;
			m_pSwapchain = new Swapchain(oDesc.oSwapDesc);
			m_pSurface = oDesc.pRenderSurface;
			m_iMaxInFlightFrames = m_pSwapchain->GetImageViews().size();

			VkSemaphoreCreateInfo oSemaphore{};
			oSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo oFenceInfo{};
			oFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			oFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			m_oImageAcquiredSemaphore.resize(m_iMaxInFlightFrames);
			m_oRenderFinishedSemaphore.resize(m_iMaxInFlightFrames);
			m_oInFlightFrames.resize(m_iMaxInFlightFrames);

			glfwSetFramebufferSizeCallback(m_pSurface->GetWindow(), OnResizeWindow);

			for (int i = 0; i < m_iMaxInFlightFrames; i++)
			{
				if (vkCreateSemaphore(*Globals::g_pDevice->GetLogicalDevice(), &oSemaphore, nullptr, &m_oImageAcquiredSemaphore[i]) != VK_SUCCESS ||
					vkCreateSemaphore(*Globals::g_pDevice->GetLogicalDevice(), &oSemaphore, nullptr, &m_oRenderFinishedSemaphore[i]) != VK_SUCCESS ||
					vkCreateFence(*Globals::g_pDevice->GetLogicalDevice(), &oFenceInfo, nullptr, &m_oInFlightFrames[i]) != VK_SUCCESS )
				{
					throw std::runtime_error("Failed to create semaphore");
				}
			}

		}

		void Output::NextFrame()
		{
			m_iCurrentFrame = (m_iCurrentFrame + 1) % m_pSwapchain->GetImageViews().size();
		}

		void Output::Recreate()
		{
			vkDeviceWaitIdle(*Globals::g_pDevice->GetLogicalDevice());
			delete m_pSwapchain;
			for (Framebuffer* pFramebuffer : m_oFramebuffers)
			{
				delete pFramebuffer;
			}
			m_oFramebuffers.clear();
			m_pSwapchain = new Swapchain(m_oRecreateDesc.oSwapDesc);
		}

		void Output::Present()
		{
			VkPresentInfoKHR oPresentInfo{};
			oPresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			oPresentInfo.swapchainCount = 1;
			oPresentInfo.pSwapchains = m_pSwapchain->GetSwapchain();
			oPresentInfo.pImageIndices = &m_iCurrentFrame;
			oPresentInfo.pResults = nullptr;
			oPresentInfo.waitSemaphoreCount = 1;
			oPresentInfo.pWaitSemaphores = &m_oRenderFinishedSemaphore[m_iCurrentFrame];
			
			VkResult eResult = vkQueuePresentKHR(*Globals::g_pDevice->GetPresentQueue(), &oPresentInfo);
		}

		bool Output::RenderOneFrame(std::vector<RenderBatch*> oBatches, bool bIncludeImGui)
		{
			if (m_bNeedResize)
			{
				m_bNeedResize = false;
				return false;
			}

			int iInFlightIndex = m_iCurrentFrame % m_iMaxInFlightFrames;

			vkWaitForFences(*Globals::g_pDevice->GetLogicalDevice(), 1, &m_oInFlightFrames[iInFlightIndex], VK_TRUE, UINT64_MAX);

			uint32_t iReturnedImageIndex = 0;

			VkResult eAcquireResult = vkAcquireNextImageKHR(*Globals::g_pDevice->GetLogicalDevice(), *m_pSwapchain->GetSwapchain(), UINT64_MAX, m_oImageAcquiredSemaphore[m_iCurrentFrame], VK_NULL_HANDLE, &iReturnedImageIndex);

			VkSubmitInfo oSubmit{};
			oSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			oSubmit.waitSemaphoreCount = 1;
			oSubmit.pWaitSemaphores = &m_oImageAcquiredSemaphore[m_iCurrentFrame];
			VkPipelineStageFlags oWaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			oSubmit.pWaitDstStageMask = oWaitStages;
			
			std::vector<VkCommandBuffer> oCmdsBuffer;
			std::vector<VkCommandBuffer*> oCmdsToDelete;
			for ( RenderBatch* pRenderBatch : oBatches )
			{
				VkCommandBuffer oCommand = pRenderBatch->GetCommandBuffer(m_oFramebuffers[iInFlightIndex]);
				oCmdsBuffer.push_back(oCommand);
				oCmdsToDelete.push_back(&oCommand);
			}

			if (bIncludeImGui)
			{
				ImGuiWrapper::Desc oImDesc;
				oImDesc.iImageIndex = iInFlightIndex;
				oCmdsBuffer.push_back(Bta::Graphic::Globals::g_pImGui->GetDrawCommand(oImDesc));
			}

			oSubmit.commandBufferCount = oCmdsBuffer.size();
			oSubmit.pCommandBuffers = oCmdsBuffer.data();
			oSubmit.signalSemaphoreCount = 1;
			oSubmit.pSignalSemaphores = &m_oRenderFinishedSemaphore[m_iCurrentFrame];

			//If you remove this line the code will never wait
			vkResetFences(*Globals::g_pDevice->GetLogicalDevice(), 1, &m_oInFlightFrames[iInFlightIndex]);
			vkQueueSubmit(*Globals::g_pDevice->GetGraphicQueue(), 1, &oSubmit, m_oInFlightFrames[iInFlightIndex]);

			for (VkCommandBuffer* pCommand : oCmdsToDelete)
			{
				Globals::g_pFactory->FreeSingleTimeCommand(*pCommand);
			}

			if (bIncludeImGui)
			{
				Bta::Graphic::Globals::g_pImGui->GetFactory()->FreeSingleTimeCommand(oCmdsBuffer[oCmdsBuffer.size() - 1]);
			}
			oCmdsBuffer.clear();
			oCmdsToDelete.clear();

			return true;
		}

		void Output::GenerateFramebuffers(std::vector<VkFormat> oFormats, RenderBatch* pRender)
		{
			std::vector<VkImageUsageFlagBits> oUsages = { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT };
			std::vector<VkImageAspectFlagBits> oAspects = { VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_ASPECT_DEPTH_BIT };

			for (int i = 0; i < m_iMaxInFlightFrames; i++)
			{
				std::vector<VkImageView> oViews;
				Framebuffer::Desc oFramebufferDesc;

				oViews.push_back(m_pSwapchain->GetImageViews()[i]);

				for (int j = 1; j < oFormats.size(); j++)
				{
					Image::Desc oImgDesc;
					oImgDesc.bEnableMip = false;
					oImgDesc.bIsCubemap = false;
					oImgDesc.eAspect = oAspects[j % oAspects.size()];
					oImgDesc.eFormat = oFormats[j];
					oImgDesc.eProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
					oImgDesc.eSampleFlag = VK_SAMPLE_COUNT_1_BIT;
					oImgDesc.eTiling = VK_IMAGE_TILING_OPTIMAL;
					oImgDesc.eUsage = oUsages[j % oUsages.size()];
					oImgDesc.iLayerCount = 1;
					Globals::g_pOutput->GetRenderSurface()->GetWindowSize(oImgDesc.iWidth, oImgDesc.iHeight);
					Texture oText(oImgDesc, nullptr);
					
					oViews.push_back(*oText.GetImage()->GetImageView());
				}		
				
				oFramebufferDesc.pImageView = &oViews;
				oFramebufferDesc.pRenderPass = pRender->GetRenderPass();
				m_oFramebuffers.push_back(new Framebuffer(oFramebufferDesc));
			}
		}

		void Output::OnResizeWindow(GLFWwindow* pWindow, int iWidth, int iHeight)
		{
			Globals::g_pOutput->m_bNeedResize = true;
		}

	}
}


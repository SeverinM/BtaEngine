#include "RenderBatch.h"
#include "../../Core/Include/GLM/glm.hpp"
#include "Globals.h"
#include "Output.h"
#include "CommandFactory.h"
#include "GraphicDevice.h"

namespace Bta
{
	namespace Graphic
	{
		RenderBatch::RenderBatch(RenderBatch::Desc oDesc)
		{
			RenderPass::Desc oRenderPassDesc;
			std::vector<VkAttachmentDescription> oDescriptions;

			std::vector<VkImageLayout> oLayouts = { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL , VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			for (int i = 0; i < oDesc.oFramebufferLayout.size(); i++)
			{
				VkAttachmentDescription oDescription{};
				oDescription.format = oDesc.oFramebufferLayout[i];
				oDescription.samples = oDesc.iSampleCount;
				oDescription.loadOp = i == 1 ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				oDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				oDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				oDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				oDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //oLayouts[i % oLayouts.size()];
				oDescription.finalLayout = oLayouts[i % oLayouts.size()];
				oDescriptions.push_back(oDescription);
			}

			if (oDescriptions.size() >= 3)
			{
				oDescriptions[2].samples = VK_SAMPLE_COUNT_1_BIT;
				
				if (oDesc.bPresentable)
					oDescriptions[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}
			else
			{
				if ( oDesc.bPresentable )
					oDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}

			std::vector<RenderPass::SubDesc> oSubs;
			for (int i = 0; i < oDesc.oSubBatches.size(); i++)
			{
				RenderPass::SubDesc oSub;
				oSub.iColorAttachmentIndex = 0;
				oSub.iDepthStencilAttachmentIndex = oDescriptions.size() >= 2 ? 1 : -1;
				oSub.iColorResolveAttachmentIndex = oDescriptions.size() >= 3 ? 2 : -1;
				oSub.pDependency = nullptr;
				oSubs.push_back(oSub);
			}
			oRenderPassDesc.oSubpasses = oSubs;
			oRenderPassDesc.oDescriptions = oDescriptions;

			m_pRenderpass = new RenderPass(oRenderPassDesc);

			for (int i = 0; i < oDesc.oSubBatches.size(); i++)
			{
				SubRenderBatch* pSubBatch = new SubRenderBatch(oDesc.oSubBatches[i]);
				m_oSubBatches.push_back(pSubBatch);
				pSubBatch->CreatePipeline(this, i);
			}
		}

		VkCommandBuffer RenderBatch::GetCommandBuffer(Framebuffer* pFramebuffer)
		{
			VkCommandBuffer oCommandBuffer = Globals::g_pFactory->BeginSingleTimeCommands();
			
			VkRenderPassBeginInfo oRenderPassInfo{};
			oRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			oRenderPassInfo.renderPass = *m_pRenderpass->GetRenderPass();
			oRenderPassInfo.framebuffer = *pFramebuffer->GetFramebuffer();

			int iWidth = 0;
			int iHeight = 0;
			Bta::Graphic::Globals::g_pOutput->GetRenderSurface()->GetWindowSize(iWidth, iHeight);

			VkExtent2D oExtent;
			oExtent.height = iHeight;
			oExtent.width = iWidth;

			oRenderPassInfo.renderArea.offset = { 0, 0 };
			oRenderPassInfo.renderArea.extent = oExtent;

			std::vector<VkClearValue> oClear(1);
			oClear[0] = { 0.1f, 0.3f, 0.8f, 1.0f };

			if ((Framebuffer::E_DEPTH & pFramebuffer->GetAttachmentFlags()) != 0)
			{
				VkClearValue oClearDepth;
				oClearDepth.depthStencil = { 1.0f, 1 };
				oClear.push_back(oClearDepth);
			}

			oRenderPassInfo.clearValueCount = oClear.size();
			oRenderPassInfo.pClearValues = oClear.data();

			vkCmdBeginRenderPass(oCommandBuffer, &oRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			for (SubRenderBatch* pSubBatch : m_oSubBatches)
			{
				pSubBatch->FillCommandBuffer(oCommandBuffer);

				if (pSubBatch != m_oSubBatches[m_oSubBatches.size() - 1])
					vkCmdNextSubpass(oCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);
			}

			vkCmdEndRenderPass(oCommandBuffer);
			if (vkEndCommandBuffer(oCommandBuffer) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to record command");
			}

			return oCommandBuffer;
		}

		void RenderBatch::Recreate()
		{
			for (SubRenderBatch* pSubRender : m_oSubBatches)
			{
				pSubRender->m_pPipeline->Recreate();
			}
		}

	}
}

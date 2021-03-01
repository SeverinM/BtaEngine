#include "RenderPass.h"
#include <iostream>
#include "Globals.h"
#include "GraphicDevice.h"

namespace Bta
{
	namespace Graphic
	{
		RenderPass::RenderPass(Desc& oDesc)
		{
			m_pRecreate = new Desc(oDesc);
			Create(oDesc);
		}

		void RenderPass::Create(Desc& oDesc)
		{
			std::vector<VkSubpassDescription> oSubpasses;
			std::vector<VkSubpassDependency> oDependencies;
			for (int i = 0; i < oDesc.oSubpasses.size(); i++)
			{
				VkSubpassDescription oSubpassInfo{};
				oSubpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

				VkAttachmentReference oAttachmentRef{};
				oAttachmentRef.attachment = oDesc.oSubpasses[i].iDepthStencilAttachmentIndex >= 0 ? oDesc.oSubpasses[i].iDepthStencilAttachmentIndex : VK_ATTACHMENT_UNUSED;
				oAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				oSubpassInfo.pDepthStencilAttachment = new VkAttachmentReference(oAttachmentRef);

				VkAttachmentReference oColorAttachmentRef{};
				oColorAttachmentRef.attachment = oDesc.oSubpasses[i].iColorAttachmentIndex >= 0 ? oDesc.oSubpasses[i].iColorAttachmentIndex : VK_ATTACHMENT_UNUSED;
				oColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				oSubpassInfo.colorAttachmentCount = 1;
				oSubpassInfo.pColorAttachments = new VkAttachmentReference(oColorAttachmentRef);

				VkAttachmentReference oColorAttachmentResolveRef{};
				oColorAttachmentResolveRef.attachment = oDesc.oSubpasses[i].iColorResolveAttachmentIndex >= 0 ? oDesc.oSubpasses[i].iColorResolveAttachmentIndex : VK_ATTACHMENT_UNUSED;
				oColorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				oSubpassInfo.pResolveAttachments = new VkAttachmentReference(oColorAttachmentResolveRef);

				oSubpasses.push_back(oSubpassInfo);

				if (oDesc.oSubpasses[i].pDependency != nullptr)
				{
					oDependencies.push_back(*oDesc.oSubpasses[i].pDependency);
				}
				else
				{
					VkSubpassDependency oDependency{};
					oDependency.srcSubpass = (i == 0 ? VK_SUBPASS_EXTERNAL : i - 1);
					oDependency.dstSubpass = i;

					//Wait being in bottom of the pipe, then set output_bit in the bottom of the pipeline
					oDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					oDependency.dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

					oDependency.srcAccessMask = 0;
					oDependency.dstAccessMask = 0;
					oDependencies.push_back(oDependency);
				}
			}

			VkRenderPassCreateInfo oRenderPassInfo{};
			oRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			oRenderPassInfo.attachmentCount = (uint32_t)oDesc.oDescriptions.size();
			oRenderPassInfo.pAttachments = oDesc.oDescriptions.data();
			oRenderPassInfo.subpassCount = (uint32_t)oSubpasses.size();
			oRenderPassInfo.pSubpasses = oSubpasses.data();
			oRenderPassInfo.dependencyCount = (uint32_t)oDependencies.size();
			oRenderPassInfo.pDependencies = oDependencies.data();

			const GraphicDevice* pDevice = Globals::g_pDevice;
			if (vkCreateRenderPass(*pDevice->GetLogicalDevice(), &oRenderPassInfo, nullptr, &m_oRenderpass) != VK_SUCCESS)
			{
				throw std::runtime_error("Cannot create render pass");
			}
		}

		RenderPass::~RenderPass()
		{
			vkDestroyRenderPass(*Globals::g_pDevice->GetLogicalDevice(), m_oRenderpass, nullptr);
		}
	}
}

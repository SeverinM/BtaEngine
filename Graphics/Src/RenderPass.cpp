#include "RenderPass.h"
#include <iostream>
#include "Globals.h"

RenderPass::RenderPass(Desc& oDesc)
{
	m_pRecreate = new Desc(oDesc);
	Create(oDesc);
}

void RenderPass::Create(Desc& oDesc)
{
	VkAttachmentDescription oColorAttachment{};
	oColorAttachment.format = oDesc.eFormatColor;
	oColorAttachment.samples = oDesc.eSample;
	oColorAttachment.loadOp = oDesc.bClearColorAttachmentAtBegin ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	oColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	oColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	oColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	oColorAttachment.initialLayout = oDesc.eInitialLayoutColorAttachment;
	oColorAttachment.finalLayout = (oDesc.eSample != VK_SAMPLE_COUNT_1_BIT || !oDesc.bPresentable ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	VkAttachmentDescription oColorAttachmentResolve{};
	if (oDesc.eSample != VK_SAMPLE_COUNT_1_BIT)
	{
		oColorAttachmentResolve.format = oDesc.eFormatColor;//oDesc.pWrapper->GetSwapchain()->GetFormat();
		oColorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		oColorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		oColorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		oColorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		oColorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		oColorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		oColorAttachmentResolve.finalLayout = (oDesc.bPresentable ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}

	VkAttachmentDescription oDepthAttachment{};
	oDepthAttachment.format = VK_FORMAT_D32_SFLOAT;
	oDepthAttachment.samples = oDesc.eSample;
	oDepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	oDepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	oDepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	oDepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	oDepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	oDepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::vector<VkSubpassDescription> oSubpasses;
	std::vector<VkSubpassDependency> oDependencies;
	for (int i = 0; i < oDesc.oSubpasses.size(); i++)
	{
		VkSubpassDescription oSubpassInfo{};
		oSubpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		std::vector<VkAttachmentReference> oAttachmentRefs;

		oSubpassInfo.pDepthStencilAttachment = nullptr;
		if (oDesc.oSubpasses[i].iDepthStencilAttachmentIndex >= 0 && oDesc.bEnableDepth)
		{
			VkAttachmentReference oAttachmentRef{};
			oAttachmentRef.attachment = oDesc.oSubpasses[i].iDepthStencilAttachmentIndex;
			oAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			oAttachmentRefs.push_back(oAttachmentRef);

			oSubpassInfo.pDepthStencilAttachment = new VkAttachmentReference(oAttachmentRef);
		}

		oSubpassInfo.colorAttachmentCount = 0;
		oSubpassInfo.pColorAttachments = nullptr;
		if (oDesc.oSubpasses[i].iColorAttachmentIndex >= 0 && oDesc.bEnableColor)
		{
			VkAttachmentReference oColorAttachmentRef{};
			oColorAttachmentRef.attachment = oDesc.oSubpasses[i].iColorAttachmentIndex;
			oColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			oAttachmentRefs.push_back(oColorAttachmentRef);

			oSubpassInfo.colorAttachmentCount = 1;
			oSubpassInfo.pColorAttachments = new VkAttachmentReference(oColorAttachmentRef);
		}

		oSubpassInfo.pResolveAttachments = nullptr;
		if (oDesc.oSubpasses[i].iColorResolveAttachmentIndex >= 0 )
		{
			VkAttachmentReference oColorAttachmentResolveRef{};
			oColorAttachmentResolveRef.attachment = oDesc.oSubpasses[i].iColorResolveAttachmentIndex;
			oColorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			oAttachmentRefs.push_back(oColorAttachmentResolveRef);

			oSubpassInfo.pResolveAttachments = new VkAttachmentReference(oColorAttachmentResolveRef);
		}

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
	std::vector<VkAttachmentDescription> oAttachments;

	if (oDesc.bEnableColor)
	{
		oAttachments.push_back(oColorAttachment);
	}
	if (oDesc.bEnableDepth)
	{
		oAttachments.push_back(oDepthAttachment);
	}
	if (oDesc.eSample != VK_SAMPLE_COUNT_1_BIT)
	{
		oAttachments.push_back(oColorAttachmentResolve);
	}

	oRenderPassInfo.attachmentCount = (uint32_t)oAttachments.size();
	oRenderPassInfo.pAttachments = oAttachments.data();
	oRenderPassInfo.subpassCount =(uint32_t)oSubpasses.size();
	oRenderPassInfo.pSubpasses = oSubpasses.data();
	oRenderPassInfo.dependencyCount = (uint32_t)oDependencies.size();
	oRenderPassInfo.pDependencies = oDependencies.data();

	if (vkCreateRenderPass(*Graphics::Globals::g_pDevice->GetLogicalDevice(), &oRenderPassInfo, nullptr, &m_oRenderpass) != VK_SUCCESS)
	{
		throw std::runtime_error("Cannot create render pass");
	}
}

RenderPass::~RenderPass()
{
	vkDestroyRenderPass(*Graphics::Globals::g_pDevice->GetLogicalDevice(), m_oRenderpass, nullptr);
}

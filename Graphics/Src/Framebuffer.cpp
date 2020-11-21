#include "Framebuffer.h"
#include <iostream>
#include "RenderPass.h"
#include "BasicWrapper.h"
#include "Globals.h"

Framebuffer::Framebuffer(Desc& oDesc)
{
	int iWidth, iHeight;
	Graphics::Globals::g_pDevice->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);
	VkFramebufferCreateInfo oFramebufferInfo{};
	oFramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

	//define compatibility
	oFramebufferInfo.renderPass = *oDesc.pRenderPass->GetRenderPass();
	oFramebufferInfo.attachmentCount = (uint32_t)oDesc.pImageView->size();
	oFramebufferInfo.pAttachments = oDesc.pImageView->data();
	oFramebufferInfo.width = iWidth;
	oFramebufferInfo.height = iHeight;
	oFramebufferInfo.layers = 1;

	//Change after
	m_iAttachments = 0;
	for (int i = 0; i < oDesc.pImageView->size(); i++)
	{
		m_iAttachments |= (1 << i);
	}

	if (vkCreateFramebuffer(*Graphics::Globals::g_pDevice->GetLogicalDevice(), &oFramebufferInfo, nullptr, &m_oFramebuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create framebuffer");
	}
}

Framebuffer::~Framebuffer()
{
	vkDestroyFramebuffer(*Graphics::Globals::g_pDevice->GetLogicalDevice(), m_oFramebuffer, nullptr);
}

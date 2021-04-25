#include "Framebuffer.h"
#include <iostream>
#include "RenderPass.h"
#include "Globals.h"
#include "Output.h"

namespace Bta
{
	namespace Graphic
	{
		Framebuffer::Framebuffer(Desc& oDesc)
		{
			int iWidth, iHeight;
			Globals::g_pOutput->GetRenderSurface()->GetWindowSize(iWidth, iHeight);
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

			if (vkCreateFramebuffer(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), &oFramebufferInfo, nullptr, &m_oFramebuffer) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create framebuffer");
			}

			m_oImageViews = oDesc.pImageView;
		}

		Framebuffer::~Framebuffer()
		{
			vkDestroyFramebuffer(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), m_oFramebuffer, nullptr);

			/*bool bFirst = true;
			for (VkImageView& oImageView : *m_oImageViews)
			{
				if (bFirst)
					bFirst = false;
				else
					vkDestroyImageView(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), oImageView, nullptr);
			}*/
		}
	}
}

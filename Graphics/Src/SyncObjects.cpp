#include "SyncObjects.h"
#include <iostream>
#include "Globals.h"

SyncObjects::SyncObjects(Desc& oDesc)
{
	m_iCurrentFrame = 0;
	m_iMaxFrame = oDesc.iFrameOnFlight;

	m_oImageAcquiredSemaphores.resize(oDesc.iNumberImages);
	m_oRenderFinishedSemaphores.resize(oDesc.iNumberImages);
	m_oRenderFinishSemaphoresImGUI.resize(oDesc.iNumberImages);
	m_oInFlightFences.resize(oDesc.iNumberImages);

	m_oSwapChainImages.resize(oDesc.iNumberImages);

	VkSemaphoreCreateInfo oSemaphore{};
	oSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo oFenceInfo{};
	oFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	oFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < oDesc.iFrameOnFlight; i++)
	{
		if (vkCreateSemaphore(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), &oSemaphore, nullptr, &m_oImageAcquiredSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), &oSemaphore, nullptr, &m_oRenderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), &oFenceInfo, nullptr, &m_oInFlightFences[i]) != VK_SUCCESS ||
			vkCreateSemaphore(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), &oSemaphore, nullptr, &m_oRenderFinishSemaphoresImGUI[i] ) )
		{
			throw std::runtime_error("Failed to create semaphore");
		}
	}
}

void SyncObjects::NextFrame()
{
	m_iCurrentFrame = (m_iCurrentFrame + 1 )% m_iMaxFrame;
}


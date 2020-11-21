#ifndef H_SYNC_OBJECT
#define H_SYNC_OBJECT
#include "vulkan/vulkan_core.h"
#include <vector>
#include "GraphicWrapper.h"

class SyncObjects
{
public:
	struct Desc
	{
		int iFrameOnFlight;
		int iNumberImages;
	};
	SyncObjects(Desc& oDesc);

	void NextFrame();

	int GetFrame() { return m_iCurrentFrame; }
	std::vector<VkSemaphore>& GetImageAcquiredSemaphore() { return m_oImageAcquiredSemaphores; };
	std::vector<VkSemaphore>& GetRenderFinishedSemaphore() { return m_oRenderFinishedSemaphores; };
	std::vector<VkFence>& GetInFlightFences() { return m_oInFlightFences; };
	std::vector<VkFence>& GetSwapChainImagesFences() { return m_oSwapChainImages; };
	std::vector<VkSemaphore>& GetRenderFinishedSemaphoreImGui() { return m_oRenderFinishSemaphoresImGUI; };

protected:
	int m_iCurrentFrame;
	int m_iMaxFrame;

	//Semaphores
	std::vector<VkSemaphore> m_oImageAcquiredSemaphores;
	std::vector<VkSemaphore> m_oRenderFinishedSemaphores;
	std::vector<VkSemaphore> m_oRenderFinishSemaphoresImGUI;

	std::vector<VkFence> m_oInFlightFences;
	std::vector<VkFence> m_oSwapChainImages;
};

#endif H_SYNC_OBJECT
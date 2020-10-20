#ifndef H_GRAPHIC_DEVICE 
#define H_GRAPHIC_DEVICE
#include "vulkan/vulkan_core.h"
#include <vector>
#include "GLFW/glfw3.h"
#include "RenderSurface.h"

class GraphicDevice
{
private:
	VkQueue m_oGraphicQueue;
	VkQueue m_oPresentQueue;
	VkPhysicalDevice* m_pPhysicalDevice;
	VkDevice m_oDevice;
	VkSampleCountFlagBits m_iMaxMsaa;
	VkSampleCountFlagBits GetMaxSampleCount();
	Window::RenderSurface* m_pRenderSurface;
	int m_iGraphicQueueIndex;
	int m_iPresentQueueIndex;

public:
	struct Desc
	{
		VkInstance* pInstance;
		bool bEnableAnisotropy;
		std::vector<const char*> oExtensions;
		Window::RenderSurface* pSurface;
	};
	const Window::RenderSurface* const GetRenderSurface() const { return m_pRenderSurface; }
	Window::RenderSurface* GetModifiableRenderSurface() { return m_pRenderSurface; }
	const VkDevice* const GetLogicalDevice() const { return &m_oDevice; }
	const VkPhysicalDevice* const GetPhysicalDevice() const { return m_pPhysicalDevice; }
	int GetGraphicQueueIndex() const { return m_iGraphicQueueIndex; }
	int GetPresentQueueIndex() const { return m_iPresentQueueIndex; }
	const VkQueue* GetGraphicQueue() const { return &m_oGraphicQueue; }
	const VkQueue* GetPresentQueue() const { return &m_oPresentQueue; }

	GraphicDevice(Desc& oDesc);
	~GraphicDevice()
	{
		vkDestroyDevice(m_oDevice, nullptr);
	};
};

#endif // H_GRAPHIC_DEVICE 

#ifndef H_GRAPHIC_WRAPPER
#define H_GRAPHIC_WRAPPER
#include "vulkan/vulkan_core.h"
#include <string>
#include <vector>
#include <GLFW/glfw3.h>
#include "GraphicDevice.h"
#include <memory>

class SyncObjects;

class GraphicWrapper
{
public:
	struct Desc
	{
		VkPhysicalDevice* pPhysicalDevice;
		bool bEnableDebug;
		std::vector<const char*> oRequiredExtensionsDevice;
		Window::RenderSurface* pSurface;
	};

	virtual void CreateInstance() = 0;
	virtual void CreateGraphicDevice() = 0;
	virtual void CreateSwapChain() = 0;
	virtual void CreateGraphicPipeline() = 0;
	virtual void CreateRenderPass() = 0;
	virtual void CreateCommandBuffer() = 0;

	virtual bool Render(SyncObjects* oSync) = 0;

	void Init()
	{
		CreateInstance();
		CreateGraphicDevice();
		CreateSwapChain();
		CreateRenderPass();
		CreateGraphicPipeline();
		CreateCommandBuffer();
	}

	GraphicDevice* GetDevice() const { return m_pDevice; }
	GraphicDevice* GetModifiableDevice() { return m_pDevice; }

protected :
	Desc* m_pDesc;
	GraphicDevice* m_pDevice;

};

#endif H_GRAPHIC_WRAPPER


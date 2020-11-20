#ifndef H_GLOBAL
#define H_GLOBAL
#include "vulkan/vulkan_core.h"
#include <iostream>

class GraphicDevice;

namespace Graphics
{
	struct Globals
	{
		static VkInstance g_oInstance;
		static float g_fElapsed;
		static GraphicDevice* g_pDevice;
	};

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback
	(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

}
#endif

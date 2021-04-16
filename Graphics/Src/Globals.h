#ifndef H_GLOBAL
#define H_GLOBAL
#include "vulkan/vulkan_core.h"
#include <iostream>

namespace Bta
{
	namespace Graphic
	{
		class GraphicDevice;
		class DescriptorPool;
		class Camera;
		class CommandFactory;
		class Output;
		class ImGuiWrapper;

		struct Globals
		{
			static VkInstance g_oInstance;
			static float g_fElapsed;
			static GraphicDevice* g_pDevice;
			static DescriptorPool* g_pPool;
			static Output* g_pOutput;
			static CommandFactory* g_pFactory;
			static ImGuiWrapper* g_pImGui;
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
}
#endif

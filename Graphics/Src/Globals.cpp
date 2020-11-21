#include "Globals.h"

namespace Graphics
{
	VkInstance Globals::g_oInstance;
	float Globals::g_fElapsed(0.0f);
	GraphicDevice* Globals::g_pDevice(nullptr);
	DescriptorPool* Globals::g_pPool(nullptr);
}
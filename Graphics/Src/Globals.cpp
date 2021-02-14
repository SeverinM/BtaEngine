#include "Globals.h"

namespace Bta
{
	namespace Graphic
	{
		VkInstance Globals::g_oInstance;
		float Globals::g_fElapsed(0.0f);
		GraphicDevice* Globals::g_pDevice(nullptr);
		DescriptorPool* Globals::g_pPool(nullptr);
		Camera* Globals::g_pCamera(nullptr);
		CommandFactory* Globals::g_pFactory(nullptr);
	}
}

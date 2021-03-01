#include "Globals.h"
#include "Output.h"

namespace Bta
{
	namespace Graphic
	{
		VkInstance Globals::g_oInstance;
		float Globals::g_fElapsed(0.0f);
		GraphicDevice* Globals::g_pDevice(nullptr);
		DescriptorPool* Globals::g_pPool(nullptr);
		CommandFactory* Globals::g_pFactory(nullptr);
		Output* Globals::g_pOutput(nullptr);
	}
}

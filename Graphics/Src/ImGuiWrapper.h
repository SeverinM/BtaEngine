#ifndef H_IM_GUI_WRAPPER
#define H_IM_GUI_WRAPPER
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "Framebuffer.h"
#include <functional>

namespace Bta
{
	namespace Graphic
	{
		class RenderPass;
		class CommandFactory;
		class Output;

		class ImGuiWrapper
		{
			public:
				typedef std::function<void(void*)> ImGuiRenderCallBack;
				struct Desc
				{
					int iImageIndex;
					ImGuiRenderCallBack pCallback;
				};
				ImGuiWrapper(Desc& oDesc);
				~ImGuiWrapper();
				VkCommandBuffer GetDrawCommand(Desc& oDesc);
				void Recreate();
				void SetCallback(ImGuiRenderCallBack pCallback) { m_pCallback = pCallback; }
				static void CheckError(VkResult eResult);
				inline CommandFactory* GetFactory() { return m_pFactory; }

			protected:
				Desc m_oRecreateDesc;
				RenderPass* m_pRenderpass;
				CommandFactory* m_pFactory;
				std::vector<Framebuffer*> m_oFramebuffer;
				ImGuiRenderCallBack m_pCallback;
		};
	}
}

#endif
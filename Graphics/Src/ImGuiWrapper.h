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
				VkCommandBuffer* GetDrawCommand(Desc& oDesc);
				void Recreate(Desc& oDesc, int iMinImage);
				void SetCallback(ImGuiRenderCallBack pCallback) { m_pCallback = pCallback; }
				static void CheckError(VkResult eResult);
				inline CommandFactory* GetFactory() { return m_pFactory; }

			protected:
				RenderPass* m_pRenderpass;
				CommandFactory* m_pFactory;
				std::vector<Framebuffer*> m_oFramebuffer;
				std::vector<VkCommandBuffer> m_oCommandBuffer;
				ImGuiRenderCallBack m_pCallback;
		};
	}
}

#endif
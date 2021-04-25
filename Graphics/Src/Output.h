#ifndef H_OUTPUT
#define H_OUTPUT

#include "Swapchain.h"
#include <vector>
#include "Framebuffer.h"

namespace Bta
{
	namespace Graphic
	{
		class RenderBatch;

		class Output
		{
			public:
				struct Desc
				{
					Swapchain::Desc oSwapDesc;
					Window::RenderSurface* pRenderSurface;
					int iMaxInFlight;
					std::vector<VkFormat> oFormats;
				};

				Output(Desc oDesc);
				Swapchain* GetSwapchain() { return m_pSwapchain; }
				Window::RenderSurface* GetRenderSurface() { return m_pSurface; }

				void NextFrame();
				void Recreate();
				void Present();
				bool RenderOneFrame(std::vector<RenderBatch*> oBatches, bool bIncludeImGui = false );
				void GenerateFramebuffers(std::vector<VkFormat> oFormats, RenderBatch* pRender);

			protected:
				Swapchain* m_pSwapchain;
				Window::RenderSurface* m_pSurface;

				Desc m_oRecreateDesc;

				std::vector<Framebuffer*> m_oFramebuffers;
				std::vector<VkSemaphore> m_oImageAcquiredSemaphore;
				std::vector<VkSemaphore> m_oRenderFinishedSemaphore;
				std::vector<VkFence> m_oInFlightFrames;
				uint32_t m_iCurrentFrame;
				int m_iMaxInFlightFrames;
				bool m_bNeedResize;

				static void OnResizeWindow(GLFWwindow* pWindow, int iWidth, int iHeight);
		};
	}
}

#endif
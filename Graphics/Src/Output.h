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
				void Present();
				void RenderOneFrame(std::vector<RenderBatch*> oBatches);
				void GenerateFramebuffers(std::vector<VkFormat> oFormats, RenderBatch* pRender);

			protected:
				Swapchain* m_pSwapchain;
				Window::RenderSurface* m_pSurface;


				std::vector<Framebuffer*> m_oFramebuffers;
				std::vector<VkSemaphore> m_oImageAcquiredSemaphore;
				std::vector<VkSemaphore> m_oRenderFinishedSemaphore;
				std::vector<VkFence> m_oInFlightFrames;
				int m_iCurrentFrame;
				int m_iMaxInFlightFrames;
		};
	}
}

#endif
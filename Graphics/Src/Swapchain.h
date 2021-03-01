#ifndef H_SWAP_CHAIN
#define H_SWAP_CHAIN
#include "GraphicDevice.h"

namespace Bta
{
	namespace Graphic
	{
		class Swapchain
		{
			public:
				struct Desc
				{
					VkFormat eImagesFormat;
					VkColorSpaceKHR eColorspace;
					VkPresentModeKHR ePresentMode;
					int iImageLayers;
					Window::RenderSurface* pRenderSurface;
				};

				Swapchain(Desc& oDesc);
				void Create(Desc& oDesc);
				~Swapchain();
				size_t GetNumberImages() const { return m_oImages.size(); }
				const VkSwapchainKHR* GetSwapchain() const { return &m_oSwapchain; }
				VkFormat GetFormat() const { return m_eFormat; }
				std::vector<VkImageView> GetImageViews()const { return m_oViews; }

			protected:
				Desc* m_pRecreate;
				std::vector<VkImage> m_oImages;
				std::vector<VkImageView> m_oViews;
				VkFormat m_eFormat;
				VkSwapchainKHR m_oSwapchain;
				void CreateViews(Desc& oDesc);
		};
	}
}

#endif // ! H_SWAP_CHAIN

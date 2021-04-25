#include "Swapchain.h"
#include <iostream>
#include "Globals.h"
#include "Output.h"

namespace Bta
{
	namespace Graphic
	{
		void FetchSwapChainCapabilities(VkSurfaceCapabilitiesKHR& oCapabilities, Window::RenderSurface* pSurface, std::vector<VkSurfaceFormatKHR>& oFormats, std::vector<VkPresentModeKHR>& oPresentModes)
		{
			const VkPhysicalDevice* const pPhysical = Globals::g_pDevice->GetPhysicalDevice();

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*pPhysical, *pSurface->GetSurface(), &oCapabilities);

			uint32_t iFormatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(*pPhysical, *pSurface->GetSurface(), &iFormatCount, nullptr);

			if (iFormatCount != 0)
			{
				oFormats.resize(iFormatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(*Globals::g_pDevice->GetPhysicalDevice(), *(pSurface->GetSurface()), &iFormatCount, oFormats.data());
			}

			uint32_t iPresentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(*Globals::g_pDevice->GetPhysicalDevice(), *(pSurface->GetSurface()), &iPresentModeCount, nullptr);

			if (iPresentModeCount != 0)
			{
				oPresentModes.resize(iPresentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(*Globals::g_pDevice->GetPhysicalDevice(), *(pSurface->GetSurface()), &iPresentModeCount, oPresentModes.data());
			}
		}

		Swapchain::Swapchain(Desc& oDesc)
		{
			m_pRecreate = new Desc(oDesc);
			Create(oDesc);
		}

		void Swapchain::Create(Desc& oDesc)
		{
			VkSurfaceCapabilitiesKHR oCapabilities;
			std::vector<VkSurfaceFormatKHR> oFormats;
			std::vector<VkPresentModeKHR> oPresentModes;

			FetchSwapChainCapabilities(oCapabilities,oDesc.pRenderSurface, oFormats, oPresentModes);

			VkSurfaceFormatKHR oFormat;
			for (VkSurfaceFormatKHR& oItFormat : oFormats)
			{
				if ((oItFormat.format == oDesc.eImagesFormat) && (oItFormat.colorSpace == oDesc.eColorspace))
				{
					oFormat = oItFormat;
					break;
				}
			}

			VkPresentModeKHR oPresentMode;
			for (VkPresentModeKHR& oItPresentMode : oPresentModes)
			{
				if (oItPresentMode == oDesc.ePresentMode)
				{
					oPresentMode = oItPresentMode;
					break;
				}
			}

			uint32_t iImageCount = oCapabilities.minImageCount + 1;

			//Clamp
			if (oCapabilities.maxImageCount > 0 && iImageCount > oCapabilities.maxImageCount)
			{
				iImageCount = oCapabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR oCreateInfo{};
			oCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

			//Where to present
			oCreateInfo.surface = *oDesc.pRenderSurface->GetSurface();
			oCreateInfo.minImageCount = iImageCount;

			//Format of each image
			m_eFormat = oCreateInfo.imageFormat = oFormat.format;
			oCreateInfo.imageColorSpace = oFormat.colorSpace;

			VkExtent2D oExtent;
			int iHeight, iWidth;
			oDesc.pRenderSurface->GetWindowSize(iWidth, iHeight);
			oExtent.height = iHeight;
			oExtent.width = iWidth;
			oCreateInfo.imageExtent = oExtent;
			oCreateInfo.imageArrayLayers = oDesc.iImageLayers;

			//Will be used as color attachment
			oCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			uint32_t iQueueFamilyIndex[] = { Bta::Graphic::Globals::g_pDevice->GetGraphicQueueIndex(), Bta::Graphic::Globals::g_pDevice->GetPresentQueueIndex() };

			if (Bta::Graphic::Globals::g_pDevice->GetGraphicQueueIndex() != Bta::Graphic::Globals::g_pDevice->GetPresentQueueIndex())
			{
				oCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				oCreateInfo.queueFamilyIndexCount = 2;
				oCreateInfo.pQueueFamilyIndices = iQueueFamilyIndex;
			}
			else
			{
				oCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			}

			//No transformation before presentation
			oCreateInfo.preTransform = oCapabilities.currentTransform;

			//No blend with other window
			oCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			oCreateInfo.presentMode = oPresentMode;

			//Ignore if a window is front
			oCreateInfo.clipped = VK_TRUE;

			//Recreate swap chain
			oCreateInfo.oldSwapchain = VK_NULL_HANDLE;

			if (vkCreateSwapchainKHR(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), &oCreateInfo, nullptr, &m_oSwapchain) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create swap chain");
			}

			vkGetSwapchainImagesKHR(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), m_oSwapchain, &iImageCount, nullptr);
			m_oImages.resize(iImageCount);
			vkGetSwapchainImagesKHR(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), m_oSwapchain, &iImageCount, m_oImages.data());

			CreateViews(oDesc);
		}

		Swapchain::~Swapchain()
		{
			m_oImages.clear();
			for (VkImageView& oImageView : m_oViews)
			{
				vkDestroyImageView(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), oImageView, nullptr);
			}
			m_oViews.clear();

			for (VkImage& oImage : m_oImages)
			{
				vkDestroyImage(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), oImage, nullptr);
			}
			m_oImages.clear();

			vkDestroySwapchainKHR(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), m_oSwapchain, nullptr);
		}

		void Swapchain::CreateViews(Desc& oDesc)
		{
			m_oViews.resize(m_oImages.size());
			for (size_t i = 0; i < m_oImages.size(); i++)
			{
				VkImageViewCreateInfo oViewCreateInfo{};
				oViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				oViewCreateInfo.image = m_oImages[i];
				oViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				oViewCreateInfo.format = oDesc.eImagesFormat;

				oViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				oViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				oViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				oViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

				oViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				oViewCreateInfo.subresourceRange.baseMipLevel = 0;
				oViewCreateInfo.subresourceRange.levelCount = 1;
				oViewCreateInfo.subresourceRange.baseArrayLayer = 0;
				oViewCreateInfo.subresourceRange.layerCount = 1;

				if (vkCreateImageView(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), &oViewCreateInfo, nullptr, &m_oViews[i]) != VK_SUCCESS)
				{
					throw std::runtime_error("Cannot create image view");
				}
			}
		}
	}
}

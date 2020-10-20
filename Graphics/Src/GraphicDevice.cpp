#include "GraphicDevice.h"
#include <set>
#include "Globals.h"
#include "BasicWrapper.h"

int RateDevice(const VkPhysicalDevice* pPhysicalDevice, std::vector<const char*>& oExtensions,const VkSurfaceKHR& oSurface, int& iGraphicQueue, int& iPresentQueue)
{
	VkPhysicalDeviceProperties oDeviceProperties;
	vkGetPhysicalDeviceProperties(*pPhysicalDevice, &oDeviceProperties);

	VkPhysicalDeviceFeatures oDeviceFeatures;
	vkGetPhysicalDeviceFeatures(*pPhysicalDevice, &oDeviceFeatures);

	int iScore = 0;

	//Separated GPU from CPU
	if (oDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		iScore += 1000;
	}

	if (oDeviceFeatures.samplerAnisotropy)
	{
		iScore += 100;
	}

	bool bAllExtensions = false;

	//MaxSize Texture
	iScore += oDeviceProperties.limits.maxImageDimension2D * 100;

	uint32_t iExtensionCount;
	vkEnumerateDeviceExtensionProperties(*pPhysicalDevice, nullptr, &iExtensionCount, nullptr);

	std::vector<VkExtensionProperties> oAvailableExtensions(iExtensionCount);
	vkEnumerateDeviceExtensionProperties(*pPhysicalDevice, nullptr, &iExtensionCount, oAvailableExtensions.data());

	//Is all requested extensions exists ?
	std::set<const char*> oExtensionRequired(oExtensions.begin(), oExtensions.end());
	for (VkExtensionProperties& oExtension : oAvailableExtensions)
	{
		iScore++;
		oExtensionRequired.erase(oExtension.extensionName);
	}

	if (oExtensionRequired.empty())
	{
		iScore += 10;
		bAllExtensions = true;
	}

	//Can present images ?
	if (bAllExtensions)
	{
		VkSurfaceCapabilitiesKHR oCapabilities;
		std::vector<VkSurfaceFormatKHR> oFormats;
		std::vector<VkPresentModeKHR> oPresentModes;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*pPhysicalDevice, oSurface, &oCapabilities);

		uint32_t iFormatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(*pPhysicalDevice, oSurface, &iFormatCount, nullptr);

		//Can support any format ?
		if (iFormatCount != 0)
		{
			oFormats.resize(iFormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(*pPhysicalDevice, oSurface, &iFormatCount, oFormats.data());
		}

		uint32_t iPresentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(*pPhysicalDevice, oSurface, &iPresentModeCount, nullptr);

		if (iPresentModeCount != 0)
		{
			oPresentModes.resize(iPresentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(*pPhysicalDevice, oSurface, &iPresentModeCount, oPresentModes.data());
		}

		iScore += (!oFormats.empty() && !oPresentModes.empty()) ? 1000 : 0;
	}

	if (!oDeviceFeatures.geometryShader)
	{
		return -1;
	}

	uint32_t iQueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(*pPhysicalDevice, &iQueueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> oQueueFamilies(iQueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(*pPhysicalDevice, &iQueueFamilyCount, oQueueFamilies.data());

	int iQueueIndex = 0;
	for (const auto& oQueueFamily : oQueueFamilies)
	{
		if (oQueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			iGraphicQueue = iQueueIndex;
		}

		VkBool32 bPresentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(*pPhysicalDevice, iQueueIndex, oSurface, &bPresentSupport);

		if (bPresentSupport)
			iPresentQueue = iQueueIndex;

		if (iGraphicQueue != -1 && iPresentQueue != -1)
		{
			iScore += 500;
			break;
		}

		iQueueIndex++;
	}

	return iScore;
}

VkSampleCountFlagBits GraphicDevice::GetMaxSampleCount()
{
	VkPhysicalDeviceProperties oPhysicalDeviceProperties;
	vkGetPhysicalDeviceProperties(*m_pPhysicalDevice, &oPhysicalDeviceProperties);

	VkSampleCountFlags oCounts = oPhysicalDeviceProperties.limits.framebufferColorSampleCounts & oPhysicalDeviceProperties.limits.framebufferDepthSampleCounts;

	if (oCounts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
	if (oCounts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
	if (oCounts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
	if (oCounts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
	if (oCounts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
	if (oCounts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

	return VK_SAMPLE_COUNT_1_BIT;
}

GraphicDevice::GraphicDevice(Desc& oDesc)
{
	m_iGraphicQueueIndex = -1;
	m_iPresentQueueIndex = -1;
	m_pRenderSurface = oDesc.pSurface;
	int iMaxScore = 0;

	uint32_t iPhysicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(Graphics::Globals::s_oInstance, &iPhysicalDeviceCount, nullptr);

	if (iPhysicalDeviceCount == 0)
	{
		throw std::runtime_error("GPU error");
	}

	std::vector<VkPhysicalDevice> oDevices(iPhysicalDeviceCount);
	vkEnumeratePhysicalDevices(Graphics::Globals::s_oInstance, &iPhysicalDeviceCount, oDevices.data());

	for (auto& oPhysicalDevice : oDevices)
	{
		int iScore = RateDevice(&oPhysicalDevice, oDesc.oExtensions, *( m_pRenderSurface->GetSurface() ), m_iGraphicQueueIndex, m_iPresentQueueIndex);

		if (iScore > iMaxScore)
		{
			iMaxScore = iScore;
			m_pPhysicalDevice = new VkPhysicalDevice(oPhysicalDevice);
		}
	}
	float fQueuePriority = 1.0f;

	std::vector<VkDeviceQueueCreateInfo> oQueuesCreateInfos{};
	VkDeviceQueueCreateInfo oQueueCreateInfo{};
	oQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	oQueueCreateInfo.queueFamilyIndex = m_iGraphicQueueIndex;
	oQueueCreateInfo.queueCount = 1;
	oQueueCreateInfo.pQueuePriorities = &fQueuePriority;
	oQueuesCreateInfos.push_back(oQueueCreateInfo);

	if (m_iGraphicQueueIndex != m_iPresentQueueIndex)
	{
		VkDeviceQueueCreateInfo oQueueCreateInfo{};
		oQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		oQueueCreateInfo.queueFamilyIndex = m_iPresentQueueIndex;
		oQueueCreateInfo.queueCount = 1;
		oQueueCreateInfo.pQueuePriorities = &fQueuePriority;
		oQueuesCreateInfos.push_back(oQueueCreateInfo);
	}

	VkPhysicalDeviceFeatures oDeviceFeatures{};
	oDeviceFeatures.samplerAnisotropy = VK_TRUE;
	oDeviceFeatures.sampleRateShading = VK_TRUE;

	VkDeviceCreateInfo oCreateInfo{};
	oCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	oCreateInfo.pQueueCreateInfos = oQueuesCreateInfos.data();
	oCreateInfo.queueCreateInfoCount = oQueuesCreateInfos.size();
	oCreateInfo.pEnabledFeatures = &oDeviceFeatures;
	oCreateInfo.enabledExtensionCount = oDesc.oExtensions.size();
	oCreateInfo.ppEnabledExtensionNames = oDesc.oExtensions.data();

	if (vkCreateDevice(*m_pPhysicalDevice, &oCreateInfo, nullptr, &m_oDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device");
	}
	m_iMaxMsaa = GetMaxSampleCount();

	vkGetDeviceQueue(m_oDevice, m_iGraphicQueueIndex, 0, &m_oGraphicQueue);
	vkGetDeviceQueue(m_oDevice, m_iPresentQueueIndex, 0, &m_oPresentQueue);
}

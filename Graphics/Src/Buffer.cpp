#include "Buffer.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include "CommandFactory.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

uint32_t Buffer::FindMemoryType(GraphicWrapper* pWrapper, uint32_t iTypeFilter, VkMemoryPropertyFlags oProperties)
{
	VkPhysicalDeviceMemoryProperties oMemProperties;
	vkGetPhysicalDeviceMemoryProperties(*pWrapper->GetDevice()->GetPhysicalDevice(), &oMemProperties);

	for (uint32_t i = 0; i < oMemProperties.memoryTypeCount; i++)
	{
		if (iTypeFilter & (1 << i) && (oMemProperties.memoryTypes[i].propertyFlags & oProperties) == oProperties)
		{
			return i;
		}
	}

	return -1;
}

BasicBuffer::~BasicBuffer()
{
	vkFreeMemory(*m_pDevice->GetLogicalDevice(), *m_pMemory, nullptr);
	vkDestroyBuffer(*m_pDevice->GetLogicalDevice(), *m_pBuffer, nullptr);
	m_pMemory = nullptr;
	m_pBuffer = nullptr;
}

BasicBuffer::BasicBuffer(Desc& oDesc)
{
	m_pDevice = oDesc.pWrapper->GetModifiableDevice();
	m_iUnitCount = oDesc.iUnitCount;
	m_iSizeUnit = oDesc.iUnitSize;

	VkBufferCreateInfo oBufferInfo{};
	oBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	oBufferInfo.size = oDesc.iUnitSize * oDesc.iUnitCount;
	oBufferInfo.usage = oDesc.eUsage;
	oBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	m_pBuffer = new VkBuffer();
	if (vkCreateBuffer(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), &oBufferInfo, nullptr, m_pBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer");
	}

	VkMemoryAllocateInfo oAllocInfo{};
	oAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkMemoryRequirements oMemoryRequirements;
	vkGetBufferMemoryRequirements(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), *m_pBuffer, &oMemoryRequirements);

	oAllocInfo.allocationSize = oMemoryRequirements.size;
	oAllocInfo.memoryTypeIndex = FindMemoryType(oDesc.pWrapper, oMemoryRequirements.memoryTypeBits, oDesc.oPropertyFlags);

	m_pMemory = new VkDeviceMemory();
	if (vkAllocateMemory(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), &oAllocInfo, nullptr, m_pMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate buffer");
	}

	vkBindBufferMemory(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), *m_pBuffer, *m_pMemory, 0);
}

size_t Buffer::GetMemorySize(VkFormat eFormat)
{
	if (eFormat >= VK_FORMAT_R8G8B8A8_UNORM && eFormat <= VK_FORMAT_A2B10G10R10_SINT_PACK32)
	{
		return 4;
	}

	if (eFormat >= VK_FORMAT_R8G8B8_UNORM && eFormat <= VK_FORMAT_B8G8R8_SRGB)
	{
		return 3;
	}

	if (eFormat >= VK_FORMAT_R8G8_UNORM && eFormat <= VK_FORMAT_R8G8_SRGB)
	{
		return 2;
	}

	if (eFormat >= VK_FORMAT_R8_UNORM && eFormat <= VK_FORMAT_R8_SRGB)
	{
		return 1;
	}

	if (eFormat == VK_FORMAT_D32_SFLOAT)
	{
		return 4;
	}

	throw std::runtime_error("Could not find a size for the specified format");
}

void Buffer::CopyFromMemory(void* pData, GraphicDevice* pDevice)
{
	void* pGpuData;
	vkMapMemory(*pDevice->GetLogicalDevice(), *m_pMemory, 0, GetMemorySize(), 0, &pGpuData);
	memcpy(pGpuData, pData, (size_t)GetMemorySize());
	vkUnmapMemory(*pDevice->GetLogicalDevice(), *m_pMemory);
}

void Buffer::CopyFromMemory(void* pData, GraphicDevice* pDevice, uint64_t iOffset, uint64_t iSize)
{
	void* pGpuData;
	vkMapMemory(*pDevice->GetLogicalDevice(), *m_pMemory, iOffset, iSize, 0, &pGpuData);
	memcpy(pGpuData, pData, iSize);
	vkUnmapMemory(*pDevice->GetLogicalDevice(), *m_pMemory);
}

void Image::SendCopyCommand(BasicBuffer* pBuffer, CommandFactory* pFactory)
{
	VkCommandBuffer oCommandBuffer = pFactory->BeginSingleTimeCommands();

	VkBufferImageCopy oRegion{};
	oRegion.bufferOffset = 0;
	oRegion.bufferRowLength = 0;
	oRegion.bufferImageHeight = 0;

	oRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	oRegion.imageSubresource.mipLevel = 0;
	oRegion.imageSubresource.baseArrayLayer = 0;
	oRegion.imageSubresource.layerCount = 1;

	oRegion.imageOffset = { 0,0,0 };
	oRegion.imageExtent = { (uint32_t)m_iWidth, (uint32_t)m_iHeight, 1 };

	vkCmdCopyBufferToImage(oCommandBuffer, *pBuffer->GetBuffer() , m_oImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &oRegion);

	pFactory->EndSingleTimeCommands(oCommandBuffer);
}

void Image::GenerateMipsInterface(MipDesc& oDesc)
{
	if (m_iMipLevel > 1)
	{
		TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, oDesc.pFactory, m_iMipLevel);
		Image::MipDesc oMipDesc;
		oMipDesc.eFormat = oDesc.eFormat;
		oMipDesc.pFactory = oDesc.pFactory;
		oMipDesc.pWrapper = oDesc.pWrapper;
		GenerateMips(oMipDesc);
	}
}

void BasicBuffer::SendCopyCommand(BasicBuffer* pDst, CommandFactory* pFactory)
{
	VkCommandBuffer oCommandBuffer = pFactory->BeginSingleTimeCommands();
	VkBufferCopy oCopyRegion{};
	oCopyRegion.srcOffset = 0;
	oCopyRegion.dstOffset = 0;
	oCopyRegion.size = GetMemorySize();

	vkCmdCopyBuffer(oCommandBuffer, *m_pBuffer, *pDst->GetBuffer(), 1, &oCopyRegion);

	pFactory->EndSingleTimeCommands(oCommandBuffer);
}

Image* Image::CreateFromFile(std::string sFilename, FromFileDesc& oDesc)
{	
	Image* pOutput = nullptr;
	int iWidth, iHeight, iTexChannels;
	stbi_uc* pPixels = stbi_load(sFilename.c_str(), &iWidth, &iHeight, &iTexChannels, STBI_rgb_alpha);

	if (!pPixels)
	{
		throw std::runtime_error("Failed to load texture image");
	}

	Image::FromBufferDesc oBufferDesc;
	oBufferDesc.bEnableMip = oDesc.bEnableMip;
	oBufferDesc.eAspect = oDesc.eAspect;
	oBufferDesc.eFormat = oDesc.eFormat;
	oBufferDesc.eSampleFlag = oDesc.eSampleFlag;
	oBufferDesc.eTiling = oDesc.eTiling;
	oBufferDesc.iHeight = iHeight;
	oBufferDesc.iWidth = iWidth;
	oBufferDesc.pBuffer = pPixels;
	oBufferDesc.pWrapper = oDesc.pWrapper;
	oBufferDesc.pFactory = oDesc.pFactory;

	pOutput = CreateFromBuffer(oBufferDesc);
	stbi_image_free(pPixels);
	return pOutput;
}

Image* Image::CreateCubeMap(std::string sFilenames[6], FromFileDesc& oDesc)
{
	std::vector<Image*> oImages;

	int iWidth = -1;
	int iHeight = -1;
	for (int i = 0; i < 6; i++)
	{
		Image* pImage = CreateFromFile(sFilenames[i], oDesc);

		if (pImage->GetWidth() != iWidth && iWidth != -1)
		{
			throw std::runtime_error("One of the face does not have the same size");
		}
		iWidth = pImage->GetWidth();

		if (pImage->GetHeight() != iHeight && iHeight != -1)
		{
			throw std::runtime_error("One of the face does not have the same size");
		}
		iHeight = pImage->GetHeight();

		oImages.push_back(pImage);
	}

	Image::Desc oImgDesc;
	oImgDesc.eAspect = VK_IMAGE_ASPECT_COLOR_BIT;
	oImgDesc.eFormat = oDesc.eFormat;
	oImgDesc.eUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	oImgDesc.eProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	oImgDesc.iLayerCount = 6;
	oImgDesc.iHeight = iHeight;
	oImgDesc.iWidth = iWidth;
	oImgDesc.pWrapper = oDesc.pWrapper;
	oImgDesc.pFactory = oDesc.pFactory;
	oImgDesc.bIsCubemap = true;

	Image* pCubemap = new Image(oImgDesc);
	pCubemap->TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, oImgDesc.pFactory, 1);

	VkCommandBuffer oCommandBuffer = oDesc.pFactory->BeginSingleTimeCommands();

	for (int i = 0; i < 6; i++)
	{
		VkImageMemoryBarrier oBarrier{};
		oBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		oBarrier.image = *oImages[i]->GetImage();
		oBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		oBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		oBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		oBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		oBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		oBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		oBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		oBarrier.subresourceRange.baseMipLevel = 0;
		oBarrier.subresourceRange.baseArrayLayer = 0;
		oBarrier.subresourceRange.layerCount = 1;
		oBarrier.subresourceRange.levelCount = 1;

		vkCmdPipelineBarrier(oCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &oBarrier);

		VkImageBlit oBlit{};
		oBlit.srcOffsets[0] = { 0,0,0 };
		oBlit.srcOffsets[1] = { iWidth, iHeight, 1 };
		oBlit.srcSubresource.mipLevel = 0;
		oBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		oBlit.srcSubresource.baseArrayLayer = 0;
		oBlit.srcSubresource.layerCount = 1;

		oBlit.dstOffsets[0] = { 0, 0, 0 };
		oBlit.dstOffsets[1] = { iWidth, iHeight, 1 };
		oBlit.dstSubresource.mipLevel = 0;
		oBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		oBlit.dstSubresource.baseArrayLayer = i;
		oBlit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(oCommandBuffer, *oImages[i]->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *pCubemap->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &oBlit, VK_FILTER_LINEAR);

		VkImageMemoryBarrier oBarrierAfter{};
		oBarrierAfter.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		oBarrierAfter.image = *oImages[i]->GetImage();
		oBarrierAfter.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		oBarrierAfter.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		oBarrierAfter.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		oBarrierAfter.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; 
		oBarrierAfter.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		oBarrierAfter.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		oBarrierAfter.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		oBarrierAfter.subresourceRange.baseMipLevel = 0;
		oBarrierAfter.subresourceRange.baseArrayLayer = 0;
		oBarrierAfter.subresourceRange.layerCount = 1;
		oBarrierAfter.subresourceRange.levelCount = 1;

		vkCmdPipelineBarrier(oCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &oBarrierAfter);
	}

	oDesc.pFactory->EndSingleTimeCommands(oCommandBuffer);

	pCubemap->TransitionLayout( VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL , VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL , oImgDesc.pFactory, 1);

	return pCubemap;
}

void Image::TransitionLayout(VkImageLayout eOldLayout, VkImageLayout eNewLayout, CommandFactory* pFactory, int iMipLevel)
{
	VkCommandBuffer oCommandBuffer = pFactory->BeginSingleTimeCommands();

	VkImageMemoryBarrier oBarrier{};
	oBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	oBarrier.oldLayout = eOldLayout;
	oBarrier.newLayout = eNewLayout;
	oBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	oBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	oBarrier.image = m_oImage;
	oBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	oBarrier.subresourceRange.baseMipLevel = 0;
	oBarrier.subresourceRange.levelCount = iMipLevel;
	oBarrier.subresourceRange.baseArrayLayer = 0;
	oBarrier.subresourceRange.layerCount = m_bIsCubemap ? 6 : 1;
	
	VkPipelineStageFlags oSourceStage(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
	VkPipelineStageFlags oDestinationStage(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);

	if (eOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && eNewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		oBarrier.srcAccessMask = 0;
		oBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		oSourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		oDestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (eOldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && eNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		oBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		oBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		oSourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		oDestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	//2nd = which pipeline stage before the barrier
	//3rd = which operation will wait
	//4th = block per-region or not
	//5th = memory barrier
	//6th = buffer memory barrier
	//7th = image memory barrier
	vkCmdPipelineBarrier(oCommandBuffer, oSourceStage, oDestinationStage, 0, 0, nullptr, 0, nullptr, 1, &oBarrier);

	pFactory->EndSingleTimeCommands(oCommandBuffer);
}

Image* Image::CreateFromBuffer(FromBufferDesc& oDesc)
{
	BasicBuffer::Desc oBufferDesc;
	oBufferDesc.eUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	oBufferDesc.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	oBufferDesc.iUnitCount = oDesc.iWidth * oDesc.iHeight;
	oBufferDesc.iUnitSize = GetMemorySize(oDesc.eFormat);
	oBufferDesc.pWrapper = oDesc.pWrapper;

	BasicBuffer* pBasicBuffer = new BasicBuffer(oBufferDesc);

	pBasicBuffer->CopyFromMemory(oDesc.pBuffer, oDesc.pWrapper->GetModifiableDevice());

	Image::Desc oImgDesc;
	oImgDesc.iHeight = oDesc.iHeight;
	oImgDesc.iWidth = oDesc.iWidth;
	oImgDesc.eFormat = oDesc.eFormat;
	oImgDesc.eAspect = oDesc.eAspect;
	oImgDesc.eProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	oImgDesc.eTiling = oDesc.eTiling;
	oImgDesc.eUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	oImgDesc.pWrapper = oDesc.pWrapper;
	oImgDesc.bEnableMip = oDesc.bEnableMip;
	oImgDesc.eSampleFlag = oDesc.eSampleFlag;
	oImgDesc.pFactory = oDesc.pFactory;
	oImgDesc.iLayerCount = 1;

	Image* pImage = new Image(oImgDesc);
	/*if (oDesc.eTiling == VK_IMAGE_TILING_LINEAR)
	{
		pBasicBuffer->CopyFromMemory(oDesc.pBuffer, oDesc.pWrapper->GetModifiableDevice(), oDesc.iWidth, pImage->m_iRowPitch, oDesc.iHeight);
	}*/

	pImage->TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, oDesc.pFactory, pImage->GetMipLevel());
	pImage->SendCopyCommand(pBasicBuffer, oDesc.pFactory);

	if (oDesc.bEnableMip)
	{
		Image::MipDesc oMipDesc;
		oMipDesc.eFormat = oDesc.eFormat;
		oMipDesc.pFactory = oDesc.pFactory;
		oMipDesc.pWrapper = oDesc.pWrapper;
		pImage->GenerateMipsInterface(oMipDesc);
	}
	delete pBasicBuffer;
	return pImage;
}

void Image::GenerateMips(MipDesc& oDesc)
{
	VkFormatProperties oFormatProperties;
	vkGetPhysicalDeviceFormatProperties(*oDesc.pWrapper->GetDevice()->GetPhysicalDevice(), oDesc.eFormat, &oFormatProperties);

	if (!(oFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		throw std::runtime_error("Texture image does not support linear blit");
	}

	VkCommandBuffer oCommandBuffer = oDesc.pFactory->BeginSingleTimeCommands();

	VkImageMemoryBarrier oBarrier{};
	oBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	oBarrier.image = m_oImage;
	oBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	oBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	oBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	oBarrier.subresourceRange.baseArrayLayer = 0;
	oBarrier.subresourceRange.layerCount = 1;
	oBarrier.subresourceRange.levelCount = 1;

	int32_t iMipWidth = m_iWidth;
	int32_t iMipHeight = m_iHeight;

	for (int i = 1; i < m_iMipLevel; i++)
	{
		oBarrier.subresourceRange.baseMipLevel = i - 1;
		oBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		oBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		oBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		oBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(oCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &oBarrier);

		VkImageBlit oBlit{};
		oBlit.srcOffsets[0] = { 0,0,0 };
		oBlit.srcOffsets[1] = { iMipWidth, iMipHeight, 1 };
		oBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		oBlit.srcSubresource.mipLevel = i - 1;
		oBlit.srcSubresource.baseArrayLayer = 0;
		oBlit.srcSubresource.layerCount = 1;
		oBlit.dstOffsets[0] = { 0, 0, 0 };
		oBlit.dstOffsets[1] = { iMipWidth > 1 ? iMipWidth / 2 : 1, iMipHeight > 1 ? iMipHeight / 2 : 1, 1 };
		oBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		oBlit.dstSubresource.mipLevel = i;
		oBlit.dstSubresource.baseArrayLayer = 0;
		oBlit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(oCommandBuffer, m_oImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_oImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &oBlit, VK_FILTER_LINEAR);

		oBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		oBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		oBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		oBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(oCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &oBarrier);

		if (iMipWidth > 1) iMipWidth /= 2;
		if (iMipHeight > 1) iMipHeight /= 2;
	}

	oBarrier.subresourceRange.baseMipLevel = m_iMipLevel - 1;
	oBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	oBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	oBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	oBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(oCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &oBarrier);

	oDesc.pFactory->EndSingleTimeCommands(oCommandBuffer);
}

Image::Image(Desc& oDesc)
{
	m_iSizeUnit = GetMemorySize(oDesc.eFormat);
	m_pDevice = oDesc.pWrapper->GetModifiableDevice();
	m_iRowPitch = m_iWidth = oDesc.iWidth;
	m_iHeight = oDesc.iHeight;
	m_bIsCubemap = oDesc.bIsCubemap;

	if (oDesc.bEnableMip)
	{
		m_iMipLevel = static_cast<uint32_t>(std::floor(std::log2(std::max(oDesc.iWidth, oDesc.iHeight)))) + 1;
	}
	else
	{
		m_iMipLevel = 1;
	}

	VkImageCreateInfo oImageInfo{};
	oImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	oImageInfo.imageType = VK_IMAGE_TYPE_2D;
	oImageInfo.extent.width = oDesc.iWidth;
	oImageInfo.extent.height = oDesc.iHeight;
	oImageInfo.extent.depth = 1;
	oImageInfo.mipLevels = m_iMipLevel;
	oImageInfo.arrayLayers = (uint32_t)oDesc.iLayerCount;
	oImageInfo.format = oDesc.eFormat;
	oImageInfo.tiling = oDesc.eTiling;
	oImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	oImageInfo.usage = oDesc.eUsage;
	oImageInfo.samples = oDesc.eSampleFlag;
	oImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	oImageInfo.flags = oDesc.bIsCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

	if (vkCreateImage(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), &oImageInfo, nullptr, &m_oImage) != VK_SUCCESS)
	{
		throw std::runtime_error("Create image");
	}

	VkMemoryAllocateInfo oAllocInfo{};
	oAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements oMemRequirements;
	vkGetImageMemoryRequirements(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), m_oImage, &oMemRequirements);
	oAllocInfo.allocationSize = oMemRequirements.size;
	oAllocInfo.memoryTypeIndex = FindMemoryType(oDesc.pWrapper, oMemRequirements.memoryTypeBits, oDesc.eProperties);
	
	m_pMemory = new VkDeviceMemory();
	if (vkAllocateMemory(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), &oAllocInfo, nullptr, m_pMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate image memory");
	}

	vkBindImageMemory(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), m_oImage, *m_pMemory, 0);

	CreateSampler(oDesc);
	CreateView(oDesc, oDesc.eAspect);

	if (oDesc.eTiling == VK_IMAGE_TILING_LINEAR)
	{
		VkImageSubresource subRes = {};
		subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subRes.mipLevel = 0;
		subRes.arrayLayer = 0;

		VkSubresourceLayout subResLayout;
		vkGetImageSubresourceLayout(*m_pDevice->GetLogicalDevice(), m_oImage, &subRes, &subResLayout);

		m_iRowPitch = subResLayout.rowPitch;
	}
}

Image::~Image()
{
	vkDestroyImageView(*m_pDevice->GetLogicalDevice(), m_oImageView, nullptr);
	vkDestroyImage(*m_pDevice->GetLogicalDevice(), m_oImage, nullptr);
	vkFreeMemory(*m_pDevice->GetLogicalDevice(), *m_pMemory, nullptr);
}

void Image::CreateSampler(Desc& oDesc)
{
	VkSamplerCreateInfo oSamplerInfo{};
	oSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	oSamplerInfo.magFilter = VK_FILTER_LINEAR;
	oSamplerInfo.minFilter = VK_FILTER_LINEAR;

	oSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	oSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	oSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	oSamplerInfo.anisotropyEnable = true;
	oSamplerInfo.maxAnisotropy = 16.0f;
	oSamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	oSamplerInfo.unnormalizedCoordinates = VK_FALSE;

	oSamplerInfo.compareEnable = VK_FALSE;
	oSamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	oSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	oSamplerInfo.mipLodBias = 0.0f;
	oSamplerInfo.minLod = 0;
	oSamplerInfo.maxLod = static_cast<float>(m_iMipLevel);

	if (vkCreateSampler(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), &oSamplerInfo, nullptr, &m_eSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sample");
	}
}

void Image::CreateView(Desc& oDesc, VkImageAspectFlags oAspect)
{
	VkImageViewCreateInfo oViewInfo{};
	oViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	oViewInfo.image = m_oImage;
	oViewInfo.viewType = oDesc.bIsCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
	oViewInfo.format = oDesc.eFormat;
	oViewInfo.subresourceRange.aspectMask = oAspect;
	oViewInfo.subresourceRange.baseMipLevel = 0;
	oViewInfo.subresourceRange.levelCount = m_iMipLevel;
	oViewInfo.subresourceRange.baseArrayLayer = 0;
	oViewInfo.subresourceRange.layerCount = oDesc.bIsCubemap ? 6 : 1;

	if (vkCreateImageView(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), &oViewInfo, nullptr, &m_oImageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture image view");
	}
}

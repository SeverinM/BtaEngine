#include "DescriptorPool.h"
#include <iostream>
#include "Pipeline.h"
#include "BasicWrapper.h"
#include "DescriptorWrapper.h"
#include "Globals.h"

DescriptorPool::DescriptorPool(Desc& oDesc)
{
	m_pRecreate = new Desc(oDesc);
	Create(oDesc);
}

void DescriptorPool::Create(Desc& oDesc)
{
	uint32_t iSize = oDesc.iSize;
	VkDescriptorPoolSize pPool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, iSize },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, iSize },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, iSize },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, iSize },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, iSize },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, iSize },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, iSize },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, iSize },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, iSize },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, iSize },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, iSize }
	};

	VkDescriptorPoolCreateInfo oPoolInfo{};
	oPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	oPoolInfo.poolSizeCount = 11;
	oPoolInfo.pPoolSizes = pPool_sizes;
	oPoolInfo.maxSets = oDesc.iMaxSet * 10;
	oPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	if (vkCreateDescriptorPool(*Graphics::Globals::g_pDevice->GetLogicalDevice(), &oPoolInfo, nullptr, &m_oPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool !");
	}
}

VkDescriptorType DescriptorPool::GetDescriptorType(E_BINDING_TYPE eType)
{
	if (eType == E_STORAGE_BUFFER)
	{
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	}
	
	if (eType == E_TEXTURE)
	{
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	}

	if (eType == E_UNIFORM_BUFFER)
	{
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	}

	return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(*Graphics::Globals::g_pDevice->GetLogicalDevice(), m_oPool, nullptr);
}


void DescriptorPool::WriteDescriptor(DescriptorSetWrapper* pDescriptorSet)
{
	VkDescriptorSet* pSet = pDescriptorSet->GetDescriptorSet();
	std::vector< VkWriteDescriptorSet> oWriteDescriptors;
	const std::vector< DescriptorSetWrapper::MemorySlot> oSlots = pDescriptorSet->GetSlots();

	int i = 0;
	for (const DescriptorSetWrapper::MemorySlot& oSlot : oSlots)
	{
		VkWriteDescriptorSet oWrite{};

		oWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		oWrite.dstSet = *pSet;
		oWrite.dstArrayElement = 0;
		oWrite.descriptorCount = 1;
		oWrite.dstBinding = i;

		if (oSlot.eType == E_TEXTURE)
		{
			Image* pImage = (Image*)oSlot.pData;

			VkDescriptorImageInfo oImageInfo{};
			oImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			oImageInfo.imageView = *pImage->GetImageView();
			oImageInfo.sampler = *pImage->GetSampler();

			oWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			oWrite.pImageInfo = new VkDescriptorImageInfo(oImageInfo);
		}
		else if (oSlot.eType == E_UNIFORM_BUFFER)
		{
			BasicBuffer* xBasicBuffer = (BasicBuffer*)oSlot.pData;

			VkDescriptorBufferInfo oBufferInfo{};
			oBufferInfo.buffer = *xBasicBuffer->GetBuffer();
			oBufferInfo.range = xBasicBuffer->GetMemorySize();
			oBufferInfo.offset = 0;

			oWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			oWrite.pBufferInfo = new VkDescriptorBufferInfo( oBufferInfo );
		}
		else
		{
			BasicBuffer* pBasicBuffer = (BasicBuffer*)oSlot.pData;

			VkDescriptorBufferInfo oBufferInfo{};
			oBufferInfo.buffer = *pBasicBuffer->GetBuffer();
			oBufferInfo.range = pBasicBuffer->GetMemorySize();
			oBufferInfo.offset = 0;

			oWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			oWrite.pBufferInfo = &oBufferInfo;
		}

		oWriteDescriptors.push_back(oWrite);
		i++;
	}

	vkUpdateDescriptorSets(*Graphics::Globals::g_pDevice->GetLogicalDevice(), static_cast<uint32_t>(oWriteDescriptors.size()), oWriteDescriptors.data(), 0, nullptr);
}

void DescriptorPool::CreateDescriptorSet(std::vector<VkDescriptorSet>& oOutput,int iSize, const VkDescriptorSetLayout& oLayout)
{
	std::vector<VkDescriptorSetLayout> oLayouts(iSize, oLayout);
	VkDescriptorSetAllocateInfo oAllocInfo{};
	oAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	oAllocInfo.descriptorPool = m_oPool;
	oAllocInfo.descriptorSetCount = static_cast<uint32_t>(iSize);
	oAllocInfo.pSetLayouts = oLayouts.data();

	oOutput.resize(iSize);
	if (vkAllocateDescriptorSets(*Graphics::Globals::g_pDevice->GetLogicalDevice(), &oAllocInfo, oOutput.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate descriptor sets");
	}
}

void DescriptorPool::CreateDescriptorSet(VkDescriptorSet& oOutput, const VkDescriptorSetLayout& oLayout)
{
	std::vector<VkDescriptorSetLayout> oLayouts(1, oLayout);
	VkDescriptorSetAllocateInfo oAllocInfo{};
	oAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	oAllocInfo.descriptorPool = m_oPool;
	oAllocInfo.descriptorSetCount = 1;
	oAllocInfo.pSetLayouts = oLayouts.data();

	if (vkAllocateDescriptorSets(*Graphics::Globals::g_pDevice->GetLogicalDevice(), &oAllocInfo, &oOutput) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate descriptor sets");
	}
}

#include "DescriptorPool.h"
#include <iostream>
#include "Pipeline.h"
#include "BasicWrapper.h"

DescriptorPool::DescriptorPool(Desc& oDesc)
{
	m_pRecreate = new Desc(oDesc);
	Create(oDesc);
}

void DescriptorPool::Create(Desc& oDesc)
{
	VkDescriptorPoolSize pPool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, oDesc.iSize },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, oDesc.iSize },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, oDesc.iSize },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, oDesc.iSize },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, oDesc.iSize },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, oDesc.iSize },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, oDesc.iSize },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, oDesc.iSize },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, oDesc.iSize },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, oDesc.iSize },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, oDesc.iSize }
	};

	VkDescriptorPoolCreateInfo oPoolInfo{};
	oPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	oPoolInfo.poolSizeCount = 11;
	oPoolInfo.pPoolSizes = pPool_sizes;
	oPoolInfo.maxSets = oDesc.iImageCount * 10;

	if (vkCreateDescriptorPool(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), &oPoolInfo, nullptr, &m_oPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool !");
	}
}

VkDescriptorType DescriptorPool::GetDescriptorType(EBufferType eType)
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
	vkDestroyDescriptorPool(*m_pRecreate->pWrapper->GetDevice()->GetLogicalDevice(), m_oPool, nullptr);
}

void DescriptorPool::WriteDescriptor(std::vector< UpdateSubDesc >& oUpdate, const VkDescriptorSetLayout& oDescriptorSetLayout)
{
	std::vector<VkDescriptorSetLayout> oLayouts(oUpdate.size(), oDescriptorSetLayout);
	
	for (size_t i = 0; i < oUpdate.size(); i++)
	{
		std::vector<VkWriteDescriptorSet> oDescriptorWrite;
		oDescriptorWrite.resize(oUpdate[i].oBuffers.size());
		for (int j = 0; j < oUpdate[i].oBuffers.size(); j++)
		{
			oDescriptorWrite[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			oDescriptorWrite[j].dstSet = *oUpdate[i].pDescriptorSet;
			oDescriptorWrite[j].dstArrayElement = 0;
			oDescriptorWrite[j].descriptorCount = 1;
			oDescriptorWrite[j].dstBinding = j;

			if (oUpdate[i].oBuffers[j].eType == E_TEXTURE)
			{
				std::shared_ptr<Image> pImage = std::static_pointer_cast<Image>(oUpdate[i].oBuffers[j].xBuffer);

				VkDescriptorImageInfo oImageInfo{};
				oImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				oImageInfo.imageView = *pImage->GetImageView();
				oImageInfo.sampler = *pImage->GetSampler();

				oDescriptorWrite[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				oDescriptorWrite[j].pImageInfo = new VkDescriptorImageInfo( oImageInfo );
			}
			else if (oUpdate[i].oBuffers[j].eType == E_UNIFORM_BUFFER)
			{
				std::shared_ptr<BasicBuffer> xBasicBuffer = std::static_pointer_cast<BasicBuffer>(oUpdate[i].oBuffers[j].xBuffer);

				VkDescriptorBufferInfo oBufferInfo{};
				oBufferInfo.buffer = *xBasicBuffer->GetBuffer();
				oBufferInfo.range = oUpdate[i].oBuffers[j].xBuffer->GetMemorySize();
				oBufferInfo.offset = 0;

				oDescriptorWrite[j].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				oDescriptorWrite[j].pBufferInfo = &oBufferInfo;
			}
			else
			{
				std::shared_ptr<BasicBuffer> xBasicBuffer = std::static_pointer_cast<BasicBuffer>(oUpdate[i].oBuffers[j].xBuffer);

				VkDescriptorBufferInfo oBufferInfo{};
				oBufferInfo.buffer = *xBasicBuffer->GetBuffer();
				oBufferInfo.range = oUpdate[i].oBuffers[j].xBuffer->GetMemorySize();
				oBufferInfo.offset = 0;

				oDescriptorWrite[j].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				oDescriptorWrite[j].pBufferInfo = &oBufferInfo;
			}
		}

		vkUpdateDescriptorSets(*m_pRecreate->pWrapper->GetDevice()->GetLogicalDevice(), static_cast<uint32_t>(oDescriptorWrite.size()), oDescriptorWrite.data(), 0, nullptr);
	}
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
	if (vkAllocateDescriptorSets(*m_pRecreate->pWrapper->GetDevice()->GetLogicalDevice(), &oAllocInfo, oOutput.data()) != VK_SUCCESS)
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

	if (vkAllocateDescriptorSets(*m_pRecreate->pWrapper->GetDevice()->GetLogicalDevice(), &oAllocInfo, &oOutput) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate descriptor sets");
	}
}

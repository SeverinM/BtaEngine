#include "DescriptorWrapper.h"
#include <iostream>

DescriptorLayoutWrapper::DescriptorLayoutWrapper(std::vector <Bindings>& oBindings, GraphicDevice& oDevice)
{
	m_oAllBindings = oBindings;
	std::vector<VkDescriptorSetLayoutBinding> oDescriptorBindings;

	for (int i = 0; i < oBindings.size(); i++)
	{
		VkDescriptorSetLayoutBinding oBinding{};
		oBinding.binding = i;
		oBinding.descriptorCount = 1;
		oBinding.descriptorType = DescriptorLayoutWrapper::GetDescriptorType(oBindings[i].eType);
		oBinding.pImmutableSamplers = nullptr;
		oBinding.stageFlags = oBindings[i].eStages;

		oDescriptorBindings.push_back(oBinding);
	}

	VkDescriptorSetLayoutCreateInfo oLayoutCreateInfo{};
	oLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	oLayoutCreateInfo.bindingCount = oDescriptorBindings.size();
	oLayoutCreateInfo.pBindings = oDescriptorBindings.data();

	if (vkCreateDescriptorSetLayout(*oDevice.GetLogicalDevice(), &oLayoutCreateInfo, nullptr, m_pLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Error creating set layout");
	}
}

DescriptorSetWrapper* DescriptorLayoutWrapper::InstantiateDescriptorSet(DescriptorPool& oPool, GraphicDevice& oDevice)
{
	DescriptorSetWrapper* pDescriptor = new DescriptorSetWrapper();

	for (Bindings& oBinding : m_oAllBindings)
	{
		DescriptorSetWrapper::MemorySlot oSlot;
		oSlot.eType = oBinding.eType;
		oSlot.pData = nullptr;
		oSlot.iSizeUnit = oBinding.iSizeUnit;
		oSlot.iUnitCount = oBinding.iUnitCount;
		pDescriptor->m_oAllDatas.push_back(oSlot);
	}

	oPool.CreateDescriptorSet(pDescriptor->m_oSet, *m_pLayout);
	
	return pDescriptor;
}

VkDescriptorType DescriptorLayoutWrapper::GetDescriptorType(E_BINDING_TYPE eType)
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

DescriptorLayoutWrapper* DescriptorLayoutWrapper::ParseShaderFiles(ShaderMap& oMap, GraphicDevice* pDevice)
{
	std::vector<DescriptorLayoutWrapper::Bindings> oBindings;
	return new DescriptorLayoutWrapper(oBindings, *pDevice);
}

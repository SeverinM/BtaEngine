#include "DescriptorWrapper.h"
#include <iostream>
#include <fstream>
#include "StringUtils.h"
#include "VectorUtils.h"
#include "GLM/glm.hpp"

namespace Bta
{
	namespace Graphic
	{
		DescriptorLayoutWrapper::DescriptorLayoutWrapper(std::vector<Bindings>& oBindings, GraphicDevice& oDevice)
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
			oLayoutCreateInfo.bindingCount = (uint32_t)oDescriptorBindings.size();
			oLayoutCreateInfo.pBindings = oDescriptorBindings.data();

			m_pLayout = new VkDescriptorSetLayout();
			if (vkCreateDescriptorSetLayout(*oDevice.GetLogicalDevice(), &oLayoutCreateInfo, nullptr, m_pLayout) != VK_SUCCESS)
			{
				throw std::runtime_error("Error creating set layout");
			}
		}

		DescriptorLayoutWrapper::~DescriptorLayoutWrapper()
		{
		}

		DescriptorSetWrapper* DescriptorLayoutWrapper::InstantiateDescriptorSet(DescriptorPool& oPool, GraphicDevice& oDevice)
		{
			DescriptorSetWrapper* pDescriptor = new DescriptorSetWrapper();
			pDescriptor->m_oSet = VkDescriptorSet();
			pDescriptor->m_pLayoutFrom = m_pLayout;

			for (Bindings& oBinding : m_oAllBindings)
			{
				DescriptorSetWrapper::MemorySlot oSlot;
				oSlot.eType = oBinding.eType;
				oSlot.pData = nullptr;
				oSlot.sTag = oBinding.sTag;
				oSlot.oElementsSize = oBinding.oAllSizes;
				pDescriptor->m_oSlots.push_back(oSlot);
			}

			pDescriptor->m_bDelete = false;
			pDescriptor->m_pPool = &oPool;
			pDescriptor->m_pDevice = &oDevice;
			oPool.CreateDescriptorSet(pDescriptor->m_oSet, *m_pLayout);

			return pDescriptor;
		}

		VkDescriptorType DescriptorLayoutWrapper::GetDescriptorType(DescriptorPool::E_BINDING_TYPE eType)
		{
			if (eType == DescriptorPool::E_STORAGE_BUFFER)
			{
				return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			}

			if (eType == DescriptorPool::E_TEXTURE)
			{
				return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			}

			if (eType == DescriptorPool::E_UNIFORM_BUFFER)
			{
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			}

			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		}

		DescriptorLayoutWrapper* DescriptorLayoutWrapper::ParseShaderFiles(ShaderMap& oMap, GraphicDevice* pDevice)
		{
			std::vector<DescriptorLayoutWrapper::Bindings> oBindings;

			for (std::pair<VkShaderStageFlags, std::string> oParse : oMap)
			{
				std::unordered_map<int, DescriptorLayoutWrapper::Bindings> oBindShader = ParseShaderFile(oParse.second);

				for (std::pair<int, DescriptorLayoutWrapper::Bindings> oPair : oBindShader)
				{
					if (oBindings.size() <= oPair.first)
					{
						int iNewSize = oPair.first + 1;
						oBindings.resize(iNewSize);

						oPair.second.eStages = oParse.first;
						oBindings[oPair.first] = oPair.second;
					}
					else if (oBindings[oPair.first].eType == DescriptorPool::E_NONE)
					{
						oPair.second.eStages = oParse.first;
						oBindings[oPair.first] = oPair.second;
					}
					else
					{
						if (oBindings[oPair.first].eType != oPair.second.eType)
						{
							throw std::runtime_error("Two different types for the same bind location");
						}
						oBindings[oPair.first].eStages |= oParse.first;
					}
				}
			}

			return new DescriptorLayoutWrapper(oBindings, *pDevice);
		}

		std::unordered_map<int, DescriptorLayoutWrapper::Bindings> DescriptorLayoutWrapper::ParseShaderFile(std::string sFilename)
		{
			std::unordered_map<int, DescriptorLayoutWrapper::Bindings> oOutput;
			DescriptorLayoutWrapper::Bindings* pCurrentBind = nullptr;

			std::fstream oReadFile;
			oReadFile.open(sFilename, std::ios::in);
			if (!oReadFile.is_open())
			{
				char pBuff[100];
				snprintf(pBuff, sizeof(pBuff), "Could not open file %s", sFilename.c_str());
				throw std::runtime_error(pBuff);
			}

			std::string sLine;
			int iIndex = -1;
			while (getline(oReadFile, sLine))
			{
				if (Bta::Utils::StringUtils::Contains(sLine, "main()"))
				{
					break;
				}

				if (Bta::Utils::StringUtils::StartWith(sLine, "layout", false) && !Bta::Utils::StringUtils::Contains(sLine, "location"))
				{
					pCurrentBind = new DescriptorLayoutWrapper::Bindings();
					std::vector<std::string> oValues = Bta::Utils::StringUtils::Split(sLine, ' ');
					iIndex = std::stoi(oValues[4]);

					if (oValues[6] == "uniform")
					{
						if (Bta::Utils::StringUtils::Contains(oValues[7], "sampler"))
						{
							pCurrentBind->eType = DescriptorPool::E_TEXTURE;
						}
						else
						{
							pCurrentBind->eType = DescriptorPool::E_UNIFORM_BUFFER;
						}
					}
					else if (oValues[6] == "buffer")
					{
						pCurrentBind->eType = DescriptorPool::E_STORAGE_BUFFER;
					}

					pCurrentBind->sTag = Bta::Utils::StringUtils::Split(oValues[pCurrentBind->eType == DescriptorPool::E_TEXTURE ? 8 : 7], ';')[0];

					if (!Bta::Utils::StringUtils::Contains(sLine, "{"))
					{
						oOutput[iIndex] = *pCurrentBind;
						delete pCurrentBind;
						pCurrentBind = nullptr;
					}
				}

				if (pCurrentBind != nullptr)
				{
					if (Bta::Utils::StringUtils::Contains(sLine, "}"))
					{
						oOutput[iIndex] = *pCurrentBind;
						delete pCurrentBind;
						pCurrentBind = nullptr;
						continue;
					}

					std::string sType = Bta::Utils::StringUtils::Split(sLine, ' ')[0];
					int iSize = Bta::Utils::StringUtils::ParseMemorySize(sType);
					if (iSize != -1)
					{
						pCurrentBind->oAllSizes.push_back(iSize);
					}
					if (Bta::Utils::StringUtils::Contains(sType, "[]"))
					{
						pCurrentBind->oAllSizes.push_back(0);
					}
				}
			}

			oReadFile.close();

			return oOutput;
		}

		bool DescriptorSetWrapper::FillSlot(int iIndex, Buffer* pBuffer)
		{
			if (iIndex < 0 || iIndex >= m_oSlots.size())
			{
				return false;
			}

			//TODO , better control
			m_oSlots[iIndex].pData = pBuffer;
			return true;
		}

		bool DescriptorSetWrapper::FillSlotAtTag(Buffer* pBuffer, std::string sTag)
		{
			for (MemorySlot& oSlot : m_oSlots)
			{
				if (oSlot.sTag == sTag)
				{
					oSlot.pData = pBuffer;
					return true;
				}
			}
			return false;
		}

		void DescriptorSetWrapper::CommitSlots(DescriptorPool* pPool)
		{
			if (m_bDelete)
			{
				vkFreeDescriptorSets(*m_pDevice->GetLogicalDevice(), pPool->GetPool(), 1, &m_oSet);
				pPool->CreateDescriptorSet(m_oSet, *m_pLayoutFrom);
			}

			for (MemorySlot& oSlot : m_oSlots)
			{
				if (oSlot.pData == nullptr)
				{
					throw std::runtime_error("A slot was not initialized");
				}
			}

			pPool->WriteDescriptor(this);
			m_bDelete = true;
		}

		DescriptorSetWrapper::~DescriptorSetWrapper()
		{
			vkFreeDescriptorSets(*m_pDevice->GetLogicalDevice(), m_pPool->GetPool(), 1, &m_oSet);
		}
	}
}

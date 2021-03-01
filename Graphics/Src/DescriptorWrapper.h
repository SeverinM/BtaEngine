#ifndef H_DESCRIPTOR_WRAPPER
#define H_DESCRIPTOR_WRAPPER
#include "DescriptorPool.h"
#include <unordered_map>
#include <string>

namespace Bta
{
	namespace Graphic
	{
		class DescriptorSetWrapper
		{
			friend class DescriptorLayoutWrapper;
			friend class BasicWrapper;

			public:
				struct MemorySlot
				{
					MemorySlot() : pData(nullptr), eType(DescriptorPool::E_NONE), sTag(""), iOffset(0) {};
					Buffer* pData;
					DescriptorPool::E_BINDING_TYPE eType;
					std::vector<size_t> oElementsSize;
					std::string sTag;
					uint32_t iOffset;
				};
				inline VkDescriptorSet* GetDescriptorSet() { return &m_oSet; }
				inline const std::vector<MemorySlot>& GetSlots() { return m_oSlots; }
				MemorySlot* GetSlotWithTag(std::string sTag);
				bool FillSlot(int iIndex, Buffer* pBuffer);
				bool FillSlotAtTag(Buffer* pBuffer, std::string sTag, int iOffset = 0);
				bool CommitSlots(DescriptorPool* pPool);
				~DescriptorSetWrapper();

			protected:
				DescriptorPool* m_pPool;
				DescriptorSetWrapper() : m_pPool(nullptr), m_pLayoutFrom(nullptr) {};
				std::vector<MemorySlot> m_oSlots;
				VkDescriptorSet m_oSet{};
				VkDescriptorSetLayout* m_pLayoutFrom;
				bool m_bDelete;
		};

		class DescriptorLayoutWrapper
		{
			public:

				typedef std::vector<size_t> BindingSizes;
				struct Bindings
				{
					Bindings() : eType(DescriptorPool::E_NONE), eStages(VK_SAMPLE_COUNT_1_BIT), oAllSizes(0), sTag("") {};
					DescriptorPool::E_BINDING_TYPE eType;
					VkShaderStageFlags eStages;
					BindingSizes oAllSizes;
					std::string sTag;
				};

				DescriptorLayoutWrapper(std::vector<Bindings>& oBindings);
				~DescriptorLayoutWrapper();

				DescriptorSetWrapper* InstantiateDescriptorSet(DescriptorPool& oPool);
				static VkDescriptorType GetDescriptorType(DescriptorPool::E_BINDING_TYPE eType);

				inline VkDescriptorSetLayout* GetLayout() { return m_pLayout; }

				typedef std::unordered_map<VkShaderStageFlags, std::string> ShaderMap;
				static DescriptorLayoutWrapper* ParseShaderFiles(ShaderMap& oMap);

			protected:
				static std::unordered_map<int, Bindings> ParseShaderFile(std::string sFilename);
				std::vector<Bindings> m_oAllBindings;
				VkDescriptorSetLayout* m_pLayout;
		};
	}
}

#endif
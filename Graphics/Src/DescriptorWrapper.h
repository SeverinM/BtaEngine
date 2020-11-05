#ifndef H_DESCRIPTOR_WRAPPER
#define H_DESCRIPTOR_WRAPPER
#include "GraphicWrapper.h"
#include "DescriptorPool.h"
#include <unordered_map>
#include <string>

class DescriptorSetWrapper
{
	friend class DescriptorLayoutWrapper;
	friend class BasicWrapper;

public:
	struct MemorySlot
	{
		MemorySlot() : pData(nullptr), eType(DescriptorPool::E_NONE), sTag(""){};
		Buffer* pData;
		DescriptorPool::E_BINDING_TYPE eType;
		std::vector<size_t> oElementsSize;
		std::string sTag;
	};
	inline VkDescriptorSet* GetDescriptorSet() { return &m_oSet; }
	inline const std::vector<MemorySlot>& GetSlots() { return m_oSlots; }
	bool FillSlot(int iIndex, Buffer* pBuffer);
	bool FillSlotAtTag(Buffer* pBuffer, std::string sTag);
	void CommitSlots(DescriptorPool* pPool);
	~DescriptorSetWrapper();

protected:
	DescriptorPool* m_pPool;
	GraphicDevice* m_pDevice;
	DescriptorSetWrapper() : m_pPool(nullptr), m_pDevice(nullptr), m_pLayoutFrom(nullptr){};
	std::vector<MemorySlot> m_oSlots;
	VkDescriptorSet m_oSet{};
	VkDescriptorSetLayout* m_pLayoutFrom;
	bool m_bDelete;
};

class DescriptorLayoutWrapper
{
public:

	//0 = repeat last memory slot with undefined amount
	typedef std::vector<size_t> BindingSizes;
	struct Bindings
	{
		Bindings() : eType(DescriptorPool::E_NONE), eStages(VK_SAMPLE_COUNT_1_BIT), oAllSizes(0), sTag("") {};
		DescriptorPool::E_BINDING_TYPE eType;
		VkShaderStageFlags eStages;
		BindingSizes oAllSizes;
		std::string sTag;
	};

	DescriptorLayoutWrapper(std::vector <Bindings>& oBindings, GraphicDevice& oDevice );
	~DescriptorLayoutWrapper();

	DescriptorSetWrapper* InstantiateDescriptorSet(DescriptorPool& oPool, GraphicDevice& oDevice);
	static VkDescriptorType GetDescriptorType(DescriptorPool::E_BINDING_TYPE eType);

	inline VkDescriptorSetLayout* GetLayout() { return m_pLayout; }

	typedef std::unordered_map<VkShaderStageFlags, std::string> ShaderMap;
	static DescriptorLayoutWrapper* ParseShaderFiles(ShaderMap& oMap, GraphicDevice* pDevice);

protected:
	static std::unordered_map<int, Bindings> ParseShaderFile(std::string sFilename);
	std::vector<Bindings> m_oAllBindings;
	VkDescriptorSetLayout* m_pLayout;
};

#endif
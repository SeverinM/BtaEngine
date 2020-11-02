#ifndef H_DESCRIPTOR_WRAPPER
#define H_DESCRIPTOR_WRAPPER
#include "GraphicWrapper.h"
#include "DescriptorPool.h"
#include <unordered_map>
#include <string>

#define TAG_MVP "MVP"
#define TAG_MODELS "DataModels"


class DescriptorSetWrapper
{
	friend class DescriptorLayoutWrapper;
	friend class BasicWrapper;

public:
	struct MemorySlot
	{
		MemorySlot() : pData(nullptr) {};
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
	DescriptorSetWrapper() {};
	std::vector<MemorySlot> m_oSlots;
	VkDescriptorSet m_oSet;
	VkDescriptorSetLayout* m_pLayoutFrom;
};

class DescriptorLayoutWrapper
{
public:

	//0 = repeat last memory slot with undefined amount
	typedef std::vector<size_t> BindingSizes;
	struct Bindings
	{
		Bindings()
		{
			eType = DescriptorPool::E_NONE;
		};
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
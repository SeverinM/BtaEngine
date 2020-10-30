#ifndef H_DESCRIPTOR_WRAPPER
#define H_DESCRIPTOR_WRAPPER
#include "GraphicWrapper.h"
#include "DescriptorPool.h"
#include "Buffer.h"
#include <unordered_map>
#include <string>

enum E_BINDING_TYPE
{
	E_TEXTURE = 0,
	E_UNIFORM_BUFFER,
	E_STORAGE_BUFFER
};

class DescriptorSetWrapper
{
	friend class DescriptorLayoutWrapper;

public:
	struct MemorySlot
	{
		void* pData;
		E_BINDING_TYPE eType;
		int iSizeUnit;
		int iUnitCount;
	};
	inline VkDescriptorSet* GetDescriptorSet() { return &m_oSet; }

protected:
	DescriptorSetWrapper() {};
	std::vector<MemorySlot> m_oAllDatas;
	VkDescriptorSet m_oSet;
};

class DescriptorLayoutWrapper
{
public:
	struct Bindings
	{
		E_BINDING_TYPE eType;
		VkShaderStageFlags eStages;
		int iSizeUnit;
		int iUnitCount;
	};

	DescriptorLayoutWrapper(std::vector <Bindings>& oBindings, GraphicDevice& oDevice );
	DescriptorSetWrapper* InstantiateDescriptorSet(DescriptorPool& oPool, GraphicDevice& oDevice);
	static VkDescriptorType GetDescriptorType(E_BINDING_TYPE eType);
	inline VkDescriptorSetLayout* GetLayout() { return m_pLayout; }

	typedef std::unordered_map<VkShaderStageFlags, std::string> ShaderMap;
	static DescriptorLayoutWrapper* ParseShaderFiles(ShaderMap& oMap, GraphicDevice* pDevice);

protected:
	std::vector<Bindings> m_oAllBindings;
	VkDescriptorSetLayout* m_pLayout;
};

#endif
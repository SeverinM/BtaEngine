#ifndef H_DESCRIPTOR
#define H_DESCRIPTOR
#include "Buffer.h"

class BasicWrapper;
class DescriptorSetWrapper;

class DescriptorPool
{
public:

	enum E_BINDING_TYPE
	{
		E_NONE = 0,
		E_TEXTURE,
		E_UNIFORM_BUFFER,
		E_STORAGE_BUFFER
	};

	struct Desc
	{
		int iSize;
		int iImageCount;
		BasicWrapper* pWrapper;
	};

	DescriptorPool(Desc& oDesc);
	void Create(Desc& oDesc);

	VkDescriptorPool& GetPool() { return m_oPool; }

	static VkDescriptorType GetDescriptorType(E_BINDING_TYPE eType);
	~DescriptorPool();

	void WriteDescriptor(DescriptorSetWrapper* pDescriptorSet);
	void CreateDescriptorSet(std::vector<VkDescriptorSet>& oOutput, int iSize, const VkDescriptorSetLayout& oLayout);
	void CreateDescriptorSet(VkDescriptorSet& oOutput,const VkDescriptorSetLayout& oLayout);

private:
	VkDescriptorPool m_oPool;
	Desc* m_pRecreate;
};

#endif // !H_DESCRIPTOR
#ifndef H_DESCRIPTOR
#define H_DESCRIPTOR
#include "Buffer.h"

class BasicWrapper;

class DescriptorPool
{
public:
	struct Desc
	{
		int iSize;
		int iImageCount;
		BasicWrapper* pWrapper;
	};

	enum EBufferType
	{
		E_TEXTURE,
		E_UNIFORM_BUFFER,
		E_STORAGE_BUFFER
	};

	struct BufferDesc
	{
		std::shared_ptr<Buffer> xBuffer;
		EBufferType eType;
	};

	struct UpdateSubDesc
	{
		std::vector<BufferDesc> oBuffers;
		VkDescriptorSet* pDescriptorSet;
	};

	DescriptorPool(Desc& oDesc);
	void Create(Desc& oDesc);

	VkDescriptorPool& GetPool() { return m_oPool; }

	static VkDescriptorType GetDescriptorType(EBufferType eType);
	~DescriptorPool();

	void WriteDescriptor(std::vector< UpdateSubDesc >& oUpdate, const VkDescriptorSetLayout& oDescriptorSetLayout);
	void CreateDescriptorSet(std::vector<VkDescriptorSet>& oOutput, int iSize, const VkDescriptorSetLayout& oLayout);
	void CreateDescriptorSet(VkDescriptorSet& oOutput,const VkDescriptorSetLayout& oLayout);

private:
	VkDescriptorPool m_oPool;
	Desc* m_pRecreate;
};

#endif // !H_DESCRIPTOR
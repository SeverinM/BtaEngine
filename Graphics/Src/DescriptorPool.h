#ifndef H_DESCRIPTOR
#define H_DESCRIPTOR
#include "Buffer.h"
#include "Resizable.h"

class BasicWrapper;

class DescriptorPool : public Resizable
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
		Buffer* pBuffer;
		EBufferType eType;
	};

	struct UpdateSubDesc
	{
		std::vector<BufferDesc> oBuffers;
		VkDescriptorSet* pDescriptorSet;
	};

	DescriptorPool(Desc& oDesc);
	void Create(Desc& oDesc);
	static VkDescriptorType GetDescriptorType(EBufferType eType);
	~DescriptorPool();

	void WriteDescriptor(std::vector< UpdateSubDesc >& oUpdate, const VkDescriptorSetLayout& oDescriptorSetLayout);
	void CreateDescriptorSet(std::vector<VkDescriptorSet>& oOutput, int iSize, const VkDescriptorSetLayout& oLayout);
	void CreateDescriptorSet(VkDescriptorSet& oOutput,const VkDescriptorSetLayout& oLayout);

	virtual void Free() override;
	virtual void Recreate(int iNewWidth, int iNewHeight, void* pData) override;

private:
	VkDescriptorPool m_oPool;
	Desc* m_pRecreate;
};

#endif // !H_DESCRIPTOR
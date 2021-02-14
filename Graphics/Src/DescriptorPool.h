#ifndef H_DESCRIPTOR
#define H_DESCRIPTOR
#include "Buffer.h"

namespace Bta
{
	namespace Graphic
	{
		class BasicWrapper;
		class DescriptorSetWrapper;

		class DescriptorPool
		{
			public:

				enum E_BINDING_TYPE
				{
					E_NONE = 0,
					E_TEXTURE = 1,
					E_UNIFORM_BUFFER = 2,
					E_STORAGE_BUFFER = 3
				};

				struct Desc
				{
					int iSize;
					int iMaxSet;
				};

				DescriptorPool(Desc& oDesc);
				void Create(Desc& oDesc);

				VkDescriptorPool& GetPool() { return m_oPool; }

				static VkDescriptorType GetDescriptorType(E_BINDING_TYPE eType);
				~DescriptorPool();

				void WriteDescriptor(DescriptorSetWrapper* pDescriptorSet);
				void CreateDescriptorSet(std::vector<VkDescriptorSet>& oOutput, int iSize, const VkDescriptorSetLayout& oLayout);
				void CreateDescriptorSet(VkDescriptorSet& oOutput, const VkDescriptorSetLayout& oLayout);

			private:
				VkDescriptorPool m_oPool;
				Desc* m_pRecreate;
		};
	}
}

#endif // !H_DESCRIPTOR
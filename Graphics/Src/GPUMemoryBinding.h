#ifndef H_GPU_MEMORY_BINDING
#define H_GPU_MEMORY_BINDING

#include "Buffer.h"
#include "GPUMemoryInterface.h"

namespace Bta
{
	namespace Graphic
	{
		struct GPUMemoryBinding
		{
				GPUMemoryBinding(Buffer* pBuffer, uint64_t iOffset, uint64_t iElementSize)
					: m_pBuffer(pBuffer), m_iOffset(iOffset), m_iElementSize(iElementSize),m_pData(nullptr), m_bMemoryResponsible(false) {}

				GPUMemoryBinding(VkBufferUsageFlags eUsage, uint64_t iElementSize)
					: m_iOffset(0), m_iElementSize(iElementSize), m_pData(nullptr), m_bMemoryResponsible(false) 
				{
					BasicBuffer::Desc oBufferDesc;
					oBufferDesc.iUnitCount = 1;
					oBufferDesc.iUnitSize = iElementSize;
					oBufferDesc.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
					oBufferDesc.eUsage = eUsage;

					m_pBuffer = GPUMemory::GetInstance()->AllocateMemory(oBufferDesc);
				}

				GPUMemoryBinding(const GPUMemoryBinding& oMemoryBinding) :
					m_pBuffer(oMemoryBinding.m_pBuffer),
					m_iOffset(oMemoryBinding.m_iOffset),
					m_iElementSize(oMemoryBinding.m_iElementSize)
				{
				}

				void operator=(const GPUMemoryBinding& oMemoryBinding)
				{
					m_pBuffer = oMemoryBinding.m_pBuffer;
					m_iOffset = oMemoryBinding.m_iOffset;
					m_iElementSize = oMemoryBinding.m_iElementSize;
				}

				GPUMemoryBinding() : m_iOffset(0), m_iElementSize(0), m_pData(nullptr), m_bMemoryResponsible(false), m_pBuffer(nullptr){}

				~GPUMemoryBinding()
				{
					/*if (m_pData != nullptr)
						throw std::runtime_error("Value not destroyed");*/
				}

				Buffer* GetBuffer()
				{
					return m_pBuffer;
				}

				uint32_t GetOffset()
				{
					return m_iOffset;
				}

				template<typename Type>
				Type* GetValue()
				{
					static_assert(sizeof(Type) <= m_iElementSize - m_iOffset, "Incorrect template size");
					return (Type*)m_pData;
				}

				template<typename Type>
				void SetValue(Type oValue)
				{
					static_assert(sizeof(Type) <= m_iElementSize - m_iOffset, "Incorrect template size");
					DestroyData<Type>();
					m_pData = new Type(oValue);
					m_bMemoryResponsible = true;
					RefreshGPU();
				}

				template<typename Type>
				void SetValue(Type* pValue)
				{
					static_assert(sizeof(Type) == m_iElementSize, "Incorrect template size");
					DestroyData<Type>();
					m_pData = pValue;
					m_bMemoryResponsible = false;
					RefreshGPU();
				}

				template<typename Type>
				void DestroyData()
				{
					if (m_bMemoryResponsible)
					{
						delete (Type*)m_pData;
					}
					m_pData = nullptr;
				}

				void RefreshGPU()
				{
					m_pBuffer->CopyFromMemory(m_pData, m_iOffset, m_iElementSize);
				}

				Buffer* m_pBuffer;
				uint64_t m_iOffset;
				uint64_t m_iElementSize;
				void* m_pData;
				bool m_bMemoryResponsible;
		};
	}
}

#endif

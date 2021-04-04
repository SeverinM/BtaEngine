#include "MeshComponent.h"
#include <algorithm>

namespace Bta
{
	namespace Graphic
	{		

		void MeshComponent::AddVertice(Vertice oVert, int iIndex)
		{
			if (iIndex < 0 || iIndex > m_oVertices.size())
			{
				iIndex = m_oVertices.size();

				if (iIndex < 0)
					iIndex = 0;
			}

			std::list<Vertice>::iterator pItVert = m_oVertices.begin();
			for (int i = 0; i < iIndex; i++)
			{
				pItVert++;
			}

			m_oVertices.insert(pItVert, oVert);

			if (m_pGPUVertices != nullptr)
			{
				int iOffset = GetVerticeSize() * iIndex;
				if (m_oVertices.size() >= m_iAllocatedVerticeCount)
				{
					m_iAllocatedVerticeCount *= 2;
					BasicBuffer* pBasicBuffer = (BasicBuffer*)m_pGPUVertices->GetBuffer();
					pBasicBuffer->Reallocate(m_iAllocatedVerticeCount, GetVerticeSize());
				}

				RefreshVerticeBinding(std::max(iIndex - 1, 0));
			}
		}

		void MeshComponent::RemoveVertice(int iIndex)
		{
			if (iIndex < 0 || iIndex >= m_oVertices.size())
			{
				iIndex = m_oVertices.size() - 1;
			}

			std::list<Vertice>::iterator pItVertice = m_oVertices.begin();
			for (int i = 0; i < iIndex; i++)
			{
				pItVertice++;
			}

			m_oVertices.erase(pItVertice);

			if (m_pGPUVertices != nullptr)
			{
				RefreshVerticeBinding(std::max(iIndex - 1, 0));
			}
		}

		Vertice MeshComponent::GetVertice(int iIndex)
		{
			if (iIndex < 0 || iIndex >= m_oVertices.size())
			{
				iIndex = m_oVertices.size() - 1;
			}

			std::list<Vertice>::iterator pItVertice = m_oVertices.begin();
			for (int i = 0; i < iIndex; i++)
			{
				pItVertice++;
			}

			return *pItVertice;
		}

		void MeshComponent::SetVertice(Vertice oVertice, int iIndex)
		{
			if (iIndex < 0 || iIndex >= m_oVertices.size())
			{
				iIndex = m_oVertices.size() - 1;
			}

			std::list<Vertice>::iterator pItVertice = m_oVertices.begin();
			for (int i = 0; i < iIndex; i++)
			{
				pItVertice++;
			}

			(*pItVertice) = oVertice;

			if (m_pGPUVertices != nullptr)
			{
				m_pGPUVertices->GetBuffer()->CopyFromMemory(&oVertice, iIndex * GetVerticeSize(), GetVerticeSize());
			}
		}

		void MeshComponent::AddIndex(IndexType iValue, int iIndex)
		{
			if (iIndex < 0 || iIndex >= m_oIndexes.size())
			{
				iIndex = m_oVertices.size() - 1;
			}

			std::list<IndexType>::iterator pItIndex = m_oIndexes.begin();
			for (int i = 0; i < iIndex; i++)
			{
				pItIndex++;
			}

			m_oIndexes.insert(pItIndex, iValue);

			if (m_pGPUIndices != nullptr)
			{
				int iOffset = sizeof(IndexType) * iIndex;
				if (m_oIndexes.size() >= m_iAllocatedIndexCount)
				{
					m_iAllocatedVerticeCount *= 2;
					BasicBuffer* pBasicBuffer = (BasicBuffer*)m_pGPUIndices->GetBuffer();
					pBasicBuffer->Reallocate(m_iAllocatedIndexCount, sizeof(IndexType));
				}

				RefreshIndicesBinding(std::max(iIndex - 1, 0));
			}
		}

		void MeshComponent::RemoveIndex(int iIndex)
		{
			if (iIndex < 0 || iIndex >= m_oIndexes.size())
			{
				iIndex = m_oVertices.size() - 1;
			}

			std::list<IndexType>::iterator pItIndex = m_oIndexes.begin();
			for (int i = 0; i < iIndex; i++)
			{
				pItIndex++;
			}

			m_oIndexes.erase(pItIndex);

			if (m_pGPUIndices != nullptr)
			{
				RefreshIndicesBinding(std::max(iIndex - 1, 0));
			}
		}

		MeshComponent::IndexType MeshComponent::GetIndex(int iIndex)
		{
			if (iIndex < 0 || iIndex >= m_oIndexes.size())
			{
				iIndex = m_oVertices.size() - 1;
			}

			std::list<IndexType>::iterator pItIndex = m_oIndexes.begin();
			for (int i = 0; i < iIndex; i++)
			{
				pItIndex++;
			}

			return *pItIndex;
		}

		void MeshComponent::SetIndex(IndexType iValue, int iIndex)
		{
			if (iIndex < 0 || iIndex >= m_oIndexes.size())
			{
				iIndex = m_oVertices.size() - 1;
			}

			std::list<IndexType>::iterator pItIndex = m_oIndexes.begin();
			for (int i = 0; i < iIndex; i++)
			{
				pItIndex++;
			}

			(*pItIndex) = iValue;

			if (m_pGPUIndices != nullptr)
			{
				IndexType iValue = *pItIndex;
				m_pGPUVertices->GetBuffer()->CopyFromMemory(&iValue, sizeof(IndexType) * iIndex, sizeof(IndexType));
			}
		}

		void MeshComponent::FreeGPUMemory(bool bIncludeVertices)
		{
			if (bIncludeVertices)
			{
				if (m_pGPUVertices != nullptr)
				{
					delete m_pGPUVertices->GetBuffer();
					delete m_pGPUVertices;
				}
			}
			
			if (m_pGPUIndices != nullptr)
			{
				delete m_pGPUIndices->GetBuffer();
				delete m_pGPUIndices;
			}			
		}

		void MeshComponent::AllocateGPUMemory(uint32_t iVerticeCount, uint32_t iIndexCount)
		{
			if ( m_pGPUVertices != nullptr )
			{
				delete m_pGPUVertices->GetBuffer();
				delete m_pGPUVertices;
			}

			if ( m_pGPUIndices != nullptr )
			{
				delete m_pGPUIndices->GetBuffer();
				delete m_pGPUIndices;
			}

			if ( iVerticeCount > 0 )
			{
				m_pGPUVertices = new GPUMemoryBinding(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, iVerticeCount * GetVerticeSize());
			}

			if (iIndexCount > 0)
			{
				m_pGPUIndices = new GPUMemoryBinding(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, iIndexCount * sizeof(IndexType));
			}

			m_iAllocatedIndexCount = iIndexCount;
			m_iAllocatedVerticeCount = iVerticeCount;
		}

		void MeshComponent::RefreshVerticeBinding(int iIndex /*= 0*/)
		{
			uint32_t iOffset = 0;
			std::list<Vertice>::iterator pIt = m_oVertices.begin();

			for (int i = 0; i < iIndex; i++)
			{
				pIt++;
				iOffset += GetVerticeSize();
			}

			while (pIt != m_oVertices.end())
			{
				Vertice oVert = *pIt;
				m_pGPUVertices->m_pBuffer->CopyFromMemory(&oVert, iOffset, GetVerticeSize());
				iOffset += GetVerticeSize();
				pIt++;
			}
		}

		void MeshComponent::RefreshIndicesBinding(int iIndex /*= 0*/)
		{
			uint32_t iOffset = 0;
			std::list<uint32_t>::iterator pIt = m_oIndexes.begin();

			for (int i = 0; i < iIndex; i++)
			{
				pIt++;
			}

			while (pIt != m_oIndexes.end())
			{
				uint32_t iValue = *pIt;
				m_pGPUVertices->m_pBuffer->CopyFromMemory(&iValue, iOffset,sizeof(uint32_t) );
				iOffset += sizeof(uint32_t);
				pIt++;
			}
		}

	}
}
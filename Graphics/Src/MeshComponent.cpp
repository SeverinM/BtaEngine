#include "MeshComponent.h"

namespace Bta
{
	namespace Graphic
	{		

		void MeshComponent::AddVertice(Vertice oVert, int iIndex)
		{
			if (iIndex < 0 || iIndex >= m_oVertices.size())
			{
				iIndex = m_oVertices.size() - 1;
			}

			std::list<Vertice>::iterator pItVert = m_oVertices.begin();
			for (int i = 0; i < iIndex; i++)
			{
				pItVert++;
			}

			m_oVertices.insert(pItVert, oVert);

			m_iTrueVerticeCount++;
			if (m_pGPUVertices != nullptr)
			{
				int iOffset = GetVerticeSize() * iIndex;
				if (m_iTrueVerticeCount >= m_iAllocatedVerticeCount)
				{
					m_iAllocatedVerticeCount *= 2;
					BasicBuffer* pBasicBuffer = (BasicBuffer*)m_pGPUVertices->GetBuffer();
					pBasicBuffer->Reallocate(m_iAllocatedVerticeCount, GetVerticeSize());
				}

				for (int i = iIndex; i < m_iTrueVerticeCount; i++)
				{
					Vertice oVert = *pItVert;
					m_pGPUVertices->GetBuffer()->CopyFromMemory(&oVert, GetVerticeSize() * i, GetVerticeSize());
					pItVert++;
				}
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
			m_iTrueVerticeCount--;

			if (m_pGPUVertices != nullptr)
			{
				int iOffset = GetVerticeSize() * iIndex;

				for (int i = iIndex; i < m_iTrueVerticeCount; i++)
				{
					Vertice oVert = *pItVertice;
					m_pGPUVertices->GetBuffer()->CopyFromMemory(&oVert, GetVerticeSize() * i, GetVerticeSize());
					pItVertice++;
				}
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

			m_iTrueIndexCount++;
			if (m_pGPUIndices != nullptr)
			{
				int iOffset = sizeof(IndexType) * iIndex;
				if (m_iTrueIndexCount >= m_iAllocatedIndexCount)
				{
					m_iAllocatedVerticeCount *= 2;
					BasicBuffer* pBasicBuffer = (BasicBuffer*)m_pGPUIndices->GetBuffer();
					pBasicBuffer->Reallocate(m_iAllocatedIndexCount, sizeof(IndexType));
				}

				for (int i = iIndex; i < m_iTrueVerticeCount; i++)
				{
					IndexType iValue = *pItIndex;
					m_pGPUVertices->GetBuffer()->CopyFromMemory(&iValue, sizeof(IndexType) * i, sizeof(IndexType));
					pItIndex++;
				}
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

			m_iTrueIndexCount--;
			if (m_pGPUIndices != nullptr)
			{
				for (int i = iIndex; i < m_iTrueVerticeCount; i++)
				{
					IndexType iValue = *pItIndex;
					m_pGPUVertices->GetBuffer()->CopyFromMemory(&iValue, sizeof(IndexType) * i, sizeof(IndexType));
					pItIndex++;
				}
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

	}
}
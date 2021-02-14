#include "MeshComponent.h"

namespace Bta
{
	namespace Core
	{
		MeshComponent::MeshComponent(uint8_t iSizes[4]) : AbstractComponent(nullptr), m_oComponentsSize(iSizes)
		{
			if (iSizes[E_POSITION] == 0)
			{
				throw std::exception("Size of positions must be positive");
			}
		}

		void MeshComponent::AddVertice(Vertice oVertice)
		{
			m_oPositions.push_back(oVertice.vPosition);
			SendChangeEvent(E_POSITION, m_oPositions.size() - 1, &m_oPositions[m_oPositions.size() - 1]);

			if (m_oComponentsSize[E_NORMAL] > 0)
			{
				m_oNormals.push_back(oVertice.vNormal);
				SendChangeEvent(E_NORMAL, m_oNormals.size() - 1, &m_oNormals[m_oNormals.size() - 1]);
			}

			if (m_oComponentsSize[E_COLOR] > 0)
			{
				m_oColors.push_back(oVertice.vColor);
				SendChangeEvent(E_COLOR, m_oColors.size() - 1, &m_oColors[m_oColors.size() - 1]);
			}

			if (m_oComponentsSize[E_UV] > 0)
			{
				m_oUVs.push_back(oVertice.vUV);
				SendChangeEvent(E_UV, m_oUVs.size() - 1, &m_oUVs[m_oUVs.size() - 1]);
			}
		}

		void MeshComponent::RemoveVertice(int iIndex /*= -1*/)
		{
			if (iIndex < 0)
			{
				iIndex = m_oPositions.size() - 1;
			}

			m_oPositions.erase(m_oPositions.begin() + iIndex);

			if (m_oComponentsSize[E_NORMAL] > 0)
			{
				m_oNormals.erase(m_oNormals.begin() + iIndex);
			}

			if (m_oComponentsSize[E_COLOR] > 0)
			{
				m_oColors.erase(m_oColors.begin() + iIndex);
			}

			if (m_oComponentsSize[E_UV] > 0)
			{
				m_oUVs.erase(m_oUVs.begin() + iIndex);
			}

		}

		Vertice MeshComponent::GetVertice(int iIndex /*= -1*/) const
		{
			if (iIndex < 0)
			{
				iIndex = m_oPositions.size() - 1;
			}

			Vertice oVertice;
			oVertice.vPosition = m_oPositions[iIndex];
			
			if (m_oComponentsSize[E_NORMAL] > 0)
				oVertice.vNormal = m_oNormals[iIndex];

			if (m_oComponentsSize[E_COLOR] > 0)
				oVertice.vColor = m_oColors[iIndex];

			if (m_oComponentsSize[E_UV] > 0)
				oVertice.vUV = m_oColors[iIndex];

			return oVertice;
		}

		void MeshComponent::SetNormal(glm::vec3 vNormal, int iIndex /*= -1*/)
		{
			if (m_oComponentsSize[E_NORMAL] == 0)
				printf("Normal is disabled for this mesh");

			if (iIndex < 0)
				iIndex = m_oNormals.size() - 1;

			m_oNormals[iIndex] = vNormal;
			SendChangeEvent(EMeshElements::E_NORMAL, iIndex, &m_oNormals[iIndex]);
		}

		void MeshComponent::SetPosition(glm::vec3 vPosition, int iIndex /*= -1*/)
		{
			if (iIndex < 0)
				iIndex = m_oPositions.size() - 1;

			m_oPositions[iIndex] = vPosition;
			SendChangeEvent(EMeshElements::E_POSITION, iIndex, &m_oPositions[iIndex]);
		}

		void MeshComponent::SetUV(glm::vec2 vUV, int iIndex /*= -1*/)
		{
			if (m_oComponentsSize[E_UV] == 0)
				printf("UV is disabled for this mesh");

			if (iIndex < 0)
				iIndex = m_oUVs.size() - 1;

			m_oUVs[iIndex] = vUV;
			SendChangeEvent(EMeshElements::E_UV, iIndex, &m_oUVs[iIndex]);
		}

		void MeshComponent::SetColor(glm::vec4 vColor, int iIndex /*= -1*/)
		{
			if (m_oComponentsSize[E_COLOR] > 0)
				printf("Color is disabled for this mesh");

			if (iIndex < 0)
				iIndex = m_oColors.size() - 1;

			m_oColors[iIndex] = vColor;
			SendChangeEvent(EMeshElements::E_COLOR, iIndex, &m_oColors[iIndex]);
		}

		void MeshComponent::SendChangeEvent(EMeshElements eChannel, int iIndex, void* pData)
		{
			EventHolder* pHolder = EventHolder::GetInstance();
			MeshUpdateEvent oUpdate;
			oUpdate.pSubject = this;
			oUpdate.eChannel = eChannel;
			oUpdate.pNewData = pData;
			oUpdate.iIndex = iIndex;
			pHolder->PushEvent(E_MODIFIED_VERTICE_BUFFER, oUpdate);
		}

		uint64_t MeshComponent::GetOffsetRawData(EMeshElements eChannel, int iIndex)
		{
			uint64_t iOffset = (uint64_t)iIndex * GetVerticeSize();
			for (int i = 0; i < eChannel; i++)
			{
				iOffset += m_oComponentsSize[i];
			}
			return iOffset;
		}

	}
}

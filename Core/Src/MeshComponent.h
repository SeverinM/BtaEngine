#ifndef H_MESH
#define H_MESH

#include "../Include/GLM/glm.hpp"
#include <vector>
#include "Entity.h"

namespace Bta
{
	namespace Core
	{
		enum EMeshElements
		{
			E_POSITION = 0,
			E_NORMAL,
			E_COLOR,
			E_UV
		};

		struct Vertice
		{
			glm::vec3 vPosition;
			glm::vec3 vNormal;
			glm::vec2 vUV;
			glm::vec4 vColor;
		};

		struct MeshUpdateEvent : BaseEvent
		{
			EMeshElements eChannel;
			int iIndex;
			void* pNewData;
		};
		
		class MeshComponent : public AbstractComponent
		{
			typedef uint32_t MeshElementsFlag;

			public:
				MeshComponent(uint8_t iSizes[4]);

				void Init() override {};
				constexpr uint8_t GetVerticeSize() 
				{
					return m_oComponentsSize[0] + m_oComponentsSize[1] + m_oComponentsSize[2] + m_oComponentsSize[3];
				}

				//-1 = the last one
				void AddVertice(Vertice oVertice);
				void RemoveVertice(int iIndex = -1);
				Vertice GetVertice(int iIndex = -1) const;
				void SetNormal(glm::vec3 vNormal, int iIndex = -1);
				void SetPosition(glm::vec3 vPosition, int iIndex = -1);
				void SetUV(glm::vec2 vUV, int iIndex = -1);
				void SetColor(glm::vec4 vColor, int iIndex = -1);
				void SendChangeEvent(EMeshElements eChannel, int iIndex, void* pData);

				uint64_t GetOffsetRawData(EMeshElements eChannel, int iIndex);

				const uint8_t* m_oComponentsSize;

			protected:

				std::vector<glm::vec3> m_oNormals;
				std::vector<glm::vec3> m_oPositions;
				std::vector<glm::vec4> m_oColors;
				std::vector<glm::vec2> m_oUVs;
		};
	}
}

#endif H_MESH

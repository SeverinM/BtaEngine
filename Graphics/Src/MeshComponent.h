#ifndef H_MESH_COMPONENT
#define H_MESH_COMPONENT

#include "../../Core/Src/Entity.h"
#include "GPUMemoryBinding.h"
#include "../../Core/Include/GLM/glm.hpp"

namespace Bta
{
	namespace Graphic
	{
		enum EMeshElements
		{
			E_POSITION = 0,
			E_NORMAL,
			E_COLOR,
			E_UV,
			E_INDEX
		};

		struct Vertice
		{
			glm::vec3 vPosition;
			glm::vec3 vNormal;
			glm::vec2 vUV;
			glm::vec4 vColor;
		};

		class MeshComponent : public Core::AbstractComponent
		{
			public:
				typedef uint32_t IndexType;
				MeshComponent(Core::Entity* pOwner) :
					m_pGPUVertices(nullptr),
					m_pGPUIndices(nullptr),
					m_iAllocatedIndexCount(0),
					m_iAllocatedVerticeCount(0) {};

				void Init() override
				{
					AllocateGPUMemory(50, 0);
				}
				constexpr int GetVerticeSize()
				{
					return sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec4);
				}

				void AddVertice(Vertice oVert, int iIndex);
				void RemoveVertice(int iIndex);
				Vertice GetVertice(int iIndex);
				void SetVertice(Vertice oVertice, int iIndex);

				void AddIndex(IndexType iValue, int iIndex);
				void RemoveIndex(int iIndex);
				IndexType GetIndex(int iIndex);
				void SetIndex(IndexType iValue, int iIndex);

				void FreeGPUMemory(bool bIncludeVertices);
				void AllocateGPUMemory(uint32_t iVerticeCount, uint32_t iIndexCount);

				GPUMemoryBinding* GetVerticeBinding() { return m_pGPUVertices; }
				GPUMemoryBinding* GetIndexBinding() { return m_pGPUIndices; }
				void RefreshVerticeBinding(int iIndex = 0);
				void RefreshIndicesBinding(int iIndex = 0);

				int GetVerticeCount() { return m_oVertices.size(); }
				int GetIndicesCount() { return m_oIndexes.size(); }

			protected:
				std::list<Vertice> m_oVertices;
				std::list<uint32_t> m_oIndexes;

				GPUMemoryBinding* m_pGPUVertices;
				GPUMemoryBinding* m_pGPUIndices;

				int m_iAllocatedVerticeCount;
				int m_iAllocatedIndexCount;
		};
	}
}

#endif

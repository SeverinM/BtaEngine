#ifndef H_RENDER_BATCH
#define H_RENDER_BATCH
#include "Mesh.h"
#include "RenderPass.h"
#include <unordered_map>
#include "DescriptorWrapper.h"
#include "AbstractRenderBatch.h"

namespace Bta
{
	namespace Graphic
	{
		class RenderBatch : public AbstractRenderBatch
		{
			public:
				struct Desc
				{
					RenderPass* pRenderpass;
					Pipeline* pPipeline;
					RenderBatch* pNext;
					std::string sTag;
				};
				RenderBatch(Desc& oDesc);
				~RenderBatch();

				VkCommandBuffer* GetDrawCommand(Framebuffer* pFramebuffer) override;
				inline void MarkAsDirty() override { m_bDirty = true; m_oCachedCommandBuffer.clear(); }
				void AddMesh(Mesh::StrongPtr xMesh, DescriptorSetWrapper* pWrapper);
				void RemoveMesh(Mesh::StrongPtr xMesh);
				inline DescriptorSetWrapper* GetDescriptor(Mesh::StrongPtr xMesh) { if (m_oEntities.count(xMesh) == 0) return nullptr; return m_oEntities[xMesh]; }
				void ChainSubpass(VkCommandBuffer* pBuffer) override;
				size_t GetVerticesCount() override;
				size_t GetInstancesCount() override;
				void ClearCache();
				void Destroy() override;

			protected:
				bool m_bDirty;
				std::unordered_map<Framebuffer*, VkCommandBuffer*> m_oCachedCommandBuffer;

				void TryFillModelsBuffer(Mesh::StrongPtr xMesh);
				void ReconstructCommand(Framebuffer* pFramebuffer);

				std::unordered_map<Mesh::StrongPtr, DescriptorSetWrapper*> m_oEntities;
		};

		class RenderBatchesHandler
		{
			public:

				enum BatchType
				{
					TEXT,
					NORMAL
				};

				struct CreationBatchDesc
				{
					CreationBatchDesc() : bWriteDepth(false), bTestDepth(false), eTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST), eTypeBatch(NORMAL) {};
					std::vector<std::string> oShaderCompiled;
					bool bWriteDepth;
					bool bTestDepth;
					std::vector<std::string> oShaderSources;
					std::string sTag;
					VkPrimitiveTopology eTopology;
					BatchType eTypeBatch;
				};

				struct Desc
				{
					Desc() : pPass(nullptr), pWrapper(nullptr), eSamples(VK_SAMPLE_COUNT_1_BIT) {};
					std::vector<CreationBatchDesc> oBatches{};
					RenderPass* pPass{};
					GraphicWrapper* pWrapper{};
					VkSampleCountFlagBits eSamples{};
				};
				RenderBatchesHandler(Desc& oDesc);
				AbstractRenderBatch* GetRenderBatch(int iIndex) { if (iIndex < 0 || iIndex >= m_oBatches.size()) return nullptr; return m_oBatches[iIndex]; }
				AbstractRenderBatch* FindRenderBatch(std::string sTag)
				{
					for (AbstractRenderBatch* pBatch : m_oBatches)
					{
						if (pBatch->GetTag() == sTag)
						{
							return pBatch;
						}
					}
					return nullptr;
				}

				Pipeline* GetPipeline(int iIndex) { if (iIndex < 0 || iIndex >= m_oBatches.size()) return nullptr; return m_oBatches[iIndex]->GetPipeline(); }
				VkCommandBuffer* GetCommand(Framebuffer* pFramebuffer);
				void ReconstructPipelines(GraphicWrapper* pWrapper);
				void MarkAllAsDirty() { for (AbstractRenderBatch* pBatch : m_oBatches) { pBatch->MarkAsDirty(); } }

				size_t GetVerticesCount()
				{
					size_t iVertices = 0;
					for (AbstractRenderBatch* pBatch : m_oBatches)
					{
						iVertices += pBatch->GetVerticesCount();
					}
					return iVertices;
				}

				size_t GetInstancesCount()
				{
					uint64_t iInstanceCount = 0;
					for (AbstractRenderBatch* pBatch : m_oBatches)
					{
						iInstanceCount += pBatch->GetInstancesCount();
					}
					return iInstanceCount;
				}

			protected:
				VkSampleCountFlagBits m_eSamples;
				RenderPass* m_pRenderpass;
				std::vector<CreationBatchDesc> m_oPipelineDesc;
				std::vector<AbstractRenderBatch*> m_oBatches;
		};
	}
}

#endif
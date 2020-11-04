#ifndef H_RENDER_BATCH
#define H_RENDER_BATCH
#include "Mesh.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include <unordered_map>
#include "DescriptorWrapper.h"

class RenderBatch
{
public:
	struct Desc
	{
		RenderPass* pRenderpass;
		Pipeline* pPipeline;
		CommandFactory* pFactory;
		GraphicWrapper* pWrapper;
		RenderBatch* pNext;
	};
	RenderBatch(Desc& oDesc);
	~RenderBatch();

	VkCommandBuffer* GetDrawCommand(Framebuffer* pFramebuffer);
	inline void MarkAsDirty() { m_bDirty = true; m_oCachedCommandBuffer.clear(); }
	void AddMesh(Mesh::StrongPtr xMesh, DescriptorSetWrapper* pWrapper);
	inline DescriptorSetWrapper* GetDescriptor(Mesh::StrongPtr xMesh) { if (m_oEntities.count(xMesh) == 0) return nullptr; return m_oEntities[xMesh]; }
	inline bool IsEnabled() { return m_bEnabled; }
	inline void SetEnabled(bool bValue) { m_bEnabled = bValue; MarkAsDirty(); }
	size_t GetVerticesCount();
	size_t GetInstancesCount();
	Pipeline* GetPipeline() { return m_pPipeline; }
	void SetPipeline(Pipeline* pNew) { m_pPipeline = pNew; }
	void SetNext(RenderBatch* pBatch) { m_pNext = pBatch; }
	void ClearCache();

protected :
	bool m_bEnabled;
	bool m_bDirty;
	std::unordered_map<Framebuffer*, VkCommandBuffer*> m_oCachedCommandBuffer;

	void ReconstructCommand(Framebuffer* pFramebuffer);
	void ChainSubpass(VkCommandBuffer* pBuffer);
	GraphicWrapper* m_pWrapper;
	CommandFactory* m_pFactory;
	RenderPass* m_pRenderpass;
	Pipeline* m_pPipeline;
	RenderBatch* m_pNext;

	std::unordered_map<Mesh::StrongPtr, DescriptorSetWrapper*> m_oEntities;
};

class RenderBatchesHandler
{
public:
	struct CreationBatchDesc
	{
		CreationBatchDesc() : bWriteDepth(false), bTestDepth(false) {};
		std::vector<std::string> oShaderCompiled;
		bool bWriteDepth{};
		bool bTestDepth{};
		std::vector<std::string> oShaderSources;
	};

	struct Desc
	{
		Desc() : pPass(nullptr), pWrapper(nullptr), eSamples(VK_SAMPLE_COUNT_1_BIT), pFactory(nullptr) {};
		std::vector<CreationBatchDesc> oBatches{};
		RenderPass* pPass{};
		GraphicWrapper* pWrapper{};
		VkSampleCountFlagBits eSamples{};
		CommandFactory* pFactory{};
	};
	RenderBatchesHandler(Desc& oDesc);
	RenderBatch* GetRenderBatch(int iIndex) { if (iIndex < 0 || iIndex >= m_oBatches.size()) return nullptr; return m_oBatches[iIndex]; }
	Pipeline* GetPipeline(int iIndex) { if (iIndex < 0 || iIndex >= m_oBatches.size()) return nullptr; return m_oBatches[iIndex]->GetPipeline(); }
	void AddMesh(Mesh::StrongPtr xMesh, int iIndex, DescriptorPool* pPool);
	VkCommandBuffer* GetCommand(Framebuffer* pFramebuffer);
	void ReconstructPipelines(GraphicWrapper* pWrapper);
	void MarkAllAsDirty() { for (RenderBatch* pBatch : m_oBatches) { pBatch->MarkAsDirty(); } }

	size_t GetVerticesCount()
	{
		size_t iVertices = 0;
		for (RenderBatch* pBatch : m_oBatches)
		{
			iVertices += pBatch->GetVerticesCount();
		}
		return iVertices;
	}

	size_t GetInstancesCount()
	{
		uint64_t iInstanceCount = 0;
		for (RenderBatch* pBatch : m_oBatches)
		{
			iInstanceCount += pBatch->GetInstancesCount();
		}
		return iInstanceCount;
	}

protected:
	VkSampleCountFlagBits m_eSamples;
	RenderPass* m_pRenderpass;
	std::vector<CreationBatchDesc> m_oPipelineDesc;
	std::vector<RenderBatch*> m_oBatches;
	GraphicDevice* m_pDevice;
};

#endif
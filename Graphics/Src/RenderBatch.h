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
	inline void MarkAsDirty() { m_bDirty = true; }
	void AddMesh(Mesh* pMesh, DescriptorSetWrapper* pWrapper);
	inline DescriptorSetWrapper* GetDescriptor(Mesh* pMesh) { if (m_oEntities.count(pMesh) == 0) return nullptr; return m_oEntities[pMesh]; }
	uint64_t GetVerticesCount();
	uint64_t GetInstancesCount();
	void SetNext(RenderBatch* pBatch) { m_pNext = pBatch; }

protected :
	bool m_bDirty;
	std::unordered_map<Framebuffer*, VkCommandBuffer*> m_oCachedCommandBuffer;

	void ReconstructCommand(Framebuffer* pFramebuffer);
	void ChainSubpass(VkCommandBuffer* pBuffer);
	GraphicWrapper* m_pWrapper;
	CommandFactory* m_pFactory;
	RenderPass* m_pRenderpass;
	Pipeline* m_pPipeline;
	RenderBatch* m_pNext;

	std::unordered_map<Mesh*, DescriptorSetWrapper*> m_oEntities;
};

class RenderBatchesHandler
{
public:
	struct CreationBatchDesc
	{
		std::vector<std::string> oShaderCompiled;
		bool bWriteDepth;
		bool bTestDepth;
		std::vector<std::string> oShaderSources;
	};

	struct Desc
	{
		std::vector<CreationBatchDesc> oBatches;
		RenderPass* m_pPass;
		GraphicWrapper* pWrapper;
		VkSampleCountFlagBits eSamples;
		CommandFactory* pFactory;
	};
	RenderBatchesHandler(Desc& oDesc);
	VkCommandBuffer* GetCommand(Framebuffer* pFramebuffer) { return m_oBatches[0]->GetDrawCommand(pFramebuffer); }

protected:
	std::vector<Pipeline*> m_oPipelines;
	std::vector<RenderBatch*> m_oBatches;
};

#endif
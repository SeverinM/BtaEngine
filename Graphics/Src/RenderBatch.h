#ifndef H_RENDER_BATCH
#define H_RENDER_BATCH
#include "Model.h"
#include "Pipeline.h"
#include "RenderPass.h"

class RenderBatch
{
public:
	struct Desc
	{
		RenderPass* pRenderpass;
		Pipeline* pPipeline;
		CommandFactory* pFactory;
		uint32_t eFlags;
		GraphicWrapper* pWrapper;
		RenderBatch* pNext;
	};
	RenderBatch(Desc& oDesc);
	VkCommandBuffer* GetDrawCommand(Framebuffer* pFramebuffer);
	inline void MarkAsDirty() { m_bDirty = true; }

protected :
	bool m_bDirty;
	VkCommandBuffer* m_pCachedCommandBuffer;
	void ReconstructCommand(Framebuffer* pFramebuffer);
	void ChainSubpass(VkCommandBuffer* pBuffer);
	std::vector<RenderModel*> m_oEntities;
	uint32_t m_eFlag;
	GraphicWrapper* m_pWrapper;
	CommandFactory* m_pFactory;
	RenderPass* m_pRenderpass;
	Pipeline* m_pPipeline;
	RenderBatch* m_pNext;
};

#endif
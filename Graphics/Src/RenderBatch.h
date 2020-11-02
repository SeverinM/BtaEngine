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
		uint32_t eFlags;
		GraphicWrapper* pWrapper;
		RenderBatch* pNext;
	};
	RenderBatch(Desc& oDesc);
	VkCommandBuffer* GetDrawCommand(Framebuffer* pFramebuffer);
	inline void MarkAsDirty() { m_bDirty = true; }
	void AddMesh(Mesh* pMesh, DescriptorSetWrapper* pWrapper);
	inline DescriptorSetWrapper* GetDescriptor(Mesh* pMesh) { if (m_oEntities.count(pMesh) == 0) return nullptr; return m_oEntities[pMesh]; }

protected :
	bool m_bDirty;
	std::unordered_map<Framebuffer*, VkCommandBuffer*> m_oCachedCommandBuffer;

	void ReconstructCommand(Framebuffer* pFramebuffer);
	void ChainSubpass(VkCommandBuffer* pBuffer);
	uint32_t m_eFlag;
	GraphicWrapper* m_pWrapper;
	CommandFactory* m_pFactory;
	RenderPass* m_pRenderpass;
	Pipeline* m_pPipeline;
	RenderBatch* m_pNext;

	std::unordered_map<Mesh*, DescriptorSetWrapper*> m_oEntities;
};

#endif
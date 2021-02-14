#ifndef H_ABSTRACT_BATCH
#define H_ABSTRACT_BATCH
#include <string>
#include "Pipeline.h"

namespace Bta
{
	namespace Graphic
	{
		class Framebuffer;

		class AbstractRenderBatch
		{
			friend class RenderBatchesHandler;

			public:

				enum TransformFilter
				{
					E_MODEL = 1 << 0,
					E_VIEW = 1 << 1,
					E_PROJECTION = 1 << 2
				};

				virtual VkCommandBuffer* GetDrawCommand(Framebuffer* pFramebuffer) = 0;

				inline bool IsEnabled() { return m_bEnabled; }
				inline void SetEnabled(bool bValue) { m_bEnabled = bValue; MarkAsDirty(); }
				virtual void MarkAsDirty() {};
				virtual size_t GetVerticesCount() = 0;
				virtual size_t GetInstancesCount() = 0;
				std::string GetTag() { return m_sTag; }
				Pipeline* GetPipeline() { return m_pPipeline; }
				void SetPipeline(Pipeline* pNew) { m_pPipeline = pNew; }
				void SetNext(AbstractRenderBatch* pBatch) { m_pNext = pBatch; }
				virtual void ChainSubpass(VkCommandBuffer* pCommand) = 0;
				virtual void Destroy() = 0;
				RenderBatchesHandler* GetParent() { return m_pParent; }

			protected:
				RenderBatchesHandler* m_pParent;
				AbstractRenderBatch* m_pNext;
				std::string m_sTag;
				bool m_bEnabled;
				Pipeline* m_pPipeline;
				RenderPass* m_pRenderpass;
			};
	}
}

#endif


#ifndef H_RENDER_BATCH
#define H_RENDER_BATCH

#include "vulkan/vulkan_core.h"
#include <vector>
#include "SubRenderBatch.h"
#include "RenderPass.h"
#include "Framebuffer.h"

namespace Bta
{
	namespace Graphic
	{
		class RenderBatch
		{
			public:
				struct Desc
				{
					VkSampleCountFlagBits iSampleCount;
					std::vector<VkFormat> oFramebufferLayout;
					bool bPresentable;
					std::vector<SubRenderBatch::Desc> oSubBatches;
				};
				VkSampleCountFlagBits GetSampleCount() { return VK_SAMPLE_COUNT_1_BIT; }
				RenderPass* GetRenderPass() { return m_pRenderpass; }
				RenderBatch(RenderBatch::Desc oDesc);
				VkCommandBuffer GetCommandBuffer(Framebuffer* pBuffer);
				std::vector<SubRenderBatch*> GetSubBatches() { return m_oSubBatches; }
				void Recreate();

			private:
				RenderPass* m_pRenderpass;
				std::vector<SubRenderBatch*> m_oSubBatches;
		};
	}
}

#endif

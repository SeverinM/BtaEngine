#ifndef H_FRAMEBUFFER
#define H_FRAMEBUFFER
#include "vulkan/vulkan_core.h"
#include <vector>

namespace Bta
{
	namespace Graphic
	{
		class RenderPass;
		class GraphicDevice;

		class Framebuffer
		{
			public:
				enum EAttachmentFlag
				{
					E_COLOR = 1,
					E_DEPTH = 1 << 1,
					E_MULTISAMPLING = 1 << 2
				};

				struct Desc
				{
					std::vector<VkImageView>* pImageView;
					RenderPass* pRenderPass;
				};

				Framebuffer(Desc& oDesc);
				~Framebuffer();

				const VkFramebuffer* GetFramebuffer() { return &m_oFramebuffer; }
				int GetAttachmentFlags() { return m_iAttachments; }

			protected:
				int m_iAttachments;
				VkFramebuffer m_oFramebuffer;
				std::vector<VkImageView>* m_oImageViews;
		};
	}
}


#endif


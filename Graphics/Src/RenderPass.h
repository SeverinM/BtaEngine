#ifndef H_RENDER_PASS
#define H_RENDER_PASS
#include "vulkan/vulkan_core.h"
#include <vector>
#include "BasicWrapper.h"

namespace Bta
{
	namespace Graphic
	{
		class RenderPass
		{
		public:

			struct SubDesc
			{
				SubDesc() : pDependency(nullptr), iDepthStencilAttachmentIndex(-1), iColorResolveAttachmentIndex(-1), iColorAttachmentIndex(-1) {}
				int iColorAttachmentIndex{};
				int iDepthStencilAttachmentIndex{};
				int iColorResolveAttachmentIndex{};
				VkSubpassDependency* pDependency;
			};

			struct Desc
			{
				bool bEnableColor;
				bool bEnableDepth;
				std::vector<SubDesc> oSubpasses;
				VkSampleCountFlagBits eSample;
				VkImageLayout eInitialLayoutColorAttachment;
				bool bClearColorAttachmentAtBegin;
				bool bPresentable;
				VkFormat eFormatColor;
			};

			const VkRenderPass* const GetRenderPass() { return &m_oRenderpass; }
			RenderPass(Desc& oDesc);
			void Create(Desc& oDesc);
			~RenderPass();

		protected:
			Desc* m_pRecreate;
			VkRenderPass m_oRenderpass;
		};
	}
}

#endif


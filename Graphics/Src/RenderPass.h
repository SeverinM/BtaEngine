#ifndef H_RENDER_PASS
#define H_RENDER_PASS
#include "vulkan/vulkan_core.h"
#include "Resizable.h"
#include <vector>
#include "BasicWrapper.h"

class RenderPass : public Resizable
{
public :
	virtual void Free() override;
	virtual void Recreate(int iNewWidth, int iNewHeight, void* pData) override;

	struct SubDesc
	{
		int iColorAttachmentIndex;
		int iDepthStencilAttachmentIndex;
		int iColorResolveAttachmentIndex;
	};

	struct Desc
	{
		bool bEnableColor;
		bool bEnableDepth;
		std::vector<SubDesc> oSubpasses;
		BasicWrapper* pWrapper;
		VkSampleCountFlagBits eSample;
	};

	const VkRenderPass* const GetRenderPass() { return &m_oRenderpass; }
	RenderPass(Desc& oDesc);
	void Create(Desc& oDesc);
	~RenderPass();

protected:
	Desc* m_pRecreate;
	VkRenderPass m_oRenderpass;
	GraphicWrapper* m_pWrapper;
};

#endif


#ifndef H_IM_GUI_WRAPPER
#define H_IM_GUI_WRAPPER
#include "BasicWrapper.h"

class ImGuiWrapper
{
public:
	struct Desc
	{
		BasicWrapper* pWrapper;
		int iImageIndex;
	};
	ImGuiWrapper(Desc& oDesc);
	VkCommandBuffer* GetDrawCommand(Desc& oDesc);
	void Recreate(Desc& oDesc, int iMinImage);
	static void CheckError(VkResult eResult);

protected:
	RenderPass* m_pRenderpass;
	CommandFactory* m_pFactory;
	std::vector<Framebuffer*> m_pFramebuffer;
	std::vector<VkCommandBuffer> m_oCommandBuffer;
};

#endif
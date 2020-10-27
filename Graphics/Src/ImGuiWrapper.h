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
	~ImGuiWrapper();
	VkCommandBuffer* GetDrawCommand(Desc& oDesc);
	void Recreate(Desc& oDesc, int iMinImage);
	static void CheckError(VkResult eResult);
	inline CommandFactory* GetFactory() { return m_pFactory; }

protected:
	RenderPass* m_pRenderpass;
	CommandFactory* m_pFactory;
	BasicWrapper* m_pWrapper;
	std::vector<Framebuffer*> m_oFramebuffer;
	std::vector<VkCommandBuffer> m_oCommandBuffer;
};

#endif
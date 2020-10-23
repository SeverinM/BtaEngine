#ifndef H_COMMAND_FACTORY
#define H_COMMAND_FACTORY
#include "GraphicWrapper.h"
#include "Framebuffer.h"
#include "Pipeline.h"

class Buffer;

class CommandFactory
{
public:
	struct Desc
	{
		GraphicWrapper* pWrapper;
	};
	CommandFactory(Desc& oDesc);
	~CommandFactory();

	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer& oCommandBuffer);

	struct SubDrawDesc
	{
		Buffer* pVertexData;
		Buffer* pIndexData;
		Pipeline* pPipeline;
		VkDescriptorSet oDescriptorSet;
		int iInstanceCount;
	};

	struct DrawDesc
	{
		RenderPass* pRenderpass;
		Framebuffer* pFramebuffer;
		std::vector<SubDrawDesc> oMultipleDraw;
	};

	VkCommandBuffer CreateDrawCommand(DrawDesc& oDesc);
	const VkCommandPool* GetCommandPool() { return &m_oCommandPool; }

protected:
	VkCommandPool m_oCommandPool;
	GraphicDevice* m_pDevice;
};

#endif // !H_COMMAND_FACTORY

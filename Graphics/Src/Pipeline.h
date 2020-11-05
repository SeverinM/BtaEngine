#ifndef H_PIPELINE
#define H_PIPELINE
#include "vulkan/vulkan_core.h"
#include <vector>
#include <string>
#include "BasicWrapper.h"

static VkShaderModule* CompileShader(std::string sFilename, GraphicDevice& oDevice);

class Pipeline
{
public:
	struct Desc
	{
		Desc() : pViewportOverride(nullptr), pScissorOverride(nullptr), eDepthTestMethod(VK_COMPARE_OP_LESS), eCulling(VK_CULL_MODE_NONE), eFillMode(VK_POLYGON_MODE_FILL), eVerticesAssemblyMode(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST), fLineWidth(1.0f) {};

		DescriptorLayoutWrapper* pInputDatas;
		std::vector<std::string> oShaderFilenames;
		GraphicWrapper* pWrapper;
		VkVertexInputBindingDescription oBindingDescription;
		std::vector<VkVertexInputAttributeDescription> oAttributeDescriptions;
		VkSampleCountFlagBits eSample;
		bool bEnableTransparent;
		bool bWriteDepth;
		bool bTestDepth;
		RenderPass* pRenderPass;
		int iSubPassIndex;
		bool bRestartPrimitives;
		VkPrimitiveTopology eVerticesAssemblyMode;
		VkViewport* pViewportOverride;
		VkRect2D* pScissorOverride;
		VkPolygonMode eFillMode;
		float fLineWidth;
		VkCullModeFlags eCulling;
		VkCompareOp eDepthTestMethod;
	};
	Pipeline(Desc& oDesc);
	void Create(Desc& oDesc);
	~Pipeline();

	void CreatePipelineLayout(Desc& oDesc);

	VkPipelineLayout* GetPipelineLayout() { return &m_oPipelineLayout; }
	DescriptorLayoutWrapper* GetDescriptorSetLayout() { return m_pDescriptorLayout; }
	VkPipeline* GetPipeline() { return &m_oPipeline; }

	static void FillVerticesDescription(VkVertexInputBindingDescription& oBindingDescription, std::vector<VkVertexInputAttributeDescription>& oAttributeDescription, std::string sFilename);

private:
	DescriptorLayoutWrapper* m_pDescriptorLayout;
	VkPipelineLayout m_oPipelineLayout;
	VkPipeline m_oPipeline;
	Desc* m_pRecreate;
};

#endif
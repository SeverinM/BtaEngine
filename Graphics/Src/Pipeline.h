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
		DescriptorLayoutWrapper* pInputDatas;
		std::vector<std::string> oShaderFilenames;
		GraphicWrapper* pWrapper;
		VkVertexInputBindingDescription oBindingDescription;
		std::vector<VkVertexInputAttributeDescription> oAttributeDescriptions;
		VkSampleCountFlagBits eSample;
		bool bEnableTransparent;
		bool bEnableDepth;
		RenderPass* pRenderPass;
		int iSubPassIndex;
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
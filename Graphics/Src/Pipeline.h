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
		std::vector< std::vector< DescriptorPool::BufferDesc> >oInputDatas;
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

	void CreateDescriptorLayout(Desc& oDesc);
	void CreatePipelineLayout(Desc& oDesc);

	VkPipelineLayout* GetPipelineLayout() { return &m_oPipelineLayout; }
	const std::vector< VkDescriptorSetLayout >& GetDescriptorSetLayout() { return m_oDescriptorSetLayout; }
	VkPipeline* GetPipeline() { return &m_oPipeline; }

private:
	std::vector< VkDescriptorSetLayout > m_oDescriptorSetLayout;
	VkPipelineLayout m_oPipelineLayout;
	VkPipeline m_oPipeline;
	GraphicWrapper* m_pWrapper;
	Desc* m_pRecreate;
};

#endif
#ifndef H_PIPELINE
#define H_PIPELINE
#include "vulkan/vulkan_core.h"
#include <vector>
#include <string>
#include "BasicWrapper.h"
#include "Resizable.h"

static VkShaderModule* CompileShader(std::string sFilename, GraphicDevice& oDevice);

class Pipeline : public Resizable
{
public:
	struct Desc
	{
		std::vector< DescriptorPool::BufferDesc> oInputDatas;
		std::vector<std::string> oShaderFilenames;
		GraphicWrapper* pWrapper;
		VkVertexInputBindingDescription oBindingDescription;
		std::vector<VkVertexInputAttributeDescription> oAttributeDescriptions;
		VkSampleCountFlagBits eSample;
		bool bEnableTransparent;
		bool bEnableDepth;
		RenderPass* pRenderPass;
	};
	Pipeline(Desc& oDesc);
	void Create(Desc& oDesc);
	~Pipeline();

	void CreateDescriptorLayout(Desc& oDesc);
	void CreatePipelineLayout(Desc& oDesc);

	virtual void Free() override;
	virtual void Recreate(int iNewWidth, int iNewHeight, void* pData) override;

	VkPipelineLayout* GetPipelineLayout() { return &m_oPipelineLayout; }
	VkDescriptorSetLayout* GetDescriptorSetLayout() { return &m_oDescriptorSetLayout; }
	VkPipeline* GetPipeline() { return &m_oPipeline; }

private:
	VkDescriptorSetLayout m_oDescriptorSetLayout;
	VkPipelineLayout m_oPipelineLayout;
	VkPipeline m_oPipeline;
	GraphicWrapper* m_pWrapper;
	Desc* m_pRecreate;
};

#endif
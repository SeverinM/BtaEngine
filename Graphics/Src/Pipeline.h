#ifndef H_PIPELINE
#define H_PIPELINE
#include "vulkan/vulkan_core.h"
#include <vector>
#include <string>
#include "DescriptorWrapper.h"
#include "RenderPass.h"

namespace Bta
{
	namespace Graphic
	{
		class Pipeline
		{
			public:
				struct Desc
				{
					Desc() : pViewportOverride(nullptr), pScissorOverride(nullptr), eDepthTestMethod(VK_COMPARE_OP_LESS), eCulling(VK_CULL_MODE_NONE), eFillMode(VK_POLYGON_MODE_FILL), eVerticesAssemblyMode(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST), fLineWidth(1.0f) {};
					DescriptorLayoutWrapper* pInputDatas;
					std::vector<std::string> oShaderFilenames;
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
				void Recreate();

				//Temp
				VkPipeline m_oPipeline;

				static void FillVerticesDescription(VkVertexInputBindingDescription& oBindingDescription, std::vector<VkVertexInputAttributeDescription>& oAttributeDescription, std::string sFilename);

			private:
				VkShaderModule* CompileShader(std::string sFilename, GraphicDevice& oDevice);
				DescriptorLayoutWrapper* m_pDescriptorLayout;
				VkPipelineLayout m_oPipelineLayout;
				Desc* m_pRecreate;
		};
	}
}

#endif
#include "Pipeline.h"
#include <fstream>
#include <iostream>
#include "RenderPass.h"

VkShaderModule* CompileShader(std::string sFilename, const GraphicDevice& oDevice)
{
	std::ifstream oFile(sFilename, std::ios::ate | std::ios::binary);

	if (!oFile.is_open())
		return nullptr;

	size_t iFileSize = (size_t)oFile.tellg();
	std::vector<char>oBuffer(iFileSize);

	oFile.seekg(0);
	oFile.read(oBuffer.data(), iFileSize);

	VkShaderModuleCreateInfo oCreateInfo{};
	oCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	oCreateInfo.codeSize = oBuffer.size();
	oCreateInfo.pCode = reinterpret_cast<const uint32_t*>(oBuffer.data());

	VkShaderModule* pOutput = new VkShaderModule();
	if (vkCreateShaderModule(*oDevice.GetLogicalDevice(), &oCreateInfo, nullptr, pOutput) != VK_SUCCESS)
	{
		delete(pOutput);
		return nullptr;
	}
	return pOutput;
}

Pipeline::Pipeline(Desc& oDesc)
{
	m_pRecreate = new Desc(oDesc);
	Create(oDesc);
}

void Pipeline::Create(Desc& oDesc)
{
	CreateDescriptorLayout(oDesc);
	CreatePipelineLayout(oDesc);

	VkPipelineShaderStageCreateInfo oVertexInfos{};
	oVertexInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	oVertexInfos.stage = VK_SHADER_STAGE_VERTEX_BIT;
	oVertexInfos.pName = "main";

	VkShaderModule* pModule = CompileShader(oDesc.oShaderFilenames[0], *oDesc.pWrapper->GetDevice());
	if (pModule == nullptr)
		throw std::runtime_error("Could not compile shader");

	oVertexInfos.module = *pModule;

	VkPipelineShaderStageCreateInfo oFragmentInfos{};
	oFragmentInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	oFragmentInfos.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	oFragmentInfos.pName = "main";

	VkShaderModule* pModuleFrag = CompileShader(oDesc.oShaderFilenames[1], *oDesc.pWrapper->GetDevice());
	if (pModuleFrag == nullptr)
		throw std::runtime_error("Could not compile shader");

	oFragmentInfos.module = *pModuleFrag;

	VkPipelineShaderStageCreateInfo oShaderStages[] = { oVertexInfos, oFragmentInfos };

	VkPipelineVertexInputStateCreateInfo oVertexInputInfo{};
	oVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	oVertexInputInfo.vertexBindingDescriptionCount = 1;
	oVertexInputInfo.pVertexBindingDescriptions = &oDesc.oBindingDescription;
	oVertexInputInfo.vertexAttributeDescriptionCount = oDesc.oAttributeDescriptions.size();
	oVertexInputInfo.pVertexAttributeDescriptions = oDesc.oAttributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo oInputVertex{};
	oInputVertex.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	oInputVertex.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	oInputVertex.primitiveRestartEnable = VK_FALSE;

	int iWidth, iHeight;
	oDesc.pWrapper->GetModifiableDevice()->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);
	VkViewport oViewport{};
	oViewport.x = 0.0f;
	oViewport.y = 0.0f;
	oViewport.width = iWidth;
	oViewport.height = iHeight;
	oViewport.minDepth = 0.0f;
	oViewport.maxDepth = 1.0f;

	VkExtent2D oExtent;
	oExtent.width = iWidth;
	oExtent.height = iHeight;

	VkRect2D oScissor{};
	oScissor.offset = { 0, 0 };
	oScissor.extent = oExtent;

	VkPipelineViewportStateCreateInfo oViewportState{};
	oViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	oViewportState.viewportCount = 1;
	oViewportState.pViewports = &oViewport;
	oViewportState.scissorCount = 1;
	oViewportState.pScissors = &oScissor;

	VkPipelineRasterizationStateCreateInfo oRasterize{};
	oRasterize.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	oRasterize.depthClampEnable = VK_FALSE;
	oRasterize.rasterizerDiscardEnable = VK_FALSE;
	oRasterize.polygonMode = VK_POLYGON_MODE_FILL;
	oRasterize.lineWidth = 1.0f;
	oRasterize.cullMode = VK_CULL_MODE_NONE;
	oRasterize.frontFace = VK_FRONT_FACE_CLOCKWISE;
	oRasterize.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo oMultisampling{};
	oMultisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	oMultisampling.sampleShadingEnable = (oDesc.eSample == VK_SAMPLE_COUNT_1_BIT ? VK_FALSE : VK_TRUE);
	oMultisampling.minSampleShading = .2f;
	oMultisampling.rasterizationSamples = oDesc.eSample;

	VkPipelineColorBlendAttachmentState oColorBlendAttachment{};
	oColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	oColorBlendAttachment.blendEnable = (oDesc.bEnableTransparent ? VK_TRUE : VK_FALSE);

	VkPipelineColorBlendStateCreateInfo oColorBlending{};
	oColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	oColorBlending.logicOpEnable = (oDesc.bEnableTransparent ? VK_TRUE : VK_FALSE);
	oColorBlending.attachmentCount = 1;
	oColorBlending.pAttachments = &oColorBlendAttachment;

	VkDynamicState oDynamicStates[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = oDynamicStates;

	VkPipelineDepthStencilStateCreateInfo oDepthStencil{};
	oDepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	oDepthStencil.depthTestEnable = oDesc.bEnableDepth ? VK_TRUE : VK_FALSE;
	oDepthStencil.depthWriteEnable = oDesc.bEnableDepth ? VK_TRUE : VK_FALSE;
	oDepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	oDepthStencil.depthBoundsTestEnable = VK_FALSE;
	oDepthStencil.minDepthBounds = 0.0f;
	oDepthStencil.maxDepthBounds = 1.0f;
	oDepthStencil.stencilTestEnable = VK_FALSE;
	oDepthStencil.minDepthBounds = 0.0f;
	oDepthStencil.maxDepthBounds = 1.0f;

	VkGraphicsPipelineCreateInfo oPipelineInfo{};
	oPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	oPipelineInfo.stageCount = 2;
	oPipelineInfo.pStages = oShaderStages;

	oPipelineInfo.pVertexInputState = &oVertexInputInfo;
	oPipelineInfo.pInputAssemblyState = &oInputVertex;
	oPipelineInfo.pViewportState = &oViewportState;
	oPipelineInfo.pRasterizationState = &oRasterize;
	oPipelineInfo.pMultisampleState = &oMultisampling;
	oPipelineInfo.pDepthStencilState = &oDepthStencil;
	oPipelineInfo.pColorBlendState = &oColorBlending;
	oPipelineInfo.layout = m_oPipelineLayout;
	oPipelineInfo.renderPass = *oDesc.pRenderPass->GetRenderPass();
	oPipelineInfo.subpass = oDesc.iSubPassIndex;

	if (vkCreateGraphicsPipelines(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), VK_NULL_HANDLE, 1, &oPipelineInfo, nullptr, &m_oPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphic pipeline");
	}
}

Pipeline::~Pipeline()
{
	vkDestroyPipeline(*m_pWrapper->GetDevice()->GetLogicalDevice(), m_oPipeline, nullptr);
	vkDestroyPipelineLayout(*m_pWrapper->GetDevice()->GetLogicalDevice(), m_oPipelineLayout, nullptr);
}

void Pipeline::CreateDescriptorLayout(Desc& oDesc)
{
	m_oDescriptorSetLayout.resize(oDesc.oInputDatas.size());

	m_pWrapper = oDesc.pWrapper;

	for (int iSetLayoutIndex = 0; iSetLayoutIndex < oDesc.oInputDatas.size(); iSetLayoutIndex++)
	{
		std::vector<VkDescriptorSetLayoutBinding> oLayoutBindings;
		for (int i = 0; i < oDesc.oInputDatas[iSetLayoutIndex].size(); i++)
		{
			VkDescriptorSetLayoutBinding oBinding{};
			oBinding.binding = i;
			oBinding.descriptorCount = 1;
			oBinding.descriptorType = DescriptorPool::GetDescriptorType(oDesc.oInputDatas[iSetLayoutIndex][i].eType);
			oBinding.pImmutableSamplers = nullptr;
			oBinding.stageFlags = (oDesc.oInputDatas[iSetLayoutIndex][i].eType == DescriptorPool::E_TEXTURE ? VK_SHADER_STAGE_FRAGMENT_BIT : VK_SHADER_STAGE_VERTEX_BIT);

			oLayoutBindings.push_back(oBinding);
		}

		VkDescriptorSetLayoutCreateInfo oLayoutCreateInfo{};
		oLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		oLayoutCreateInfo.bindingCount = oLayoutBindings.size();
		oLayoutCreateInfo.pBindings = oLayoutBindings.data();

		if (vkCreateDescriptorSetLayout(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), &oLayoutCreateInfo, nullptr, &m_oDescriptorSetLayout[iSetLayoutIndex]) != VK_SUCCESS)
		{
			throw std::runtime_error("Error creating set layout");
		}
	}
}

void Pipeline::CreatePipelineLayout(Desc& oDesc)
{
	VkPipelineLayoutCreateInfo oPipelineLayoutInfo{};
	oPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	oPipelineLayoutInfo.setLayoutCount = m_oDescriptorSetLayout.size();
	oPipelineLayoutInfo.pSetLayouts = m_oDescriptorSetLayout.data();

	if (vkCreatePipelineLayout(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), &oPipelineLayoutInfo, nullptr, &m_oPipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Cannot create layout");
	}
}

void Pipeline::Free()
{
	vkDestroyPipeline(*m_pWrapper->GetDevice()->GetLogicalDevice(), m_oPipeline, nullptr);
}

void Pipeline::Recreate(int iNewWidth, int iNewHeight, void* pData)
{
	Create(*m_pRecreate);
}

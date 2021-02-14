#include "Pipeline.h"
#include <fstream>
#include <iostream>
#include "RenderPass.h"
#include "StringUtils.h"
#include <fstream>
#include "Globals.h"

namespace Bta
{
	namespace Graphic
	{
		VkShaderModule* Pipeline::CompileShader(std::string sFilename, GraphicDevice& oDevice)
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
			m_pDescriptorLayout = oDesc.pInputDatas;
			CreatePipelineLayout(oDesc);

			VkPipelineShaderStageCreateInfo oVertexInfos{};
			oVertexInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			oVertexInfos.stage = VK_SHADER_STAGE_VERTEX_BIT;
			oVertexInfos.pName = "main";

			VkShaderModule* pModule = CompileShader(oDesc.oShaderFilenames[0], *Bta::Graphic::Globals::g_pDevice);
			if (pModule == nullptr)
				throw std::runtime_error("Could not compile shader");

			oVertexInfos.module = *pModule;

			VkPipelineShaderStageCreateInfo oFragmentInfos{};
			oFragmentInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			oFragmentInfos.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			oFragmentInfos.pName = "main";

			VkShaderModule* pModuleFrag = CompileShader(oDesc.oShaderFilenames[1], *Bta::Graphic::Globals::g_pDevice);
			if (pModuleFrag == nullptr)
				throw std::runtime_error("Could not compile shader");

			oFragmentInfos.module = *pModuleFrag;

			VkPipelineShaderStageCreateInfo oShaderStages[] = { oVertexInfos, oFragmentInfos };

			VkPipelineVertexInputStateCreateInfo oVertexInputInfo{};
			oVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			oVertexInputInfo.vertexBindingDescriptionCount = 1;
			oVertexInputInfo.pVertexBindingDescriptions = &oDesc.oBindingDescription;
			oVertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)oDesc.oAttributeDescriptions.size();
			oVertexInputInfo.pVertexAttributeDescriptions = oDesc.oAttributeDescriptions.data();

			VkPipelineInputAssemblyStateCreateInfo oInputVertex{};
			oInputVertex.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			oInputVertex.topology = oDesc.eVerticesAssemblyMode;
			oInputVertex.primitiveRestartEnable = VK_FALSE;

			int iWidth, iHeight;
			Bta::Graphic::Globals::g_pDevice->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);
			VkViewport oViewport{};
			oViewport.x = 0.0f;
			oViewport.y = 0.0f;
			oViewport.width = (float)iWidth;
			oViewport.height = (float)iHeight;
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
			oViewportState.pViewports = (oDesc.pViewportOverride == nullptr ? &oViewport : oDesc.pViewportOverride);
			oViewportState.scissorCount = 1;
			oViewportState.pScissors = (oDesc.pScissorOverride == nullptr ? &oScissor : oDesc.pScissorOverride);

			VkPipelineRasterizationStateCreateInfo oRasterize{};
			oRasterize.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			oRasterize.depthClampEnable = VK_FALSE;
			oRasterize.rasterizerDiscardEnable = VK_FALSE;
			oRasterize.polygonMode = oDesc.eFillMode;
			oRasterize.lineWidth = oDesc.fLineWidth;
			oRasterize.cullMode = oDesc.eCulling;
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
			oColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			oColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			oColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			oColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			oColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			oColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

			VkPipelineColorBlendStateCreateInfo oColorBlending{};
			oColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			oColorBlending.logicOpEnable = VK_FALSE;
			oColorBlending.logicOp = VK_LOGIC_OP_COPY;
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
			oDepthStencil.depthTestEnable = oDesc.bTestDepth ? VK_TRUE : VK_FALSE;
			oDepthStencil.depthWriteEnable = oDesc.bWriteDepth ? VK_TRUE : VK_FALSE;
			oDepthStencil.depthCompareOp = oDesc.eDepthTestMethod;
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

			if (vkCreateGraphicsPipelines(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), VK_NULL_HANDLE, 1, &oPipelineInfo, nullptr, &m_oPipeline) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create graphic pipeline");
			}
		}

		Pipeline::~Pipeline()
		{
			vkDestroyPipeline(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), m_oPipeline, nullptr);
			vkDestroyPipelineLayout(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), m_oPipelineLayout, nullptr);
		}

		void Pipeline::CreatePipelineLayout(Desc& oDesc)
		{
			VkPipelineLayoutCreateInfo oPipelineLayoutInfo{};
			oPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			oPipelineLayoutInfo.setLayoutCount = 1;
			oPipelineLayoutInfo.pSetLayouts = m_pDescriptorLayout->GetLayout();

			if (vkCreatePipelineLayout(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), &oPipelineLayoutInfo, nullptr, &m_oPipelineLayout) != VK_SUCCESS)
			{
				throw std::runtime_error("Cannot create layout");
			}
		}

		void Pipeline::FillVerticesDescription(VkVertexInputBindingDescription& oBindingDescription, std::vector<VkVertexInputAttributeDescription>& oAttributeDescription, std::string sFilename)
		{
			oAttributeDescription.clear();

			std::ifstream oReadFile;
			oReadFile.open(sFilename.c_str(), std::ios::in);

			if (!oReadFile.is_open())
			{
				throw std::runtime_error("Vertex shader file does not exist");
			}

			int iOffset = 0;
			std::string sLine;
			while (getline(oReadFile, sLine))
			{
				if (Bta::Utils::StringUtils::Contains(sLine, "main()"))
				{
					break;
				}

				if (Bta::Utils::StringUtils::Contains(sLine, "location") && Bta::Utils::StringUtils::StartWith(sLine, "layout") && Bta::Utils::StringUtils::Contains(sLine, " in "))
				{
					int iBindingIndex = std::stoi(Bta::Utils::StringUtils::Split(sLine, ' ')[4]);
					if (iBindingIndex != oAttributeDescription.size())
					{
						throw std::runtime_error("location index must be in ascending order");
					}

					VkVertexInputAttributeDescription oAttribute{};
					oAttribute.binding = 0;
					oAttribute.location = iBindingIndex;
					oAttribute.offset = iOffset;

					size_t iSize = Bta::Utils::StringUtils::ParseMemorySize(Bta::Utils::StringUtils::Split(sLine, ' ')[7]);

					if (iSize == sizeof(glm::vec3))
					{
						oAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
					}
					else if (iSize == sizeof(glm::vec2))
					{
						oAttribute.format = VK_FORMAT_R32G32_SFLOAT;
					}
					else if (iSize == sizeof(glm::vec4))
					{
						oAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
					}
					iOffset += (int)iSize;
					oAttributeDescription.push_back(oAttribute);
				}
			}

			oBindingDescription.binding = 0;
			oBindingDescription.stride = iOffset;
			oBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		}
	}
}

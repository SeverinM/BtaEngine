#include "SubRenderBatch.h"
#include "RenderBatch.h"
#include "Globals.h"
#include <numeric>
#include "ShaderTags.h"
#include "Pipeline.h"
#include "MaterialComponent.h"
#include "MeshComponent.h"
#include <numeric>
#include "../../Core/Src/TransformComponent.h"
#include "CameraComponent.h"
//#include "GLM/glm.hpp"

namespace Bta
{
	namespace Graphic
	{
		SubRenderBatch::SubRenderBatch(SubRenderBatch::Desc oDesc)
		{
			m_oDesc = oDesc;
			SetCamera(oDesc.pCamera);
			m_bEnabled = true;
		}

		void SubRenderBatch::CreatePipeline(RenderBatch* m_pBatch, int iIndex)
		{
			Pipeline::Desc oDesc;
			oDesc.oShaderFilenames = m_oDesc.oCompiledShaderFiles;
			oDesc.bWriteDepth = m_oDesc.bWriteDepth;
			oDesc.bTestDepth = m_oDesc.bTestDepth;
			oDesc.eVerticesAssemblyMode = m_oDesc.eVerticesAssemblyMode;
			oDesc.eFillMode = m_oDesc.eFillMode;
			oDesc.eCulling = m_oDesc.eCulling;
			oDesc.eDepthTestMethod = m_oDesc.eDepthTestMethod;
			oDesc.pScissorOverride = nullptr;
			oDesc.pViewportOverride = nullptr;
			oDesc.eSample = m_pBatch->GetSampleCount();
			oDesc.pRenderPass = m_pBatch->GetRenderPass();
			oDesc.iSubPassIndex = iIndex;

			DescriptorLayoutWrapper::ShaderMap oMap;
			oMap[VK_SHADER_STAGE_VERTEX_BIT] = m_oDesc.oShaderFiles[0];
			oMap[VK_SHADER_STAGE_FRAGMENT_BIT] = m_oDesc.oShaderFiles[1];
			oDesc.pInputDatas = DescriptorLayoutWrapper::ParseShaderFiles(oMap);
			
			VkVertexInputBindingDescription oBindingDescription{};
			oBindingDescription.binding = 0;
			oBindingDescription.stride = sizeof(glm::vec2) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec4);
			oBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			std::vector<VkVertexInputAttributeDescription> oInputs;
			
			VkVertexInputAttributeDescription oInput{};
			oInput.binding = 0;
			oInput.location = 0;
			oInput.format = VK_FORMAT_R32G32B32_SFLOAT;
			oInput.offset = 0;
			oInputs.push_back(oInput);

			oInput.binding = 0;
			oInput.location = 1;
			oInput.format = VK_FORMAT_R32G32B32_SFLOAT;
			oInput.offset = sizeof(glm::vec3);
			oInputs.push_back(oInput);

			oInput.binding = 0;
			oInput.location = 2;
			oInput.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			oInput.offset = sizeof(glm::vec3) * 2;
			oInputs.push_back(oInput);

			oInput.binding = 0;
			oInput.location = 3;
			oInput.format = VK_FORMAT_R32G32_SFLOAT;
			oInput.offset = (sizeof(glm::vec3) * 2) + sizeof(glm::vec4);
			oInputs.push_back(oInput);

			oDesc.oBindingDescription = oBindingDescription;
			oDesc.oAttributeDescriptions = oInputs;

			m_pPipeline = new Pipeline(oDesc);
		}

		void SubRenderBatch::RefreshModels()
		{
			for (std::unordered_map<MeshComponent*, DescriptorSetWrapper*>::iterator it = m_oAllMeshes.begin(); it != m_oAllMeshes.end(); it++)
			{
				DescriptorSetWrapper::MemorySlot* pSlot = it->second->GetSlotWithTag(TAG_M);
				if (pSlot == nullptr)
					return;

				Buffer* pBuffer = pSlot->pData;

				if (pBuffer == nullptr)
					return;

				glm::mat4x4 mMat = it->first->GetOwner()->FindFirstComponent<Core::TransformComponent>()->GetModelMatrix();
				pBuffer->CopyFromMemory(&mMat);
			}
		}

		void SubRenderBatch::FillCommandBuffer(VkCommandBuffer& oCommandBuffer)
		{
			if (!m_bEnabled)
			{
				return;
			}

			vkCmdBindPipeline(oCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pPipeline->GetPipeline());
			std::unordered_map<MeshComponent*, DescriptorSetWrapper*>::iterator pMeshIt = m_oAllMeshes.begin();

			while (pMeshIt != m_oAllMeshes.end())
			{
				MeshComponent* pMesh = pMeshIt->first;
				DescriptorSetWrapper* pMat = pMeshIt->second;
				vkCmdBindDescriptorSets(oCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pPipeline->GetPipelineLayout(), 0, 1, pMat->GetDescriptorSet(), 0, nullptr);

				VkDeviceSize oOffsets[] = { 0 };
				BasicBuffer* pBasicBuffer = (BasicBuffer*)pMesh->GetVerticeBinding()->GetBuffer();
				vkCmdBindVertexBuffers(oCommandBuffer, 0, 1, pBasicBuffer->GetBuffer(), oOffsets);

				if (pMesh->GetIndexBinding() != nullptr)
				{
					BasicBuffer* pBasicIndiceBuffer = (BasicBuffer*)pMesh->GetIndexBinding()->GetBuffer();
					vkCmdBindIndexBuffer(oCommandBuffer, *pBasicIndiceBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
					vkCmdDrawIndexed(oCommandBuffer, pMesh->GetIndicesCount() , 1, 0, 0, 0);
				}
				else
				{
					vkCmdDraw(oCommandBuffer, pMesh->GetIndicesCount(), 1, 0, 0);
				}

				pMeshIt++;
			}
		}

		MaterialComponent* SubRenderBatch::AddMesh(MeshComponent* pComponent)
		{
			if (m_pCamera == nullptr)
			{
				std::cout << "You should add camera first" << std::endl;
				return nullptr;
			}
			
			DescriptorSetWrapper* pSetWrapper = m_pPipeline->GetDescriptorSetLayout()->InstantiateDescriptorSet(*Globals::g_pPool);
			Core::Entity* pOwner = pComponent->GetOwner();
			MaterialComponent* pMat = new MaterialComponent(pSetWrapper, pOwner);
			
			for (DescriptorSetWrapper::MemorySlot oSlot : pSetWrapper->GetSlots())
			{
				Buffer* pBuffer = nullptr;
				if (oSlot.eType == DescriptorPool::E_BINDING_TYPE::E_TEXTURE)
				{
					pMat->AddTexture(nullptr, oSlot.sTag);
				}
				else
				{	
					if (oSlot.sTag == TAG_V)
					{
						pMat->AddGPUMemory(*m_pCamera->GetMemoryBindingV() ,oSlot.sTag);
					}
					else if ( oSlot.sTag == TAG_P)
					{
						pMat->AddGPUMemory(*m_pCamera->GetMemoryBindingP(), oSlot.sTag);
					}
					else if (oSlot.sTag == TAG_VP)
					{
						m_pCamera->GetMemoryBindingP()->m_iElementSize *= 2;
						pMat->AddGPUMemory(*m_pCamera->GetMemoryBindingP(), oSlot.sTag);
					}
					else
					{
						int iSizeAll = oSlot.oElementsSize[0];
						GPUMemoryBinding oBinding( VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, iSizeAll);
						pMat->AddGPUMemory(oBinding, oSlot.sTag);
					}
				}
			}

			m_oAllMeshes[pComponent] = pSetWrapper;
			RefreshModels();
			return pMat;
		}

		void SubRenderBatch::SetCamera(CameraComponent* pCamera)
		{
			m_pCamera = pCamera;
		}

	}
}
#include "RenderBatch.h"
#include <iostream>
#include "CommandFactory.h"

RenderBatch::RenderBatch(Desc& oDesc)
{
	m_bEnabled = true;
	m_bDirty = false;
	m_pWrapper = oDesc.pWrapper;
	m_pFactory = oDesc.pFactory;
	m_pRenderpass = oDesc.pRenderpass;
	m_pPipeline = oDesc.pPipeline;
	m_pNext = oDesc.pNext;
}

RenderBatch::~RenderBatch()
{
	for (std::pair<Framebuffer*, VkCommandBuffer*> oCmds : m_oCachedCommandBuffer)
	{
		if ( oCmds.second != nullptr)
			vkFreeCommandBuffers(*m_pWrapper->GetDevice()->GetLogicalDevice(), *m_pFactory->GetCommandPool(), 1, oCmds.second);
	}

	for (std::pair<Mesh::StrongPtr, DescriptorSetWrapper*> oEntity : m_oEntities)
	{
		delete oEntity.second;
	}
}

VkCommandBuffer* RenderBatch::GetDrawCommand(Framebuffer* pFramebuffer)
{
	if (m_oEntities.empty())
	{
		return nullptr;
	}

	if (m_oCachedCommandBuffer.count(pFramebuffer) == 0)
	{
		m_oCachedCommandBuffer[pFramebuffer] = nullptr;
		m_bDirty = true;
	}

	if (m_bDirty)
	{
		if (m_oCachedCommandBuffer[pFramebuffer] != nullptr)
		{
			vkFreeCommandBuffers(*m_pWrapper->GetDevice()->GetLogicalDevice(), *m_pFactory->GetCommandPool(), 1, m_oCachedCommandBuffer[pFramebuffer]);
			delete m_oCachedCommandBuffer[pFramebuffer];
		}
		m_oCachedCommandBuffer[pFramebuffer] = m_pFactory->CreateCommand();

		ReconstructCommand(pFramebuffer);
		m_bDirty = false;
	}
	return m_oCachedCommandBuffer[pFramebuffer];
}

void RenderBatch::AddMesh(Mesh::StrongPtr xMesh, DescriptorSetWrapper* pWrapper)
{
	if (m_oEntities.find(xMesh) == m_oEntities.end())
	{
		m_oEntities[xMesh] = pWrapper;
		MarkAsDirty();
	}
}

size_t RenderBatch::GetVerticesCount()
{
	size_t iOutput = 0;
	for (std::pair<Mesh::StrongPtr, DescriptorSetWrapper*> oPair : m_oEntities)
	{
		iOutput += oPair.first->GetVerticeCount();
	}
	return iOutput;
}

uint64_t RenderBatch::GetInstancesCount()
{
	uint64_t iOutput = 0;
	for (std::pair<Mesh::StrongPtr, DescriptorSetWrapper*> oPair : m_oEntities)
	{
		iOutput += oPair.first->GetInstanceCount();
	}
	return iOutput;
}

void RenderBatch::ClearCache()
{
	for (std::pair<Framebuffer*, VkCommandBuffer*> oCmd : m_oCachedCommandBuffer)
	{
		vkFreeCommandBuffers(*m_pWrapper->GetModifiableDevice()->GetLogicalDevice(), *m_pFactory->GetCommandPool(), 1, oCmd.second);
	}
	m_oCachedCommandBuffer.clear();
}

void RenderBatch::ReconstructCommand(Framebuffer* pFramebuffer)
{
	VkCommandBufferBeginInfo oCommandBeginInfo{};
	oCommandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkCommandBuffer* pBuffer = m_oCachedCommandBuffer[pFramebuffer];
	if (vkBeginCommandBuffer(*m_oCachedCommandBuffer[pFramebuffer], &oCommandBeginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo oBeginInfo{};
	oBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	oBeginInfo.renderPass = *m_pRenderpass->GetRenderPass();
	oBeginInfo.framebuffer = *pFramebuffer->GetFramebuffer();

	int iWidth, iHeight;
	m_pWrapper->GetModifiableDevice()->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);

	VkExtent2D oExtent;
	oExtent.height = iHeight;
	oExtent.width = iWidth;

	oBeginInfo.renderArea.offset = { 0, 0 };
	oBeginInfo.renderArea.extent = oExtent;

	std::vector<VkClearValue> oClear(1);
	oClear[0].color = { 0.1f, 0.3f,0.8f, 1.0f };

	if ((Framebuffer::E_DEPTH & pFramebuffer->GetAttachmentFlags()) != 0)
	{
		VkClearValue oClearDepth;
		oClearDepth.depthStencil = { 1.0f, 0 };
		oClear.push_back(oClearDepth);
	}

	oBeginInfo.clearValueCount = static_cast<uint32_t>(oClear.size());
	oBeginInfo.pClearValues = oClear.data();

	vkCmdBeginRenderPass(*m_oCachedCommandBuffer[pFramebuffer], &oBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
	ChainSubpass(m_oCachedCommandBuffer[pFramebuffer]);

	vkCmdEndRenderPass(*m_oCachedCommandBuffer[pFramebuffer]);
	if (vkEndCommandBuffer(*m_oCachedCommandBuffer[pFramebuffer]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command");
	}

}

void RenderBatch::ChainSubpass(VkCommandBuffer* pBuffer)
{
	if (m_bEnabled)
	{
		vkCmdBindPipeline(*pBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pPipeline->GetPipeline());

		for (std::pair<Mesh::StrongPtr, DescriptorSetWrapper*> pEntity : m_oEntities)
		{
			vkCmdBindDescriptorSets(*pBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pPipeline->GetPipelineLayout(), 0, 1, pEntity.second->GetDescriptorSet(), 0, nullptr);

			VkDeviceSize oOffsets[] = { 0 };
			std::shared_ptr<BasicBuffer> xBasicBuffer = std::static_pointer_cast<BasicBuffer>(pEntity.first->GetVerticesBuffer());
			vkCmdBindVertexBuffers(*pBuffer, 0, 1, xBasicBuffer->GetBuffer(), oOffsets);

			if (pEntity.first->GetIndexesBuffer() != nullptr)
			{
				std::shared_ptr<BasicBuffer> xBasicBufferIndex = std::static_pointer_cast<BasicBuffer>(pEntity.first->GetIndexesBuffer());
				vkCmdBindIndexBuffer(*pBuffer, *xBasicBufferIndex->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

				size_t iInstanceCount = pEntity.first->GetInstanceCount();
				vkCmdDrawIndexed(*pBuffer, (uint32_t)pEntity.first->GetIndexesBuffer()->GetUnitCount(), iInstanceCount, 0, 0, 0);
			}
			else
			{
				vkCmdDraw(*pBuffer, (uint32_t)pEntity.first->GetVerticesBuffer()->GetUnitCount(), (uint32_t)pEntity.first->GetInstanceCount(), 0, 0);
			}
		}
	}

	if (m_pNext != nullptr)
	{
		vkCmdNextSubpass(*pBuffer, VK_SUBPASS_CONTENTS_INLINE);
		m_pNext->ChainSubpass(pBuffer);
	}
}

RenderBatchesHandler::RenderBatchesHandler(Desc& oDesc)
{
	m_pDevice = oDesc.pWrapper->GetModifiableDevice();
	int i = 0;
	m_eSamples = oDesc.eSamples;
	m_pRenderpass = oDesc.pPass;
	for (CreationBatchDesc& oBatchCreateDesc : oDesc.oBatches)
	{
		Pipeline::Desc oPipelineDesc;
		oPipelineDesc.oShaderFilenames = oBatchCreateDesc.oShaderCompiled;
		Pipeline::FillVerticesDescription(oPipelineDesc.oBindingDescription, oPipelineDesc.oAttributeDescriptions, oBatchCreateDesc.oShaderSources[0]);
		oPipelineDesc.bTestDepth = oBatchCreateDesc.bTestDepth;
		oPipelineDesc.bWriteDepth = oBatchCreateDesc.bWriteDepth;
		oPipelineDesc.bEnableTransparent = false;
		oPipelineDesc.eSample = oDesc.eSamples;
		oPipelineDesc.iSubPassIndex = i;

		DescriptorLayoutWrapper::ShaderMap oMap;
		oMap[VK_SHADER_STAGE_VERTEX_BIT] = oBatchCreateDesc.oShaderSources[0];
		oMap[VK_SHADER_STAGE_FRAGMENT_BIT] = oBatchCreateDesc.oShaderSources[1];
		oPipelineDesc.pInputDatas = DescriptorLayoutWrapper::ParseShaderFiles(oMap, oDesc.pWrapper->GetModifiableDevice());
		oPipelineDesc.pRenderPass = oDesc.pPass;
		oPipelineDesc.pWrapper = oDesc.pWrapper;
		
		Pipeline* pPipeline = new Pipeline(oPipelineDesc);

		RenderBatch::Desc oBatchDesc;
		oBatchDesc.pWrapper = oDesc.pWrapper;
		oBatchDesc.pRenderpass = oDesc.pPass;
		oBatchDesc.pPipeline = pPipeline;
		oBatchDesc.pFactory = oDesc.pFactory;
		oBatchDesc.pNext = nullptr;

		m_oBatches.push_back(new RenderBatch(oBatchDesc));

		m_oPipelineDesc.push_back(oBatchCreateDesc);

		if (i > 0)
		{
			m_oBatches[i - 1]->SetNext(m_oBatches[i]);
		}

		i++;
	}
}

void RenderBatchesHandler::AddMesh(Mesh::StrongPtr xMesh, int iIndex, DescriptorPool* pPool)
{
	Pipeline* pPipeline = GetPipeline(iIndex);
	RenderBatch* pBatch = GetRenderBatch(iIndex);

	if (pPipeline != nullptr && pBatch != nullptr)
	{
		pBatch->AddMesh(xMesh, pPipeline->GetDescriptorSetLayout()->InstantiateDescriptorSet(*pPool, *m_pDevice));
	}
}

VkCommandBuffer* RenderBatchesHandler::GetCommand(Framebuffer* pFramebuffer)
{
	return m_oBatches[0]->GetDrawCommand(pFramebuffer);
}

void RenderBatchesHandler::ReconstructPipelines(GraphicWrapper* pWrapper)
{
	for (RenderBatch* pBatch : m_oBatches)
	{
		pBatch->ClearCache();
		delete pBatch->GetPipeline();
	}

	for (int i = 0; i < m_oBatches.size(); i++)
	{
		CreationBatchDesc oBatchCreateDesc = m_oPipelineDesc[i];

		Pipeline::Desc oPipelineDesc;
		oPipelineDesc.oShaderFilenames = oBatchCreateDesc.oShaderCompiled;
		Pipeline::FillVerticesDescription(oPipelineDesc.oBindingDescription, oPipelineDesc.oAttributeDescriptions, oBatchCreateDesc.oShaderSources[0]);
		oPipelineDesc.bTestDepth = oBatchCreateDesc.bTestDepth;
		oPipelineDesc.bWriteDepth = oBatchCreateDesc.bWriteDepth;
		oPipelineDesc.bEnableTransparent = false;
		oPipelineDesc.eSample = m_eSamples;
		oPipelineDesc.iSubPassIndex = i;

		DescriptorLayoutWrapper::ShaderMap oMap;
		oMap[VK_SHADER_STAGE_VERTEX_BIT] = oBatchCreateDesc.oShaderSources[0];
		oMap[VK_SHADER_STAGE_FRAGMENT_BIT] = oBatchCreateDesc.oShaderSources[1];
		oPipelineDesc.pInputDatas = DescriptorLayoutWrapper::ParseShaderFiles(oMap, pWrapper->GetModifiableDevice());
		oPipelineDesc.pRenderPass = m_pRenderpass;
		oPipelineDesc.pWrapper = pWrapper;

		Pipeline* pPipeline = new Pipeline(oPipelineDesc);
		m_oBatches[i]->SetPipeline(pPipeline);
	}
}

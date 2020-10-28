#include "RenderBatch.h"
#include <iostream>

RenderBatch::RenderBatch(Desc& oDesc)
{
	m_pCachedCommandBuffer = nullptr;
	m_bDirty = false;
	m_pWrapper = oDesc.pWrapper;
	m_pFactory = oDesc.pFactory;
	m_eFlag = oDesc.eFlags;
	m_pRenderpass = oDesc.pRenderpass;
	m_pPipeline = oDesc.pPipeline;
	m_pNext = oDesc.pNext;
}

VkCommandBuffer* RenderBatch::GetDrawCommand(Framebuffer* pFramebuffer)
{
	if (m_oEntities.empty())
	{
		return nullptr;
	}

	if (m_pCachedCommandBuffer != nullptr || m_bDirty)
	{
		ReconstructCommand(pFramebuffer);
		m_bDirty = false;
	}
	return m_pCachedCommandBuffer;
}

void RenderBatch::ReconstructCommand(Framebuffer* pFramebuffer)
{
	VkCommandBufferBeginInfo oCommandBeginInfo{};
	oCommandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(*m_pCachedCommandBuffer, &oCommandBeginInfo) != VK_SUCCESS)
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

	vkCmdBeginRenderPass(*m_pCachedCommandBuffer, &oBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
	ChainSubpass(m_pCachedCommandBuffer);

	vkCmdEndRenderPass(*m_pCachedCommandBuffer);
	if (vkEndCommandBuffer(*m_pCachedCommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command");
	}

}

void RenderBatch::ChainSubpass(VkCommandBuffer* pBuffer)
{
	vkCmdBindPipeline(*m_pCachedCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pPipeline->GetPipeline());

	for (RenderModel* pEntity : m_oEntities)
	{
		vkCmdBindDescriptorSets(*m_pCachedCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pPipeline->GetPipelineLayout(), 0, 1, pEntity->GetDescriptorSet(), 0, nullptr);

		VkDeviceSize oOffsets[] = { 0 };
		std::shared_ptr<BasicBuffer> xBasicBuffer = std::static_pointer_cast<BasicBuffer>(pEntity->GetVerticesBuffer());
		vkCmdBindVertexBuffers(*m_pCachedCommandBuffer, 0, 1, xBasicBuffer->GetBuffer(), oOffsets);

		if (pEntity->GetIndexesBuffer() != nullptr)
		{
			std::shared_ptr<BasicBuffer> xBasicBufferIndex = std::static_pointer_cast<BasicBuffer>(pEntity->GetIndexesBuffer());
			vkCmdBindIndexBuffer(*m_pCachedCommandBuffer, *xBasicBufferIndex->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(*m_pCachedCommandBuffer, pEntity->GetIndexesBuffer()->GetUnitCount(), pEntity->GetInstanceCount(), 0, 0, 0);
		}
		else
		{
			vkCmdDraw(*m_pCachedCommandBuffer, pEntity->GetVerticesBuffer()->GetUnitCount(), pEntity->GetInstanceCount(), 0, 0);
		}
	}

	if (m_pNext != nullptr)
	{
		vkCmdNextSubpass(*m_pCachedCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);
		m_pNext->ChainSubpass(m_pCachedCommandBuffer);
	}
}


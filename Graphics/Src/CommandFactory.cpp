#include "CommandFactory.h"
#include <iostream>
#include "Buffer.h"
#include <array>
#include "RenderPass.h"

CommandFactory::CommandFactory(Desc& oDesc)
{
	VkCommandPoolCreateInfo oPoolInfo{};
	oPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	oPoolInfo.queueFamilyIndex = oDesc.pWrapper->GetDevice()->GetGraphicQueueIndex();
	oPoolInfo.flags = 0;

	if (vkCreateCommandPool(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), &oPoolInfo, nullptr, &m_oCommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool");
	}

	m_pDevice = oDesc.pWrapper->GetModifiableDevice();
}

CommandFactory::~CommandFactory()
{
	vkDestroyCommandPool(*m_pDevice->GetLogicalDevice(), m_oCommandPool, nullptr);
}

VkCommandBuffer CommandFactory::BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo oAllocateInfo{};
	oAllocateInfo.commandPool = m_oCommandPool;
	oAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	oAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	oAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer oCommandBuffer;
	if (vkAllocateCommandBuffers(*m_pDevice->GetLogicalDevice(), &oAllocateInfo, &oCommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Error allocating command buffer");
	}

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(oCommandBuffer, &beginInfo);

	return oCommandBuffer;
}

void CommandFactory::EndSingleTimeCommands(VkCommandBuffer& oCommandBuffer)
{
	vkEndCommandBuffer(oCommandBuffer);

	VkSubmitInfo oSubmit{};
	oSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	oSubmit.commandBufferCount = 1;
	oSubmit.pCommandBuffers = &oCommandBuffer;

	vkQueueSubmit(*m_pDevice->GetGraphicQueue() , 1, &oSubmit, VK_NULL_HANDLE);
	vkQueueWaitIdle(*m_pDevice->GetGraphicQueue());

	vkFreeCommandBuffers(*m_pDevice->GetLogicalDevice(), m_oCommandPool, 1, &oCommandBuffer);
}

VkCommandBuffer CommandFactory::CreateDrawCommand(DrawDesc& oDesc)
{
	VkCommandBuffer oCmdBuffer;

	VkCommandBufferAllocateInfo oAllocateInfo{};
	oAllocateInfo.commandPool = m_oCommandPool;
	oAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	oAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	oAllocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(*m_pDevice->GetLogicalDevice(), &oAllocateInfo, &oCmdBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Error allocating command buffer");
	}

	VkCommandBufferBeginInfo oCommandBeginInfo{};
	oCommandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(oCmdBuffer, &oCommandBeginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo oBeginInfo{};
	oBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	oBeginInfo.renderPass = *oDesc.pRenderpass->GetRenderPass();
	oBeginInfo.framebuffer = *oDesc.pFramebuffer->GetFramebuffer();

	int iWidth, iHeight;
	m_pDevice->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);

	VkExtent2D oExtent;
	oExtent.height = iHeight;
	oExtent.width = iWidth;

	oBeginInfo.renderArea.offset = { 0, 0 };
	oBeginInfo.renderArea.extent = oExtent;

	std::vector<VkClearValue> oClear(1);
	oClear[0].color = { 0.1f, 0.3f,0.8f, 1.0f };

	if ((oDesc.pFramebuffer->E_DEPTH & oDesc.pFramebuffer->GetAttachmentFlags()) != 0)
	{
		VkClearValue oClearDepth;
		oClearDepth.depthStencil = { 1.0f, 0 };
		oClear.push_back(oClearDepth);
	}

	oBeginInfo.clearValueCount = static_cast<uint32_t>(oClear.size());
	oBeginInfo.pClearValues = oClear.data();

	vkCmdBeginRenderPass(oCmdBuffer, &oBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(oCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *oDesc.pPipeline->GetPipeline());

	VkDeviceSize oOffsets[] = { 0 };
	BasicBuffer* pBasicBuffer = (BasicBuffer*)oDesc.pVertexData;
	vkCmdBindVertexBuffers(oCmdBuffer, 0, 1, pBasicBuffer->GetBuffer(), oOffsets);

	if (oDesc.pIndexData != nullptr)
	{
		BasicBuffer* pBasicBufferIndex = (BasicBuffer*)oDesc.pIndexData;
		vkCmdBindIndexBuffer(oCmdBuffer, *pBasicBufferIndex->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	vkCmdBindDescriptorSets(oCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *oDesc.pLayout, 0, 1, oDesc.oDescriptorSet.data(), 0, nullptr);

	//TODO
	if (oDesc.pIndexData != nullptr)
	{
		int iUnitCount = oDesc.pIndexData->GetUnitCount();
		vkCmdDrawIndexed(oCmdBuffer, iUnitCount, 1, 0, 0, 0);
	}
	else
	{
		vkCmdDraw(oCmdBuffer, oDesc.pVertexData->GetUnitCount(), 1, 0, 0);
	}

	vkCmdEndRenderPass(oCmdBuffer);
	if (vkEndCommandBuffer(oCmdBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command");
	}
	return oCmdBuffer;
}


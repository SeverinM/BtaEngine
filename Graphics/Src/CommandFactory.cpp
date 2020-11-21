#include "CommandFactory.h"
#include <iostream>
#include "Buffer.h"
#include <array>
#include "RenderPass.h"
#include "Globals.h"

CommandFactory::CommandFactory(Desc oDesc)
{
	VkCommandPoolCreateInfo oPoolInfo{};
	oPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	oPoolInfo.queueFamilyIndex = Graphics::Globals::g_pDevice->GetGraphicQueueIndex();
	oPoolInfo.flags = oDesc.bResettable ? 0 : VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(*Graphics::Globals::g_pDevice->GetLogicalDevice(), &oPoolInfo, nullptr, &m_oCommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool");
	}
}

CommandFactory::~CommandFactory()
{
	vkDestroyCommandPool(*Graphics::Globals::g_pDevice->GetLogicalDevice(), m_oCommandPool, nullptr);
}

VkCommandBuffer CommandFactory::BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo oAllocateInfo{};
	oAllocateInfo.commandPool = m_oCommandPool;
	oAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	oAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	oAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer oCommandBuffer;
	if (vkAllocateCommandBuffers(*Graphics::Globals::g_pDevice->GetLogicalDevice(), &oAllocateInfo, &oCommandBuffer) != VK_SUCCESS)
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

	vkQueueSubmit(*Graphics::Globals::g_pDevice->GetGraphicQueue() , 1, &oSubmit, VK_NULL_HANDLE);
	vkQueueWaitIdle(*Graphics::Globals::g_pDevice->GetGraphicQueue());

	vkFreeCommandBuffers(*Graphics::Globals::g_pDevice->GetLogicalDevice(), m_oCommandPool, 1, &oCommandBuffer);
}

VkCommandBuffer* CommandFactory::CreateCommand()
{
	VkCommandBuffer* pCmdBuffer = new VkCommandBuffer();;

	VkCommandBufferAllocateInfo oAllocateInfo{};
	oAllocateInfo.commandPool = m_oCommandPool;
	oAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	oAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	oAllocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(*Graphics::Globals::g_pDevice->GetLogicalDevice(), &oAllocateInfo, pCmdBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Error allocating command buffer");
	}

	return pCmdBuffer;
}

VkCommandBuffer CommandFactory::CreateDrawCommand(DrawDesc& oDesc)
{
	VkCommandBuffer oCmdBuffer;

	VkCommandBufferAllocateInfo oAllocateInfo{};
	oAllocateInfo.commandPool = m_oCommandPool;
	oAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	oAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	oAllocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(*Graphics::Globals::g_pDevice->GetLogicalDevice(), &oAllocateInfo, &oCmdBuffer) != VK_SUCCESS)
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
	Graphics::Globals::g_pDevice->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);

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

	for (int i = 0; i < oDesc.oMultipleDraw.size(); i++)
	{
		vkCmdBindPipeline(oCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *oDesc.oMultipleDraw[i].pPipeline->GetPipeline());

		VkDeviceSize oOffsets[] = { 0 };
		std::shared_ptr<BasicBuffer> xBasicBuffer = std::static_pointer_cast<BasicBuffer>( oDesc.oMultipleDraw[i].xVertexData );
		vkCmdBindVertexBuffers(oCmdBuffer, 0, 1, xBasicBuffer->GetBuffer(), oOffsets);

		if (oDesc.oMultipleDraw[i].xIndexData != nullptr)
		{
			std::shared_ptr<BasicBuffer> xBasicBufferIndex = std::static_pointer_cast<BasicBuffer>(oDesc.oMultipleDraw[i].xIndexData);
			vkCmdBindIndexBuffer(oCmdBuffer, *xBasicBufferIndex->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}

		vkCmdBindDescriptorSets(oCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *oDesc.oMultipleDraw[i].pPipeline->GetPipelineLayout(), 0, 1, &oDesc.oMultipleDraw[i].oDescriptorSet, 0, nullptr);

		if (oDesc.oMultipleDraw[i].xIndexData != nullptr)
		{
			int iUnitCount = oDesc.oMultipleDraw[i].xIndexData->GetUnitCount();
			vkCmdDrawIndexed(oCmdBuffer, iUnitCount, oDesc.oMultipleDraw[i].iInstanceCount, 0, 0, 0);
		}
		else
		{
			vkCmdDraw(oCmdBuffer, oDesc.oMultipleDraw[i].xVertexData->GetUnitCount(), 1, 0, 0);
		}

		if ( i != oDesc.oMultipleDraw.size() - 1)
			vkCmdNextSubpass(oCmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
	}

	vkCmdEndRenderPass(oCmdBuffer);
	if (vkEndCommandBuffer(oCmdBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command");
	}
	return oCmdBuffer;
}


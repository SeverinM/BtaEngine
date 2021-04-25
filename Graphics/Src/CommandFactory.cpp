#include "CommandFactory.h"
#include <iostream>
#include "Buffer.h"
#include <array>
#include "RenderPass.h"
#include "Globals.h"
#include <algorithm>

namespace Bta
{
	namespace Graphic
	{
		CommandFactory::CommandFactory(Desc oDesc)
		{
			VkCommandPoolCreateInfo oPoolInfo{};
			oPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			oPoolInfo.queueFamilyIndex = Bta::Graphic::Globals::g_pDevice->GetGraphicQueueIndex();
			oPoolInfo.flags = oDesc.bResettable ? 0 : VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			if (vkCreateCommandPool(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), &oPoolInfo, nullptr, &m_oCommandPool) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create command pool");
			}
		}

		CommandFactory::~CommandFactory()
		{
			vkDestroyCommandPool(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), m_oCommandPool, nullptr);
		}

		/*VkCommandBuffer* CommandFactory::BeginSingleTimeCommands()
		{
			VkCommandBufferAllocateInfo oAllocateInfo{};
			oAllocateInfo.commandPool = m_oCommandPool;
			oAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			oAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			oAllocateInfo.commandBufferCount = 1;

			VkCommandBuffer* pCommandBuffer = new VkCommandBuffer();
			if (vkAllocateCommandBuffers(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), &oAllocateInfo, pCommandBuffer) != VK_SUCCESS)
			{
				throw std::runtime_error("Error allocating command buffer");
			}

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(*pCommandBuffer, &beginInfo);

			return pCommandBuffer;
		}*/

		VkCommandBuffer CommandFactory::BeginSingleTimeCommands()
		{
			VkCommandBufferAllocateInfo oAllocateInfo{};
			oAllocateInfo.commandPool = m_oCommandPool;
			oAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			oAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			oAllocateInfo.commandBufferCount = 1;

			VkCommandBuffer oCommandBuffer;
			if (vkAllocateCommandBuffers(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), &oAllocateInfo, &oCommandBuffer) != VK_SUCCESS)
			{
				throw std::runtime_error("Error allocating command buffer");
			}

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(oCommandBuffer, &beginInfo);

			return oCommandBuffer;
		}

		void CommandFactory::EndSingleTimeCommands(VkCommandBuffer oCommandBuffer)
		{
			vkEndCommandBuffer(oCommandBuffer);

			VkSubmitInfo oSubmit{};
			oSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			oSubmit.commandBufferCount = 1;
			oSubmit.pCommandBuffers = &oCommandBuffer;

			vkQueueSubmit(*Bta::Graphic::Globals::g_pDevice->GetGraphicQueue(), 1, &oSubmit, VK_NULL_HANDLE);
			FreeSingleTimeCommand(oCommandBuffer);
		}

		void CommandFactory::FreeSingleTimeCommand(VkCommandBuffer oCommandBuffer)
		{
			vkQueueWaitIdle(*Bta::Graphic::Globals::g_pDevice->GetGraphicQueue());
			vkFreeCommandBuffers(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), m_oCommandPool, 1, &oCommandBuffer);
		}
	}
}

#include "ImGuiWrapper.h"
#include "Globals.h"
#include <chrono>

#include "BasicWrapper.h"
#include "RenderPass.h"
#include "CommandFactory.h"

namespace Bta
{
	namespace Graphic
	{
		ImGuiWrapper::ImGuiWrapper(Desc& oDesc)
		{
			m_pCallback = oDesc.pCallback;
			m_oCommandBuffer.resize(oDesc.pWrapper->m_pSwapchain->GetImageViews().size());

			VkSubpassDependency oDependency{};
			oDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			oDependency.dstSubpass = 0;
			oDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			oDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			oDependency.srcAccessMask = 0;
			oDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			RenderPass::SubDesc oSubDesc;
			oSubDesc.iColorAttachmentIndex = 0;
			oSubDesc.iColorResolveAttachmentIndex = -1;
			oSubDesc.iDepthStencilAttachmentIndex = -1;
			oSubDesc.pDependency = &oDependency;

			RenderPass::Desc oPassDesc;
			oPassDesc.bEnableColor = true;
			oPassDesc.bEnableDepth = false;
			oPassDesc.eSample = VK_SAMPLE_COUNT_1_BIT;
			oPassDesc.eInitialLayoutColorAttachment = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			oPassDesc.bClearColorAttachmentAtBegin = false;
			oPassDesc.oSubpasses = { oSubDesc };
			oPassDesc.bPresentable = true;
			oPassDesc.eFormatColor = oDesc.pWrapper->GetSwapchain()->GetFormat();
			m_pRenderpass = new RenderPass(oPassDesc);


			// Setup Dear ImGui context
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

			// Setup Dear ImGui style
			ImGui::StyleColorsDark();
			//ImGui::StyleColorsClassic();

			ImGui_ImplGlfw_InitForVulkan(Bta::Graphic::Globals::g_pDevice->GetModifiableRenderSurface()->GetWindow(), true);
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = Bta::Graphic::Globals::g_oInstance;
			init_info.PhysicalDevice = *Bta::Graphic::Globals::g_pDevice->GetPhysicalDevice();
			init_info.Device = *Bta::Graphic::Globals::g_pDevice->GetLogicalDevice();
			init_info.QueueFamily = Bta::Graphic::Globals::g_pDevice->GetGraphicQueueIndex();
			init_info.Queue = *Bta::Graphic::Globals::g_pDevice->GetGraphicQueue();
			init_info.PipelineCache = VK_NULL_HANDLE;
			init_info.DescriptorPool = oDesc.pWrapper->m_pPool->GetPool();
			init_info.Allocator = nullptr;
			init_info.MinImageCount = 2;
			init_info.ImageCount = (uint32_t)oDesc.pWrapper->m_pSwapchain->GetImageViews().size();
			init_info.CheckVkResultFn = ImGuiWrapper::CheckError;
			ImGui_ImplVulkan_Init(&init_info, *m_pRenderpass->GetRenderPass());

			VkCommandBuffer oCommandBuffer = Bta::Graphic::Globals::g_pFactory->BeginSingleTimeCommands();
			ImGui_ImplVulkan_CreateFontsTexture(oCommandBuffer);
			Bta::Graphic::Globals::g_pFactory->EndSingleTimeCommands(oCommandBuffer);

			CommandFactory::Desc oFactoryDesc;
			oFactoryDesc.bResettable = true;
			m_pFactory = new CommandFactory(oFactoryDesc);

			for (int i = 0; i < oDesc.pWrapper->m_pSwapchain->GetImageViews().size(); i++)
			{
				Framebuffer::Desc oFramebufferDesc;
				oFramebufferDesc.pRenderPass = m_pRenderpass;

				std::vector<VkImageView> oView;
				oView.push_back(oDesc.pWrapper->m_pSwapchain->GetImageViews()[i]);
				oFramebufferDesc.pImageView = &oView;

				m_oFramebuffer.push_back(new Framebuffer(oFramebufferDesc));
			}
		}

		ImGuiWrapper::~ImGuiWrapper()
		{
			for (Framebuffer* pFramebuffer : m_oFramebuffer)
			{
				delete pFramebuffer;
			}
			m_oFramebuffer.clear();

			delete m_pRenderpass;

			vkFreeCommandBuffers(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), *m_pFactory->GetCommandPool(), m_oCommandBuffer.size(), m_oCommandBuffer.data());
			delete m_pFactory;

			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}

		VkCommandBuffer* ImGuiWrapper::GetDrawCommand(Desc& oDesc)
		{
			Camera* pCamera = Globals::g_pCamera;
			glm::vec3 vPos = pCamera->GetTransform()->GetPosition();
			glm::vec3 vForward = pCamera->GetTransform()->GetForward();

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			m_pCallback(oDesc.pWrapper);
			ImGui::Render();

			int iWidth, iHeight;
			Bta::Graphic::Globals::g_pDevice->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);

			vkFreeCommandBuffers(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice(), *m_pFactory->GetCommandPool(), 1, &m_oCommandBuffer[oDesc.iImageIndex]);
			m_oCommandBuffer[oDesc.iImageIndex] = m_pFactory->BeginSingleTimeCommands();

			VkRenderPassBeginInfo oBegin{};
			oBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			oBegin.renderPass = *m_pRenderpass->GetRenderPass();
			oBegin.framebuffer = *m_oFramebuffer[oDesc.iImageIndex]->GetFramebuffer();
			oBegin.renderArea.extent.width = iWidth;
			oBegin.renderArea.extent.height = iHeight;

			std::vector<VkClearValue> oClear(1);
			oClear[0].color = { 0.1f, 0.3f,0.8f, 1.0f };

			oBegin.clearValueCount = 1;
			oBegin.pClearValues = oClear.data();
			vkCmdBeginRenderPass(m_oCommandBuffer[oDesc.iImageIndex], &oBegin, VK_SUBPASS_CONTENTS_INLINE);
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_oCommandBuffer[oDesc.iImageIndex]);
			vkCmdEndRenderPass(m_oCommandBuffer[oDesc.iImageIndex]);

			vkEndCommandBuffer(m_oCommandBuffer[oDesc.iImageIndex]);

			return &m_oCommandBuffer[oDesc.iImageIndex];
		}


		void ImGuiWrapper::Recreate(Desc& oDesc, int iMinImage)
		{
			ImGui_ImplVulkan_SetMinImageCount(iMinImage);
		}

		void ImGuiWrapper::CheckError(VkResult eResult)
		{
			if (eResult != VK_SUCCESS)
			{
				throw std::runtime_error("Error imgui");
			}
		}
	}
}


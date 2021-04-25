#include "ImGuiWrapper.h"
#include "Globals.h"
#include "RenderPass.h"
#include "Output.h"
#include "DescriptorPool.h"
#include "CommandFactory.h"

namespace Bta
{
	namespace Graphic
	{
		ImGuiWrapper::ImGuiWrapper(Desc& oDesc)
		{
			if (oDesc.pCallback != nullptr)
				m_pCallback = oDesc.pCallback;
			else
				m_pCallback = [](void* pData) {};

			m_oRecreateDesc = oDesc;

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
			
			VkAttachmentDescription oAttachmentDescription{};
			oAttachmentDescription.format = Globals::g_pOutput->GetSwapchain()->GetFormat();
			oAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
			oAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			oAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			oAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			oAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			oAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			oAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			RenderPass::Desc oPassDesc;
			oPassDesc.oSubpasses = { oSubDesc };
			oPassDesc.oDescriptions = { oAttachmentDescription };

			m_pRenderpass = new RenderPass( oPassDesc );

			// Setup Dear ImGui context
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

			// Setup Dear ImGui style
			ImGui::StyleColorsDark();
			//ImGui::StyleColorsClassic();

			ImGui_ImplGlfw_InitForVulkan(Bta::Graphic::Globals::g_pOutput->GetRenderSurface()->GetWindow(), true);
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = Bta::Graphic::Globals::g_oInstance;
			init_info.PhysicalDevice = *Bta::Graphic::Globals::g_pDevice->GetPhysicalDevice();
			init_info.Device = *Bta::Graphic::Globals::g_pDevice->GetLogicalDevice();
			init_info.QueueFamily = Bta::Graphic::Globals::g_pDevice->GetGraphicQueueIndex();
			init_info.Queue = *Bta::Graphic::Globals::g_pDevice->GetGraphicQueue();
			init_info.PipelineCache = VK_NULL_HANDLE;
			init_info.DescriptorPool = Bta::Graphic::Globals::g_pPool->GetPool();
			init_info.Allocator = nullptr;
			init_info.MinImageCount = 2;
			init_info.ImageCount = (uint32_t)Globals::g_pOutput->GetSwapchain()->GetImageViews().size();
			init_info.CheckVkResultFn = ImGuiWrapper::CheckError;
			ImGui_ImplVulkan_Init(&init_info, *m_pRenderpass->GetRenderPass());

			VkCommandBuffer oCommandBuffer = Bta::Graphic::Globals::g_pFactory->BeginSingleTimeCommands();
			ImGui_ImplVulkan_CreateFontsTexture(oCommandBuffer);
			Bta::Graphic::Globals::g_pFactory->EndSingleTimeCommands(oCommandBuffer);

			CommandFactory::Desc oFactoryDesc;
			oFactoryDesc.bResettable = true;
			m_pFactory = new CommandFactory(oFactoryDesc);

			for (int i = 0; i < Bta::Graphic::Globals::g_pOutput->GetSwapchain()->GetImageViews().size(); i++)
			{
				Framebuffer::Desc oFramebufferDesc;
				oFramebufferDesc.pRenderPass = m_pRenderpass;

				std::vector<VkImageView> oView;
				oView.push_back(Bta::Graphic::Globals::g_pOutput->GetSwapchain()->GetImageViews()[i]);
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

			delete m_pFactory;

			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}

		VkCommandBuffer ImGuiWrapper::GetDrawCommand(Desc& oDesc)
		{
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Bta Debug");

			m_pCallback(nullptr);

			ImGui::End();

			ImGui::Render();

			int iWidth, iHeight;
			Bta::Graphic::Globals::g_pOutput->GetRenderSurface()->GetWindowSize(iWidth, iHeight);

			VkCommandBuffer oOutput = m_pFactory->BeginSingleTimeCommands();

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
			vkCmdBeginRenderPass(oOutput, &oBegin, VK_SUBPASS_CONTENTS_INLINE);
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), oOutput);
			vkCmdEndRenderPass(oOutput);

			vkEndCommandBuffer(oOutput);

			return oOutput;
		}


		void ImGuiWrapper::Recreate()
		{
			ImGui_ImplVulkan_SetMinImageCount(Bta::Graphic::Globals::g_pOutput->GetSwapchain()->GetImageViews().size());
			for (Framebuffer* pFramebuffer : m_oFramebuffer)
			{
				delete pFramebuffer;
			}

			m_oFramebuffer.clear();

			for (int i = 0; i < Bta::Graphic::Globals::g_pOutput->GetSwapchain()->GetImageViews().size(); i++)
			{
				Framebuffer::Desc oFramebufferDesc;
				oFramebufferDesc.pRenderPass = m_pRenderpass;

				std::vector<VkImageView> oView;
				oView.push_back(Bta::Graphic::Globals::g_pOutput->GetSwapchain()->GetImageViews()[i]);
				oFramebufferDesc.pImageView = &oView;

				m_oFramebuffer.push_back(new Framebuffer(oFramebufferDesc));
			}
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


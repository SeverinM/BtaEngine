#include "ImGuiWrapper.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "Globals.h"
#include "RenderPass.h"
#include "CommandFactory.h"
#include <chrono>

ImGuiWrapper::ImGuiWrapper(Desc& oDesc)
{
	m_pWrapper = oDesc.pWrapper;
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
	oPassDesc.pWrapper = oDesc.pWrapper;
	oPassDesc.bEnableColor = true;
	oPassDesc.bEnableDepth = false;
	oPassDesc.eSample = VK_SAMPLE_COUNT_1_BIT;
	oPassDesc.eInitialLayoutColorAttachment = VK_IMAGE_LAYOUT_UNDEFINED; //VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	oPassDesc.bClearColorAttachmentAtBegin = false;
	oPassDesc.oSubpasses = { oSubDesc };
	oPassDesc.bPresentable = true;
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

	ImGui_ImplGlfw_InitForVulkan(oDesc.pWrapper->GetModifiableDevice()->GetModifiableRenderSurface()->GetWindow(), true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = Graphics::Globals::s_oInstance;
	init_info.PhysicalDevice = *oDesc.pWrapper->GetDevice()->GetPhysicalDevice();
	init_info.Device = *oDesc.pWrapper->GetDevice()->GetLogicalDevice();
	init_info.QueueFamily = oDesc.pWrapper->GetDevice()->GetGraphicQueueIndex();
	init_info.Queue = *oDesc.pWrapper->GetDevice()->GetGraphicQueue();
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = oDesc.pWrapper->m_pPool->GetPool();
	init_info.Allocator = nullptr;
	init_info.MinImageCount = 2;
	init_info.ImageCount = oDesc.pWrapper->m_pSwapchain->GetImageViews().size();
	init_info.CheckVkResultFn = ImGuiWrapper::CheckError;
	ImGui_ImplVulkan_Init(&init_info, *m_pRenderpass->GetRenderPass());

	VkCommandBuffer oCommandBuffer = oDesc.pWrapper->m_pFactory->BeginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(oCommandBuffer);
	oDesc.pWrapper->m_pFactory->EndSingleTimeCommands(oCommandBuffer);

	CommandFactory::Desc oFactoryDesc;
	oFactoryDesc.bResettable = true;
	oFactoryDesc.pWrapper = oDesc.pWrapper;
	m_pFactory = new CommandFactory(oFactoryDesc);

	for (int i = 0; i < oDesc.pWrapper->m_pSwapchain->GetImageViews().size(); i++)
	{
		Framebuffer::Desc oFramebufferDesc;
		oFramebufferDesc.pRenderPass = m_pRenderpass;
		oFramebufferDesc.pGraphicDevice = oDesc.pWrapper->m_pDevice;

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

	vkFreeCommandBuffers(*m_pWrapper->GetDevice()->GetLogicalDevice(), *m_pFactory->GetCommandPool(), m_oCommandBuffer.size(), m_oCommandBuffer.data());
	delete m_pFactory;

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

VkCommandBuffer* ImGuiWrapper::GetDrawCommand(Desc& oDesc)
{
	glm::vec3 vPos = oDesc.pWrapper->m_pCamera->GetPosition();
	glm::vec3 vForward = oDesc.pWrapper->m_pCamera->GetForward();

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Bta Debug");
	ImGui::Text("FPS : %i", (int)(1.0f / oDesc.pWrapper->m_fElapsed));
	ImGui::Text("Instances rendered : %i", oDesc.pWrapper->m_iInstanceCount);
	ImGui::Text("Vertices count : %i", oDesc.pWrapper->m_iVerticesCount);
	ImGui::Text("Camera position : %f / %f / %f", vPos.x, vPos.y, vPos.z);
	ImGui::Text("Camera forward : %f / %f / %f", vForward.x, vForward.y, vForward.z);
	ImGui::End();
	//ImGui::ShowDemoWindow();
	ImGui::Render();

	int iWidth, iHeight;
	oDesc.pWrapper->m_pDevice->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);

	vkFreeCommandBuffers(*oDesc.pWrapper->GetDevice()->GetLogicalDevice(), *m_pFactory->GetCommandPool(), 1, &m_oCommandBuffer[oDesc.iImageIndex]);
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


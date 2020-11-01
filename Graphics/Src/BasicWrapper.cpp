#include "BasicWrapper.h"
#include "Globals.h"
#include <set>
#include "CommandFactory.h"
#include "SyncObjects.h"
#include <iostream>
#include "RenderPass.h"
#include <chrono>
#include "GLM/glm.hpp"
#include "InputHandling.h"
#include "glm/gtx/string_cast.hpp"
#include <memory>

bool BasicWrapper::s_bFramebufferResized(false);

void BasicWrapper::CreateInstance()
{
	m_iInstanceCount = m_iVerticesCount = 0;

	Window::RenderSurface::Init();

	VkApplicationInfo oAppInfo{};
	oAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	oAppInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
	oAppInfo.applicationVersion = oAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	oAppInfo.pApplicationName = "BtaEngine";
	oAppInfo.pEngineName = "Bta";

	VkInstanceCreateInfo oCreateInfo{};
	oCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	oCreateInfo.pApplicationInfo = &oAppInfo;
	
	if (m_pDesc->bEnableDebug)
	{
		const char* sLayerName = "VK_LAYER_KHRONOS_validation";
		oCreateInfo.enabledLayerCount = 1;
		oCreateInfo.ppEnabledLayerNames = &sLayerName;

		VkDebugUtilsMessengerCreateInfoEXT oDebugCreateInfo{};
		oDebugCreateInfo.pfnUserCallback = Graphics::DebugCallback;
		oDebugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		oDebugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		oDebugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		oCreateInfo.pNext = &oDebugCreateInfo;
	}
	else
	{
		oCreateInfo.enabledLayerCount = 0;
	}

	uint32_t iGlfwExtensionsCount = 0;
	const char** pGlfwExtensions;
	pGlfwExtensions = glfwGetRequiredInstanceExtensions(&iGlfwExtensionsCount);
	std::vector<const char*> oExtensions(pGlfwExtensions, pGlfwExtensions + iGlfwExtensionsCount);

	if (m_pDesc->bEnableDebug)
	{
		oExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	oCreateInfo.ppEnabledExtensionNames = oExtensions.data();
	oCreateInfo.enabledExtensionCount = static_cast<uint32_t>( oExtensions.size() );

	if (vkCreateInstance(&oCreateInfo, nullptr, &Graphics::Globals::s_oInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("Cannot create instance");
	}

	Window::RenderSurface::Desc oDesc;
	oDesc.iHeight = 1000;
	oDesc.iWidth = 1000;
	oDesc.sWindowName = "Bta";
	oDesc.pInstance = &Graphics::Globals::s_oInstance;
	oDesc.pCallback = BasicWrapper::ResizeWindow;
	m_pDesc->pSurface = new Window::RenderSurface(oDesc);
	std::cout << "Instance created" << std::endl;
}

void BasicWrapper::CreateGraphicDevice()
{
	GraphicDevice::Desc oDesc;
	oDesc.bEnableAnisotropy = true;
	oDesc.pInstance = &Graphics::Globals::s_oInstance;
	oDesc.oExtensions = m_pDesc->oRequiredExtensionsDevice;
	oDesc.pSurface = m_pDesc->pSurface;
	m_pDevice = new GraphicDevice(oDesc);

	std::cout << "Graphic device picked" << std::endl;

	CommandFactory::Desc oFactoryDesc;
	oFactoryDesc.pWrapper = this;

	m_pFactory = new CommandFactory(oFactoryDesc);

	Camera::Desc oCamDesc;
	oCamDesc.fAngleDegree = 45.0f;
	oCamDesc.fRatio = 1.0f;
	oCamDesc.fNearPlane = 0.1f;
	oCamDesc.fFarPlane = 10.0f;
	oCamDesc.pWrapper = this;
	oCamDesc.mInitialMatrix = glm::lookAt(glm::vec3(2, 2, 2), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	m_pCamera = new Camera(oCamDesc);

	InputHandling::Desc oInputDesc;
	oInputDesc.pWrapper = this;

	m_pHandling = new InputHandling(oInputDesc);
}

void BasicWrapper::CreateSwapChain()
{
	Swapchain::Desc oDesc;
	oDesc.eColorspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	oDesc.eImagesFormat = VK_FORMAT_B8G8R8A8_SRGB;
	oDesc.ePresentMode = VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR;
	oDesc.pGraphicWrapper = this;
	oDesc.iImageLayers = 1;

	m_pSwapchain = new Swapchain(oDesc);

	std::cout << "Swapchain created" << std::endl;
}

void BasicWrapper::CreateRenderPass()
{
	RenderPass::SubDesc oSubDesc;
	oSubDesc.iColorAttachmentIndex = 0;
	oSubDesc.iDepthStencilAttachmentIndex = 1;
	oSubDesc.iColorResolveAttachmentIndex = 2;

	RenderPass::SubDesc oSkyboxDesc;
	oSkyboxDesc.iColorAttachmentIndex = 0;
	oSkyboxDesc.iColorResolveAttachmentIndex = 2;
	oSkyboxDesc.iDepthStencilAttachmentIndex = -1;

	RenderPass::Desc oDesc;
	oDesc.eSample = VK_SAMPLE_COUNT_8_BIT;
	oDesc.pWrapper = this;
	oDesc.bEnableColor = true;
	oDesc.bEnableDepth = true;
	oDesc.oSubpasses = { oSkyboxDesc, oSubDesc };
	oDesc.eInitialLayoutColorAttachment = VK_IMAGE_LAYOUT_UNDEFINED;
	oDesc.bClearColorAttachmentAtBegin = true;
	oDesc.bPresentable = false;

	m_pRenderpass = new RenderPass(oDesc);

	std::cout << "Renderpass created" << std::endl;
}

void BasicWrapper::CreateGraphicPipeline()
{
	DescriptorLayoutWrapper::ShaderMap oMap;
	oMap[VK_SHADER_STAGE_VERTEX_BIT] = "./Shader/Src/vs.vert";
	oMap[VK_SHADER_STAGE_FRAGMENT_BIT] = "./Shader/Src/fs.frag";
	m_pPrototype = DescriptorLayoutWrapper::ParseShaderFiles(oMap, m_pDevice);

	//Skybox
	oMap[VK_SHADER_STAGE_VERTEX_BIT] = "./Shader/Skybox/Src/vs.vert";
	oMap[VK_SHADER_STAGE_FRAGMENT_BIT] = "./Shader/Skybox/Src/fs.frag";
	m_pPrototypeSky = DescriptorLayoutWrapper::ParseShaderFiles(oMap, m_pDevice);

	//Create pipeline
	Pipeline::Desc oDesc;
	oDesc.pInputDatas = m_pPrototype;
	oDesc.bEnableDepth = true;
	oDesc.bEnableTransparent = false;
	oDesc.eSample = VK_SAMPLE_COUNT_8_BIT;
	oDesc.oBindingDescription = Vertex::GetBindingDescription();
	oDesc.oAttributeDescriptions = Vertex::GetAttributeDescriptions();
	oDesc.oShaderFilenames = { "./Shader/vert.spv", "./Shader/frag.spv" };
	oDesc.pWrapper = this;
	oDesc.pRenderPass = m_pRenderpass;
	oDesc.iSubPassIndex = 1;
	m_pPipeline = new Pipeline(oDesc);

	//SkyboxPipeline
	oDesc.iSubPassIndex = 0;
	oDesc.oShaderFilenames = { "./Shader/Skybox/vert.spv", "./Shader/Skybox/frag.spv" };
	oDesc.bEnableDepth = false;
	oDesc.eSample = VK_SAMPLE_COUNT_8_BIT;
	oDesc.iSubPassIndex = 0;
	oDesc.pInputDatas = m_pPrototypeSky;
	m_pSkyboxPipeline = new Pipeline(oDesc);

	
	DescriptorPool::Desc oDescPool;
	oDescPool.iImageCount = m_pSwapchain->GetImageViews().size();
	oDescPool.iSize = 1000;
	oDescPool.pWrapper = this;
	m_pPool = new DescriptorPool(oDescPool);

	InitVerticesBuffers();
	FillDescriptorsBuffer();
	InitFramebuffer();
}

void BasicWrapper::FillDescriptorsBuffer()
{
	std::vector<glm::mat4> oVP{ m_pCamera->GetViewMatrix(), m_pCamera->GetProjectionMatrix() };
	std::shared_ptr<BasicBuffer> xVPMatrice(m_pCamera->GetVPMatriceBuffer());
	xVPMatrice->CopyFromMemory(oVP.data(), GetModifiableDevice());

	m_pInputDatas = m_pPrototype->InstantiateDescriptorSet(*m_pPool, *m_pDevice);

	//0 -> Uniform buffer
	m_pInputDatas->FillSlot(0, m_pCamera->GetVPMatriceBuffer().get());

	//1 -> Storage buffer 
	m_pInputDatas->FillSlot(1, m_pRenderModel->GetModelMatrices().get());

	//2 -> Texture
	m_pInputDatas->FillSlot(2, m_pRenderModel->GetTextures()[0].get());

	m_pInputDatas->CommitSlots(m_pPool);

	std::vector<glm::mat4> oMPMatrices = { glm::mat4(1.0f), m_pCamera->GetProjectionMatrix() };

	Image::FromFileDesc oFileDesc;
	oFileDesc.bEnableMip = false;
	oFileDesc.eAspect = VK_IMAGE_ASPECT_COLOR_BIT;
	oFileDesc.eFormat = VK_FORMAT_R8G8B8A8_SRGB;
	oFileDesc.eSampleFlag = VK_SAMPLE_COUNT_1_BIT;
	oFileDesc.eTiling = VK_IMAGE_TILING_OPTIMAL;
	oFileDesc.pFactory = m_pFactory;
	oFileDesc.pWrapper = this;
	std::string sFilenames[6] = { "./Textures/bkg1_back.png", "./Textures/bkg1_bot.png", "./Textures/bkg1_front.png", "./Textures/bkg1_left.png", "./Textures/bkg1_right.png", "./Textures/bkg1_top.png" };
	Image* pImage = Image::CreateCubeMap(sFilenames, oFileDesc);

	m_pInputDatasSky = m_pPrototypeSky->InstantiateDescriptorSet(*m_pPool, *m_pDevice);

	//0 -> Uniform Buffer
	BasicBuffer::Desc oBuffer;
	oBuffer.iUnitSize = sizeof(glm::mat4);
	oBuffer.iUnitCount = 2;
	oBuffer.pWrapper = this;
	oBuffer.eUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	oBuffer.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	BasicBuffer* pBuffer = new BasicBuffer(oBuffer);
	m_pInputDatasSky->FillSlot(0, pBuffer);

	glm::mat4 mMat = glm::mat4(1.0f);
	pBuffer->CopyFromMemory(&mMat, m_pDevice, 0, sizeof(glm::mat4));
	pBuffer->CopyFromMemory(&m_pCamera->GetProjectionMatrix(),m_pDevice, sizeof(glm::mat4), sizeof(glm::mat4));

	//1-> texture
	m_pInputDatasSky->FillSlot(1, pImage);
	
	m_pInputDatasSky->CommitSlots(m_pPool);

	std::cout << "matrix buffer created" << std::endl;
}

void BasicWrapper::InitImGui()
{
	ImGuiWrapper::Desc oDesc;
	oDesc.iImageIndex = 0;
	oDesc.pWrapper = this;
	oDesc.pCallback = BasicWrapper::RenderGui;

	m_pImGui = new ImGuiWrapper(oDesc);
}

void BasicWrapper::CreateCommandBuffer()
{
	for (int i = 0; i < m_pSwapchain->GetImageViews().size(); i++)
	{
		CommandFactory::SubDrawDesc oSubSky;
		oSubSky.oDescriptorSet = *m_pInputDatasSky->GetDescriptorSet();
		oSubSky.pPipeline = m_pSkyboxPipeline;
		oSubSky.xVertexData = m_pRenderModelSky->GetVerticesBuffer();
		oSubSky.xIndexData = m_pRenderModelSky->GetIndexesBuffer();
		oSubSky.iInstanceCount = 1;

		CommandFactory::SubDrawDesc oSub;
		oSub.oDescriptorSet = *m_pInputDatas->GetDescriptorSet();
		oSub.pPipeline = m_pPipeline;
		oSub.xVertexData = m_pRenderModel->GetVerticesBuffer();
		oSub.xIndexData = m_pRenderModel->GetIndexesBuffer();
		oSub.iInstanceCount = m_pRenderModel->GetModels().size();

		CommandFactory::DrawDesc oDesc;
		oDesc.oMultipleDraw = { oSubSky, oSub };
		oDesc.pRenderpass = m_pRenderpass;
		oDesc.pFramebuffer = m_oFramebuffers[i];

		m_oAllDrawCommands.push_back(m_pFactory->CreateDrawCommand(oDesc));
	}
	std::cout << "Draw commands created" << std::endl;
	InitImGui();
}

void BasicWrapper::InitVerticesBuffers()
{
	RenderModel::Desc oDesc;
	oDesc.pFactory = m_pFactory;
	oDesc.pWrapper = this;
	oDesc.sFilenameModel = "./Models/viking_room.obj";
	oDesc.oFilenamesTextures = { "./Textures/viking_room.png" };
	oDesc.oModels = { std::shared_ptr<Transform>( new Transform() ), std::shared_ptr<Transform>(new Transform()) };
	oDesc.oModels[1]->SetPosition(glm::vec3(2.5f, 0, 0), true);
	oDesc.eFlag = RenderModel::eVerticesAttributes::E_UV | RenderModel::eVerticesAttributes::E_POSITIONS;

	m_iInstanceCount += oDesc.oModels.size();

	m_pRenderModel = new RenderModel(oDesc);
	m_pRenderModel->ConvertToVerticesBuffer(m_pRenderModel->GetBufferFlags(), true, oDesc.pWrapper);
	m_iVerticesCount += m_pRenderModel->GetVerticeCount();

	oDesc.oFilenamesTextures = {};
	oDesc.sFilenameModel = { "./Models/cube.obj" };
	oDesc.oModels = { std::shared_ptr<Transform>(new Transform() )};
	oDesc.oModels[0]->SetScale(glm::vec3(3.0f));
	m_pRenderModelSky = new RenderModel(oDesc);
	m_pRenderModelSky->ConvertToVerticesBuffer(m_pRenderModelSky->GetBufferFlags(), true, oDesc.pWrapper);

	m_iInstanceCount += oDesc.oModels.size();
	m_iVerticesCount += m_pRenderModelSky->GetVerticeCount();

	std::cout << "vertex buffer created" << std::endl;
}

void BasicWrapper::InitFramebuffer()
{
	for (int i = 0; i < m_pSwapchain->GetImageViews().size(); i++)
	{
		Image::Desc oImgDesc;
		oImgDesc.bEnableMip = false;
		oImgDesc.eFormat = VK_FORMAT_D32_SFLOAT;
		oImgDesc.eProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		oImgDesc.eSampleFlag = VK_SAMPLE_COUNT_8_BIT;
		oImgDesc.eTiling = VK_IMAGE_TILING_OPTIMAL;
		oImgDesc.eUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		oImgDesc.pWrapper = this;
		oImgDesc.eAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		oImgDesc.iLayerCount = 1;

		int iWidth, iHeight;
		GetModifiableDevice()->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);
		oImgDesc.iHeight = iHeight;
		oImgDesc.iWidth = iWidth;
		Image* pImg = new Image(oImgDesc);
		m_oAllDepths.push_back(std::shared_ptr<Image>( pImg ));

		Image::Desc oMultisampleDesc;
		oMultisampleDesc.bEnableMip = false;
		oMultisampleDesc.eFormat = m_pSwapchain->GetFormat();
		oMultisampleDesc.eSampleFlag = VK_SAMPLE_COUNT_8_BIT;
		oMultisampleDesc.eTiling = VK_IMAGE_TILING_OPTIMAL;
		oMultisampleDesc.eProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		oMultisampleDesc.pWrapper = this;
		oMultisampleDesc.pFactory = m_pFactory;
		oMultisampleDesc.iWidth = iWidth;
		oMultisampleDesc.iHeight = iHeight;
		oMultisampleDesc.eUsage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		oMultisampleDesc.eAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		oMultisampleDesc.iLayerCount = 1;

		Image* pImgMultisample = new Image(oMultisampleDesc);

		Framebuffer::Desc oDesc;
		oDesc.pGraphicDevice = m_pDevice;

		std::vector<VkImageView> oImages;
		oImages.push_back(*pImgMultisample->GetImageView());
		oImages.push_back(*pImg->GetImageView());
		oImages.push_back(m_pSwapchain->GetImageViews()[i]);
		oDesc.pImageView = &oImages;
		oDesc.pRenderPass = m_pRenderpass;

		m_oFramebuffers.push_back(new Framebuffer(oDesc));
	}
	std::cout << "Framebuffer created" << std::endl;
}

bool BasicWrapper::Render(SyncObjects* pSync)
{
	m_pRenderModel->GetModels()[1]->Rotate(glm::vec3(0, 0, 1), 0.1f);

	auto start = std::chrono::system_clock::now();
	VkResult eResult = VK_SUCCESS;
	if (!s_bFramebufferResized)
	{

		int iFrame = pSync->GetFrame();
		vkWaitForFences(*GetDevice()->GetLogicalDevice(), 1, &pSync->GetInFlightFences()[iFrame], VK_TRUE, UINT64_MAX);

		uint32_t iImageIndex;
		eResult = vkAcquireNextImageKHR(*GetDevice()->GetLogicalDevice(), *GetSwapchain()->GetSwapchain(), UINT64_MAX, pSync->GetImageAcquiredSemaphore()[iFrame], VK_NULL_HANDLE, &iImageIndex);

		if (eResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return true;
		}
		else if (eResult != VK_SUCCESS && eResult != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire swap chain images");
		}

		if (pSync->GetSwapChainImagesFences()[iImageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(*GetDevice()->GetLogicalDevice(), 1, &pSync->GetSwapChainImagesFences()[iImageIndex], VK_TRUE, UINT64_MAX);
		}
		pSync->GetSwapChainImagesFences()[iImageIndex] = pSync->GetInFlightFences()[pSync->GetFrame()];

		ImGuiWrapper::Desc oDesc;
		oDesc.iImageIndex = iImageIndex;
		oDesc.pWrapper = this;

		VkSubmitInfo oSubmit{};
		oSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		oSubmit.waitSemaphoreCount = 1;
		oSubmit.pWaitSemaphores = &pSync->GetImageAcquiredSemaphore()[pSync->GetFrame()];
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		std::vector<VkCommandBuffer> oCmds = { m_oAllDrawCommands[iImageIndex], *m_pImGui->GetDrawCommand(oDesc) };
		oSubmit.pWaitDstStageMask = waitStages;
		oSubmit.commandBufferCount = oCmds.size();
		oSubmit.pCommandBuffers = oCmds.data();
		oSubmit.signalSemaphoreCount = 1;
		oSubmit.pSignalSemaphores = &pSync->GetRenderFinishedSemaphore()[pSync->GetFrame()];

		vkResetFences(*GetDevice()->GetLogicalDevice(), 1, &pSync->GetInFlightFences()[pSync->GetFrame()]);
		if (vkQueueSubmit(*GetDevice()->GetGraphicQueue(), 1, &oSubmit, pSync->GetInFlightFences()[pSync->GetFrame()]) != VK_SUCCESS)
		{
			return false;
		}

		VkPresentInfoKHR oPresentInfo{};
		oPresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		oPresentInfo.swapchainCount = 1;
		oPresentInfo.pSwapchains = GetSwapchain()->GetSwapchain();
		oPresentInfo.pResults = nullptr;
		oPresentInfo.pImageIndices = &iImageIndex;
		oPresentInfo.waitSemaphoreCount = 1;
		oPresentInfo.pWaitSemaphores = &pSync->GetRenderFinishedSemaphore()[pSync->GetFrame()];

		eResult = vkQueuePresentKHR(*GetDevice()->GetPresentQueue(), &oPresentInfo);
	}

	if (eResult == VK_ERROR_OUT_OF_DATE_KHR || eResult == VK_SUBOPTIMAL_KHR || s_bFramebufferResized)
	{
		s_bFramebufferResized = false;
		RecreateSwapChain();
	}
	else if (eResult != VK_SUCCESS)
	{
		return false;
	}

	pSync->NextFrame();
	auto end = std::chrono::system_clock::now();
	m_fElapsed = std::chrono::duration<float>(end - start).count();
	return true;
}

void BasicWrapper::ResizeWindow(GLFWwindow* pWindow, int iWidth , int iHeight)
{
	s_bFramebufferResized = true;
}

void BasicWrapper::RenderGui(BasicWrapper* pWrapper)
{
	glm::vec3 vPos = pWrapper->m_pCamera->GetTransform()->GetPosition();
	glm::vec3 vForward = pWrapper->m_pCamera->GetTransform()->GetForward();

	ImGui::Begin("Bta Debug");
	ImGui::Text("FPS : %i", (int)(1.0f / pWrapper->m_fElapsed));
	ImGui::Text("Instances rendered : %i", pWrapper->m_iInstanceCount);
	ImGui::Text("Vertices count : %i", pWrapper->m_iVerticesCount);
	ImGui::Text("Camera position : %f / %f / %f", vPos.x, vPos.y, vPos.z);
	ImGui::Text("Camera forward : %f / %f / %f", vForward.x, vForward.y, vForward.z);
	ImGui::End();
}

void BasicWrapper::RecreateSwapChain()
{
	int iWidth, iHeight;
	m_pDevice->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);

	if (iHeight == 0 || iWidth == 0)
	{
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(*m_pDevice->GetLogicalDevice());

	m_oAllDepths.clear();
	m_oAllMultisample.clear();

	for (Framebuffer* pFramebuffer : m_oFramebuffers)
	{
		delete pFramebuffer;
	}
	m_oFramebuffers.clear();

	vkFreeCommandBuffers(*m_pDevice->GetLogicalDevice(), *m_pFactory->GetCommandPool(), m_oAllDrawCommands.size(), m_oAllDrawCommands.data());
	m_oAllDrawCommands.clear();

	delete m_pPipeline;
	delete m_pSkyboxPipeline;
	delete m_pRenderpass;
	delete m_pSwapchain;

	/*for (DescriptorPool::UpdateSubDesc& oSub : m_oInputDatas)
	{
		oSub.oBuffers.clear();
		delete oSub.pDescriptorSet;
	}
	m_oInputDatas.clear();

	m_oInputDatasSky.clear();*/
	delete m_pPool;
	delete m_pImGui;

	CreateSwapChain();
	CreateRenderPass();
	CreateGraphicPipeline();
	CreateCommandBuffer();
}

BasicWrapper::~BasicWrapper()
{
	delete m_pSwapchain;
	delete m_pRenderpass;

	//for (DescriptorPool::UpdateSubDesc& oSub : m_oInputDatas)
	//{
	//	oSub.oBuffers.clear();

	//	//vkFreeDescriptorSets(*m_pDevice->GetLogicalDevice(), m_pPool->GetPool(), 1, oSub.xDescriptorSet);
	//}
	//m_oInputDatas.clear();

	//for (DescriptorPool::UpdateSubDesc& oSub : m_oInputDatasSky)
	//{
	//	oSub.oBuffers.clear();

	//	//vkFreeDescriptorSets(*m_pDevice->GetLogicalDevice(), m_pPool->GetPool(), 1, oSub.pDescriptorSet);
	//}
	//m_oInputDatasSky.clear();

	vkFreeCommandBuffers(*m_pDevice->GetLogicalDevice(), *m_pFactory->GetCommandPool(), m_oAllDrawCommands.size(), m_oAllDrawCommands.data());
	delete m_pPool;
	delete m_pPipeline;
	delete m_pSkyboxPipeline;
	delete m_pFactory;
	delete m_pRenderModel;
	delete m_pRenderModelSky;

	m_oAllDepths.clear();
	m_oAllMultisample.clear();

	for (Framebuffer* pFramebuffer : m_oFramebuffers)
	{
		delete pFramebuffer;
	}
	m_oFramebuffers.clear();

	delete m_pImGui;
	delete m_pDevice;
}

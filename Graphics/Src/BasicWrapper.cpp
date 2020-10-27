#include "BasicWrapper.h"
#include "Globals.h"
#include <set>
#include "CommandFactory.h"
#include "SyncObjects.h"
#include <iostream>
#include "RenderPass.h"
#include <chrono>
#include "GLM/glm.hpp"

bool BasicWrapper::s_bFramebufferResized(false);

void BasicWrapper::CreateInstance()
{
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

	m_pCamera = new Camera(oCamDesc);
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
	//Setup inputs
	int iSwapCount = m_pSwapchain->GetImageViews().size();

	DescriptorPool::BufferDesc oBufferDesc;
	oBufferDesc.eType = DescriptorPool::E_UNIFORM_BUFFER;

	DescriptorPool::BufferDesc oBufferDesc2;
	oBufferDesc2.eType = DescriptorPool::E_TEXTURE;

	DescriptorPool::BufferDesc oBufferDesc3;
	oBufferDesc3.eType = DescriptorPool::E_STORAGE_BUFFER;

	std::vector<DescriptorPool::BufferDesc> oLayout;
	oLayout.push_back(oBufferDesc);
	oLayout.push_back(oBufferDesc3);
	oLayout.push_back(oBufferDesc2);

	m_oPrototype.push_back(oLayout);

	//Skybox
	DescriptorPool::BufferDesc oBufferDescSky;
	oBufferDescSky.eType = DescriptorPool::E_UNIFORM_BUFFER;

	DescriptorPool::BufferDesc oBufferDesc2Sky;
	oBufferDesc2Sky.eType = DescriptorPool::E_TEXTURE;

	std::vector<DescriptorPool::BufferDesc> oLayoutSky;
	oLayoutSky.push_back(oBufferDescSky);
	oLayoutSky.push_back(oBufferDesc2Sky);

	m_oPrototypeSkybox.push_back(oLayoutSky);

	//Create pipeline
	Pipeline::Desc oDesc;
	oDesc.oInputDatas = m_oPrototype;
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
	oDesc.oInputDatas = m_oPrototypeSkybox;
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
	int iSwapCount = m_pSwapchain->GetImageViews().size();

	for (int i = 0; i < iSwapCount; i++)
	{
		VkDescriptorSet* pDescriptor = new VkDescriptorSet();
		m_pPool->CreateDescriptorSet(*pDescriptor, m_pPipeline->GetDescriptorSetLayout()[0]);

		DescriptorPool::UpdateSubDesc oDescriptorSet;
		oDescriptorSet.xDescriptorSet = std::shared_ptr<VkDescriptorSet>( pDescriptor );
		oDescriptorSet.oBuffers = m_oPrototype[0];

		//0 -> Uniform buffer
		BasicBuffer::Desc oBuffer;
		oBuffer.iUnitSize = sizeof(glm::mat4);
		oBuffer.iUnitCount = 2;
		oBuffer.pWrapper = this;
		oBuffer.eUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		oBuffer.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		
		std::vector<glm::mat4> oMVP{ m_pCamera->GetViewMatrix(), m_pCamera->GetProjectionMatrix() };
		oDescriptorSet.oBuffers[0].xBuffer = std::shared_ptr<Buffer>( new BasicBuffer(oBuffer) );
		oDescriptorSet.oBuffers[0].xBuffer->CopyFromMemory(oMVP.data(), GetModifiableDevice());

		//1 -> Storage buffer 
		oDescriptorSet.oBuffers[1].xBuffer = m_pRenderModel->GetModelMatrices();

		//2 -> Texture
		oDescriptorSet.oBuffers[2].xBuffer = m_pRenderModel->GetTextures()[0];

		m_oInputDatas.push_back(oDescriptorSet);
	}
	m_pPool->WriteDescriptor(m_oInputDatas, m_pPipeline->GetDescriptorSetLayout()[0]);

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
	std::shared_ptr<Image> xImage = std::shared_ptr<Image>(pImage);

	for (int i = 0; i < iSwapCount; i++)
	{
		VkDescriptorSet* pDescriptorSet = new VkDescriptorSet();
		m_pPool->CreateDescriptorSet(*pDescriptorSet, m_pSkyboxPipeline->GetDescriptorSetLayout()[0]);

		DescriptorPool::UpdateSubDesc oDescriptorSet;
		oDescriptorSet.xDescriptorSet = std::shared_ptr<VkDescriptorSet>( pDescriptorSet );
		oDescriptorSet.oBuffers = m_oPrototypeSkybox[0];

		//0 -> Uniform Buffer
		BasicBuffer::Desc oBuffer;
		oBuffer.iUnitSize = sizeof(glm::mat4);
		oBuffer.iUnitCount = 2;
		oBuffer.pWrapper = this;
		oBuffer.eUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		oBuffer.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		oDescriptorSet.oBuffers[0].xBuffer = std::shared_ptr<Buffer>( new BasicBuffer(oBuffer) );
		oDescriptorSet.oBuffers[0].xBuffer->CopyFromMemory(oMPMatrices.data(), GetModifiableDevice());

		//1-> texture
		Image::FromFileDesc oFileDesc;
		oFileDesc.bEnableMip = false;
		oFileDesc.eAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		oFileDesc.eFormat = VK_FORMAT_R8G8B8A8_SRGB;
		oFileDesc.eSampleFlag = VK_SAMPLE_COUNT_1_BIT;
		oFileDesc.eTiling = VK_IMAGE_TILING_OPTIMAL;
		oFileDesc.pFactory = m_pFactory;
		oFileDesc.pWrapper = this;

		oDescriptorSet.oBuffers[1].xBuffer = std::static_pointer_cast<Buffer>(xImage);

		m_oInputDatasSky.push_back(oDescriptorSet);
	}
	m_pPool->WriteDescriptor(m_oInputDatasSky, m_pSkyboxPipeline->GetDescriptorSetLayout()[0]);

	m_oPrototype.clear();
	m_oPrototypeSkybox.clear();

	std::cout << "matrix buffer created" << std::endl;
}


void BasicWrapper::InitImGui()
{
	ImGuiWrapper::Desc oDesc;
	oDesc.iImageIndex = 0;
	oDesc.pWrapper = this;

	m_pImGui = new ImGuiWrapper(oDesc);
}

void BasicWrapper::CreateCommandBuffer()
{
	for (int i = 0; i < m_pSwapchain->GetImageViews().size(); i++)
	{
		CommandFactory::SubDrawDesc oSubSky;
		oSubSky.oDescriptorSet = *m_oInputDatasSky[i].xDescriptorSet;
		oSubSky.pPipeline = m_pSkyboxPipeline;
		oSubSky.xVertexData = m_pRenderModelSky->GetVerticesBuffer();
		oSubSky.xIndexData = m_pRenderModelSky->GetIndexesBuffer();
		oSubSky.iInstanceCount = 1;

		CommandFactory::SubDrawDesc oSub;
		oSub.oDescriptorSet = *m_oInputDatas[i].xDescriptorSet;
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
	oDesc.oModels = { std::shared_ptr<Transform>( new Transform() ) };
	oDesc.eFlag = RenderModel::eVerticesAttributes::E_UV | RenderModel::eVerticesAttributes::E_POSITIONS;

	m_pRenderModel = new RenderModel(oDesc);
	m_pRenderModel->ConvertToBuffer(m_pRenderModel->GetBufferFlags(), true, oDesc.pWrapper);

	oDesc.oFilenamesTextures = {};
	oDesc.sFilenameModel = { "./Models/cube.obj" };
	m_pRenderModelSky = new RenderModel(oDesc);
	m_pRenderModelSky->ConvertToBuffer(m_pRenderModelSky->GetBufferFlags(), true, oDesc.pWrapper);

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

void BasicWrapper::UpdateUniformBuffer(int iImageIndex)
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	std::vector<glm::mat4> oModels;
	oModels.resize(m_pRenderModel->GetModels().size());
	for (int i = 0; i < m_pRenderModel->GetModels().size(); i++)
	{
		glm::vec3 vTranslate(0, 0, 0);
		m_pRenderModel->GetModels()[i]->SetPosition(vTranslate, true);
		oModels[i] = m_pRenderModel->GetModels()[i]->GetModelMatrix();
	}
	m_pRenderModel->GetModelMatrices()->CopyFromMemory(oModels.data(), m_pDevice);

	glm::mat4 mView = m_pCamera->GetViewMatrix();
	mView = glm::translate(mView, glm::vec3(0, 0, 0.1f * time));

	glm::mat4 mProj = m_pCamera->GetProjectionMatrix();
	std::vector<glm::mat4> oMat = { mView, mProj };
	m_pCamera->GetVPMatriceBuffer()->CopyFromMemory(oMat.data(), m_pDevice);
}

bool BasicWrapper::Render(SyncObjects* pSync)
{
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

		UpdateUniformBuffer(iImageIndex);
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
	return true;
}

void BasicWrapper::ResizeWindow(GLFWwindow* pWindow, int iWidth , int iHeight)
{
	s_bFramebufferResized = true;
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

	m_oInputDatas.clear();
	m_oInputDatasSky.clear();
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

	for (DescriptorPool::UpdateSubDesc& oSub : m_oInputDatas)
	{
		oSub.oBuffers.clear();

		//vkFreeDescriptorSets(*m_pDevice->GetLogicalDevice(), m_pPool->GetPool(), 1, oSub.xDescriptorSet);
	}
	m_oInputDatas.clear();

	for (DescriptorPool::UpdateSubDesc& oSub : m_oInputDatasSky)
	{
		oSub.oBuffers.clear();

		//vkFreeDescriptorSets(*m_pDevice->GetLogicalDevice(), m_pPool->GetPool(), 1, oSub.pDescriptorSet);
	}
	m_oInputDatasSky.clear();

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

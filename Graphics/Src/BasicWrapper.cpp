#include "BasicWrapper.h"
#include "Globals.h"
#include <set>
#include "CommandFactory.h"
#include "SyncObjects.h"
#include <iostream>
#include "RenderPass.h"
#include <chrono>
#include "GLM/glm.hpp"

BasicWrapper* BasicWrapper::s_pInstance(nullptr);

void BasicWrapper::CreateInstance()
{
	s_pInstance = this;

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
	oSubDesc.iColorResolveAttachmentIndex = -1;

	RenderPass::SubDesc oSkyboxDesc;
	oSkyboxDesc.iColorAttachmentIndex = 0;
	oSkyboxDesc.iColorResolveAttachmentIndex = -1;
	oSkyboxDesc.iDepthStencilAttachmentIndex = -1;

	RenderPass::Desc oDesc;
	oDesc.eSample = VK_SAMPLE_COUNT_1_BIT;
	oDesc.pWrapper = this;
	oDesc.bEnableColor = true;
	oDesc.bEnableDepth = true;
	oDesc.oSubpasses = { oSkyboxDesc, oSubDesc };

	m_pRenderpass = new RenderPass(oDesc);

	std::cout << "Renderpass created" << std::endl;
}

void BasicWrapper::GetMatrices(glm::mat4& vView, glm::mat4& vProjection, std::vector<glm::mat4>& oModels, int iSize)
{
	vView = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	int iWidth, iHeight;
	m_pDevice->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);
	vProjection = glm::perspective(glm::radians(45.0f), iWidth / (float)iHeight, 0.1f, 10.0f);
	vProjection[1][1] *= -1;

	for (int i = 0; i < iSize; i++)
	{
		glm::mat4 vModel = glm::mat4(1.0f);
		vModel = glm::translate(vModel, glm::vec3(i, 0.0f, 0.0f));
		oModels.push_back(vModel);
	}

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
	oDesc.eSample = VK_SAMPLE_COUNT_1_BIT;
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
	oDesc.eSample = VK_SAMPLE_COUNT_1_BIT;
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

	m_iInstanceCount = 2;
	m_pPool->CreateDescriptorSet(m_oDescriptors, iSwapCount, m_pPipeline->GetDescriptorSetLayout()[0]);

	for (int i = 0; i < iSwapCount; i++)
	{
		DescriptorPool::UpdateSubDesc oDescriptorSet;
		oDescriptorSet.pDescriptorSet = &m_oDescriptors[i];
		oDescriptorSet.oBuffers = m_oPrototype[0];

		//0 -> Uniform buffer
		BasicBuffer::Desc oBuffer;
		oBuffer.iUnitSize = sizeof(glm::mat4);
		oBuffer.iUnitCount = 2;
		oBuffer.pWrapper = this;
		oBuffer.eUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		oBuffer.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		
		std::vector<glm::mat4> oMVP{ m_pCamera->GetViewMatrix(), m_pCamera->GetProjectionMatrix() };
		oDescriptorSet.oBuffers[0].pBuffer = new BasicBuffer(oBuffer);
		oDescriptorSet.oBuffers[0].pBuffer->CopyFromMemory(oMVP.data(), GetModifiableDevice());

		//1 -> Storage buffer 
		oDescriptorSet.oBuffers[1].pBuffer = m_pRenderModel->GetModelMatrices();

		//2 -> Texture
		oDescriptorSet.oBuffers[2].pBuffer = m_pRenderModel->GetTextures()[0];

		m_oInputDatas.push_back(oDescriptorSet);
		m_oMPMatrices.push_back((BasicBuffer*)oDescriptorSet.oBuffers[0].pBuffer);
		m_oAllMatricesInstance.push_back((BasicBuffer*)oDescriptorSet.oBuffers[1].pBuffer);
	}
	m_pPool->WriteDescriptor(m_oInputDatas, m_pPipeline->GetDescriptorSetLayout()[0]);


	m_pPool->CreateDescriptorSet(m_oDescriptorsSky, iSwapCount, m_pSkyboxPipeline->GetDescriptorSetLayout()[0]);
	MP oMatricesSky = GetMatricesSky();
	for (int i = 0; i < iSwapCount; i++)
	{
		DescriptorPool::UpdateSubDesc oDescriptorSet;
		oDescriptorSet.pDescriptorSet = &m_oDescriptorsSky[i];
		oDescriptorSet.oBuffers = m_oPrototypeSkybox[0];

		//0 -> Uniform Buffer
		BasicBuffer::Desc oBuffer;
		oBuffer.iUnitSize = sizeof(glm::mat4);
		oBuffer.iUnitCount = 2;
		oBuffer.pWrapper = this;
		oBuffer.eUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		oBuffer.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		oDescriptorSet.oBuffers[0].pBuffer = new BasicBuffer(oBuffer);
		oDescriptorSet.oBuffers[0].pBuffer->CopyFromMemory(&oMatricesSky, GetModifiableDevice());

		//1-> texture
		Image::FromFileDesc oFileDesc;
		oFileDesc.bEnableMip = false;
		oFileDesc.eAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		oFileDesc.eFormat = VK_FORMAT_R8G8B8A8_SRGB;
		oFileDesc.eSampleFlag = VK_SAMPLE_COUNT_1_BIT;
		oFileDesc.eTiling = VK_IMAGE_TILING_OPTIMAL;
		oFileDesc.pFactory = m_pFactory;
		oFileDesc.pWrapper = this;

		std::string sFilenames[6] = { "./Textures/bkg1_back.png", "./Textures/bkg1_bot.png", "./Textures/bkg1_front.png", "./Textures/bkg1_left.png", "./Textures/bkg1_right.png", "./Textures/bkg1_top.png" };
		oDescriptorSet.oBuffers[1].pBuffer = Image::CreateCubeMap(sFilenames, oFileDesc);

		m_oInputDatasSky.push_back(oDescriptorSet);
		m_oAllMatricesSky.push_back((BasicBuffer*)oDescriptorSet.oBuffers[0].pBuffer);
	}
	m_pPool->WriteDescriptor(m_oInputDatasSky, m_pSkyboxPipeline->GetDescriptorSetLayout()[0]);

	std::cout << "matrix buffer created" << std::endl;
}


void BasicWrapper::CreateCommandBuffer()
{
	InitCommands();
}

void BasicWrapper::InitVerticesBuffers()
{
	RenderModel::Desc oDesc;
	oDesc.pFactory = m_pFactory;
	oDesc.pWrapper = this;
	oDesc.sFilenameModel = "./Models/viking_room.obj";
	oDesc.oFilenamesTextures = { "./Textures/viking_room.png" };
	oDesc.oModels = { new Transform(),new Transform() };
	oDesc.eFlag = RenderModel::eVerticesAttributes::E_UV | RenderModel::eVerticesAttributes::E_POSITIONS;

	m_pRenderModel = new RenderModel(oDesc);
	m_pRenderModel->ConvertToBuffer(m_pRenderModel->GetBufferFlags(), true, oDesc.pWrapper);

	m_oAllVertexBuffers.push_back(m_pRenderModel->GetVerticesBuffer());
	m_oAllVertexBuffers.push_back(m_pRenderModel->GetIndexesBuffer());

	oDesc.oFilenamesTextures = {};
	oDesc.sFilenameModel = { "./Models/cube.obj" };
	m_pRenderModelSky = new RenderModel(oDesc);
	m_pRenderModelSky->ConvertToBuffer(m_pRenderModelSky->GetBufferFlags(), true, oDesc.pWrapper);

	m_oAllVertexBuffersSky.push_back(m_pRenderModelSky->GetVerticesBuffer());
	m_oAllVertexBuffersSky.push_back(m_pRenderModelSky->GetIndexesBuffer());

	std::cout << "vertex buffer created" << std::endl;
}

void BasicWrapper::InitCommands()
{
	for (int i = 0; i < m_pSwapchain->GetImageViews().size(); i++)
	{
		CommandFactory::SubDrawDesc oSubSky;
		oSubSky.oDescriptorSet = m_oDescriptorsSky[i];
		oSubSky.pPipeline = m_pSkyboxPipeline;
		oSubSky.pVertexData = m_oAllVertexBuffersSky[0];
		oSubSky.pIndexData = m_oAllVertexBuffersSky[1];
		oSubSky.iInstanceCount = 1;

		CommandFactory::SubDrawDesc oSub;
		oSub.oDescriptorSet = *m_oInputDatas[i].pDescriptorSet;
		oSub.pPipeline = m_pPipeline;
		oSub.pVertexData = m_oAllVertexBuffers[0];
		oSub.pIndexData = m_oAllVertexBuffers[1];
		oSub.iInstanceCount = m_iInstanceCount;

		CommandFactory::DrawDesc oDesc;
		oDesc.oMultipleDraw = { oSubSky, oSub };
		oDesc.pRenderpass = m_pRenderpass;
		oDesc.pFramebuffer = m_oFramebuffers[i];

		m_oAllDrawCommands.push_back(m_pFactory->CreateDrawCommand(oDesc));
	}
	std::cout << "Draw commands created" << std::endl;
}

void BasicWrapper::InitFramebuffer()
{
	for (int i = 0; i < m_pSwapchain->GetImageViews().size(); i++)
	{
		Image::Desc oImgDesc;
		oImgDesc.bEnableMip = false;
		oImgDesc.eFormat = VK_FORMAT_D32_SFLOAT;
		oImgDesc.eProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		oImgDesc.eSampleFlag = VK_SAMPLE_COUNT_1_BIT;
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
		m_oAllDepths.push_back(pImg);

		Image::Desc oMultisampleDesc;
		oMultisampleDesc.bEnableMip = false;
		oMultisampleDesc.eFormat = m_pSwapchain->GetFormat();
		oMultisampleDesc.eSampleFlag = VK_SAMPLE_COUNT_1_BIT;
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
		oImages.push_back(m_pSwapchain->GetImageViews()[i]);
		oImages.push_back(*pImg->GetImageView());
		//oImages.push_back(*pImgMultisample->GetImageView());
		oDesc.pImageView = &oImages;
		oDesc.pRenderPass = m_pRenderpass;

		m_oFramebuffers.push_back(new Framebuffer(oDesc));
	}
	std::cout << "Framebuffer created" << std::endl;
}

BasicWrapper::MP BasicWrapper::GetMatricesSky()
{
	MP oOutput;

	oOutput.vModel = glm::mat4(1.0f);

	int iWidth, iHeight;
	m_pDevice->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);
	oOutput.vProjection = glm::perspective(glm::radians(45.0f), iWidth / (float)iHeight, 0.1f, 10.0f);
	oOutput.vProjection[1][1] *= -1;

	return oOutput;
}


void BasicWrapper::UpdateUniformBuffer(int iImageIndex)
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	std::vector<glm::mat4> oModels;
	oModels.resize(m_iInstanceCount);
	for (int i = 0; i < m_iInstanceCount; i++)
	{
		glm::vec3 vTranslate(0.0001f * ( i + 1 ), 0, 0);
		m_pRenderModel->GetModels()[i]->SetPosition(vTranslate, true);
		oModels[i] = m_pRenderModel->GetModels()[i]->GetModelMatrix();
	}

	glm::mat4 mView = m_pCamera->GetViewMatrix();
	mView = glm::translate(mView, glm::vec3(0, 0, 0.1f * time));

	glm::mat4 mProj = m_pCamera->GetProjectionMatrix();
	std::vector<glm::mat4> oMat = { mView, mProj };
	m_oMPMatrices[iImageIndex]->CopyFromMemory(oMat.data(), m_pDevice);

	m_pRenderModel->GetModelMatrices()->CopyFromMemory(oModels.data(), m_pDevice);
}

bool BasicWrapper::Render(SyncObjects* pSync)
{
	int iFrame = pSync->GetFrame();
	vkWaitForFences(*GetDevice()->GetLogicalDevice(), 1, &pSync->GetInFlightFences()[iFrame], VK_TRUE, UINT64_MAX);

	uint32_t iImageIndex;
	VkResult eResult = vkAcquireNextImageKHR(*GetDevice()->GetLogicalDevice(), *GetSwapchain()->GetSwapchain(), UINT64_MAX, pSync->GetImageAcquiredSemaphore()[iFrame], VK_NULL_HANDLE, &iImageIndex);

	UpdateUniformBuffer(iImageIndex);

	if (eResult == VK_ERROR_OUT_OF_DATE_KHR)
	{
		ResizeWindow();
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

	VkSubmitInfo oSubmit{};
	oSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	oSubmit.waitSemaphoreCount = 1;
	oSubmit.pWaitSemaphores = &pSync->GetImageAcquiredSemaphore()[pSync->GetFrame()];
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	oSubmit.pWaitDstStageMask = waitStages;
	oSubmit.commandBufferCount = 1;
	oSubmit.pCommandBuffers = &m_oAllDrawCommands[iImageIndex];
	oSubmit.signalSemaphoreCount = 1;
	oSubmit.pSignalSemaphores = &pSync->GetRenderFinishedSemaphore()[pSync->GetFrame()];

	vkResetFences(*GetDevice()->GetLogicalDevice(), 1, &pSync->GetInFlightFences()[pSync->GetFrame()]);
	if (vkQueueSubmit( *GetDevice()->GetGraphicQueue(), 1, &oSubmit, pSync->GetInFlightFences()[pSync->GetFrame()]) != VK_SUCCESS )
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
	if (eResult == VK_ERROR_OUT_OF_DATE_KHR || eResult == VK_SUBOPTIMAL_KHR)
	{
		ResizeWindow();
	}
	else if (eResult != VK_SUCCESS)
	{
		return false;
	}

	pSync->NextFrame();
	return true;
}

void BasicWrapper::ResizeWindow()
{
}

BasicWrapper::~BasicWrapper()
{
	delete m_pSwapchain;
	delete m_pRenderpass;

	vkFreeCommandBuffers(*m_pDevice->GetLogicalDevice(), *m_pFactory->GetCommandPool(), m_oAllDrawCommands.size(), m_oAllDrawCommands.data());
	delete m_pPool;
	delete m_pPipeline;
	delete m_pFactory;
	delete m_pDevice;
	m_oInputDatas.clear();
	m_oAllVertexBuffers.clear();

	for (Buffer* pBuffer : m_oAllVertexBuffers)
	{
		delete pBuffer;
	}
	m_oAllVertexBuffers.clear();

	for (Image* pImage : m_oAllDepths)
	{
		delete pImage;
	}
	m_oAllDepths.clear();

	for (Framebuffer* pFramebuffer : m_oFramebuffers)
	{
		delete pFramebuffer;
	}
	m_oFramebuffers.clear();
	m_oDescriptors.clear();
}

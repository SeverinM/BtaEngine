#include "BasicWrapper.h"
#include "Globals.h"
#include <set>
#include "CommandFactory.h"
#include "SyncObjects.h"
#include <iostream>
#include "RenderPass.h"
#include <chrono>

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

BasicWrapper::MVP BasicWrapper::GetMatrices()
{
	MVP oOutput;
	oOutput.vModel = glm::mat4(1.0f);
	oOutput.vView = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	int iWidth, iHeight;
	m_pDevice->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);
	oOutput.vProjection = glm::perspective(glm::radians(45.0f), iWidth / (float)iHeight, 0.1f, 10.0f);
	oOutput.vProjection[1][1] *= -1;

	return oOutput;
}

BasicWrapper::MP BasicWrapper::GetMatricesSky()
{
	MP oOutput;
	
	glm::mat4 oMat = glm::mat4(1.0f);
	oOutput.vModel = glm::rotate(oMat, 45.0f, glm::vec3(0.4f, 0.1f, 1.2f));

	int iWidth, iHeight;
	m_pDevice->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);
	oOutput.vProjection = glm::perspective(glm::radians(45.0f), iWidth / (float)iHeight, 0.1f, 10.0f);
	oOutput.vProjection[1][1] *= -1;

	return oOutput;
}

void BasicWrapper::CreateGraphicPipeline()
{
	//Setup inputs
	int iSwapCount = m_pSwapchain->GetImageViews().size();

	DescriptorPool::BufferDesc oBufferDesc;
	oBufferDesc.eType = DescriptorPool::E_UNIFORM_BUFFER;

	DescriptorPool::BufferDesc oBufferDesc2;
	oBufferDesc2.eType = DescriptorPool::E_TEXTURE;

	std::vector<DescriptorPool::BufferDesc> oLayout;
	oLayout.push_back(oBufferDesc);
	oLayout.push_back(oBufferDesc2);

	m_oPrototype.push_back(oLayout);

	DescriptorPool::BufferDesc oBufferDescSky;
	oBufferDescSky.eType = DescriptorPool::E_UNIFORM_BUFFER;

	DescriptorPool::BufferDesc oBufferDesc2Sky;
	oBufferDesc2Sky.eType = DescriptorPool::E_TEXTURE;

	std::vector<DescriptorPool::BufferDesc> oLayoutSky;
	oLayoutSky.push_back(oBufferDescSky);
	oLayoutSky.push_back(oBufferDesc2Sky);

	m_oPrototypeSkybox.push_back(oLayout);

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

	m_pPool->CreateDescriptorSet(m_oDescriptors, iSwapCount, m_pPipeline->GetDescriptorSetLayout()[0]);

	MVP oMatrices = GetMatrices();

	for (int i = 0; i < iSwapCount; i++)
	{
		DescriptorPool::UpdateSubDesc oDescriptorSet;
		oDescriptorSet.pDescriptorSet = &m_oDescriptors[i];
		oDescriptorSet.oBuffers = m_oPrototype[0];

		//0 -> Uniform buffer
		BasicBuffer::Desc oBuffer;
		oBuffer.iUnitSize = sizeof(glm::mat4);
		oBuffer.iUnitCount = 3;
		oBuffer.pWrapper = this;
		oBuffer.eUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		oBuffer.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		
		oDescriptorSet.oBuffers[0].pBuffer = new BasicBuffer(oBuffer);
		oDescriptorSet.oBuffers[0].pBuffer->CopyFromMemory(&oMatrices, GetModifiableDevice());

		//1 -> Texture
		oDescriptorSet.oBuffers[1].pBuffer = m_pModel->GetTexture();

		m_oInputDatas.push_back(oDescriptorSet);
		m_oAllMatrices.push_back((BasicBuffer*)oDescriptorSet.oBuffers[0].pBuffer);
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
	Model::Desc oDesc;
	oDesc.pFactory = m_pFactory;
	oDesc.pWrapper = this;
	oDesc.sModelPath = "./Models/viking_room.obj";
	oDesc.sTexturepath = "./Textures/viking_room.png";
	m_pModel = new Model(oDesc);
	
	std::unordered_map<Model::eVerticesAttributes, size_t> oInput;
	oInput[Model::E_POSITIONS] = sizeof(glm::vec3);
	oInput[Model::E_UV] = sizeof(glm::vec2);
	oInput[Model::E_COLOR] = 0;
	oInput[Model::E_NORMALS] = 0;

	Model::BuffersVertices oOutput = m_pModel->ConvertToBuffer(oInput, this, m_pFactory);
	m_oAllVertexBuffers.push_back(oOutput.pVertices);
	m_oAllVertexBuffers.push_back(oOutput.pIndices);

	oDesc.sModelPath = "./Models/cube.obj";
	oDesc.sTexturepath = "";
	oDesc.bEnableMip = false;
	m_pSkyModel = new Model(oDesc);

	Model::BuffersVertices oOutput2 = m_pSkyModel->ConvertToBuffer(oInput, this, m_pFactory);
	m_oAllVertexBuffersSky.push_back(oOutput2.pVertices);
	m_oAllVertexBuffersSky.push_back(oOutput2.pIndices);

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

		CommandFactory::SubDrawDesc oSub;
		oSub.oDescriptorSet = *m_oInputDatas[i].pDescriptorSet;
		oSub.pPipeline = m_pPipeline;
		oSub.pVertexData = m_oAllVertexBuffers[0];
		oSub.pIndexData = m_oAllVertexBuffers[1];

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

void BasicWrapper::UpdateUniformBuffer(int iImageIndex)
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	MVP oMatrices;
	oMatrices.vModel = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	oMatrices.vView = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	int iWidth, iHeight;
	m_pDevice->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);
	oMatrices.vProjection = glm::perspective(glm::radians(45.0f), iWidth / (float)iHeight, 0.1f, 10.0f);
	oMatrices.vProjection[1][1] *= -1;

	m_oAllMatrices[iImageIndex]->CopyFromMemory(&oMatrices, m_pDevice);
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
	int iWidth, iHeight;
	GetModifiableDevice()->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);
	while (iWidth == 0 || iHeight == 0)
	{
		GetModifiableDevice()->GetModifiableRenderSurface()->GetWindowSize(iWidth, iHeight);
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(*GetDevice()->GetLogicalDevice());

	for (Image* pImage : m_oAllDepths)
	{
		delete pImage;
	}
	m_oAllDepths.clear();

	for (Image* pImage : m_oAllMultisample)
	{
		delete pImage;
	}
	m_oAllMultisample.clear();

	for (Framebuffer* pFramebuffer : m_oFramebuffers)
	{
		delete pFramebuffer;
	}
	m_oFramebuffers.clear();

	vkFreeCommandBuffers(*GetDevice()->GetLogicalDevice(), *m_pFactory->GetCommandPool(), m_oAllDrawCommands.size(), m_oAllDrawCommands.data());

	std::vector<Resizable*> oAllResizable = { m_pPipeline, m_pRenderpass, m_pSwapchain, m_pPool };

	for (DescriptorPool::UpdateSubDesc& oUpdate : m_oInputDatas)
	{
		for (DescriptorPool::BufferDesc oBuffer : oUpdate.oBuffers)
		{
			delete oBuffer.pBuffer;
		}
		oUpdate.oBuffers.clear();
	}
	m_oInputDatas.clear();

	for (Resizable* pResizable : oAllResizable)
	{
		pResizable->Free();
	}

	m_pSwapchain->Recreate(iWidth, iHeight, nullptr);
	m_pRenderpass->Recreate(iWidth, iHeight, nullptr);
	InitFramebuffer();
	m_pPool->Recreate(iWidth, iHeight, nullptr);
	FillDescriptorsBuffer();
	m_pPipeline->Recreate(iWidth, iHeight, nullptr);
	m_oAllDrawCommands.clear();
	CreateCommandBuffer();
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

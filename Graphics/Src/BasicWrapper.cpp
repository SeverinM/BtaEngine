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
#include "Globals.h"
#include "RenderBatch.h"
#include "ShaderTags.h"
#include "Font.h"
#include "FontRenderBatch.h"

bool BasicWrapper::s_bFramebufferResized(false);

void BasicWrapper::CreateInstance()
{
	m_bAdd = false;
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

	if (vkCreateInstance(&oCreateInfo, nullptr, &Graphics::Globals::g_oInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("Cannot create instance");
	}

	Window::RenderSurface::Desc oDesc;
	oDesc.iHeight = 1000;
	oDesc.iWidth = 1000;
	oDesc.sWindowName = "Bta";
	oDesc.pInstance = &Graphics::Globals::g_oInstance;
	oDesc.pCallback = BasicWrapper::ResizeWindow;
	m_pDesc->pSurface = new Window::RenderSurface(oDesc);
	std::cout << "Instance created" << std::endl;
}

void BasicWrapper::CreateGraphicDevice()
{
	GraphicDevice::Desc oDesc;
	oDesc.bEnableAnisotropy = true;
	oDesc.pInstance = &Graphics::Globals::g_oInstance;
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
	oCamDesc.mInitialMatrix = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0.0f, 0.0f, 1.0f));

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

	RenderPass::SubDesc oDebugDesc;
	oDebugDesc.iColorAttachmentIndex = 0;
	oDebugDesc.iDepthStencilAttachmentIndex = -1;
	oDebugDesc.iColorResolveAttachmentIndex = -1;

	RenderPass::SubDesc oTextDesc;
	oTextDesc.iColorAttachmentIndex = 0;
	oTextDesc.iDepthStencilAttachmentIndex = -1;
	oTextDesc.iColorResolveAttachmentIndex = -1;

	RenderPass::Desc oDebugRenderPassDesc;
	oDebugRenderPassDesc.eSample = VK_SAMPLE_COUNT_1_BIT;
	oDebugRenderPassDesc.pWrapper = this;
	oDebugRenderPassDesc.bEnableColor = true;
	oDebugRenderPassDesc.bEnableDepth = false;
	oDebugRenderPassDesc.oSubpasses = { oDebugDesc, oTextDesc };
	oDebugRenderPassDesc.eInitialLayoutColorAttachment = VK_IMAGE_LAYOUT_UNDEFINED;
	oDebugRenderPassDesc.bClearColorAttachmentAtBegin = false;
	oDebugRenderPassDesc.bPresentable = false;

	m_pDebugRenderpass = new RenderPass(oDebugRenderPassDesc);

	std::cout << "Renderpass created" << std::endl;
}

void BasicWrapper::CreateGraphicPipeline()
{
	RenderBatchesHandler::CreationBatchDesc oCreationBatch;
	oCreationBatch.bTestDepth = true;
	oCreationBatch.bWriteDepth = true;
	oCreationBatch.oShaderCompiled = { "./Shader/vert.spv","./Shader/frag.spv" };
	oCreationBatch.oShaderSources = { "./Shader/Src/vs.vert", "./Shader/Src/fs.frag" };

	RenderBatchesHandler::CreationBatchDesc oCreationBatchSky;
	oCreationBatchSky.bTestDepth = false;
	oCreationBatchSky.bWriteDepth = false;
	oCreationBatchSky.oShaderCompiled = { "./Shader/Skybox/vert.spv","./Shader/Skybox/frag.spv" };
	oCreationBatchSky.oShaderSources = { "./Shader/Skybox/Src/vs.vert", "./Shader/Skybox/Src/fs.frag" };
	
	RenderBatchesHandler::Desc oBatchesHandler;
	oBatchesHandler.eSamples = VK_SAMPLE_COUNT_8_BIT;
	oBatchesHandler.pPass = m_pRenderpass;
	oBatchesHandler.pWrapper = this;
	oBatchesHandler.pFactory = m_pFactory;
	oBatchesHandler.oBatches = { oCreationBatchSky, oCreationBatch };
	m_pHandler = new RenderBatchesHandler(oBatchesHandler);

	RenderBatchesHandler::CreationBatchDesc oCreationBatchDebug;
	oCreationBatchDebug.bTestDepth = false;
	oCreationBatchDebug.bWriteDepth = false;
	oCreationBatchDebug.eTopology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	oCreationBatchDebug.oShaderCompiled = { "./Shader/Debug/vert.spv", "./Shader/Debug/frag.spv" };
	oCreationBatchDebug.oShaderSources = { "./Shader/Debug/Src/vs.vert", "./Shader/Debug/Src/fs.frag" };
	oCreationBatchDebug.sTag = DEBUG_TAG;

	RenderBatchesHandler::CreationBatchDesc oCreationTextDebug;
	oCreationTextDebug.bTestDepth = false;
	oCreationTextDebug.bWriteDepth = false;
	oCreationTextDebug.oShaderCompiled = { "./Shader/Text/vert.spv", "./Shader/Text/frag.spv" };
	oCreationTextDebug.oShaderSources = { "./Shader/Text/Src/vs.vert", "./Shader/Text/Src/fs.frag" };
	oCreationTextDebug.sTag = TEXT_TAG;
	oCreationTextDebug.eTypeBatch = RenderBatchesHandler::TEXT;

	DescriptorPool::Desc oDescPool;
	oDescPool.iImageCount = (int)m_pSwapchain->GetImageViews().size();
	oDescPool.iSize = 1000;
	oDescPool.iMaxSet = 150;
	oDescPool.pWrapper = this;
	m_pPool = new DescriptorPool(oDescPool);
	Graphics::Globals::g_pPool = m_pPool;

	RenderBatchesHandler::Desc oBatchesHandlerDebug;
	oBatchesHandlerDebug.eSamples = VK_SAMPLE_COUNT_1_BIT;
	oBatchesHandlerDebug.pPass = m_pDebugRenderpass;
	oBatchesHandlerDebug.pWrapper = this;
	oBatchesHandlerDebug.pFactory = m_pFactory;
	oBatchesHandlerDebug.oBatches = { oCreationBatchDebug, oCreationTextDebug };
	m_pDebugHandler = new RenderBatchesHandler(oBatchesHandlerDebug);

	glm::mat4 vTrans = glm::mat4(1.0f);
	vTrans[3][0] = 0;
	vTrans[3][1] = 500.0f;
	FontRenderBatch::TextInstance* pInstance = ((FontRenderBatch*)m_pDebugHandler->GetRenderBatch(1))->AddText(std::string("test"), glm::vec4(1, 1, 1, 1), vTrans);

	Mesh::Desc oMeshDesc;
	oMeshDesc.pFactory = m_pFactory;
	oMeshDesc.pWrapper = this;
	oMeshDesc.sFilenameModel = "./Models/viking_room.obj";
	oMeshDesc.oModels = { std::shared_ptr<Transform>(new Transform()), std::shared_ptr<Transform>(new Transform()) };
	oMeshDesc.oModels[1]->SetPosition(glm::vec3(2.5f, 0, 0), true);
	oMeshDesc.eFlag = Mesh::eVerticesAttributes::E_UV | Mesh::eVerticesAttributes::E_POSITIONS;

	m_xMesh = Mesh::StrongPtr( new Mesh(oMeshDesc) );
	m_xMesh->ConvertToVerticesBuffer(m_xMesh->GetBufferFlags(), true, oMeshDesc.pWrapper);

	RenderBatch* pMainHandler = (RenderBatch*)m_pHandler->GetRenderBatch(1);
	Pipeline* pMainPipeline = m_pHandler->GetPipeline(1);

	pMainHandler->AddMesh(m_xMesh, pMainPipeline->GetDescriptorSetLayout()->InstantiateDescriptorSet(*m_pPool, *m_pDevice));

	RenderBatch* pSkyHandler = (RenderBatch*)m_pHandler->GetRenderBatch(0);
	Pipeline* pSkyPipeline = m_pHandler->GetPipeline(0);

	oMeshDesc.sFilenameModel = { "./Models/cube.obj" };
	oMeshDesc.oModels = { std::shared_ptr<Transform>(new Transform()) };
	oMeshDesc.eFlag = Mesh::eVerticesAttributes::E_POSITIONS;

	m_xMeshSky = Mesh::StrongPtr( new Mesh(oMeshDesc) );
	m_xMeshSky->ConvertToVerticesBuffer(m_xMeshSky->GetBufferFlags(), true, this);
	pSkyHandler->AddMesh(m_xMeshSky, pSkyPipeline->GetDescriptorSetLayout()->InstantiateDescriptorSet(*m_pPool, *m_pDevice));

	oMeshDesc.oPositions = { glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0,0,1) };
	oMeshDesc.oUVs = { glm::vec2(0,0) , glm::vec2(1,0) , glm::vec2(0,0) , glm::vec2(0,1) };
	oMeshDesc.oIndexes = { 0,1,3, 1,2,3, 2,0,3, 0,1,2 };

	oMeshDesc.oModels = { std::shared_ptr<Transform>(new Transform()) };
	oMeshDesc.eFlag = Mesh::eVerticesAttributes::E_UV | Mesh::eVerticesAttributes::E_POSITIONS;
	m_xCubeMesh = Mesh::StrongPtr(new Mesh(oMeshDesc));
	m_xCubeMesh->ConvertToVerticesBuffer(m_xCubeMesh->GetBufferFlags(), true, this);
	m_xCubeMesh->GetTransforms()[0]->SetPosition(glm::vec3(-1), true);
	pMainHandler->AddMesh(m_xCubeMesh, pMainPipeline->GetDescriptorSetLayout()->InstantiateDescriptorSet(*m_pPool, *m_pDevice));

	oMeshDesc.oModels = { std::shared_ptr<Transform>(new Transform()) };
	m_xCubeMeshChild = Mesh::StrongPtr(new Mesh(oMeshDesc));
	m_xCubeMeshChild->ConvertToVerticesBuffer(m_xCubeMeshChild->GetBufferFlags(), true, this);
	m_xCubeMeshChild->GetTransforms()[0]->SetPosition(glm::vec3(0), false);
	m_xCubeMesh->GetTransforms()[0]->AddChild(m_xCubeMeshChild->GetTransforms()[0]);
	pMainHandler->AddMesh(m_xCubeMeshChild, pMainPipeline->GetDescriptorSetLayout()->InstantiateDescriptorSet(*m_pPool, *m_pDevice));

	DelayedCommands::QueueCommands oCmds;
	oCmds.oOnStart = [this]()
	{
		RenderBatch* pMainHandler = (RenderBatch*)m_pHandler->GetRenderBatch(1);
		Pipeline* pMainPipeline = m_pHandler->GetPipeline(1);
		pMainHandler->AddMesh(m_xCubeMeshChild, pMainPipeline->GetDescriptorSetLayout()->InstantiateDescriptorSet(*m_pPool, *m_pDevice));
	};

	oCmds.oTimeOutFunction = [this]()
	{
		RenderBatch* pBatch = (RenderBatch*)m_pHandler->GetRenderBatch(1);
		Pipeline* pMainPipeline = m_pHandler->GetPipeline(1);
		pBatch->RemoveMesh(m_xCubeMeshChild);
	};
	m_oCommandsQueue.PushCommand(oCmds, 5.0f);

	oCmds.oOnStart = [this]()
	{};

	oCmds.oTimeOutFunction = [pInstance, this]()
	{
		pInstance->sText = std::string("Severin");
		m_pDebugHandler->MarkAllAsDirty();
	};
	m_oCommandsQueue.PushCommand(oCmds, 2.0f);

	oCmds.oOnStart = [this]()
	{};

	oCmds.oTimeOutFunction = [pInstance, this]()
	{
		pInstance->sText = std::string("Seve");
		m_pDebugHandler->MarkAllAsDirty();
	};
	m_oCommandsQueue.PushCommand(oCmds, 4.0f);

	InitFramebuffer();
	FillDescriptorsBuffer();
}

void BasicWrapper::FillDescriptorsBuffer()
{
	std::vector<glm::mat4> oVP{ m_pCamera->GetViewMatrix(), m_pCamera->GetProjectionMatrix() };
	std::shared_ptr<BasicBuffer> xVPMatrice(m_pCamera->GetVPMatriceBuffer());
	xVPMatrice->CopyFromMemory(oVP.data(), GetModifiableDevice());
	
	DescriptorSetWrapper* pMainRender = ((RenderBatch*) m_pHandler->GetRenderBatch(1))->GetDescriptor(m_xMesh);

	pMainRender->FillSlotAtTag(m_pCamera->GetVPMatriceBuffer().get(), TAG_VP);

	Image::FromFileDesc oFileDesc;
	oFileDesc.bEnableMip = true;
	oFileDesc.eAspect = VK_IMAGE_ASPECT_COLOR_BIT;
	oFileDesc.eFormat = VK_FORMAT_R8G8B8A8_SRGB;
	oFileDesc.eSampleFlag = VK_SAMPLE_COUNT_1_BIT;
	oFileDesc.eTiling = VK_IMAGE_TILING_OPTIMAL;
	oFileDesc.pFactory = m_pFactory;
	oFileDesc.pWrapper = this;

	pMainRender->FillSlotAtTag(Image::CreateFromFile("./Textures/viking_room.png", oFileDesc), TAG_COLORMAP);

	pMainRender->CommitSlots(m_pPool);

	DescriptorSetWrapper* pMainRenderGun = ((RenderBatch*) m_pHandler->GetRenderBatch(1))->GetDescriptor(m_xCubeMesh);
	pMainRenderGun->FillSlotAtTag(m_pCamera->GetVPMatriceBuffer().get(), TAG_VP);
	pMainRenderGun->FillSlotAtTag(Image::CreateFromFile("./Textures/test.png", oFileDesc), TAG_COLORMAP);
	pMainRenderGun->CommitSlots(m_pPool);

	DescriptorSetWrapper* pMainRenderGunChild = ((RenderBatch*) m_pHandler->GetRenderBatch(1))->GetDescriptor(m_xCubeMeshChild);

	if (pMainRenderGunChild)
	{
		pMainRenderGunChild->FillSlotAtTag(m_pCamera->GetVPMatriceBuffer().get(), TAG_VP);
		pMainRenderGunChild->FillSlotAtTag(Image::CreateFromFile("./Textures/test.png", oFileDesc), TAG_COLORMAP);
		pMainRenderGunChild->CommitSlots(m_pPool);
	}

	std::vector<glm::mat4> oMPMatrices = { glm::mat4(1.0f), m_pCamera->GetProjectionMatrix() };

	Image::FromFileDesc oFileDescSky;
	oFileDescSky.bEnableMip = false;
	oFileDescSky.eAspect = VK_IMAGE_ASPECT_COLOR_BIT;
	oFileDescSky.eFormat = VK_FORMAT_R8G8B8A8_SRGB;
	oFileDescSky.eSampleFlag = VK_SAMPLE_COUNT_1_BIT;
	oFileDescSky.eTiling = VK_IMAGE_TILING_OPTIMAL;
	oFileDescSky.pFactory = m_pFactory;
	oFileDescSky.pWrapper = this;
	std::string sFilenames[6] = { "./Textures/bkg1_right.png", "./Textures/bkg1_left.png", "./Textures/bkg1_top.png", "./Textures/bkg1_bot.png", "./Textures/bkg1_front.png", "./Textures/bkg1_back.png" };
	Image* pImage = Image::CreateCubeMap(sFilenames, oFileDescSky);

	DescriptorSetWrapper* pRenderSky = ((RenderBatch*)m_pHandler->GetRenderBatch(0))->GetDescriptor(m_xMeshSky);

	BasicBuffer::Desc oBuffer;
	oBuffer.iUnitSize = sizeof(glm::mat4);
	oBuffer.iUnitCount = 2;
	oBuffer.pWrapper = this;
	oBuffer.eUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	oBuffer.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	m_pSkyBuffer = new BasicBuffer(oBuffer);
	pRenderSky->FillSlotAtTag(m_pSkyBuffer, TAG_MP);

	m_mModel = glm::mat4(1.0f);
	m_pSkyBuffer->CopyFromMemory(&m_mModel, m_pDevice, 0, sizeof(glm::mat4));
	m_pSkyBuffer->CopyFromMemory(&m_pCamera->GetProjectionMatrix(),m_pDevice, sizeof(glm::mat4), sizeof(glm::mat4));

	pRenderSky->FillSlotAtTag(pImage, TAG_COLORMAP);
	
	pRenderSky->CommitSlots(m_pPool);

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
	InitImGui();
}

void BasicWrapper::InitBatch()
{
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

		Framebuffer::Desc oDebugDesc;
		oDebugDesc.pGraphicDevice = m_pDevice;
		oDebugDesc.pRenderPass = m_pDebugRenderpass;

		std::vector<VkImageView> oImagesDebug = { m_pSwapchain->GetImageViews()[i] };
		oDebugDesc.pImageView = &oImagesDebug;
		m_oFramebuffersDebug.push_back(new Framebuffer(oDebugDesc));
	}

	std::cout << "Framebuffer created" << std::endl;
}

bool BasicWrapper::Render(SyncObjects* pSync)
{
	m_oCommandsQueue.Update(Graphics::Globals::g_fElapsed);
	m_xMesh->GetTransforms()[1]->Rotate(glm::vec3(0, 0, 1), 0.1f);

	glm::mat4 mCam = m_pCamera->GetViewMatrix();
	mCam[3][0] = 0.0f;
	mCam[3][1] = 0.0f;
	mCam[3][2] = 0.0f;
	m_pSkyBuffer->CopyFromMemory(&mCam, m_pDevice, 0, sizeof(glm::mat4));


	auto start = std::chrono::system_clock::now();
	VkResult eResult = VK_SUCCESS;
	if (!s_bFramebufferResized)
	{
		int iFrame = pSync->GetFrame();
		vkWaitForFences(*GetDevice()->GetLogicalDevice(), 1, &pSync->GetInFlightFences()[iFrame], VK_TRUE, UINT64_MAX);

		if (m_bAdd)
		{
			vkDeviceWaitIdle(*m_pDevice->GetLogicalDevice());

			BasicBuffer::Desc oDesc;
			oDesc.eUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			oDesc.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			oDesc.iUnitSize = sizeof(glm::mat4);
			oDesc.iUnitCount = 1;
			oDesc.pWrapper = this;
			std::shared_ptr<BasicBuffer> xBuffer = std::shared_ptr<BasicBuffer>(new BasicBuffer(oDesc));

			glm::mat4 mInitial(1.0f);
			BufferedTransform* pTrsf = new BufferedTransform(mInitial, 0, std::static_pointer_cast<Buffer>(xBuffer), m_pDevice);
			pTrsf->SetScale(glm::vec3(0.5f), true);
			pTrsf->SetPosition(glm::vec3(std::rand() % 10, std::rand() % 10, std::rand() % 10));

			std::vector<std::shared_ptr<BufferedTransform>> oTransforms = m_xMesh->GetTransforms();
			oTransforms.push_back(std::shared_ptr<BufferedTransform>(pTrsf));
			m_xMesh->SetTransforms(oTransforms, this);

			RenderBatch* pBatch = (RenderBatch*)m_pHandler->GetRenderBatch(1);
			pBatch->GetDescriptor(m_xMesh)->FillSlot(1,m_xMesh->GetModelMatrices().get());
			pBatch->GetDescriptor(m_xMesh)->CommitSlots(m_pPool);
			m_pHandler->MarkAllAsDirty();
			m_bAdd = false;
		}

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

		/*if (pSync->GetSwapChainImagesFences()[iImageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(*GetDevice()->GetLogicalDevice(), 1, &pSync->GetSwapChainImagesFences()[iImageIndex], VK_TRUE, UINT64_MAX);
		}
		pSync->GetSwapChainImagesFences()[iImageIndex] = pSync->GetInFlightFences()[iFrame];*/

		ImGuiWrapper::Desc oDesc;
		oDesc.iImageIndex = iImageIndex;
		oDesc.pWrapper = this;

		VkSubmitInfo oSubmit{};
		oSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		oSubmit.waitSemaphoreCount = 1;
		oSubmit.pWaitSemaphores = &pSync->GetImageAcquiredSemaphore()[pSync->GetFrame()];
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkCommandBuffer* pCmd = m_pDebugHandler->GetCommand(m_oFramebuffersDebug[iImageIndex]);
		std::vector<VkCommandBuffer> oCmds = { *m_pHandler->GetCommand( m_oFramebuffers[iImageIndex] ),*m_pDebugHandler->GetCommand(m_oFramebuffersDebug[iImageIndex]), *m_pImGui->GetDrawCommand(oDesc) };

		oSubmit.pWaitDstStageMask = waitStages;
		oSubmit.commandBufferCount = (uint32_t)oCmds.size();
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
	Graphics::Globals::g_fElapsed = std::chrono::duration<float>(end - start).count();
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
	ImGui::Text("FPS : %i", (int)(1.0f / Graphics::Globals::g_fElapsed));
	ImGui::Text("Instances rendered : %i", pWrapper->m_pHandler->GetInstancesCount());
	ImGui::Text("Vertices count : %i", pWrapper->m_pHandler->GetVerticesCount());
	ImGui::Text("Camera position : %f / %f / %f", vPos.x, vPos.y, vPos.z);
	ImGui::Text("Camera forward : %f / %f / %f", vForward.x, vForward.y, vForward.z);
	ImGui::SliderFloat("Move speed camera", &pWrapper->m_pCamera->GetModifiableMoveSpeed(), 1.0f, 100.0f);
	ImGui::SliderFloat("Rotate speed camera", &pWrapper->m_pCamera->GetModifiableRotateSpeed(), 50.0f, 5000.0f);
	if (ImGui::Button("Toggle skybox"))
	{
		pWrapper->m_pHandler->MarkAllAsDirty();
		pWrapper->m_pHandler->GetRenderBatch(0)->SetEnabled(!pWrapper->m_pHandler->GetRenderBatch(0)->IsEnabled());
	}
	if (ImGui::Button("Toggle models"))
	{
		pWrapper->m_pHandler->MarkAllAsDirty();
		pWrapper->m_pHandler->GetRenderBatch(1)->SetEnabled(!pWrapper->m_pHandler->GetRenderBatch(1)->IsEnabled());
	}

	if (ImGui::Button("Add random room"))
	{
		pWrapper->m_bAdd = true;
	}

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

	int i = 0;
	while (m_pHandler->GetRenderBatch(i) != nullptr)
	{
		m_pHandler->GetRenderBatch(i)->MarkAsDirty();
		i++;
	}

	m_oAllDepths.clear();
	m_oAllMultisample.clear();

	for (Framebuffer* pFramebuffer : m_oFramebuffers)
	{
		delete pFramebuffer;
	}
	m_oFramebuffers.clear();

	delete m_pSwapchain;
	delete m_pImGui;

	//Recreate swap chain
	Swapchain::Desc oDesc;
	oDesc.eColorspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	oDesc.eImagesFormat = VK_FORMAT_B8G8R8A8_SRGB;
	oDesc.ePresentMode = VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR;
	oDesc.pGraphicWrapper = this;
	oDesc.iImageLayers = 1;

	m_pSwapchain = new Swapchain(oDesc);

	m_pHandler->ReconstructPipelines(this);

	InitFramebuffer();
	InitImGui();
}

BasicWrapper::~BasicWrapper()
{
	delete m_pSwapchain;
	delete m_pRenderpass;

	delete m_pPool;
	delete m_pFactory;

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

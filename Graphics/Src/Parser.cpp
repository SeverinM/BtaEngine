#include "Parser.h"
#include <fstream>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <sstream>
#include <iostream>
#include "RenderSurface.h"
#include "Globals.h"
#include "GraphicDevice.h"
#include "CommandFactory.h"
#include "StringUtils.h"
#include "Output.h"

namespace Bta
{
	namespace Graphic
	{
		bool Parser::s_bInitialized(false);

		Image::Desc Parser::FetchTextureTemplate(std::string s, Context& c)
		{
			Image::Desc oDesc;

			std::vector<std::string> oSubStrings = Bta::Utils::StringUtils::Split(s, ':');

			if (oSubStrings.size() < 2)
			{
				throw std::runtime_error("No id found");
			}

			int iId = std::stoi(oSubStrings[1]);
			std::string sTag = oSubStrings[0];

			for (TextureTemplate& oTemplate : m_oTemplates)
			{
				if (oTemplate.sTag == sTag && oTemplate.iId == iId)
				{
					oDesc.eFormat = oTemplate.eFormat;
					oDesc.bEnableMip = oTemplate.bMipMabEnabled;
					oDesc.iLayerCount = oTemplate.iLayers;
					break;
				}
			}

			oDesc.bIsCubemap = false;
			oDesc.eAspect = c.eAspect;
			oDesc.eSampleFlag = VK_SAMPLE_COUNT_1_BIT;
			oDesc.eTiling = VK_IMAGE_TILING_OPTIMAL;

			for (int i = 2; i < oSubStrings.size(); i++)
			{
				std::string s = oSubStrings[i];

				if (s == "MultiSample")
				{
					oDesc.eSampleFlag = c.eSample;
				}
			}

			return oDesc;
		}

		VkFormat Parser::ParseFormat(std::string sValue)
		{
			if (sValue == "D32_FLOAT")
			{
				return VK_FORMAT_D32_SFLOAT;
			}
			else if (sValue == "B8G8R8A8_SRGB")
			{
				return VK_FORMAT_B8G8R8A8_SRGB;
			}
			throw std::runtime_error("No format found");
			return VK_FORMAT_D32_SFLOAT;
		}

		VkPresentModeKHR Parser::ParsePresentMode(std::string sValue)
		{
			if (sValue == "Mailbox")
			{
				return VK_PRESENT_MODE_MAILBOX_KHR;
			}
			throw std::runtime_error("Error finding a present mode");
			return VK_PRESENT_MODE_MAILBOX_KHR;
		}

		VkColorSpaceKHR Parser::ParseColorSpace(std::string sValue)
		{
			if (sValue == "SRGB_NONLINEAR_KHR")
			{
				return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			}
			throw std::runtime_error("Error parsing color space");
			return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		}

		Parser::Parser(Desc& oDesc)
		{
			std::ifstream oInput(oDesc.sFileName);
			std::stringstream oStringStream;
			std::string sString;
			if (oInput)
			{
				oStringStream << oInput.rdbuf();
				oInput.close();
				sString = oStringStream.str();
			}
			else
			{
				throw std::runtime_error("Error");
			}
			m_oDocument.Parse(sString.c_str());
		}

		void Parser::InitGlobals()
		{
			if (s_bInitialized)
				return;

			Value& oValue = m_oDocument["Application"];
			Window::RenderSurface::Init();

			VkApplicationInfo oAppInfo{};
			oAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			oAppInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
			oAppInfo.applicationVersion = oAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			oAppInfo.pApplicationName = oValue["Name"].GetString();
			oAppInfo.pEngineName = oValue["EngineName"].GetString();

			VkInstanceCreateInfo oInstanceCreateInfo{};
			oInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			oInstanceCreateInfo.pApplicationInfo = &oAppInfo;
			if (oValue["Instance"]["EnableDebug"].GetBool())
			{
				const char* sLayerName = "VK_LAYER_KHRONOS_validation";
				oInstanceCreateInfo.enabledLayerCount = 1;
				oInstanceCreateInfo.ppEnabledLayerNames = &sLayerName;

				VkDebugUtilsMessengerCreateInfoEXT oDebugCreateInfo{};
				oDebugCreateInfo.pfnUserCallback = Bta::Graphic::DebugCallback;
				oDebugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
				oDebugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
				oDebugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
				oInstanceCreateInfo.pNext = &oDebugCreateInfo;
			}
			else
			{
				oInstanceCreateInfo.enabledLayerCount = 0;
			}

			uint32_t iGlfwExtensionsCount = 0;
			const char** pGlfwExtensions;
			pGlfwExtensions = glfwGetRequiredInstanceExtensions(&iGlfwExtensionsCount);
			std::vector<const char*> oExtensions(pGlfwExtensions, pGlfwExtensions + iGlfwExtensionsCount);

			if (oValue["Instance"]["EnableDebug"].GetBool())
			{
				oExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}

			oInstanceCreateInfo.ppEnabledExtensionNames = oExtensions.data();
			oInstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(oExtensions.size());

			if (vkCreateInstance(&oInstanceCreateInfo, nullptr, &Bta::Graphic::Globals::g_oInstance) != VK_SUCCESS)
			{
				throw std::runtime_error("Cannot create instance");
			}

			Window::RenderSurface::Desc oRenderDesc;
			oRenderDesc.iHeight = oValue["Outputs"][0]["Window"]["InitialHeight"].GetInt();
			oRenderDesc.iWidth = oValue["Outputs"][0]["Window"]["InitialWidth"].GetInt();
			oRenderDesc.sWindowName = oValue["Outputs"][0]["Window"]["WindowName"].GetString();
			oRenderDesc.pInstance = &Bta::Graphic::Globals::g_oInstance;
			oRenderDesc.pCallback = nullptr;
			//oDesc.pCallback = BasicWrapper::ResizeWindow;

			Swapchain::Desc oSwapDesc;
			oSwapDesc.eColorspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			oSwapDesc.eImagesFormat = VK_FORMAT_B8G8R8A8_SRGB;
			oSwapDesc.ePresentMode = VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR;
			oSwapDesc.iImageLayers = 1;

			Output::Desc oOutputDesc;
			oOutputDesc.pRenderSurface = new Window::RenderSurface(oRenderDesc);
			oSwapDesc.pRenderSurface = oOutputDesc.pRenderSurface;
			oOutputDesc.oSwapDesc = oSwapDesc;

			GraphicDevice::Desc oGraphicDeviceDesc;
			oGraphicDeviceDesc.bEnableAnisotropy = true;
			oGraphicDeviceDesc.pInstance = &Bta::Graphic::Globals::g_oInstance;
			oGraphicDeviceDesc.pSurface = oOutputDesc.pRenderSurface;

			std::vector<const char*> oDeviceExtensions;
			for (Value& oExtensionValue : oValue["GraphicDevice"]["Extensions"].GetArray())
			{
				oDeviceExtensions.push_back(oExtensionValue.GetString());
			}
			oGraphicDeviceDesc.oExtensions = oDeviceExtensions;
			oGraphicDeviceDesc.bEnableAnisotropy = oValue["GraphicDevice"]["Features"]["Anisotropy"].GetBool();
			Bta::Graphic::Globals::g_pDevice = new GraphicDevice(oGraphicDeviceDesc);
			Bta::Graphic::Globals::g_pOutput = new Output(oOutputDesc);

			CommandFactory::Desc oFactoryDesc{};
			oFactoryDesc.bResettable = oValue["CommandFactory"]["Resettable"].GetBool();
			Bta::Graphic::Globals::g_pFactory = new CommandFactory(oFactoryDesc);

			DescriptorPool::Desc oPoolDesc;
			oPoolDesc.iMaxSet = oValue["DescriptorPool"]["MaxSet"].GetInt();
			oPoolDesc.iSize = oValue["DescriptorPool"]["Size"].GetInt();
			Bta::Graphic::Globals::g_pPool = new DescriptorPool(oPoolDesc);

			s_bInitialized = true;
		}

		template <int Nb>
		std::vector<float> Parser::ParseVector(rapidjson::Value& oValue)
		{
			if (!oValue.IsArray())
				return std::vector<float>();

			std::vector<float> oOutput;
			oOutput.resize(Nb);

			for (int i = 0; i < Nb; i++)
			{
				oOutput[i] = oValue[i].GetFloat();
			}

			return oOutput;
		}

		Swapchain::Desc Parser::ParseSwapChain()
		{
			Value& oValue = m_oDocument["Application"]["Outputs"][0]["Swapchain"];

			Swapchain::Desc oDesc;
			oDesc.iImageLayers = oValue["Layers"].GetInt();
			oDesc.ePresentMode = ParsePresentMode(oValue["PresentMode"].GetString());
			oDesc.eColorspace = ParseColorSpace(oValue["ColorSpace"].GetString());
			oDesc.eImagesFormat = ParseFormat(oValue["ImageFormat"].GetString());

			TextureTemplate oTemplate;
			oTemplate.iId = oValue["Id"].GetInt();
			oTemplate.sTag = "Swapchain";
			oTemplate.eFormat = ParseFormat(oValue["ImageFormat"].GetString());
			oTemplate.bMipMabEnabled = false;
			oTemplate.iLayers = oValue["Layers"].GetInt();
			m_oTemplates.push_back(oTemplate);

			return oDesc;
		}

		void Parser::RecordTemplates()
		{
			Value& oValue = m_oDocument["Application"];

			Value::MemberIterator pIterator = oValue.FindMember("TextureTemplate");
			if (pIterator != oValue.MemberEnd())
			{
				TextureTemplate oTemplate;
				oTemplate.iId = pIterator->value["Id"].GetInt();
				oTemplate.eFormat = ParseFormat(pIterator->value["Format"].GetString());
				oTemplate.sTag = "TextureTemplate";
				oTemplate.bMipMabEnabled = pIterator->value["MipEnable"].GetBool();
				oTemplate.iLayers = pIterator->value["Layers"].GetInt();
				m_oTemplates.push_back(oTemplate);
			}
		}

		RenderPass::Desc Parser::ParseRenderPass()
		{
			Value& oValue = m_oDocument["Application"]["Outputs"][0]["RenderPass"];
			Value& oSubValue = oValue["Subpasses"];

			std::vector<RenderPass::SubDesc> oSubPasses;
			for (Value& oVal : oSubValue.GetArray())
			{
				uint16_t iMask = oVal["AttachmentMask"].GetInt();

				RenderPass::SubDesc oSubDesc;
				oSubDesc.iColorAttachmentIndex = (1 & iMask ? 0 : -1);
				oSubDesc.iDepthStencilAttachmentIndex = ((1 << 1) & iMask ? 1 : -1);
				oSubDesc.iColorResolveAttachmentIndex = ((1 << 2) & iMask ? 2 : -1);
				oSubPasses.push_back(oSubDesc);
			}

			RenderPass::Desc oDesc;
			//oDesc.bPresentable = oValue["Presentable"].GetBool();
			/*oDesc.bClearColorAttachmentAtBegin = (1 & oValue["ClearAttachmentMask"].GetInt() ? true : false);
			oDesc.bEnableColor = (1 & oValue["AttachmentMask"].GetInt() ? true : false);
			oDesc.bEnableDepth = ((1 << 1) & oValue["AttachmentMask"].GetInt() ? true : false);
			oDesc.oSubpasses = oSubPasses;*/

			return oDesc;
		}
	}
}

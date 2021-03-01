#ifndef H_PARSER
#define H_PARSER
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <string>
#include <vulkan/vulkan_core.h>
#include "Buffer.h"
#include "RenderPass.h"
#include "Swapchain.h"

namespace Bta
{
	namespace Graphic
	{
		using namespace rapidjson;

		class Parser
		{
		public:
			struct Desc
			{
				std::string sFileName;
			};
			Parser(Desc& oDesc);
			void InitGlobals();
			Swapchain::Desc ParseSwapChain();
			void RecordTemplates();
			RenderPass::Desc ParseRenderPass();

			struct TextureTemplate
			{
				int iId;
				VkFormat eFormat;
				std::string sTag;
				bool bMipMabEnabled;
				int iLayers;
			};

			struct Context
			{
				VkSampleCountFlagBits eSample;
				VkImageAspectFlags eAspect;
			};

		protected:
			Document m_oDocument;
			static bool s_bInitialized;
			std::vector<TextureTemplate> m_oTemplates;
			Image::Desc FetchTextureTemplate(std::string s, Context& c);

			template <int Nb>
			std::vector<float> ParseVector(rapidjson::Value& oValue);

			VkFormat ParseFormat(std::string sValue);
			VkPresentModeKHR ParsePresentMode(std::string sValue);
			VkColorSpaceKHR ParseColorSpace(std::string sValue);
		};
	}
}

#endif

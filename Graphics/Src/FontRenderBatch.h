#ifndef H_FONT_RENDER_BATCH
#define H_FONT_RENDER_BATCH
#include "AbstractRenderBatch.h"
#include "GLM/glm.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

class FontRenderBatch : public AbstractRenderBatch
{
public:
	struct Desc
	{
		RenderPass* pRenderpass;
		Pipeline* pPipeline;
		std::string sFontName;
		CommandFactory* pFactory;
		DescriptorPool* pPool;
		uint16_t iFilterMVP;
	};
	FontRenderBatch(Desc& oDesc);

	struct TextInstance
	{
		std::string sText;
		bool bDirty;
		glm::mat4 vTransform;
		glm::vec4 vColor;

	};

	TextInstance* AddText(std::string pText, glm::vec4 vColor, glm::mat4 vTransform);
	VkCommandBuffer* GetDrawCommand(Framebuffer* pFramebuffer) override;
	void ChainSubpass(VkCommandBuffer* pCommand) override;
	void Destroy() override { };
	size_t GetInstancesCount() override;
	size_t GetVerticesCount() override
	{
		return GetInstancesCount() * 6;
	}

	protected:
		struct CharacterUnit
		{
			Image* pImage;
			glm::ivec2 vSize;
			glm::ivec2 vBearing;
			unsigned int iAdvance;
			DescriptorSetWrapper* pDescriptorSet;
			BasicBuffer* oMVPfilter;
		};
		std::unordered_map<char, CharacterUnit> m_oCacheTextures;

		std::vector<TextInstance*> m_oAllInstances;
		std::vector<BasicBuffer*> m_oBuffers;

		static void Init()
		{
			if (FT_Init_FreeType(&s_oFt))
			{
				std::cout << "Error initializing free type" << std::endl;
			}
			s_bIsInitialized = true;
		}
		static bool s_bIsInitialized;
		static FT_Library s_oFt;
		FT_Face m_oFace;

		DescriptorPool* m_pPool;
};

#endif H_FONT_RENDER_BATCH
#ifndef H_FONT_RENDER_BATCH
#define H_FONT_RENDER_BATCH
#include "RenderBatch.h"
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
		std::shared_ptr<BasicBuffer> xVPBuffer;
	};
	FontRenderBatch(Desc& oDesc);

	struct TextInstance
	{
		std::string sText;
		glm::vec4 vColor;
		RenderBatchesHandler* pHandler;
		BufferedTransform* pBufferedTransform;
		std::vector<DescriptorSetWrapper*> oDescriptorSet;

		void SetText(std::string sNewText)
		{
			sText = sNewText;
			pHandler->MarkAllAsDirty();
		}

		void SetTransform(glm::mat4 mNewTransform)
		{
			pBufferedTransform->ForceMatrix(mNewTransform);
			pHandler->MarkAllAsDirty();
		}

		void SetColor(glm::vec4 vNewColor)
		{
			vColor = vNewColor;
			pHandler->MarkAllAsDirty();
		}

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
	std::shared_ptr<BasicBuffer> m_xVP;

	void SetCamera(std::shared_ptr<Camera> xCam);

	protected:
		struct CharacterUnit
		{
			Image* pImage;
			glm::ivec2 vSize;
			glm::ivec2 vBearing;
			unsigned int iAdvance;
		};
		std::unordered_map<char, CharacterUnit> m_oCacheTextures;

		std::shared_ptr<Camera> m_xCam;
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
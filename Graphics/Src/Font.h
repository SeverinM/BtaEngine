#ifndef H_FONT
#define H_FONT
#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>
#include "GLM/glm.hpp"
#include <unordered_map>
#include "BasicWrapper.h"

class Font
{
public:

	struct Desc
	{
		RenderPass* pRenderpass;
		Pipeline* pPipeline;
		std::string sFontName;
		GraphicWrapper* pWrapper;
		CommandFactory* pFactory;
		DescriptorPool* pPool;
	};

	Font(Desc& oDesc);

	struct Character
	{
		Image* pImage;
		glm::ivec2 vSize;
		glm::ivec2 vBearing;
		unsigned int iAdvance;
		DescriptorSetWrapper* pDescriptorSet;
	};
	VkCommandBuffer GetDrawCommand(std::string sText,Framebuffer* pFramebuffer, float x, float y, float fScale);

private:
	RenderPass* m_pRenderpass;
	Pipeline* m_pPipeline;
	GraphicWrapper* m_pWrapper;
	CommandFactory* m_pFactory;
	DescriptorPool* m_pPool;
	std::vector<BasicBuffer*> m_oVerticesBuffer;

	FT_Face m_oFace;
	std::unordered_map<char, Character> m_oCacheTextures;
	std::unordered_map<Framebuffer*, VkCommandBuffer*> m_oCacheCommand;

	static void Init();
	static bool s_bIsInitialized;
	static FT_Library s_oFt;
	static BasicBuffer* s_pProjectionBuffer;
};

#endif
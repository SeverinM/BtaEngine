#ifndef H_FONT_RENDER_BATCH
#define H_FONT_RENDER_BATCH
#include "RenderBatch.h"
#include "GLM/glm.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Globals.h"

namespace Bta
{
	namespace Graphic
	{
		class FontRenderBatch : public AbstractRenderBatch
		{
			public:
				struct Desc
				{
					RenderPass* pRenderpass;
					Pipeline* pPipeline;
					std::string sFontName;
					DescriptorPool* pPool;
					std::shared_ptr<BasicBuffer> xVPBuffer;
				};
				FontRenderBatch(Desc& oDesc);

				struct TextInstance
				{
					std::string sText;
					glm::vec4 vColor;
					BufferedTransform* pBufferedTransform;
					std::vector<DescriptorSetWrapper*> oDescriptorSet;

					FontRenderBatch* m_pBatch;

					void SetText(std::string sNewText)
					{
						for (int i = 0; i < glm::max(sNewText.size(), sText.size()); i++)
						{
							if (i < sText.size())
							{
								if (i < sNewText.size())
								{
									oDescriptorSet[i]->FillSlot(0, m_pBatch->m_oCacheTextures[sNewText[i]].pImage);
									oDescriptorSet[i]->CommitSlots(Bta::Graphic::Globals::g_pPool);
								}
								else
								{
									delete oDescriptorSet[i];
								}
							}
							else
							{
								oDescriptorSet.push_back(m_pBatch->m_pPipeline->GetDescriptorSetLayout()->InstantiateDescriptorSet(*Bta::Graphic::Globals::g_pPool, *Bta::Graphic::Globals::g_pDevice));
								oDescriptorSet[i]->FillSlot(0, m_pBatch->m_oCacheTextures[sText[i]].pImage);
								oDescriptorSet[i]->FillSlot(1, m_pBatch->m_xVP.get());
								oDescriptorSet[i]->FillSlot(2, pBufferedTransform->GetBuffer());
							}
						}
						sText = sNewText;
						m_pBatch->m_pParent->MarkAllAsDirty();
					}

					void SetTransform(glm::mat4 mNewTransform)
					{
						pBufferedTransform->ForceMatrix(mNewTransform);
						m_pBatch->m_pParent->MarkAllAsDirty();
					}

					void SetColor(glm::vec4 vNewColor)
					{
						vColor = vNewColor;
						m_pBatch->m_pParent->MarkAllAsDirty();
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
	}
}

#endif H_FONT_RENDER_BATCH
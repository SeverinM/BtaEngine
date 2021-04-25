#ifndef H_MATERIAL_COMPONENT
#define H_MATERIAL_COMPONENT

#include "../../Core/Src/Entity.h"
#include "DescriptorWrapper.h"
#include <unordered_map>
#include "Texture.h"
#include "GPUMemoryBinding.h"
#include "Globals.h"

namespace Bta
{
	namespace Graphic
	{
		class MaterialComponent : public Core::AbstractComponent
		{
			private:
				std::unordered_map<std::string, Texture*> m_oAllTextures;
				std::unordered_map<std::string, GPUMemoryBinding> m_oAllVariables;
				DescriptorSetWrapper* m_pWrapper;

			public:
				MaterialComponent(DescriptorSetWrapper* pWrapper, Core::Entity* pOwner)
				{
					m_pWrapper = pWrapper;
				}

				void CommitChange()
				{
					m_pWrapper->CommitSlots(Globals::g_pPool);
				}

				void Init() override {};

				void AddTexture(Texture* pText, std::string sName)
				{
					if (FetchTexture(sName) != nullptr)
					{
						printf("Slot %s already taken", sName.c_str());
					}

					m_oAllTextures[sName] = pText;

					if ( pText != nullptr )
						m_pWrapper->FillSlotAtTag( std::dynamic_pointer_cast<Buffer>(pText->GetImage()).get(), sName);
				}

				void AddGPUMemory(GPUMemoryBinding oBinding, std::string sName)
				{
					if (FetchGPUMemoryBinding(sName) != nullptr)
					{
						printf("Slot %s already taken", sName.c_str());
					}

					m_oAllVariables.insert({ {sName, oBinding} });

					std::shared_ptr<Buffer>xBuffer = oBinding.GetBuffer();
					m_pWrapper->FillSlotAtTag(xBuffer.get(), sName, oBinding.GetOffset());
				}

				Texture* FetchTexture(std::string sName)
				{
					if (m_oAllTextures.count(sName) == 0)
					{
						return nullptr;
					}

					return m_oAllTextures[sName];
				}

				GPUMemoryBinding* FetchGPUMemoryBinding(std::string sName)
				{
					if (m_oAllVariables.count(sName) == 0)
					{
						return nullptr;
					}

					return &m_oAllVariables[sName];
				}

				void UpdateDescriptor()
				{
					m_pWrapper->CommitSlots(Globals::g_pPool);
				}
		};
	}
}

#endif 

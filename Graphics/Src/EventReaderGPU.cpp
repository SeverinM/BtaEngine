#include "EventReaderGPU.h"
#include "GPUMemoryInterface.h"
#include "../../Core/Src/MeshComponent.cpp"
#include "Globals.h"
#include "../../Core/Src/EventHolder.cpp"

namespace Bta
{
	namespace Graphic
	{
		void EventReaderGPU::Update()
		{
			Core::EventHolder* pHolder = Core::EventHolder::GetInstance();
			for (int i = 0; i < 10; i++)
			{
				CheckNewComponent();
				CheckVerticeBufferChange();
			}
		}

		void EventReaderGPU::CheckNewComponent()
		{
			Core::EventHolder* pHolder = Core::EventHolder::GetInstance();
			Core::BaseEvent oEvent = pHolder->PeekEvent(Core::E_NEW_COMPONENT);
			if (oEvent.pSubject == nullptr)
				return;

			Core::MeshComponent* pMeshComponent = dynamic_cast<Core::MeshComponent*>(oEvent.pSubject);

			if (pMeshComponent)
			{
				BasicBuffer::Desc oDesc;
				oDesc.iUnitCount = 20;
				oDesc.iUnitSize = pMeshComponent->GetVerticeSize();
				oDesc.eUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				oDesc.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				GPUMemory::GetInstance()->AllocateMemory(oDesc, pMeshComponent);
				pHolder->ConsumeEvent(Core::E_NEW_COMPONENT);
			}
		}

		void EventReaderGPU::CheckVerticeBufferChange()
		{
			Core::EventHolder* pHolder = Core::EventHolder::GetInstance();
			Core::BaseEvent oEvent = pHolder->PeekEvent(Core::E_MODIFIED_VERTICE_BUFFER);
			if (oEvent.pSubject != nullptr)
			{
				Core::MeshUpdateEvent* pMeshUpdate = static_cast<Core::MeshUpdateEvent*>(&oEvent);
				Core::MeshComponent* pMeshComponent = dynamic_cast<Core::MeshComponent*>(oEvent.pSubject);

				BasicBuffer* pBuffer = static_cast<BasicBuffer*>(GPUMemory::GetInstance()->FetchBuffer(oEvent.pSubject));
				if (pBuffer != nullptr)
				{
					if (pMeshUpdate->iIndex >= pMeshComponent->GetVerticeCount())
					{
						pBuffer->Reallocate(pMeshUpdate->iIndex, pMeshComponent->GetVerticeSize());
					}
					else
					{
						uint64_t iOffset = pMeshComponent->GetOffsetRawData(pMeshUpdate->eChannel, pMeshUpdate->iIndex);
						pBuffer->CopyFromMemory(pMeshUpdate->pNewData, Globals::g_pDevice, iOffset, pMeshComponent->m_oComponentsSize[pMeshUpdate->eChannel]);
					}
				}
				else
				{
					printf("Non existing buffer");
				}
			}
		}

	}
}
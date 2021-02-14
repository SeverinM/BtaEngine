#include "GraphicUtils.h"

namespace Bta
{
	namespace Utils
	{
		Graphic::RenderBatch* GraphicUtils::s_pBatch(nullptr);
		DelayedCommands* GraphicUtils::s_pDelay(nullptr);

		void Bta::Utils::GraphicUtils::DisplayDebugSphere(float fRadius, glm::vec4 vColor, float fDuration, Graphic::BasicWrapper* pWrapper)
		{
			Graphic::AbstractRenderBatch* pBatch = pWrapper->GetHandler()->FindRenderBatch(DEBUG_TAG);

			if (pBatch != nullptr)
			{
			}
		}

		void Bta::Utils::GraphicUtils::DisplayDebugLine(glm::vec3 vStart, glm::vec3 vEnd, glm::vec4 vColor, float fDuration)
		{
			if (s_pBatch != nullptr)
			{
				Graphic::Mesh::Desc oDesc;
				oDesc.oPositions = { vStart, vEnd };
				oDesc.oColors = { vColor, vColor };
				oDesc.eFlag = Graphic::Mesh::E_COLOR | Graphic::Mesh::E_POSITIONS;
				oDesc.oModels = { std::shared_ptr<Graphic::Transform>(new Graphic::Transform()) };

				Graphic::Mesh::StrongPtr xMesh = Graphic::Mesh::StrongPtr(new Graphic::Mesh(oDesc));
				xMesh->ConvertToVerticesBuffer(xMesh->GetBufferFlags(), false);

				DelayedCommands::QueueCommands oCmds;
				oCmds.oOnStart = [xMesh]()
				{
					s_pBatch->AddMesh(xMesh, s_pBatch->GetPipeline()->GetDescriptorSetLayout()->InstantiateDescriptorSet(*Bta::Graphic::Globals::g_pPool, *Bta::Graphic::Globals::g_pDevice));
					s_pBatch->GetDescriptor(xMesh)->FillSlotAtTag(Bta::Graphic::Globals::g_pCamera->GetVPMatriceBuffer().get(), TAG_VP);
					s_pBatch->GetDescriptor(xMesh)->CommitSlots(Bta::Graphic::Globals::g_pPool);
					s_pBatch->GetParent()->MarkAllAsDirty();
				};

				oCmds.oTimeOutFunction = [xMesh]()
				{
					vkDeviceWaitIdle(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice());
					s_pBatch->RemoveMesh(xMesh);
					s_pBatch->GetParent()->MarkAllAsDirty();
				};

				if (fDuration < 0)
				{
					oCmds.oOnStart();
				}
				else
				{
					s_pDelay->PushCommand(oCmds, fDuration);
				}
			}
		}
	}
}
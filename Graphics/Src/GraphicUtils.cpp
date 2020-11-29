#include "GraphicUtils.h"

RenderBatch* Bta::Utils::GraphicUtils::s_pBatch(nullptr);
CommandFactory* Bta::Utils::GraphicUtils::s_pFactory(nullptr);
DelayedCommands* Bta::Utils::GraphicUtils::s_pDelay(nullptr);

void Bta::Utils::GraphicUtils::DisplayDebugSphere(float fRadius, glm::vec4 vColor, float fDuration, BasicWrapper* pWrapper)
{
	AbstractRenderBatch* pBatch = pWrapper->GetHandler()->FindRenderBatch(DEBUG_TAG);

	if (pBatch != nullptr)
	{
	}
}

void Bta::Utils::GraphicUtils::DisplayDebugLine(glm::vec3 vStart, glm::vec3 vEnd, glm::vec4 vColor, float fDuration)
{
	if (s_pBatch != nullptr)
	{
		Mesh::Desc oDesc;
		oDesc.oPositions = { vStart, vEnd };
		oDesc.oColors = { vColor, vColor };
		oDesc.pFactory = s_pFactory;
		oDesc.eFlag = Mesh::E_COLOR | Mesh::E_POSITIONS;
		oDesc.oModels = { std::shared_ptr<Transform>(new Transform()) };

		Mesh::StrongPtr xMesh = Mesh::StrongPtr(new Mesh(oDesc));
		xMesh->ConvertToVerticesBuffer(xMesh->GetBufferFlags(), false);

		DelayedCommands::QueueCommands oCmds;
		oCmds.oOnStart = [xMesh]()
		{
			s_pBatch->AddMesh(xMesh, s_pBatch->GetPipeline()->GetDescriptorSetLayout()->InstantiateDescriptorSet(*Graphics::Globals::g_pPool, *Graphics::Globals::g_pDevice));
			s_pBatch->GetDescriptor(xMesh)->FillSlotAtTag(Graphics::Globals::g_pCamera->GetVPMatriceBuffer().get(), TAG_VP);
			s_pBatch->GetDescriptor(xMesh)->CommitSlots(Graphics::Globals::g_pPool);
			s_pBatch->GetParent()->MarkAllAsDirty();
		};

		oCmds.oTimeOutFunction = [xMesh]()
		{
			vkDeviceWaitIdle(*Graphics::Globals::g_pDevice->GetLogicalDevice());
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
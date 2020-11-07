#ifndef H_GRAPHIC_UTILS
#define H_GRAPHIC_UTILS
#include "BasicWrapper.h"
#include "GLM/glm.hpp"
#include "RenderBatch.h"
#include "ShaderTags.h"

namespace Bta
{
	namespace Utils
	{
		class GraphicUtils
		{
		public:

			static void DisplayDebugSphere(float fRadius, glm::vec4 vColor, float fDuration, BasicWrapper* pWrapper)
			{
				RenderBatch* pBatch = pWrapper->GetHandler()->FindRenderBatch(DEBUG_TAG);

				if (pBatch != nullptr)
				{
				}
			}

			static void DisplayDebugLine(glm::vec3 vStart, glm::vec3 vEnd, glm::vec4 vColor, float fDuration, BasicWrapper* pWrapper)
			{
				RenderBatch* pBatch = pWrapper->GetHandler()->FindRenderBatch(DEBUG_TAG);

				if (pBatch != nullptr)
				{
					Mesh::Desc oDesc;
					oDesc.oPositions = { vStart, vEnd };
					oDesc.oColors = { vColor, vColor };
					oDesc.pFactory = pWrapper->GetFactory();
					oDesc.eFlag = Mesh::E_COLOR | Mesh::E_POSITIONS;
					oDesc.oModels = { std::shared_ptr<Transform>(new Transform()) };
					oDesc.pWrapper = pWrapper;

					Mesh::StrongPtr xMesh = Mesh::StrongPtr(new Mesh(oDesc));
					xMesh->ConvertToVerticesBuffer(xMesh->GetBufferFlags(), false, pWrapper);

					DelayedCommands::QueueCommands oCmds;
					oCmds.oOnStart = [pWrapper, pBatch, xMesh]()
					{
						pBatch->AddMesh(xMesh, pBatch->GetPipeline()->GetDescriptorSetLayout()->InstantiateDescriptorSet(*pWrapper->GetDescriptorPool(), *pWrapper->GetDevice()));
						pBatch->GetDescriptor(xMesh)->FillSlotAtTag(pWrapper->GetCamera()->GetVPMatriceBuffer().get(), TAG_VP);
						pBatch->GetDescriptor(xMesh)->CommitSlots(pWrapper->GetDescriptorPool());
						pWrapper->GetHandler()->MarkAllAsDirty();
					};

					oCmds.oTimeOutFunction = [pWrapper, pBatch, xMesh]()
					{
						vkDeviceWaitIdle(*pWrapper->GetModifiableDevice()->GetLogicalDevice());
						pBatch->RemoveMesh(xMesh);
						pWrapper->GetHandler()->MarkAllAsDirty();
					};

					if (fDuration < 0)
					{
						oCmds.oOnStart();
					}
					else
					{
						pWrapper->GetDelayedCommands()->PushCommand(oCmds, fDuration);
					}
				}
			}
		};
	}
}

#endif

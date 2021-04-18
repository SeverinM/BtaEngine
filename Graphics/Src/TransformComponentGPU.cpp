#include "./TransformComponentGPU.h"
#include "./CameraComponent.h"

namespace Bta
{
	namespace Graphic
	{
		void TransformComponentGPU::RefreshGPUMemory()
		{
			m_pModelMatrix->SetValue<glm::mat4x4>(m_pTransformComponent->GetModelMatrix());

			CameraComponent* pCamComponent = m_pOwner->FindFirstComponent<CameraComponent>();
			if (pCamComponent != nullptr)
				pCamComponent->RefreshGPUBindingV();
		}

	}
}
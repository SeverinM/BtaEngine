#include "./TransformComponentGPU.h"
#include "./CameraComponent.h"

namespace Bta
{
	namespace Graphic
	{
		void TransformComponentGPU::SetPosition(glm::vec3 vPosition, bool bRelative)
		{
			m_pTransformComponent->SetPosition(vPosition, bRelative);
			m_pModelMatrix->SetValue<glm::mat4x4>(m_pTransformComponent->GetModelMatrix());

			CameraComponent* pCamComponent = m_pOwner->FindFirstComponent<CameraComponent>();
			if (pCamComponent != nullptr)
				pCamComponent->RefreshGPUBindingV();
		}

		void TransformComponentGPU::SetRotation(glm::vec3 vAngleAxis, float fValue, bool bRelative, bool bWorldAxis)
		{
			m_pTransformComponent->SetRotation(vAngleAxis,fValue,bRelative, bWorldAxis);
			m_pModelMatrix->SetValue<glm::mat4x4>(m_pTransformComponent->GetModelMatrix());

			CameraComponent* pCamComponent = m_pOwner->FindFirstComponent<CameraComponent>();
			if (pCamComponent != nullptr)
				pCamComponent->RefreshGPUBindingV();
		}

	}
}
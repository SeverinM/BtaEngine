#include "./TransformComponentGPU.h"

namespace Bta
{
	namespace Graphic
	{
		void TransformComponentGPU::SetPosition(glm::vec3 vPosition, bool bRelative)
		{
			m_pTransformComponent->SetPosition(vPosition, bRelative);
			m_pModelMatrix->SetValue<glm::mat4x4>(m_pTransformComponent->GetModelMatrix());
		}

		void TransformComponentGPU::SetRotation(glm::vec3 vRotation, bool bRelative)
		{
			m_pTransformComponent->SetRotation(vRotation, bRelative);
			m_pModelMatrix->SetValue<glm::mat4x4>(m_pTransformComponent->GetModelMatrix());
		}
	}
}
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
	}
}
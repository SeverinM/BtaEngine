#include "TransformComponent.h"
#include "GLM/gtx/rotate_normalized_axis.hpp"
#include "GLM/gtx/projection.hpp"
#include "GLM/gtx/vector_angle.hpp"
#include <cmath>

namespace Bta
{
	namespace Core
	{
		glm::vec3 TransformComponent::GetLocalDirection(glm::vec3 vWorldDirection)
		{
			glm::vec4 vInput = glm::mat4_cast(m_vLocalQuat) * glm::vec4(vWorldDirection.x,vWorldDirection.y,vWorldDirection.z,0);
			return glm::vec3(vInput.x, -vInput.y, vInput.z);
		}

		glm::vec3 TransformComponent::GetWorldScale() const
		{
			glm::vec3 vScale = m_vLocalScale;
			const Entity* pParent = m_pOwner->GetParent();
			if (pParent == nullptr)
				return vScale;

			const TransformComponent* pParentTransform = pParent->FindFirstComponent<TransformComponent>();
			vScale += pParentTransform->GetWorldScale();
			return vScale;
		}

		glm::vec3 TransformComponent::GetWorldPosition() const
		{
			glm::vec3 vPosition = m_vLocalPosition;
			const Entity* pParent = m_pOwner->GetParent();
			if (pParent == nullptr)
				return m_vLocalPosition;

			const TransformComponent* pParentTransform = pParent->FindFirstComponent<TransformComponent>();
			vPosition += pParentTransform->GetWorldPosition();
			return vPosition;
		}

		glm::quat TransformComponent::GetWorldRotation() const
		{
			return m_vLocalQuat;
		}

		glm::mat4x4 TransformComponent::GetModelMatrix() const
		{
			glm::mat4 vTranslate = glm::translate(glm::mat4(1.0), GetWorldPosition());
			glm::mat4 vRotate = glm::mat4_cast(GetWorldRotation());
			glm::mat4 vScale = glm::scale(glm::mat4(1.0), GetWorldScale());
			return vTranslate * vRotate * vScale;
		}

		void TransformComponent::SetPosition(glm::vec3 vNewPosition, bool bRelative)
		{
			if (bRelative)
				m_vLocalPosition += vNewPosition;
			else
				m_vLocalPosition = vNewPosition;
		}

		void TransformComponent::SetRotation(glm::vec3 angleAxis, float value, bool bRelative, bool bWorldAxis)
		{
			if (bRelative)
			{
				if (bWorldAxis)
					m_vLocalQuat = glm::angleAxis(value, angleAxis) * m_vLocalQuat;
				else
					m_vLocalQuat = m_vLocalQuat * glm::angleAxis(value, angleAxis);
			}
			else
				m_vLocalQuat = glm::angleAxis(value, angleAxis);
		}

		void TransformComponent::SetForward(glm::vec3 vDirection)
		{
			glm::vec3 vProjX = glm::proj(vDirection, glm::vec3(0, 0, 1));
			vProjX.y = vDirection.y;
			float fAngleY = glm::angle(vDirection, vProjX);
			float fAngleX = glm::angle(vProjX, glm::vec3(0, 0, 1));

			m_vLocalQuat = glm::quat(glm::vec3(fAngleX, fAngleY, 0));
		}
	}
}

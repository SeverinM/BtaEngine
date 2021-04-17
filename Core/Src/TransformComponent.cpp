#include "TransformComponent.h"


namespace Bta
{
	namespace Core
	{
		glm::vec3 TransformComponent::GetLocalDirection(glm::vec3 vWorldDirection)
		{
			glm::vec4 vInput = glm::vec4(vWorldDirection.x,vWorldDirection.y,vWorldDirection.z,0) * glm::mat4_cast(m_vLocalQuat);
			return glm::vec3(vInput.x, vInput.y, vInput.z);
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
			glm::mat4x4 mOutput = glm::mat4x4(1.0f);
			mOutput = glm::scale(mOutput, GetWorldScale());
			mOutput = glm::translate(mOutput, GetWorldPosition());
			mOutput *= glm::mat4_cast(GetWorldRotation());
			return mOutput;
		}

		void TransformComponent::SetPosition(glm::vec3 vNewPosition, bool bRelative)
		{
			if (bRelative)
				m_vLocalPosition += vNewPosition;
			else
				m_vLocalPosition = vNewPosition;
		}

		void TransformComponent::SetRotation(glm::vec3 angleAxis, float value, bool bRelative)
		{
			if (bRelative)
				m_vLocalQuat *= glm::angleAxis(value, angleAxis);
			else
				m_vLocalQuat = glm::angleAxis(value, angleAxis);
		}
	}
}

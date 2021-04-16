#include "TransformComponent.h"


namespace Bta
{
	namespace Core
	{
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

		glm::vec3 TransformComponent::GetWorldRotation() const
		{
			glm::vec3 vOutput = m_vLocalRotation;

			const Entity* pParent = m_pOwner->GetParent();
			if (pParent == nullptr)
				return m_vLocalRotation;

			const TransformComponent* pParentTransform = pParent->FindFirstComponent<TransformComponent>();
			vOutput += pParentTransform->GetWorldRotation();
			return vOutput;
		}

		glm::mat4x4 TransformComponent::GetModelMatrix() const
		{
			glm::mat4x4 mOutput = glm::mat4x4(1.0f);
			mOutput = glm::scale(mOutput, GetWorldScale());
			mOutput = glm::translate(mOutput, GetWorldPosition());

			glm::vec3 vRotation = GetWorldRotation();
			mOutput = glm::rotate(mOutput, vRotation.x, glm::vec3(1, 0, 0));
			mOutput = glm::rotate(mOutput, vRotation.y, glm::vec3(0, 1, 0));
			mOutput = glm::rotate(mOutput, vRotation.z, glm::vec3(0, 0, 1));
			return mOutput;
		}

		void TransformComponent::SetPosition(glm::vec3 vNewPosition, bool bRelative)
		{
			if (bRelative)
				m_vLocalPosition += vNewPosition;
			else
				m_vLocalPosition = vNewPosition;
		}

		void TransformComponent::SetRotation(glm::vec3 vNewRotation, bool bRelative)
		{
			if (bRelative)
				m_vLocalRotation += vNewRotation;
			else
				m_vLocalPosition = vNewRotation;
		}

	}
}

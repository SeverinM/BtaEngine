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

		glm::quat TransformComponent::GetWorldRotation() const
		{
			glm::quat mOutput = glm::quat();

			const Entity* pParent = m_pOwner->GetParent();
			if (pParent == nullptr)
				return m_vLocalPosition;

			const TransformComponent* pParentTransform = pParent->FindFirstComponent<TransformComponent>();
			mOutput *= pParentTransform->GetWorldRotation();
			return mOutput;
		}

		glm::mat4x4 TransformComponent::GetModelMatrix() const
		{
			glm::mat4x4 mOutput = glm::mat4x4(1.0f);
			mOutput = glm::scale(mOutput, GetWorldScale());
			//mOutput *= GetWorldRotation();
			mOutput = glm::translate(mOutput, GetWorldPosition());
			return mOutput;
		}

		void TransformComponent::SetPosition(glm::vec3 vNewPosition, bool bRelative)
		{
			if (bRelative)
				m_vLocalPosition += vNewPosition;
			else
				m_vLocalPosition = vNewPosition;
		}
	}
}

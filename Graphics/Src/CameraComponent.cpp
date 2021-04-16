#include "CameraComponent.h"
#include "GPUMemoryInterface.h"

namespace Bta
{
	namespace Graphic
	{
		CameraComponent::CameraComponent(CameraComponent::Desc oDesc) :  m_pMemoryBindingV(nullptr), m_pMemoryBindingP(nullptr)
		{
			m_fFarPlane = oDesc.fFarPlane;
			m_fNearPlane = oDesc.fNearPlane;
			m_fFOVDegrees = oDesc.fFOVDegrees;
			m_fRatio = oDesc.fRatio;
			m_bEnablePerspective = oDesc.bEnablePerspective;
		}

		CameraComponent::~CameraComponent()
		{
			if (m_pMemoryBindingV != nullptr)
			{
				delete m_pMemoryBindingV->GetBuffer();
				delete m_pMemoryBindingV;
			}

			if (m_pMemoryBindingP != nullptr)
			{
				delete m_pMemoryBindingP->GetBuffer();
				delete m_pMemoryBindingP;
			}
		}

		void CameraComponent::Init()
		{
			m_pMemoryBindingV = new GPUMemoryBinding(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(glm::mat4x4));
			glm::mat4x4 mMat = GetViewMatrix();
			m_pMemoryBindingV->GetBuffer()->CopyFromMemory(&mMat);

			m_pMemoryBindingP = new GPUMemoryBinding(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(glm::mat4x4));
			mMat = GetProjectionMatrix();
			m_pMemoryBindingP->GetBuffer()->CopyFromMemory(&mMat);
		}

		glm::mat4x4 CameraComponent::GetViewMatrix()
		{
			Core::TransformComponent* pTransform = m_pOwner->FindFirstComponent<Core::TransformComponent>();
			glm::vec3 vDirection = glm::quat(pTransform->GetWorldRotation()) * glm::vec3(0, 0, 1);
			return glm::lookAt(pTransform->GetWorldPosition(), vDirection + pTransform->GetWorldPosition(), glm::vec3(0, 1, 0));
		}

		glm::mat4x4 CameraComponent::GetProjectionMatrix()
		{
			return glm::perspective(glm::radians(m_fFOVDegrees), m_fRatio, m_fNearPlane, m_fFarPlane);
		}

		void CameraComponent::RefreshGPUBindingV()
		{
			if (m_pMemoryBindingV != nullptr)
			{
				glm::mat4x4 mMat = GetViewMatrix();
				m_pMemoryBindingV->GetBuffer()->CopyFromMemory(&mMat);
			}
		}

	}
}
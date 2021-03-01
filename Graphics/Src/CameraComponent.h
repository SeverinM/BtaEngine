#ifndef H_CAMERA_COMPONENT
#define H_CAMERA_COMPONENT

#include "../../Core/Src/TransformComponent.h"
#include "GPUMemoryBinding.h"

namespace Bta
{
	namespace Graphic
	{
		class CameraComponent : public Core::AbstractComponent
		{
			private:
				float m_fFOVDegrees;
				float m_fNearPlane;
				float m_fFarPlane;
				float m_fRatio;
				bool m_bEnablePerspective;
				glm::vec3 m_vForward;

				GPUMemoryBinding* m_pMemoryBindingV;
				GPUMemoryBinding* m_pMemoryBindingP;

			public:

				struct Desc
				{
					float fFOVDegrees;
					float fNearPlane;
					float fFarPlane;
					float fRatio;
					bool bEnablePerspective;
				};

				CameraComponent(CameraComponent::Desc oDesc, Core::Entity* pOwner);
				~CameraComponent();

				void Init() override;
				glm::mat4x4 GetViewMatrix();
				glm::mat4x4 GetProjectionMatrix();
				GPUMemoryBinding* GetMemoryBindingV() { return m_pMemoryBindingV; }
				GPUMemoryBinding* GetMemoryBindingP() { return m_pMemoryBindingP; }
				void RefreshGPUBindingV();
		};
	}
}

#endif

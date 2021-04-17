#ifndef H_TRANSFORM_COMPONENT_GPU
#define H_TRANSFORM_COMPONENT_GPU

#include "../../Core/Src/TransformComponent.h"
#include "./GPUMemoryBinding.h"

namespace Bta
{
	namespace Graphic
	{
		class TransformComponentGPU : public Core::AbstractComponent
		{
			protected :
				GPUMemoryBinding* m_pModelMatrix;
				Core::TransformComponent* m_pTransformComponent;

			public:
				TransformComponentGPU() : AbstractComponent()
				{
					m_pModelMatrix = new GPUMemoryBinding(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(glm::mat4x4));
				}

				void Init() override
				{
					m_pTransformComponent = m_pOwner->FindFirstComponent<Core::TransformComponent>();
					m_pModelMatrix->SetValue<glm::mat4x4>(m_pTransformComponent->GetModelMatrix());
				}

				void SetPosition(glm::vec3 vPosition, bool bRelative);
				void SetRotation(glm::vec3 vAngleAxis, float fValue, bool bRelative);

				GPUMemoryBinding* GetMemoryBinding() { return m_pModelMatrix; }

				~TransformComponentGPU()
				{
					delete m_pModelMatrix;
				}
		};
	}
}

#endif
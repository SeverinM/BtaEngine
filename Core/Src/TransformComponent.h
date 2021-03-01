#ifndef H_TRANSFORM_COMPONENT
#define H_TRANSFORM_COMPONENT

#include "Entity.h"
#include "../Include/GLM/glm.hpp"
#include "../Include/GLM/gtc/quaternion.hpp"

namespace Bta
{
	namespace Core
	{
		class TransformComponent : public AbstractComponent
		{
		protected:
			glm::vec3 m_vLocalPosition;
			glm::quat m_vLocalRotation;
			glm::vec3 m_vLocalScale;

		public:
			void Init() override {};
			TransformComponent() : AbstractComponent(nullptr)
			{
				m_vLocalPosition = glm::vec3();
			};

			glm::vec3 GetWorldScale() const;
			glm::vec3 GetWorldPosition() const;
			glm::quat GetWorldRotation() const;
			glm::mat4x4 GetModelMatrix() const;
			void SetPosition(glm::vec3 vNewPosition, bool bRelative);
		};
	}
}

#endif

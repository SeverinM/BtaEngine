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
			glm::vec3 m_vLocalScale;
			glm::quat m_vLocalQuat;

		public:
			void Init() override {};
			TransformComponent()
			{
				m_vLocalPosition = glm::vec3();
				m_vLocalScale = glm::vec3(1);
				m_vLocalQuat = glm::angleAxis(0.0f, glm::vec3(0, 0, 1));
				m_vLocalQuat = glm::normalize(m_vLocalQuat);
			};

			glm::vec3 GetLocalDirection(glm::vec3 vWorldDirection);
			glm::vec3 GetWorldScale() const;
			glm::vec3 GetWorldPosition() const;
			glm::quat GetWorldRotation() const;
			glm::mat4x4 GetModelMatrix() const;
			virtual void SetPosition(glm::vec3 vNewPosition, bool bRelative);
			virtual void SetRotation(glm::vec3 angleAxis, float value, bool bRelative, bool bWorldAxis);

			virtual ~TransformComponent() {};
		};
	}
}

#endif

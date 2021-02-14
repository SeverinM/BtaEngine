#ifndef H_TRANSFORM_COMPONENT
#define H_TRANSFORM_COMPONENT

#include "Entity.h"
#include "../Include/GLM/glm.hpp"

namespace Bta
{
	namespace Core
	{
		class TransformComponent : public AbstractComponent
		{
		protected:
			glm::vec3 m_vLocalPosition;

		public:
			void Init() override {};
			TransformComponent() : AbstractComponent(nullptr)
			{
				m_vLocalPosition = glm::vec3();
			};

			glm::vec3 GetWorldPosition() const;
		};
	}
}

#endif

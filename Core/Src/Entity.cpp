#include "Entity.h"
#include "TransformComponent.h"

namespace Bta
{
	namespace Core
	{
		void Entity::GenerateID()
		{
			m_iID = 0;
		}

		Entity::Entity(Entity* pParent) : m_pParent(pParent)
		{
			GenerateID();
			TransformComponent* pTrsf = new TransformComponent();
			AddExistingComponent(pTrsf);
		}

		void Entity::AddChild(Entity* m_pChildEntity)
		{
			m_oChildrens.push_back(m_pChildEntity);
			m_pChildEntity->m_pParent = this;
		}

		void Entity::AddExistingComponent(AbstractComponent* pComponent)
		{
			m_oComponents.push_back(pComponent);
			pComponent->Init();
			pComponent->m_pOwner = this;
		}

		template<typename Type>
		std::vector<const Type*> Entity::FindAllComponents() const
		{
			std::vector<const Type*> oOutput;
			for (AbstractComponent* pComponent : m_oComponents)
			{
				const Type* pOutput = dynamic_cast<Type*>(pComponent);
				if (pOutput)
					oOutput.push_back(pOutput);
			}
			return oOutput;
		}
	}
}


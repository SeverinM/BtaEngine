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

		Entity::~Entity()
		{
			for (AbstractComponent* pComponent : m_oComponents)
			{
				delete pComponent;
			}
			m_oComponents.clear();
		}

		void Entity::AddChild(Entity* m_pChildEntity)
		{
			m_oChildrens.push_back(m_pChildEntity);
			m_pChildEntity->m_pParent = this;
		}

		void Entity::AddExistingComponent(AbstractComponent* pComponent)
		{
			m_oComponents.push_back(pComponent);
			pComponent->m_pOwner = this;

			if ( !pComponent->m_bInit )
				pComponent->Init();
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


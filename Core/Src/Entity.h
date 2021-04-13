#ifndef H_ENTITY
#define H_ENTITY

#include <vector>

namespace Bta
{
	namespace Core
	{
		class Entity;

		class AbstractComponent
		{
			friend Entity;
			protected:
				Entity* m_pOwner;
				bool m_bInit;

			public:
				virtual void Init(){}

				Entity* GetOwner()
				{
					return m_pOwner;
				}
		};


		class Entity
		{
			protected:
				Entity* m_pParent;
				std::vector<Entity*> m_oChildrens;
				std::vector<AbstractComponent*> m_oComponents;
				uint64_t m_iID;
				void GenerateID();

			public:
				Entity(Entity* pParent);
				void AddChild(Entity* m_pChildEntity);
				void AddExistingComponent(AbstractComponent* pComponent);

				template<typename Type>
				Type* FindFirstComponent() const;

				template<typename Type>
				std::vector<const Type*> FindAllComponents() const;

				inline uint64_t GetID() const { return m_iID; }
				inline const Entity* GetParent() const { return m_pParent; }
		};

		template<typename Type>
		Type* Entity::FindFirstComponent() const
		{
			Type* pOutput = nullptr;
			for (AbstractComponent* pComponent : m_oComponents)
			{
				pOutput = dynamic_cast<Type*>(pComponent);
				if (pOutput)
					return pOutput;
			}
			return nullptr;
		}

	}
}

#endif

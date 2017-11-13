
#pragma once

#include <core/action_listener.h>
#include <core/entity.h>
#include <core/entity_factory.h>

namespace rm
{
	class Character : public Entity
	{
	public:
		DECLARE_ENTITY(EntityType::Character);

	public:
		Character();
		virtual ~Character();

		virtual void update(float deltaTime)      override;
		virtual void fixedUpdate(float deltaTime) override;
		virtual void debugDraw()                  override;

		/** Serialize whole object */
		template<typename Stream>
		bool serializeFull(Stream& stream);

		/** Serialize commonly updated properties */
		template<typename Stream>
		bool serialize(Stream& stream);

		virtual void Fire();

		virtual void startContact(Entity* other) override;
		virtual void endContact(Entity* other)   override;

		Rigidbody* getRigidbody() const;
		void posessbyPlayer(int32_t playerId);

	private:
		Rigidbody*      m_rigidbody;
		ActionListener* m_actionListener;
	};
}; // namespace rm
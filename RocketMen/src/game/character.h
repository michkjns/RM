
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

		virtual bool Fire();

		virtual void startContact(Entity* other) override;
		virtual void endContact(Entity* other)   override;

		Rigidbody* getRigidbody() const;
		void posessbyPlayer(int16_t playerId);

	private:
		Rigidbody*      m_rigidbody;
		ActionListener* m_actionListener;

		Vector2 m_aimDirection;

	public:
		/** Serialize complete object */
		template<typename Stream>
		bool serializeFull(Stream& stream);

		/** Serialize commonly updated variables */
		template<typename Stream>
		bool serialize(Stream& stream);

		/** Serialize client-owned variables */
		template<typename Stream>
		bool reverseSerialize(Stream& stream);
	};
}; // namespace rm
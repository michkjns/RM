
#pragma once

#include <core/entity.h>

namespace rm
{
	struct RocketExplode
	{
		Vector2 pos;
		int32_t networkID;
		float radius;
		float power;
	};

	class Rocket : public Entity
	{
	public:
		EntityType getType() const { return EntityType::Rocket; }
	public:
		Rocket();
		Rocket(Vector2 direction, float accelerationPower);
		virtual ~Rocket();

		void initialize(Entity* owner, Vector2 direction, float power,
		                bool shouldReplicate = false);

		virtual void update(float deltaTime)      override;
		virtual void fixedUpdate(float deltaTime) override;

		void setAccelerationPower(float accelerationPower);
		float getAccelerationPower() const;
		Rigidbody* getRigidbody() const;

		/** Serialize whole object */
		template<typename Stream>
		bool serializeFull(Stream& stream);

		/** Serialize commonly updated properties */
		template<typename Stream>
		bool serialize(Stream& stream);

		virtual void startContact(Entity* other) override;
		virtual void endContact(Entity* other)   override;

	private:
		virtual void setupRigidBody();
		Rigidbody* m_rigidbody;
		Entity*    m_owner;
		Vector2    m_direction;
		float      m_accelerationPower;
		float      m_lifetimeSeconds;
	};

	//==========================================================================

	class RocketFactory : public EntityFactory
	{
	public:
		RocketFactory() : EntityFactory() {};
		static void initialize() {
			EntityFactory::registerFactory(getType(), &s_factory);
		}

	protected:
		static	EntityType getType() { return EntityType::Rocket; }
		Entity* instantiateEntity(ReadStream& rs,
		                          bool shouldReplicate = false);

		virtual bool serializeFull(Entity* entity, WriteStream& stream) override;
		virtual bool serializeFull(Entity* entity, ReadStream& stream) override;
		virtual bool serialize(Entity* entity, WriteStream& stream) override;
		virtual bool serialize(Entity* entity, ReadStream& stream) override;

		static RocketFactory s_factory;
	};

};

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
		virtual void initialize(bool shouldReplicate = false);
		virtual void update(float deltaTime)      override;
		virtual void fixedUpdate(float deltaTime) override;

		void setDirection(Vector2 direction, float power);
		//	Vector2 getDirection() const;

		void setAccelerationPower(float accelerationPower);
		float getAccelerationPower() const;
		Rigidbody* getRigidbody() const;

		virtual void serializeFull(BitStream& stream);
		//	virtual void deserializeFull(BitStream* stream);

		virtual void startContact(Entity* other) override;
		virtual void endContact(Entity* other)   override;

	private:
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
		struct RocketInitializer : public EntityInitializer
		{
			RocketInitializer() { type = getType(); }
			Vector2   position;
			Vector2   direction;
			float     power;
			int32_t   ownerNetworkID;
		};

		RocketFactory() : EntityFactory() {};
		static void initialize() {
			EntityFactory::registerFactory(getType(), &s_factory);
		}

	protected:
		static	EntityType getType() { return EntityType::Rocket; }
		Entity* instantiateEntity(EntityInitializer* initializer, 
		                          bool shouldReplicate = false,
		                          Entity* toReplace = nullptr) override;

		static RocketFactory s_factory;
	};

};
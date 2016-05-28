
#pragma once

#include <core/action_listener.h>
#include <core/entity.h>

namespace rm
{
	class Character : public Entity
	{
	public:
		EntityType getType() const { return EntityType::Character; }

	public:
		Character();
		virtual ~Character();

		virtual void initialize(bool isNetworked = true);
		virtual void update(float deltaTime)            override;
		virtual void fixedUpdate(float deltaTime)       override;
		virtual void debugDraw()                        override;

		virtual void serializeFull(BitStream& stream)   override;
	//	virtual void deserializeFull(BitStream* stream) override;

		virtual void Fire();

		virtual void startContact(Entity* other) override;
		virtual void endContact(Entity* other)   override;

		Rigidbody* getRigidbody() const;

	private:
		Rigidbody*      m_rigidbody;
		ActionListener* m_actionListener;
	};

	//===========================================================================

	class CharacterFactory : public EntityFactory
	{
	public:
		struct CharacterInitializer : public EntityInitializer
		{
			CharacterInitializer() { type = getType(); }
			Vector2     position;
			Vector2     velocity;
			std::string sprite;
		};

		CharacterFactory() : EntityFactory() {};
		static void initialize() {
			EntityFactory::registerFactory(getType(), &s_factory);
		}

	protected:
		static	EntityType getType() { return EntityType::Character; }
		Entity* instantiateEntity(EntityInitializer* initialize,
								  bool shouldReplicate = false,
		                          Entity* toReplace = nullptr) override;
		static CharacterFactory s_factory;
	};


}; // namespace rm
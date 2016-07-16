
#pragma once

#include <core/action_listener.h>
#include <core/entity_factory.h>

namespace rm
{
	class Character : public Entity
	{
	public:
		DefineEntityType(EntityType::Character);

	public:
		Character();
		virtual ~Character();

		virtual void initialize(bool isNetworked = true);
		virtual void update(float deltaTime)            override;
		virtual void fixedUpdate(float deltaTime)       override;
		virtual void debugDraw()                        override;

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

	private:
		Rigidbody*      m_rigidbody;
		ActionListener* m_actionListener;
	};

	//===========================================================================

	//class CharacterFactory : public EntityFactory
	//{
	//public:
	//	CharacterFactory() : EntityFactory() {};
	//	static void initialize() {
	//		EntityFactory::registerFactory(getType(), &s_factory);
	//	}

	//protected:
	//	static	EntityType getType() { return EntityType::Character; }
	//	Entity* instantiateEntity(ReadStream& stream,
	//							  bool shouldReplicate = false);
	//	virtual bool serializeFull(Entity* entity, WriteStream& stream) override;
	//	virtual bool serializeFull(Entity* entity, ReadStream& stream) override;
	//	virtual bool serialize(Entity* entity, WriteStream& ws) override;
	//	virtual bool serialize(Entity* entity, ReadStream& rs)  override;

	//	static CharacterFactory s_factory;
	//};


}; // namespace rm
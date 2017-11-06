
#pragma once

#include <bitstream.h>
#include <core/entity_type.h>

class Entity;

static const int32_t s_numEntityTypes = static_cast<int32_t>(EntityType::NUM_ENTITY_TYPES);

class IEntityFactory
{
public:
	virtual EntityType getType() = 0;

	virtual Entity* instantiate(ReadStream& rs) = 0;

	virtual bool serializeFull(Entity* entity, WriteStream& stream) = 0;

	virtual bool serializeFull(Entity* entity, ReadStream& stream) = 0;

	virtual bool serialize(Entity* entity, WriteStream& stream) = 0;

	virtual bool serialize(Entity* entity, ReadStream& stream) = 0;
};

template<typename T>
class EntityFactory : public IEntityFactory
{
public:
	EntityFactory() {};

	static void initialize()
	{
		EntityManager::registerFactory(dynamic_cast<IEntityFactory*>(&s_factory));
	}

	EntityType getType() override 
	{
		return T::getTypeStatic();
	}

	Entity* instantiate(ReadStream& rs) override
	{
		T* entity = new T();
		if (!entity->serializeFull(rs))
		{
			entity->kill();
			return nullptr;
		}

		return entity;
	}

	virtual bool serializeFull(Entity* entity, WriteStream& stream) override
	{
		return dynamic_cast<T*>(entity)->serializeFull(stream);
	}

	bool serializeFull(Entity* entity, ReadStream& stream) override
	{
		return dynamic_cast<T*>(entity)->serializeFull(stream);
	}

	bool serialize(Entity* entity, WriteStream& stream) override
	{
		return dynamic_cast<T*>(entity)->serialize(stream);
	}

	bool serialize(Entity* entity, ReadStream& stream) override
	{
		return dynamic_cast<T*>(entity)->serialize(stream);
	}

	static EntityFactory<T> s_factory;
};

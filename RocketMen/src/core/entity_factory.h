
#pragma once

#include <utility/bitstream.h>
#include <core/entity_type.h>
#include <core/entity_manager.h>

class Entity;

static const int32_t s_numEntityTypes = static_cast<int32_t>(EntityType::NUM_ENTITY_TYPES);

class IEntityFactory
{
public:
	virtual EntityType getType() = 0;

	virtual Entity* instantiate(ReadStream& rs) = 0;

	virtual bool serializeFull(Entity* entity, WriteStream& stream) = 0;
	virtual bool serializeFull(Entity* entity, ReadStream& stream) = 0;
	virtual bool serializeFull(Entity* entity, MeasureStream& stream) = 0;

	virtual bool serialize(Entity* entity, WriteStream& stream) = 0;
	virtual bool serialize(Entity* entity, ReadStream& stream) = 0;
	virtual bool serialize(Entity* entity, MeasureStream& stream) = 0;
};

template<typename T>
class EntityFactory : public IEntityFactory
{
public:
	EntityFactory()
	{
		assert(EntityManager::getFactory(getType()) == nullptr);
		EntityManager::registerFactory(dynamic_cast<IEntityFactory*>(this));
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
			delete entity;
			return nullptr;
		}

		return entity;
	}

	virtual bool serializeFull(Entity* entity, WriteStream& stream) override
	{
		assert(entity != nullptr);
		return dynamic_cast<T*>(entity)->serializeFull(stream);
	}

	bool serializeFull(Entity* entity, ReadStream& stream) override
	{
		assert(entity != nullptr);
		return dynamic_cast<T*>(entity)->serializeFull(stream);
	}

	bool serializeFull(Entity* entity, MeasureStream& stream) override
	{
		assert(entity != nullptr);
		return dynamic_cast<T*>(entity)->serializeFull(stream);
	}

	bool serialize(Entity* entity, WriteStream& stream) override
	{
		assert(entity != nullptr);
		return dynamic_cast<T*>(entity)->serialize(stream);
	}

	bool serialize(Entity* entity, ReadStream& stream) override
	{
		assert(entity != nullptr);
		return dynamic_cast<T*>(entity)->serialize(stream);
	}

	bool serialize(Entity* entity, MeasureStream& stream) override
	{
		assert(entity != nullptr);
		return dynamic_cast<T*>(entity)->serialize(stream);
	}

	//bool serializeClientVars(Entity* entity, WriteStream& stream) override
	//{
	//	assert(entity != nullptr);
	//	return dynamic_cast<T*>(entity)->reverseSerialize(stream);
	//}

	//bool serializeClientVars(Entity* entity, ReadStream& stream) override
	//{
	//	assert(entity != nullptr);
	//	return dynamic_cast<T*>(entity)->reverseSerialize(stream);
	//}

	static EntityFactory<T> s_factory;

#define DEFINE_ENTITY_FACTORY(EntityClass) \
	EntityFactory<EntityClass> EntityFactory<EntityClass>::s_factory
};

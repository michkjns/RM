
#pragma once

#include "entity_manager.h"

#include <utility/bitstream.h>
#include <utility/buffer.h>
#include <core/entity.h>
#include <core/entity_factory.h>
#include <network/network.h>
#include <utility/id_manager.h>
#include <map>

static IEntityFactory* s_factories[static_cast<int32_t>(EntityType::NUM_ENTITY_TYPES)];
static std::vector<Entity*> s_entities;
static std::vector<Entity*> s_newEntities;
static IdManager s_entityIds(s_maxEntities);

inline bool isReplicated(Entity* entity)
{
	assert(entity != nullptr);
	for (const auto& ent : s_entities)
	{
		if (ent == entity)
			return true;
	}

	return false;
}

IEntityFactory* EntityManager::getFactory(EntityType type)
{
	return s_factories[static_cast<int32_t>(type)];
}

void EntityManager::registerFactory(IEntityFactory* factory)
{
	assert(factory != nullptr);
	s_factories[static_cast<int32_t>(factory->getType())] = factory;
}

void EntityManager::instantiateEntity(Entity* entity, bool enableReplication)
{
	assert(entity != nullptr);
	assert(!isReplicated(entity));
	ensure(s_entityIds.hasAvailable());

	entity->m_id = s_entityIds.getNext();
	s_newEntities.push_back(entity);

	if (enableReplication && entity->getNetworkId() == INDEX_NONE)
	{
		Network::generateNetworkId(entity);
	}
}

Entity* EntityManager::instantiateEntity(ReadStream& stream)
{
	EntityType type = EntityType::Entity;
	int32_t intType = INDEX_NONE;
	stream.serializeInt(intType, 0, s_numEntityTypes);

	assert(intType > INDEX_NONE && intType < s_numEntityTypes);
	type = static_cast<EntityType>(intType);
	Entity* entity = getFactory(type)->instantiate(stream);

	instantiateEntity(entity);

	return entity;
}

Entity* EntityManager::instantiateEntity(ReadStream& stream, int32_t networkId)
{
	assert(networkId >= INDEX_NONE);
	EntityType type = EntityType::Entity;
	int32_t intType = INDEX_NONE;
	stream.serializeInt(intType, 0, s_numEntityTypes);

	assert(intType > INDEX_NONE && intType < s_numEntityTypes);
	type = static_cast<EntityType>(intType);
	Entity* entity = getFactory(type)->instantiate(stream);

	if (entity->getNetworkId() <= INDEX_NONE)
	{
		entity->setNetworkId(networkId);
	}

	instantiateEntity(entity);
	return entity;
}

bool EntityManager::serializeFullEntity(Entity* entity, ReadStream& stream, bool includeType)
{
	assert(entity != nullptr);
	if (includeType)
	{
		int32_t intType = static_cast<int32_t>(entity->getType());
		int32_t readType = INDEX_NONE;
		stream.serializeInt(readType, 0, s_numEntityTypes);
		ensure(readType == intType);
	}

	const bool result = getFactory(entity->getType())->serializeFull(entity, stream);

	return result;
}

bool EntityManager::serializeFullEntity(Entity* entity, WriteStream& stream, bool includeType)
{
	assert(entity != nullptr);
	if (includeType)
	{
		int32_t intType = static_cast<int32_t>(entity->getType());
		stream.serializeInt(intType, 0, s_numEntityTypes);
	}

	const bool result = getFactory(entity->getType())->serializeFull(entity, stream);

	return result;
}

bool EntityManager::serializeFullEntity(Entity* entity, MeasureStream& stream, bool includeType)
{
	assert(entity != nullptr);
	if (includeType)
	{
		int32_t intType = static_cast<int32_t>(entity->getType());
		stream.serializeInt(intType, 0, s_numEntityTypes);
	}

	const bool result = getFactory(entity->getType())->serializeFull(entity, stream);

	return result;
}

bool EntityManager::serializeEntity(Entity* entity, WriteStream& stream)
{
	assert(entity != nullptr);
	const bool result = getFactory(entity->getType())->serialize(entity, stream);
	return result;
}

bool EntityManager::serializeEntity(Entity* entity, ReadStream& stream)
{
	assert(entity != nullptr);
	const bool result = getFactory(entity->getType())->serialize(entity, stream);
	return result;
}

bool EntityManager::serializeEntity(Entity* entity, MeasureStream& stream)
{
	assert(entity != nullptr);
	const bool result = getFactory(entity->getType())->serialize(entity, stream);
	return result;
}

//bool EntityManager::serializeClientVars(Entity* entity, WriteStream& stream)
//{
//	assert(entity != nullptr);
//	return getFactory(entity->getType())->serializeClientVars(entity, stream);
//}
//
//bool EntityManager::serializeClientVars(Entity* entity, ReadStream& stream)
//{
//	assert(entity != nullptr);
//	return getFactory(entity->getType())->serializeClientVars(entity, stream);
//}

void EntityManager::flushEntities()
{
	for (auto it = s_entities.begin(); it != s_entities.end();)
	{
		if ((*it)->isAlive() == false)
		{
			delete (*it);
			it = s_entities.erase(it);
		}
		else
		{
			++it;
		}
	}

	for (auto it : s_newEntities)
	{
		s_entities.push_back(it);
	}
	s_newEntities.clear();
}

void EntityManager::killEntities()
{ 
	for (auto it = s_entities.begin(); it != s_entities.end();)
	{
		delete (*it);
		it = s_entities.erase(it);
	}

	s_entityIds.clear();
}

Entity* EntityManager::findNetworkedEntity(int32_t networkId)
{
	for (Entity* entity : s_newEntities)
	{
		if (entity->getNetworkId() == networkId)
		{
			return entity;
		}
	}

	for (Entity* entity : s_entities)
	{
		if (entity->getNetworkId() == networkId)
		{
			return entity;
		}
	}

	return nullptr;
}

void EntityManager::freeEntityId(int32_t id)
{
	s_entityIds.remove(id);
}

std::vector<Entity*>& EntityManager::getEntities()
{
	return s_entities;
}

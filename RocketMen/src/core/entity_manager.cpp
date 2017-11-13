
#pragma once

#include "entity_manager.h"

#include <bitstream.h>
#include <core/entity.h>
#include <core/entity_factory.h>
#include <network.h>

#include <map>

static std::map<EntityType, IEntityFactory*> s_factoryMap;
static std::vector<Entity*> s_entities;
static std::vector<Entity*> s_newEntities;
static uint32_t s_entityId;

inline bool isReplicated(Entity* entity)
{
	if (assert(entity != nullptr))
	{
		for (const auto& ent : s_entities)
		{
			if (ent == entity)
				return true;
		}
	}

	return false;
}

IEntityFactory* EntityManager::getFactory(EntityType type)
{
	return s_factoryMap.at(type);
}

void EntityManager::registerFactory(IEntityFactory* factory)
{
	assert(factory != nullptr);
	s_factoryMap[factory->getType()] = factory;
}

void EntityManager::instantiateEntity(Entity* entity, bool enableReplication)
{
	assert(entity != nullptr);
	assert(!isReplicated(entity));

	if (!isReplicated(entity))
	{
		entity->m_id = ++s_entityId;
		s_newEntities.push_back(entity);
	}

	if (enableReplication && entity->getNetworkId() == INDEX_NONE)
	{
		Network::generateNetworkId(entity);
	}
}

Entity* EntityManager::instantiateEntity(ReadStream& stream, bool enableReplication)
{
	EntityType type = EntityType::Entity;
	int32_t intType = INDEX_NONE;
	stream.serializeInt(intType, 0, s_numEntityTypes);

	assert(intType > INDEX_NONE && intType < s_numEntityTypes);
	type = static_cast<EntityType>(intType);
	Entity* entity = getFactory(type)->instantiate(stream);
	instantiateEntity(entity, enableReplication);

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

	return getFactory(entity->getType())->serializeFull(entity, stream);
}

bool EntityManager::serializeFullEntity(Entity* entity, WriteStream& stream, bool includeType)
{
	assert(entity != nullptr);
	if (includeType)
	{
		int32_t intType = static_cast<int32_t>(entity->getType());
		stream.serializeInt(intType, 0, s_numEntityTypes);
	}

	return getFactory(entity->getType())->serializeFull(entity, stream);
}

bool EntityManager::serializeEntity(Entity* entity, WriteStream& stream)
{
	assert(entity != nullptr);
	return getFactory(entity->getType())->serialize(entity, stream);
}

bool EntityManager::serializeEntity(Entity* entity, ReadStream& stream)
{
	assert(entity != nullptr);
	return getFactory(entity->getType())->serialize(entity, stream);
}

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
			it++;
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
}

std::vector<Entity*>& EntityManager::getEntities()
{
	return s_entities;
}
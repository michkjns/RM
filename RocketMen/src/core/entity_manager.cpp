
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
static Game* s_gameInstance;

inline bool isReplicated(Entity* entity)
{
	ASSERT(entity != nullptr);
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
	ASSERT(factory != nullptr);
	s_factories[static_cast<int32_t>(factory->getType())] = factory;
}

void EntityManager::instantiateEntity(Entity* entity, bool enableReplication)
{
	ASSERT(entity != nullptr);
	ASSERT(!isReplicated(entity));
	ASSERT(s_entityIds.hasIdsAvailable(), "Ran out of Entity Ids");

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

	ASSERT(intType > INDEX_NONE && intType < s_numEntityTypes);
	type = static_cast<EntityType>(intType);
	Entity* entity = getFactory(type)->instantiate(stream);

	instantiateEntity(entity);

	return entity;
}

Entity* EntityManager::instantiateEntity(ReadStream& stream, int32_t networkId)
{
	ASSERT(networkId >= INDEX_NONE, "Entity already has a NetworkId");
	EntityType type = EntityType::Entity;
	int32_t intType = INDEX_NONE;
	stream.serializeInt(intType, 0, s_numEntityTypes);

	ASSERT(intType > INDEX_NONE && intType < s_numEntityTypes, "Illegal entity type");
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
	ASSERT(entity != nullptr);
	if (includeType)
	{
		int32_t intType = static_cast<int32_t>(entity->getType());
		int32_t readType = INDEX_NONE;
		stream.serializeInt(readType, 0, s_numEntityTypes);
		ASSERT(readType == intType, "Serialized EntityType does not match the entity's type");
	}

	const bool result = getFactory(entity->getType())->serializeFull(entity, stream);

	return result;
}

bool EntityManager::serializeFullEntity(Entity* entity, WriteStream& stream, bool includeType)
{
	ASSERT(entity != nullptr);
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
	ASSERT(entity != nullptr);
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
	ASSERT(entity != nullptr);
	const bool result = getFactory(entity->getType())->serialize(entity, stream);
	return result;
}

bool EntityManager::serializeEntity(Entity* entity, ReadStream& stream)
{
	ASSERT(entity != nullptr);
	const bool result = getFactory(entity->getType())->serialize(entity, stream);
	return result;
}

bool EntityManager::serializeEntity(Entity* entity, MeasureStream& stream)
{
	ASSERT(entity != nullptr);
	const bool result = getFactory(entity->getType())->serialize(entity, stream);
	return result;
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

void EntityManager::setGameInstance(Game* game)
{
	s_gameInstance = game;
}

Game* EntityManager::getGame()
{
	return s_gameInstance;
}

std::vector<Entity*>& EntityManager::getEntities()
{
	return s_entities;
}

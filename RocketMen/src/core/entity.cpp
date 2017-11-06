
#include <core/entity.h>

#include <bitstream.h>
#include <core/debug.h>
#include <core/entity_factory.h>
#include <network.h>
#include <physics.h>

#include <map>
#include <functional>

static std::map<EntityType, IEntityFactory*> s_factoryMap;

static std::vector<Entity*> s_entities;
static std::vector<Entity*> s_newEntities;
static uint32_t s_entityId;

inline bool isReplicated(Entity* entity)
{
	for (const auto& ent : s_entities)
	{
		if (ent == entity)
			return true;
	}

	return false;
}

Entity::Entity() :
	m_id(0),
	m_isInitialized(false),
	m_networkId(INDEX_NONE)
{
}

Entity::~Entity()
{
}

void Entity::initialize(bool replicate)
{
 	if (!isReplicated(this))
	{
		m_id = ++s_entityId;
		s_newEntities.push_back(this);
	}

	m_isInitialized = true;
	
	if (replicate && m_networkId == INDEX_NONE)
	{
		Network::generateNetworkId(this);
	}
}

Entity* Entity::instantiate(ReadStream& stream)
{
	EntityType type = EntityType::Entity;
	int32_t intType = INDEX_NONE;
	stream.serializeInt(intType, 0, s_numEntityTypes);

	ensure(intType > INDEX_NONE && intType < s_numEntityTypes);
	type = static_cast<EntityType>(intType);
	return s_factoryMap.at(type)->instantiate(stream);
}

bool Entity::serializeFull(Entity* entity, ReadStream& stream, bool includeType)
{
	if (includeType)
	{
		int32_t intType = static_cast<int32_t>(entity->getType());
		int32_t readType = INDEX_NONE;
		stream.serializeInt(readType, 0, s_numEntityTypes);
		ensure(readType == intType);
	}
	return s_factoryMap.at(entity->getType())->serializeFull(entity, stream);
}

bool Entity::serializeFull(Entity* entity, WriteStream& stream, bool includeType)
{
	if (includeType)
	{
		int32_t intType = static_cast<int32_t>(entity->getType());
		stream.serializeInt(intType, 0, s_numEntityTypes);
	}
	return s_factoryMap.at(entity->getType())->serializeFull(entity, stream);
}

bool Entity::serialize(Entity* entity, WriteStream& stream)
{
	return s_factoryMap.at(entity->getType())->serialize(entity, stream);
}

bool Entity::serialize(Entity* entity, ReadStream& stream)
{
	return s_factoryMap.at(entity->getType())->serialize(entity, stream);;
}

void Entity::flushEntities()
{
	for (auto it = s_entities.begin(); it != s_entities.end();)
	{
		if ((*it)->isAlive() == false)
		{
			delete (*it);
			it = s_entities.erase(it);
		}
		else
			it++;
	}

	for (auto it : s_newEntities)
	{
		s_entities.push_back(it);
	}
	s_newEntities.clear();
}

void Entity::killEntities()
{
	for (auto it = s_entities.begin(); it != s_entities.end();)
	{
		delete (*it);
		it = s_entities.erase(it);
	}
}

std::vector<Entity*>& Entity::getList()
{
	return s_entities;
}

void Entity::registerFactory(IEntityFactory* factory)
{
	assert(factory);
	s_factoryMap[factory->getType()] = factory;
}

//==============================================================================

void Entity::kill()
{
	m_id = 0;
	if (Network::isServer())
	{
		Network::destroyEntity(m_networkId);
	}
	else if (m_networkId < 0)
	{

	}
}

bool Entity::isAlive() const
{
	return (m_id != 0);
}

std::string Entity::getSpriteName() const
{
	return m_sprite;
}

void Entity::setNetworkId(int32_t networkId)
{
	m_networkId = networkId;
}

int32_t Entity::getNetworkId() const
{
	return m_networkId;
}

bool Entity::isNetworkPrediction() const
{
	return m_networkId < INDEX_NONE;
}

#ifdef _DEBUG
uint32_t Entity::getId() const
{
	return m_id;
}
#endif // _DEBUG

void Entity::startContact(Entity* /*other*/)
{
}

void Entity::endContact(Entity* /*other*/)
{
}

void Entity::setSprite(std::string name)
{
	m_sprite = name;
}

Transform& Entity::getTransform()
{
	return m_transform;
}

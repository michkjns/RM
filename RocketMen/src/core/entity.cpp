
#include <core/entity.h>

#include <utility/bitstream.h>
#include <core/debug.h>
#include <core/entity_factory.h>
#include <core/entity_manager.h>
#include <network/network.h>
#include <physics/physics.h>

#include <functional>

void Entity::instantiate(Entity* entity)
{
	EntityManager::instantiateEntity(entity);
}

//=============================================================================

Entity::Entity() :
	m_id(INDEX_NONE),
	m_networkId(INDEX_NONE),
	m_ownerPlayerId(INDEX_NONE)
{
}

Entity::~Entity()
{
}

void Entity::kill()
{
	EntityManager::freeEntityId(m_id);
	m_id = INDEX_NONE;
	if (Network::isServer())
	{
		Network::destroyEntity(m_networkId);
	}
}

bool Entity::isAlive() const
{
	return (m_id != INDEX_NONE);
}

std::string Entity::getSpriteName() const
{
	return m_sprite;
}

void Entity::setNetworkId(int32_t networkId)
{
	assert(m_networkId <= INDEX_NONE);
	m_networkId = networkId;
}

int32_t Entity::getNetworkId() const
{
	return m_networkId;
}

int16_t Entity::getOwnerPlayerId() const
{
	return m_ownerPlayerId;
}

bool Entity::isReplicated() const
{
	return m_networkId > INDEX_NONE;
}

bool Entity::isSpawnPrediction() const
{
	return m_networkId < INDEX_NONE;
}

int32_t Entity::getId() const
{
	return m_id;
}

void Entity::startContact(Entity* /*other*/)
{
}

void Entity::endContact(Entity* /*other*/)
{
}

void Entity::setSprite(const std::string& name)
{
	m_sprite = name;
}

Transform2D& Entity::getTransform()
{
	return m_transform;
}

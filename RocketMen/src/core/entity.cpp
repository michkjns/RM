
#include <core/entity.h>

#include <bitstream.h>
#include <core/debug.h>
#include <core/entity_factory.h>
#include <core/entity_manager.h>
#include <network.h>
#include <physics.h>

#include <functional>

void Entity::instantiate(Entity* entity)
{
	EntityManager::instantiateEntity(entity);
}


Entity::Entity() :
	m_id(0),
	m_networkId(INDEX_NONE)
{
}

Entity::~Entity()
{
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

bool Entity::isSpawnPrediction() const
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

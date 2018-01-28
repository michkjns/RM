
#pragma once

#include <utility/bitstream.h>
#include <common.h>
#include <core/entity_type.h>
#include <core/transform2d.h>

#include <functional>

#define DECLARE_ENTITY(x) \
	static const EntityType s_type = x; \
	static EntityType getTypeStatic() { return s_type; }; \
	virtual EntityType getType() override { return s_type; };

//=============================================================================

class Entity
{
public:
	static const EntityType s_type = EntityType::Entity;
	static EntityType getTypeStatic() { return s_type; };

	static void instantiate(Entity* entity);
	static class Game* getGame();

public:
	Entity();
	virtual ~Entity();
	virtual EntityType getType() { return s_type; }

	virtual void update(float deltaTime) = 0;
	virtual void fixedUpdate(float deltaTime) = 0;
	virtual void debugDraw() {};
	
	void setSprite(const std::string& name);

	Transform2D& getTransform();

	/* Flags entity to be destroyed */
	void kill();

	/* @return false if entity is flagged to be destroyed */
	bool isAlive() const;

	std::string getSpriteName() const;
	
	void setNetworkId(int32_t networkId);
	int32_t getNetworkId() const;

	int16_t getOwnerPlayerId() const;

	/* @return true if entity is replicated between server and client*/
	bool isReplicated() const;

	/* @return true if entity is spawned locally and not yet acknowledged by the server*/
	bool isSpawnPrediction() const;

	virtual void startContact(Entity* other);
	virtual void endContact(Entity* other);

	int32_t getId() const;

protected:
	Transform2D m_transform;
	std::string m_spriteName;
	int32_t     m_networkId;
	int16_t     m_ownerPlayerId;

private:
	int32_t     m_id;

public:
	friend class EntityManager;

	template<typename Stream>
	bool serializeFull(Stream& stream)
	{
		int32_t spriteNameLength = 0;

		if (Stream::isWriting)
		{
			spriteNameLength = int32_t(m_spriteName.length());
		}

		serializeInt(stream, m_networkId, -s_maxSpawnPredictedEntities - 1, s_maxNetworkedEntities);
		serializeInt(stream, spriteNameLength, 0, 32);

		if (Stream::isReading)
		{
			m_spriteName.resize(spriteNameLength);
		}

		for (int32_t i = 0; i < spriteNameLength; i++)
		{
			int32_t character = int32_t(m_spriteName[i]);
			serializeInt(stream, character, CHAR_MIN, CHAR_MAX);
			if (Stream::isReading)
			{
				m_spriteName[i] = char(character);
			}
		}

		return true;
	}

	template<typename Stream>
	bool serialize(Stream& stream)
	{
		return true;
	}
};

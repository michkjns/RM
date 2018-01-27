
#pragma once

#include <core/entity_type.h>

#include <vector>

static const int32_t  s_maxSpawnPredictedEntities = 16;
static const uint32_t s_maxNetworkedEntities = 256;
static const int32_t  s_maxEntities = 256;
static const int16_t  s_firstTempNetworkId = -2; // Reserve -1 for INDEX_NONE

//=============================================================================
class Entity;

class EntityManager
{
public:
	static class IEntityFactory* getFactory(EntityType type);
	static void registerFactory(class IEntityFactory* factory);

	static void instantiateEntity(Entity* entity, bool enableReplication = true);
	static Entity* instantiateEntity(class ReadStream& stream);
	static Entity* instantiateEntity(class ReadStream& stream, int32_t networkId);
	static Entity* instantiateEntity(class WriteStream&, int32_t) { assert(false); return nullptr; }

	static bool serializeFullEntity(Entity* entity, class ReadStream& stream,
		bool includeType = true);

	static bool serializeFullEntity(Entity* entity, class WriteStream& stream,
		bool includeType = true);

	static bool serializeFullEntity(Entity* entity, class MeasureStream& stream,
		bool includeType = true);

	static bool serializeEntity(Entity* entity, class WriteStream& stream);
	static bool serializeEntity(Entity* entity, class ReadStream& stream);
	static bool serializeEntity(Entity* entity, class MeasureStream& stream);

	static bool serializeClientVars(Entity* entity, class WriteStream& stream);
	static bool serializeClientVars(Entity* entity, class ReadStream& stream);

	static void flushEntities();
	static void killEntities();

	static Entity* findNetworkedEntity(int32_t networkId);

	static std::vector<Entity*>& getEntities();

private:
	static void freeEntityId(int32_t id);

public:
	friend class Entity;
};

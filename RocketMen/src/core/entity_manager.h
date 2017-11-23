
#pragma once

#include <core/entity_type.h>

#include <vector>

static const int32_t  s_maxSpawnPredictedEntities = 16;
static const uint32_t s_maxNetworkedEntities = 256;
static const int32_t  s_maxEntities = 12;

//=============================================================================
class Entity;

class EntityManager
{
public:
	static class IEntityFactory* getFactory(EntityType type);
	static void registerFactory(class IEntityFactory* factory);

	static void instantiateEntity(Entity* entity, bool enableReplication = true);
	static Entity* instantiateEntity(class ReadStream& stream, bool enableReplication = true);

	static bool serializeFullEntity(Entity* entity, class ReadStream& stream,
		bool includeType = true);

	static bool serializeFullEntity(Entity* entity, class WriteStream& stream,
		bool includeType = true);

	static bool serializeEntity(Entity* entity, class WriteStream& stream);
	static bool serializeEntity(Entity* entity, class ReadStream& stream);

	static bool serializeClientVars(Entity* entity, class WriteStream& stream);
	static bool serializeClientVars(Entity* entity, class ReadStream& stream);

	static void flushEntities();
	static void killEntities();

	static std::vector<Entity*>& getEntities();

private:
	static void freeEntityId(int32_t id);

public:
	friend class Entity;
};

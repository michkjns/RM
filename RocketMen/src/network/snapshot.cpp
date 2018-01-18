
#include "snapshot.h"

#include <core/entity.h>
#include <core/entity_manager.h>
#include <network/common_network.h>
#include <utility/utility.h>

using namespace network;

Message* Snapshot::createMessage(std::vector<Entity*>& entities)
{
	Message* message = new Message(MessageType::Snapshot);

	std::vector<Entity*> replicatedEntities;
	for (Entity* entity : entities)
	{
		if (entity->isReplicated())
		{
			replicatedEntities.push_back(entity);
		}	
	}

	int32_t numReplicatedEntities = static_cast<int32_t>(replicatedEntities.size());
	serializeInt(message->data, numReplicatedEntities);

	for (int32_t networkId = 0; networkId < s_maxNetworkedEntities; networkId++)
	{
		Entity* netEntity = findPtrByPredicate(replicatedEntities.begin(), replicatedEntities.end(),
			[networkId](Entity* entity) -> bool { return entity->getNetworkId() == networkId; });

		bool writeEntity = netEntity != nullptr;
		serializeBool(message->data, writeEntity);
		if (writeEntity)
		{
			EntityManager::serializeEntity(netEntity, message->data);
		}
	}

	//LOG_DEBUG("Snapshot size: %d", message->data.getDataLength());
	return message;
}

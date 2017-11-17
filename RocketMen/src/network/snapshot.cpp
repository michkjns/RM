
#include "snapshot.h"

#include <core/entity.h>
#include <core/entity_manager.h>
#include <network/common_network.h>
#include <utility.h>

using namespace network;

Snapshot::Snapshot(std::vector<Entity*>& entities) :
	m_writeStream(s_maxSnapshotSize)
{
	std::vector<Entity*> replicatedEntities;
	for (Entity* entity : entities)
	{
		if (entity->isReplicated())
		{
			replicatedEntities.push_back(entity);
		}
	}

	int32_t numReplicatedEntities = static_cast<int32_t>(replicatedEntities.size());
	serializeInt(m_writeStream, numReplicatedEntities);
	for (int32_t networkId = 0; networkId < s_maxNetworkedEntities; networkId++)
	{
		Entity* netEntity = findPtrByPredicate(replicatedEntities.begin(), replicatedEntities.end(),
			[networkId](Entity* entity) -> bool { return entity->getNetworkId() == networkId; });

		bool writeEntity = netEntity != nullptr;
		serializeBit(m_writeStream, writeEntity);
		if (writeEntity)
		{
			EntityManager::serializeEntity(netEntity, m_writeStream);
		}
	}
}

size_t Snapshot::getSize() const
{
	return m_writeStream.getLength();
}

const char* Snapshot::getBuffer() const
{
	return (char*)m_writeStream.getBuffer();
}

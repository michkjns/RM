
#include "snapshot.h"

#include <core/entity.h>
#include <core/entity_manager.h>
#include <utility.h>

using namespace network;

static const uint32_t s_maxSnapshotSize = 512;

Snapshot::Snapshot(std::vector<Entity*>& entities) :
	m_writeStream(s_maxSnapshotSize)
{
	for (int32_t networkId = 0; networkId < s_maxNetworkedEntities; networkId++)
	{
		Entity* netEntity = findPtrByPredicate(entities.begin(), entities.end(),
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

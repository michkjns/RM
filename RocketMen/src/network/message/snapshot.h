
#pragma once

#include <core/entity.h>
#include <core/entity_manager.h>
#include <network/message.h>
#include <utility/utility.h>

namespace network {
namespace message {

	struct Snapshot : public Message
	{
		DECLARE_MESSAGE(Snapshot, UnreliableUnordered);
		static const int32_t maxMissingEntityIds = 8;

		bool serialize_impl(WriteStream& stream)
		{
			serializeCheck(stream, "begin_snapshot");

			std::vector<Entity*> networkEntities;
			auto& entities = EntityManager::getEntities();
			for (Entity* entity : entities)
			{
				if (entity->isReplicated())
				{
					networkEntities.push_back(entity);
				}
			}

			int32_t numEntities = static_cast<int32_t>(networkEntities.size());
			ASSERT(numEntities <= s_maxNetworkedEntities, "Number of networked entities exceeds the maximum");
			serializeInt(stream, numEntities, 0, s_maxNetworkedEntities);

			for (int32_t networkId = 0; networkId < s_maxNetworkedEntities; networkId++)
			{
				if (Entity* netEntity = findPtrByPredicate(networkEntities.begin(), networkEntities.end(),
					[networkId](Entity* entity) -> bool { return entity->getNetworkId() == networkId; }))
				{
					serializeCheck(stream, "begin_entity");

					MeasureStream measureStream;
					EntityManager::serializeEntity(netEntity, measureStream);
					int32_t entitySize = measureStream.getMeasuredBits();

					bool writeEntity = entitySize > 0;
					serializeBool(stream, writeEntity);
					if (writeEntity)
					{
						serializeInt(stream, entitySize);
						serializeCheck(stream, "begin_entity_data");
						if (!EntityManager::serializeEntity(netEntity, stream))
						{
							return false;
						}
						serializeCheck(stream, "end_entity_data");

					}
					serializeCheck(stream, "end_entity");
				}
			}

			serializeCheck(stream, "end_snapshot");

			return true;
		}

		bool serialize_impl(ReadStream& stream)
		{
			std::fill(missingEntityIds, missingEntityIds + maxMissingEntityIds, INDEX_NONE);

			serializeCheck(stream, "begin_snapshot");

			std::vector<Entity*> replicatedEntities;
			auto& entities = EntityManager::getEntities();
			for (Entity* entity : entities)
			{
				if (entity->isReplicated())
				{
					replicatedEntities.push_back(entity);
				}
			}

			int32_t numReceivedEntities;
			serializeInt(stream, numReceivedEntities, 0, s_maxNetworkedEntities);
			if (numReceivedEntities == 0)
			{
				serializeCheck(stream, "end_snapshot");
				return true;
			}

			int32_t numEntitiesRead = 0;
			for (int32_t networkId = 0; networkId < s_maxNetworkedEntities; networkId++)
			{
				serializeCheck(stream, "begin_entity");

				int32_t receivedEntitySizeBits = -1;
				bool readEntity = false;
				serializeBool(stream, readEntity);

				if(readEntity)
				{
					serializeInt(stream, receivedEntitySizeBits);
					if (receivedEntitySizeBits <= 0)
					{
						return false;
					}
					if (Entity* netEntity = findPtrByPredicate(replicatedEntities.begin(), replicatedEntities.end(),
						[networkId](Entity* entity) -> bool { return entity->getNetworkId() == networkId; }))
					{
						serializeCheck(stream, "begin_entity_data");
						if (!EntityManager::serializeEntity(netEntity, stream))
						{
							return false;
						}
						serializeCheck(stream, "end_entity_data");
					}
					else
					{
						if (numMissingEntities < maxMissingEntityIds)
						{
							missingEntityIds[numMissingEntities++] = networkId;
						}

						serializeCheck(stream, "begin_entity_data");
						stream.skipBits(receivedEntitySizeBits);
						serializeCheck(stream, "end_entity_data");
					}
					numEntitiesRead++;
				}
				serializeCheck(stream, "end_entity");
				if (numEntitiesRead == numReceivedEntities)
				{
					break;
				}
			}


			serializeCheck(stream, "end_snapshot");

			return true;
		}

		int32_t missingEntityIds[maxMissingEntityIds];
		int32_t numMissingEntities = 0;

	};

}; // namespace message
};// namespace network
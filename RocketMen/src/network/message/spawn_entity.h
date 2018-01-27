
#pragma once

#include <network/message.h>
#include <core/entity_manager.h>

namespace network {
namespace message {

	struct SpawnEntity : public Message
	{
		DECLARE_MESSAGE(SpawnEntity, ReliableOrdered);

		template<typename Stream>
		bool serialize_impl(Stream& stream)
		{
			if(!serializeCheck(stream, "begin_spawn_entity"))
			{
				return false;
			}
			
			if (Stream::isWriting)
			{
				serializeCheck(stream, "begin_entity");
				LOG_DEBUG("SpawnEntity::write %d", rand());
				assert(entity != nullptr);
				int32_t networkId = entity->getNetworkId();
				assert(networkId >= 0 && networkId < s_maxNetworkedEntities);

				serializeInt(stream, networkId);

				MeasureStream measureStream;
				EntityManager::serializeFullEntity(entity, measureStream);

				int32_t entitySize = measureStream.getMeasuredBits();
				assert(entitySize > 0);

				serializeInt(stream, entitySize);
				serializeCheck(stream, "begin_entity_data");
				if (!EntityManager::serializeFullEntity(entity, stream))
				{
					assert(false);
					return false;
				}
				serializeCheck(stream, "end_entity_data");
			}

			if (Stream::isReading)
			{
				serializeCheck(stream, "begin_entity");
				int32_t networkId = INDEX_NONE;
				serializeInt(stream, networkId);
				if (networkId < 0 || networkId >= s_maxNetworkedEntities)
				{
					return false;
				}

				int32_t receivedEntitySizeBits = -1;
				serializeInt(stream, receivedEntitySizeBits);
				if (receivedEntitySizeBits < 0)
				{
					return false;
				}

				entity = EntityManager::findNetworkedEntity(networkId);
				if (entity == nullptr)
				{
					serializeCheck(stream, "begin_entity_data");
					entity = EntityManager::instantiateEntity(stream, networkId);
					serializeCheck(stream, "end_entity_data");
				}
				else
				{
					serializeCheck(stream, "begin_entity_data");
					stream.skipBits(receivedEntitySizeBits);
					serializeCheck(stream, "end_entity_data");
				}
			}

			serializeCheck(stream, "end_entity");

			if (!serializeCheck(stream, "end_spawn_entity"))
			{
				return false;
			}

			return true;
		}

	public:
		Entity* entity;
	};

}; // namespace message
};// namespace network
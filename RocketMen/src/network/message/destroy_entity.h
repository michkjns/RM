
#pragma once

#include  <network/message.h>

namespace network {
namespace message {

		struct DestroyEntity : public Message
		{
			DECLARE_MESSAGE(DestroyEntity, ReliableOrdered);

			template<typename Stream>
			bool serialize_impl(Stream& stream)
			{
				serializeCheck(stream, "begin_Destroy_entity");
				if (Stream::isWriting)
				{
					ASSERT(entityNetworkId >= 0 && entityNetworkId < s_maxNetworkedEntities);
				}

				serializeInt(stream, entityNetworkId, 0, s_maxNetworkedEntities);

				if (Stream::isReading)
				{
					if (entityNetworkId < 0 || entityNetworkId >= s_maxNetworkedEntities)
					{
						return false;
					}
				}
				serializeCheck(stream, "end_destroy_entity");
				return true;
			}

			int32_t entityNetworkId;
		};

}; // namespace message
};// namespace network
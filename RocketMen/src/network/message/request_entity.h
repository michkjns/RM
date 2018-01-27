
#pragma once

#include <network/message.h>

namespace network {
namespace message {

	struct RequestEntity : public Message
	{
		DECLARE_MESSAGE(RequestEntity, ReliableOrdered);

		template<typename Stream>
		bool serialize_impl(Stream& stream)
		{
			serializeCheck(stream, "begin_request_entity");
			if (Stream::isWriting)
			{
				assert(entityNetworkId >= 0 && entityNetworkId < s_maxNetworkedEntities);
			}

			serializeInt(stream, entityNetworkId, 0, s_maxNetworkedEntities);

			if (Stream::isReading)
			{
				if (entityNetworkId < 0 || entityNetworkId >= s_maxNetworkedEntities)
				{
					return false;
				}
			}
			serializeCheck(stream, "end_request_entity");
			return true;
		}

		int32_t entityNetworkId;
	};

}; // namespace message
};// namespace network
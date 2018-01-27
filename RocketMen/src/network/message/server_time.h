
#pragma once

#include <network/message.h>

namespace network {
namespace message {

	struct ServerTime : public Message
	{
		DECLARE_MESSAGE(ServerTime, UnreliableUnordered);

		template<typename Stream>
		bool serialize_impl(Stream& stream)
		{
			if (!serializeCheck(stream, "begin_server_time"))
			{
				return false;
			}

			serializeBits(stream, clientTimestamp, 64);

			serializeBits(stream, serverTimestamp, 64);

			if (!serializeCheck(stream, "end_server_time"))
			{
				return false;
			}

			return true;
		}

		uint64_t clientTimestamp;
		uint64_t serverTimestamp;
	};

};// namespace message
};// namespace network
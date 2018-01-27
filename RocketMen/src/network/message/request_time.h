
#pragma once

#include <network/message.h>

namespace network {
namespace message {

		struct RequestTime : public Message
		{
			DECLARE_MESSAGE(RequestTime, UnreliableUnordered);

			template<typename Stream>
			bool serialize_impl(Stream& stream)
			{
				if (!serializeCheck(stream, "begin_request_time"))
				{
					return false;
				}

				serializeBits(stream, clientTimestamp, 64);

				if (!serializeCheck(stream, "end_request_time"))
				{
					return false;
				}

				return true;
			}

			uint64_t clientTimestamp;
		};

}; // namespace message
};// namespace network
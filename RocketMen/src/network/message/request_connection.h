
#pragma once

#include  <network/message.h>

namespace network {
namespace message {

	struct RequestConnection : public Message
	{
		DECLARE_MESSAGE(RequestConnection, ReliableOrdered);

		template<typename Stream>
		bool serialize_impl(Stream& stream)
		{
			serializeCheck(stream, "request_connection");
			return true;
		}
	};

}; // namespace message
};// namespace network
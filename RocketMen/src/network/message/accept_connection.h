
#pragma once

#include <network/message.h>

namespace network {
namespace message {

	struct AcceptConnection : public Message
	{
		DECLARE_MESSAGE(AcceptConnection, ReliableOrdered);

		template<typename Stream>
		bool serialize_impl(Stream& stream)
		{
			serializeCheck(stream, "begin_accept_connection");

			serializeInt(stream, clientId, 0, s_maxPlayersPerClient);

			serializeCheck(stream, "end_accept_connection");

			return true;
		}

		int32_t clientId;
	};

}; // namespace message
};// namespace network
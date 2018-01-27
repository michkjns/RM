
#pragma once

#include <network/message.h>

namespace network {
namespace message {

	struct KeepAlive : public Message
	{
		DECLARE_MESSAGE(KeepAlive, ReliableOrdered);

		template<typename Stream>
		bool serialize_impl(Stream& stream)
		{
			serializeCheck(stream, "begin_keep_alive");
			serializeCheck(stream, "end_keep_alive");
			return true;
		}
	};

}; // namespace message
};// namespace network
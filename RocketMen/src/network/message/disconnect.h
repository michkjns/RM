
#pragma once

#include <network/message.h>

namespace network {
namespace message {

		struct Disconnect : public Message
		{
			DECLARE_MESSAGE(Disconnect, ReliableOrdered);

			template<typename Stream>
			bool serialize_impl(Stream& stream)
			{
				serializeCheck(stream, "disconnect");

				return true;
			}
		};

}; // namespace message
};// namespace network
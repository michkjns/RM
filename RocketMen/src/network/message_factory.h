
#pragma once

#include <network/message_type.h>


namespace network {

	struct Message;
	class MessageFactory
	{
	public:
		MessageFactory() {};
		virtual ~MessageFactory() {};

		virtual Message* createMessage(MessageType type) = 0;
	};

}; // namespace network
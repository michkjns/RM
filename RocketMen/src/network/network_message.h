
#pragma once

#include <utility/bitstream.h>
#include <common.h>
#include <network/address.h>
#include <network/message_type.h>

namespace network
{
	struct Message
	{
		MessageType type;
		BitStream   data;
	};

	struct OutgoingMessage : public Message
	{
		Sequence sequence;
		float    timeLastSent;
	};

	struct IncomingMessage : public Message
	{
		Sequence sequence;
		Address  address;
	};

	inline ChannelType getMessageChannel(const struct Message message)
	{
		return getMessageChannel(message.type);
	}

}; // namespace network
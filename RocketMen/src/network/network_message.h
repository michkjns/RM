
#pragma once

#include <bitstream.h>
#include <common.h>
#include <network/address.h>
#include <network/message_type.h>

namespace network
{
	static const int32_t s_messageQueueSize = 64;

	enum class ChannelType : uint8_t
	{
		Unreliable,
		ReliableOrdered
	};

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

	inline ChannelType getMessageChannel(MessageType type)
	{
		switch (type)
		{
			case MessageType::ClockSync:
			case MessageType::Gamestate:
			{
				return ChannelType::Unreliable;
			}
			case MessageType::AcceptClient:
			case MessageType::AcceptEntity:
			case MessageType::AcceptPlayer:
			case MessageType::DestroyEntity:
			case MessageType::Disconnect:
			case MessageType::GameEvent:
			case MessageType::IntroducePlayer:
			case MessageType::KeepAlive:
			case MessageType::PlayerInput:
			case MessageType::RequestConnection:
			case MessageType::RequestEntity:
			case MessageType::SpawnEntity:
			{
				return ChannelType::ReliableOrdered;
			}
			case MessageType::None:
			case MessageType::NUM_MESSAGE_TYPES:
			{
				assert(false);
				return ChannelType::Unreliable;
			};
		}
		assert(false);

		return ChannelType::Unreliable;
	};

	inline ChannelType getMessageChannel(struct Message message)
	{
		return getMessageChannel(message.type);
	}

}; // namespace network
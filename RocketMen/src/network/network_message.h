
#pragma once

#include <network/address.h>
#include <bitstream.h>
#include <common.h>

//#include <climits>

namespace network
{
	static const int32_t s_maxPendingMessages = 64;

	enum class MessageType : int8_t
	{
		None = 0,

		Ack,
		Ping,
		Disconnect,

		// Server to client
		AcceptClient,
		AcceptPlayer,
		Gamestate,
		SpawnEntity,
		AcceptEntity,
		DestroyEntity,
		GameEvent,

		// Client to server
		RequestConnection,
		IntroducePlayer,
		PlayerInput,
		RequestEntity,

		NUM_MESSAGE_TYPES
	};

#ifdef _DEBUG
		static const char* s_messageTypeString[static_cast<int32_t>(MessageType::NUM_MESSAGE_TYPES)] =
		{
			"None",
			"Ack",
			"Ping",
			"Disconnect",
			"AcceptClient",
			"AcceptPlayer",
			"Gamestate",
			"SpawnEntity",
			"AcceptEntity",
			"GameEvent",
			"RequestConnection",
			"IntroducePlayer",
			"PlayerInput",
			"RequestEntity",
			"NUM_MESSAGE_TYPES"
		};

		static const char* messageTypeAsString(MessageType type)
		{
			return s_messageTypeString[static_cast<int32_t>(type)];
		}
#endif

	struct NetworkMessage
	{
		MessageType type;
		BitStream   data;
		uint32_t    isReliable : 1;
		uint32_t    isOrdered  : 1;
	};

	struct OutgoingMessage
	{
		Sequence    sequence;
		float       timestamp;
		MessageType type;
		BitStream   data;
		uint32_t    isReliable : 1;
		uint32_t    isOrdered  : 1;
	};

	struct IncomingMessage
	{
		Sequence    sequence;
		float       timestamp;
		MessageType type;
		BitStream   data;
		Address     address; // sender address
		uint32_t    isReliable : 1;
		uint32_t    isOrdered  : 1;
	};

	struct SentMessage
	{
		bool acked;
	};

}; // namespace network

#pragma once

#include <network/address.h>
#include <bitstream.h>
#include <climits>

namespace network
{
	static const int32_t s_maxPendingMessages = 64;

	// TODO: Split server->client and client->server message types..?
	enum class MessageType : int8_t
	{
		Clear = 0,
		Ack,
		Ping,

		// Client to server
		ClientConnectRequest,
		ClientDisconnect,

		// Server to client
		AcceptClient,
		AcceptPlayer,
		Gamestate,
		SpawnEntity,
		AcceptEntity,
		DestroyEntity,
		GameEvent,

		// Client to server
		IntroducePlayer,
		PlayerInput,
		RequestEntity,

		NUM_MESSAGE_TYPES
	};

	struct NetworkMessage
	{
		MessageType type;
		BitStream   data;
		bool        isReliable;
		bool        isOrdered;
		int32_t     sequenceNr; // Packet seq
		float       timeOfCreation;
	};

	struct IncomingMessage
	{
		MessageType type;
		BitStream   data;
		bool        isReliable;
		bool        isOrdered;
		Address     address; // sender address
		int32_t     sequence; 
	};

	//inline void destroyMessage(NetworkMessage& message)
	//{
	//	if (message.data != nullptr)
	//	{
	//		delete message.data;
	//	}
	//}

	//inline void destroyMessage(IncomingMessage& message)
	//{
	//	if (message.data != nullptr)
	//	{
	//		delete message.data;
	//	}
	//}

}; // namespace network

#pragma once

#include "bitstream.h"

#include <stdint.h>

namespace network
{
	enum ECommand : char
	{
		EVENT_CLEAR,

		// Client to server
		CLIENT_CONNECT,
		CLIENT_DISCONNECT,

		// Server to client
		SERVER_HANDSHAKE,
		SERVER_GAMESTATE,
		
		// Client to server
		PLAYER_INTRO,
		PLAYER_INPUT,

		/////////////
		NUM_COMMANDS
	};

	enum class EBroadcast
	{
		BROADCAST_SINGLE,
		BROADCAST_ALL,
	};

	enum class EReliable
	{
		UNRELIABLE,
		RELIABLE,
	};

	struct PacketHeader
	{
		ECommand type;
		uint16_t dataLength;
		int32_t  sequenceNumber;
		union
		{
			int8_t   recipientID;
			int8_t   senderID;
		};
	};

	struct Packet
	{
		PacketHeader header;
		BitStream*   data;
		EBroadcast   broadcast;
		EReliable    reliable;
	};

	extern Packet createPacket(ECommand message,
						BitStream* data,
						int8_t recipient,
						EBroadcast broadcast = EBroadcast::BROADCAST_ALL,
						EReliable reliable = EReliable::RELIABLE);

}; // namespace network
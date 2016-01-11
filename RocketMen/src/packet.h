
#pragma once

#include "bitstream.h"

#include <stdint.h>

namespace network
{
	enum class EPacketType : int8_t
	{
		EVENT_CLEAR,

		// Connection
		CONNECTION_CONNECT,
		CONNECTION_DISCONNECT,

		// Server to client
		SERVER_HANDSHAKE,
		SERVER_GAMESTATE,
		
		// Client to server
		PLAYER_INTRO,
		PLAYER_INPUT
	};

	struct PacketHeader
	{
		EPacketType type;
		uint16_t dataLength;
	};

	struct Packet
	{
		PacketHeader header;
		BitStream* data;
		union
		{
			int8_t recipientID;
			int8_t senderID;
		};
		bool broadcast;
		bool reliable;
	};

}; // namespace network
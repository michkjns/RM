
#include "packet.h"

using namespace network;

Packet network::createPacket(ECommand message,
					BitStream* data,
					int8_t recipient,
					EBroadcast broadcast,
					EReliable reliable)
{
	Packet packet;
	packet.header.type = message;
	packet.header.dataLength = (data)?static_cast<uint16_t>(data->getLength()) : 0;
	packet.data = data;
	packet.header.recipientID = recipient;
	packet.reliable = reliable;
	return packet;
}
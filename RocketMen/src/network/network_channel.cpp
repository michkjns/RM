
#include "network_channel.h"

#include <core/debug.h>
#include <network/socket.h>

extern "C" unsigned long crcFast(unsigned char const message[], int nBytes);

using namespace network;

void NetworkChannel::sendPacket(Socket* socket, const Address& address, Packet* packet)
{
	assert(socket->isInitialized());
	assert(!packet->isEmpty());

	assert(g_maxBlockSize - packet->header.dataLength >= sizeof(uint32_t));

	// Write protocol ID after packet to include in the checksum
	memcpy(packet->getData() + packet->header.dataLength, &g_protocolId, sizeof(g_protocolId));
	uint32_t checksum = crcFast((const unsigned char*)packet->getData(),
		packet->header.dataLength + sizeof(uint32_t));

	BitStream stream;
	stream.writeInt32(checksum);
	stream.writeData(reinterpret_cast<char*>(&packet->header), sizeof(PacketHeader));
	stream.writeData(packet->getData(), packet->header.dataLength);

	socket->send(address, stream.getBuffer(), stream.getLength());
}

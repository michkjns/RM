
#include "network_channel.h"

#include <core/debug.h>
#include <network/socket.h>

extern "C" unsigned long crcFast(unsigned char const message[], int nBytes);

using namespace network;

void NetworkChannel::sendPacket(Socket* socket, const Address& address, Packet* packet)
{
	assert(socket != nullptr);
	assert(packet != nullptr);
	assert(socket->isInitialized());
	
	WriteStream packetStream(g_maxPacketSize);

	int32_t protocolId = g_protocolId;
	serializeBits(packetStream, protocolId, 32);

	packet->serialize(packetStream);

	const uint32_t checksum = crcFast((unsigned char*)packetStream.getData(), packetStream.getDataLength());

	// swap protocolId for checksum
	(uint32_t&)packetStream.getData()[0] = checksum;

	socket->send(address, packetStream.getData(), packetStream.getDataLength());
}

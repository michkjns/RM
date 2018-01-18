
#include "packet_receiver.h"

#include <common.h>
#include <core/debug.h>
#include <network/packet.h>
#include <network/socket.h>

using namespace network;

extern "C" unsigned long crcFast(unsigned char const message[], int nBytes);

PacketReceiver::PacketReceiver(int32_t bufferSize) :
	m_packets(bufferSize)
{
}

PacketReceiver::~PacketReceiver()
{
#ifdef _DEBUG
	LOG_DEBUG("~PacketReceiver: mismatched checksums: %d", m_numChecksumMismatches);
#endif
}

void PacketReceiver::receivePackets(Socket* socket)
{
	assert(socket != nullptr);
	assert(socket->isInitialized());

	Address address;
	char    buffer[g_maxPacketSize];
	int32_t length = 0;

	while (socket->receive(address, buffer, length))
	{
		if (length > g_maxPacketSize)
		{
			continue;
		}

		ReadStream stream(buffer, length);
		
		uint32_t receivedChecksum = 0;
		serializeBits(stream, receivedChecksum, 32);

		// swap checksum with protocolId
		(int32_t&)stream.getData()[0] = g_protocolId;

		if (receivedChecksum != crcFast((unsigned char*)stream.getData(), length))
		{
#ifdef _DEBUG
			m_numChecksumMismatches++;
			LOG_DEBUG("PacketReceiver::receivePackets: Checksum mismatched, packet discarded.");
#endif
			continue;
		}

		Packet* packet = new Packet();
		packet->address = address;
		if (packet->serialize(stream))
		{
			auto& entry = m_packets.insert();
			entry = packet;
		}
		else
		{
			delete packet;
		}
	}
}

Buffer<Packet*>& PacketReceiver::getPackets()
{
	return m_packets;
}

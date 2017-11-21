
#include "packet_receiver.h"

#include <common.h>
#include <core/debug.h>
#include <network/packet.h>
#include <network/socket.h>

using namespace network;

extern "C" unsigned long crcFast(unsigned char const message[], int nBytes);

PacketReceiver::PacketReceiver(int32_t bufferSize) :
	m_packets(bufferSize),
	m_addresses(bufferSize)
{
}

PacketReceiver::~PacketReceiver()
{
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

		// Reconstruct packet
		BitStream stream;
		stream.writeBuffer(buffer, length);

		const uint32_t checksum = stream.readInt32();

		PacketHeader packetHeader;
		stream.readBytes((char*)&packetHeader, sizeof(PacketHeader));

		if (packetHeader.dataLength < g_maxBlockSize)
		{
			ChannelType channel = (packetHeader.sequence     == (Sequence)-1
				                 && packetHeader.ackBits     == (uint32_t)-1
				                 && packetHeader.ackSequence == (Sequence)-1) ?
				ChannelType::Unreliable :
				ChannelType::ReliableOrdered;

			Packet packet(channel);
			packet.header = packetHeader;
			stream.readBytes(packet.getData(), packet.header.dataLength);

			// Write protocol ID after packet to include in the checksum
			memcpy(packet.getData() + packet.header.dataLength, &g_protocolId, sizeof(g_protocolId));

			if (checksum == crcFast((const unsigned char*)packet.getData(),
				packet.header.dataLength + sizeof(uint32_t)))
			{
				Packet& packetEntry = m_packets.insert();
				packetEntry = packet;
				Address& addressEntry = m_addresses.insert();
				addressEntry = address;
			}
			else
			{
				LOG_DEBUG("PacketReceiver::receivePackets: Checksum mismatched, packet discarded.");
			}
		}
	}
}

Buffer<Packet>& PacketReceiver::getPackets()
{
	return m_packets;
}

Buffer<Address>& PacketReceiver::getAddresses()
{
	return m_addresses;
}

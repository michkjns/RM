
#pragma once

#include <buffer.h>
#include <network/address.h>

namespace network
{
	class Packet;
	class Socket;

	class PacketReceiver
	{
	public:
		PacketReceiver(int32_t bufferSize);
		~PacketReceiver();

		void receivePackets(Socket* socket);
		Buffer<Packet>& getPackets();
		Buffer<Address>& getAddresses();

	private:
		Buffer<Packet>  m_packets;
		Buffer<Address> m_addresses;
	};

}; // namespace network

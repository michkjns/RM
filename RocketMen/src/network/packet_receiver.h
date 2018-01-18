
#pragma once

#include <utility/buffer.h>
#include <network/address.h>

namespace network
{
	struct Packet;
	class Socket;

	class PacketReceiver
	{
	public:
		PacketReceiver(int32_t bufferSize);
		~PacketReceiver();

		void receivePackets(Socket* socket);
		Buffer<Packet*>& getPackets();

	private:
		Buffer<Packet*>  m_packets;

#ifdef _DEBUG
		int32_t m_numChecksumMismatches;
#endif
	};

}; // namespace network


#pragma once

#include <utility/buffer.h>
#include <network/address.h>

namespace network
{
	struct Packet;
	class Socket;

	enum class ReceiveRestriction
	{
		LAN,
		Public
	};

	class PacketReceiver
	{
	public:
		PacketReceiver(int32_t bufferSize);
		~PacketReceiver();

		void receivePackets(Socket* socket, class MessageFactory* messageFactory);
		Buffer<Packet*>& getPackets();

		void clearPackets();

		void setRestriction(ReceiveRestriction restriction) { m_restriction = restriction; }
		ReceiveRestriction getRestriction() const { return m_restriction; }

	private:
		Buffer<Packet*>  m_packets;
		ReceiveRestriction m_restriction;

#ifdef _DEBUG
		int32_t m_numChecksumMismatches;
#endif
	};

}; // namespace network

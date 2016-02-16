
#pragma once

#include "game_time.h"
#include "packet.h"
#include "peer.h"

#include <cstdint>
#include <vector>
#include <queue>

namespace network
{
	class Socket;
	class Address;

	class NetworkInterface
	{
	public:

		NetworkInterface();
		virtual	~NetworkInterface();

		enum class EState
		{
			STATE_DISCONNECTED,
			STATE_CONNECTING,
			STATE_CONNECTED,
			STATE_DISCONNECTING,
			STATE_HOSTING
		};

		void tick();

		void connect(const Address& destination, const Time& time);
		void host(uint32_t port);
		void disconnect();
		void send(Packet& packet, int32_t sequenceNumber, uint8_t channel = 0);
		
		Packet& getPacket();
		Peer&   getPeer(Address address);
		Peer&   getPeer(int32_t peerID);
		int32_t getPeerID() const;

	private:
		void handleIncomingPackets();
		Peer& newPeer(const Address& address);
		std::vector<Peer>   m_peers;
		std::queue<Packet>  m_incomingPackets;
		std::vector<Packet> m_reliablePackets;
		Socket*             m_socket;
		uint32_t            m_peerIDCounter;
		uint32_t            m_peerID;
		EState              m_state;
	};

} // namespace network
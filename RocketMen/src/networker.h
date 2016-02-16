
#pragma once

#include "bitstream.h"
#include "network_interface.h"
#include "packet.h"

#include <queue>

namespace network
{
	class Networker
	{
	public:
		Networker();
		virtual ~Networker();
	
		virtual bool initialize();
		virtual void tick(float deltaTime);
	
		bool isInitialized()	const;
		bool isConnected()		const;

	protected:
		/** Process outgoing packet queue */
		void handleOutgoingPackets(float deltaTime);

		/** Process incoming packet queue */
		void handleIncomingPackets();

		/** Transmit an outgoing packet */
		void sendPacket(Packet& packet);

		/** Push packet to outgoing queue */
		void queuePacket(const Packet& packet);

		/** Handle an incoming packet */
		virtual void handlePacket(const Packet& packet) = 0;

		/** Set sendrate for reliable packets  */
		void setReliableSendRate(uint32_t ratePerSecond);

		/** Set sendrate for unreliable packets */
		void setUnreliableSendRate(uint32_t ratePerSecond);

	protected:
		NetworkInterface    m_networkInterface;
		std::queue<Packet>  m_outgoingReliablePackets;
		std::queue<Packet>  m_outgoingUnreliablePackets;
		std::vector<Peer>   m_connectedPeers;
		int32_t             m_sequenceCounter;

		float               m_unreliableSendRate;
		float               m_updatedTimeReliable;

		float               m_reliableSendRate;
		float               m_updatedTimeUnreliable;

		bool				m_isInitialized;
		bool				m_isConnected;
	
	};	
}; // namespace network
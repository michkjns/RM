
#pragma once

#include "bitstream.h"
#include "network_interface.h"
#include "packet.h"
#include "time.h"

#include <queue>

namespace network
{
	class Networker
	{
	public:
		Networker();
		virtual ~Networker();
	
		virtual bool initialize();
		virtual void tick();
	
		bool isInitialized()	const;
		bool isConnected()		const;

	protected:
		/** Process outgoing packet queue */
		void sendPackets();

		/** Process incoming packet queue */
		void receivePackets();

		/** Transmit an outgoing packet */
		void sendPacket(const Packet& packet);

		/** Push packet to outgoing queue */
		void queuePacket(const Packet& packet);

		/** Handle an incoming packet */
		virtual void handlePacket(const Packet& packet) = 0;

	private:
		NetworkInterface	m_networkInterface;
		std::queue<Packet>	m_outgoingPackets;
		std::queue<Packet>	m_incomingPackets;
		uint64_t			m_sequenceCounter;
		bool				m_isInitialized;
		bool				m_isConnected;
	
	};	
}; // namespace network
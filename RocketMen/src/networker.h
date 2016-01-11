
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

		/** Handle an incoming packet */
		void handlePacket(const Packet& packet);

	private:
		NetworkInterface	m_networkInterface;
		std::queue<Packet>	m_outgoingPackets;
		std::queue<Packet>	m_incomingPackets;
		unsigned int		m_numClients;
		bool				m_isInitialized;
		bool				m_isConnected;
	
	};	
}; // namespace network

#pragma once

#include "address.h"
#include "network_message.h"

#include <array>
#include <vector>

namespace network
{
	class RemoteClient
	{
	public:
		RemoteClient() :
			m_address((uint32_t)0, 0),
			m_id(-1),
			m_numPlayers(0),
			m_duplicatePeers(0),
			m_sequenceCounter(0)
		{}

		bool isUsed() const;
		void queueMessage(const NetworkMessage& message);

		Address	 m_address;
		int32_t	 m_id;
		uint32_t m_numPlayers;
		uint32_t m_duplicatePeers;
		int32_t  m_sequenceCounter;

		std::array<NetworkMessage, s_maxPendingMessages> m_messageBuffer;

		/** List tracking received and unacknowledged sequenceNrs */
		std::vector<int32_t> m_reliableAckList;
	};	

}; // namespace network
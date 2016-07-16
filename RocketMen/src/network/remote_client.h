
#pragma once

#include "address.h"
#include "network_message.h"

#include <array>
#include <vector>

namespace network
{
	class RemoteClient
	{
	private:
		static const uint32_t s_sequenceMemorySize = 256;
	public:
		RemoteClient() :
			m_address((uint32_t)0, 0),
			m_id(-1),
			m_numPlayers(0),
			m_duplicatePeers(0),
			m_sequenceCounter(0),
			m_nextNetworkID(0),
			m_nextProcessed(0),
			m_timeFromLastMsg(0.0f)
		{
			std::fill(m_recentNetworkIDs, m_recentNetworkIDs + 16, -1);

			std::fill(m_recentlyProcessed, m_recentlyProcessed +
					  s_sequenceMemorySize, -1);
		}

		bool isUsed() const;
		void queueMessage(const NetworkMessage& message);

		Address	 m_address;
		int32_t	 m_id;
		uint32_t m_numPlayers;
		uint32_t m_duplicatePeers;
		int32_t  m_sequenceCounter;
		int32_t  m_recentNetworkIDs[16];
		int8_t   m_nextNetworkID;
		int32_t  m_recentlyProcessed[s_sequenceMemorySize];
		int32_t  m_nextProcessed;
		float    m_timeFromLastMsg;

		std::array<NetworkMessage, s_maxPendingMessages> m_messageBuffer;

		/** List tracking received and unacknowledged sequenceNrs */
		std::vector<int32_t> m_reliableAckList;

		friend bool operator== (const RemoteClient& a, const RemoteClient& b);
		friend bool operator!= (const RemoteClient& a, const RemoteClient& b);
	};	


}; // namespace network
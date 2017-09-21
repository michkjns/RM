
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
		static const uint32_t s_sequenceMemorySize  = 256;
		static const int32_t  s_networkIDBufferSize = 16;

	public:
		RemoteClient();

		bool isUsed() const;
		void queueMessage(const NetworkMessage& message, float time);
		void queueMessage(const OutgoingMessage& message, float time);

		Address	 m_address;
		int32_t	 m_id;
		uint32_t m_numPlayers;
		uint32_t m_duplicatePeers;
		int32_t  m_recentNetworkIDs[s_networkIDBufferSize];
		int32_t  m_recentlyProcessed[s_sequenceMemorySize];
		uint32_t m_nextMessageID;
		int32_t  m_guid;
		int8_t   m_nextNetworkID;
		float    m_timeFromLastMessage;

		std::array<OutgoingMessage, s_maxPendingMessages> m_messageBuffer;
		std::array<OutgoingMessage, s_maxPendingMessages> m_reliableBuffer;
	
		friend bool operator== (const RemoteClient& a, const RemoteClient& b);
		friend bool operator!= (const RemoteClient& a, const RemoteClient& b);
	};	
}; // namespace network
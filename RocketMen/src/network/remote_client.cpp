
#include <network/remote_client.h>
#include <core/debug.h>

using namespace network;

RemoteClient::RemoteClient() :
	m_address((uint32_t)0, 0),
	m_id(INDEX_NONE),
	m_numPlayers(0),
	m_duplicatePeers(0),
	m_nextNetworkID(0),
	m_nextProcessed(0),
	m_timeFromLastMessage(0.f)
{
	std::fill(m_recentNetworkIDs, m_recentNetworkIDs + s_networkIDBufferSize, INDEX_NONE);

	std::fill(m_recentlyProcessed, m_recentlyProcessed +
			  s_sequenceMemorySize, INDEX_NONE);
}

bool RemoteClient::isUsed() const
{
	return (m_id > INDEX_NONE);
}

void RemoteClient::queueMessage(const NetworkMessage& message, float time)
{
	for (NetworkMessage& msg : m_messageBuffer)
	{
		if (msg.type == MessageType::Clear)
		{
			msg.type       = message.type;
			msg.data.reset();
			msg.data.writeBuffer(message.data.getBuffer(), message.data.getLength());
			msg.isReliable = message.isReliable;
			msg.isOrdered  = message.isOrdered;
			break;
		}
	}

	if (message.isReliable)
	{
		for (NetworkMessage& msg : m_reliableBuffer)
		{
			if (msg.type == MessageType::Clear)
			{
				msg.type = message.type;
				msg.data.reset();
				msg.data.writeBuffer(message.data.getBuffer(), message.data.getLength());
				msg.isReliable = message.isReliable;
				msg.isOrdered = message.isOrdered;
				msg.timeOfCreation = time;
				break;
			}
		}
	}
}

bool network::operator==(const RemoteClient& a, const RemoteClient& b)
{
	return (a.m_id == b.m_id);
}

bool network::operator!=(const RemoteClient& a, const RemoteClient& b)
{
	return (a.m_id != b.m_id);
}

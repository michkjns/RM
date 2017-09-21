
#include <network/remote_client.h>
#include <core/debug.h>

using namespace network;

RemoteClient::RemoteClient() :
	m_id(INDEX_NONE),
	m_numPlayers(0),
	m_duplicatePeers(0),
	m_nextMessageID(0),
	m_guid(0),
	m_timeFromLastMessage(0.f),
	m_nextNetworkID(0)
{
	std::fill(m_recentNetworkIDs,  m_recentNetworkIDs  + s_networkIDBufferSize, INDEX_NONE);
	std::fill(m_recentlyProcessed, m_recentlyProcessed + s_sequenceMemorySize,  INDEX_NONE);
}

bool RemoteClient::isUsed() const
{
	return (m_id > INDEX_NONE);
}

void RemoteClient::queueMessage(const NetworkMessage& inMessage, float currentTime)
{
	for (OutgoingMessage& message : m_messageBuffer)
	{
		if (message.type == MessageType::None)
		{
			message.data.reset();
			message.data.writeBuffer(inMessage.data.getBuffer(), inMessage.data.getLength());
			message.sequence         = m_nextMessageID++;
			message.type       = inMessage.type;
			message.isReliable = inMessage.isReliable;
			message.isOrdered  = inMessage.isOrdered;
			message.timestamp  = currentTime;

#ifdef _DEBUG
			if (message.type != MessageType::Gamestate)
			{
				LOG_DEBUG("Server: queueMessage: ID: %d, type: %s", message.sequence, messageTypeAsString(message.type));
			}
#endif // ifdef _DEBUG
			break;
		}
	}
}

void RemoteClient::queueMessage(const OutgoingMessage& inMessage, float time)
{
	for (OutgoingMessage& message : m_messageBuffer)
	{
		if(message.type == MessageType::None)
		{
			message = inMessage;
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

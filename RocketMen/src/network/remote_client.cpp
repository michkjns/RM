
#include <network/remote_client.h>
#include <core/debug.h>

using namespace network;

bool RemoteClient::isUsed() const
{
	return (m_id >= 0);
}

void RemoteClient::queueMessage(const NetworkMessage& message)
{
	for (NetworkMessage& msg : m_messageBuffer)
	{
		if (msg.type == MessageType::MESSAGE_CLEAR)
		{
			msg.type       = message.type;
			msg.data       = message.data;
			msg.isReliable = message.isReliable;
			msg.isOrdered  = message.isOrdered;
			msg.sequenceNr = ++m_sequenceCounter;
			return;
		}
	}

	LOG_WARNING("Message Queue is full! Message discarded.");
	delete message.data;

}

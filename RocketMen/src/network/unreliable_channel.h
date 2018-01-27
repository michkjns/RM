
#pragma once

#include <utility/circular_buffer.h>
#include <network/network_channel.h>
#include <network/packet.h>

namespace network
{
	class UnreliableChannel : public NetworkChannel
	{
	public:
		UnreliableChannel();
		~UnreliableChannel();

		virtual void sendMessage(Message* message) override;

		virtual void sendPendingMessages(Socket* socket, 
			const Address& address, const Time& time, MessageFactory* messageFactory) override;

		virtual void receivePacket(Packet& packet) override;
		virtual Message* getNextMessage() override;

	private:
		bool hasMessagesToSend() const;
		Packet* createPacket(const Time& time);

		Sequence m_nextMessageId;
		uint32_t m_queuedSendMessages;
		CircularBuffer<Message*> m_sendQueue;
		CircularBuffer<Message*> m_receiveQueue;
	};
}; // namespace network

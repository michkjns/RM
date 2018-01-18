
#pragma once

#include <utility/circular_buffer.h>
#include <network/network_channel.h>
#include <network/packet.h>
#include <network/sequence_buffer.h>

namespace network
{
	class ReliableOrderedChannel : public NetworkChannel
	{
	public:
		ReliableOrderedChannel();
		~ReliableOrderedChannel();

		virtual void sendMessage(Message* message) override;

		virtual void sendPendingMessages(Socket* socket,
			const Address& address, const Time& time) override;

		virtual void receivePacket(Packet& packet) override;
		virtual IncomingMessage* getNextMessage()  override;

		float getLastPacketSendTime() const;

	private:
		void writeAcksToPacket(Packet& packet);
		void readAcksFromPacket(const Packet& packet);
		void ack(Sequence ackSequence);
		bool hasMessagesToSend(const Time& time) const;
		bool canSendMessage() const;

		Packet* createPacket(const Time& time);

		Sequence m_sendMessageId;
		Sequence m_receiveMessageId;
		Sequence m_lastReceivedSequence;
		float    m_lastPacketSendTime;

		SequenceBuffer<OutgoingMessageEntry> m_messageSendQueue;
		SequenceBuffer<IncomingMessageEntry> m_messageReceiveQueue;
		SequenceBuffer<SentPacketEntry>      m_sentPackets;
		SequenceBuffer<SentPacketEntry>      m_receivedPackets;
	};
}; // namespace network
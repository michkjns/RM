
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
			const Address& address, const Time& time, MessageFactory* messageFactory) override;

		virtual void receivePacket(Packet& packet) override;

		virtual Message* getNextMessage() override;

		float getLastPacketSendTime() const;

	private:
		void writeAcksToPacket(Packet& packet);
		void readAcksFromPacket(const Packet& packet);
		void ack(Sequence ackSequence);
		bool hasMessagesToSend(const Time& time) const;
		bool canSendMessage() const;

		Packet* createPacket(const Time& time);

		Sequence m_nextSendMessageId;
		Sequence m_nextReceiveMessageId;
		Sequence m_lastReceivedSequence;
		float    m_lastPacketSendTime;

		SequenceBuffer<OutgoingMessageEntry> m_messageSendQueue;
		SequenceBuffer<IncomingMessageEntry> m_messageReceiveQueue;
		SequenceBuffer<SentPacketEntry>      m_sentPackets;
		SequenceBuffer<SentPacketEntry>      m_receivedPackets;

		int32_t m_numReceivedPackets;
		int32_t m_numReceivedMessages;

		int32_t m_numSentPackets;
		int32_t m_numSentMessages;
		int32_t m_numAcksReceived;
	};

}; // namespace network
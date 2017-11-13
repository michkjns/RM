
#pragma once

#include <circular_buffer.h>
#include <network/network_channel.h>
#include <network/packet.h>
#include <network/sequence_buffer.h>

namespace network
{
	class ReliableOrderedChannel : public NetworkChannel
	{
	public:
		ReliableOrderedChannel(bool keepAlive);
		~ReliableOrderedChannel();

		virtual void sendMessage(Message& message)       override;

		virtual void sendPendingMessages(Socket* socket,
			const Address& address, const Time& time)    override;

		virtual void receivePacket(Packet& packet)       override;
		virtual IncomingMessage* getNextMessage()        override;

		float getLastPacketSendTime() const;

	private:
		void writeAcks(Packet* packet);
		void readAcks(const PacketHeader& packetHeader);
		bool hasMessagesToSend(const Time& time) const;
		bool canSendMessage() const;

		Packet* createPacket(const Time& time);

		Sequence m_sendMessageId;
		Sequence m_receiveMessageId;
		Sequence m_lastReceivedSequence;
		float    m_lastPacketSendTime;
		bool     m_keepAlive;

		SequenceBuffer<OutgoingMessage> m_messageSendQueue;
		SequenceBuffer<IncomingMessage> m_messageReceiveQueue;
		SequenceBuffer<SentPacketData>  m_sentPackets;
		SequenceBuffer<SentPacketData>  m_receivedPackets;
	};
}; // namespace network
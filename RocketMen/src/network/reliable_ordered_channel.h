
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

		virtual void sendMessage(const Message& message) override;

		virtual void sendPendingMessages(Socket* socket,
			const Address& address, const Time& time)    override;

		virtual void receivePacket(Packet& packet)       override;
		virtual IncomingMessage* getNextMessage()        override;

		float getLastPacketSendTime() const;

	private:
		void writeAcksToPacket(Packet* packet);
		void readAcksFromPacket(const PacketHeader& packetHeader);
		bool hasMessagesToSend(const Time& time) const;
		bool canSendMessage() const;

		Packet* createPacket(const Time& time);

		Sequence m_sendMessageId;
		Sequence m_receiveMessageId;
		Sequence m_lastReceivedSequence;
		float    m_lastPacketSendTime;

		SequenceBuffer<OutgoingMessage> m_messageSendQueue;
		SequenceBuffer<IncomingMessage> m_messageReceiveQueue;
		SequenceBuffer<SentPacketData>  m_sentPackets;
		SequenceBuffer<SentPacketData>  m_receivedPackets;
	};
}; // namespace network
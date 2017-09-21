
#pragma once

#include <game_time.h>
#include <network/address.h>
#include <network/packet.h>
#include <network/sequence_buffer.h>

#include <cstdint>
#include <vector>
#include <queue>

namespace network
{
	/** Common vars shared by Server and Client */
	static const uint32_t s_maxDuplicatePeers     = 4;
	static const uint32_t s_maxPlayersPerClient   = 4;
	static const float    s_snapshotCreationRate  = 1/20.f;
	static const uint32_t s_sentPacketsBufferSize = 1024;
	//==========================================================================

	class Socket;
	class NetworkInterface
	{
	public:
		NetworkInterface();
		virtual	~NetworkInterface();

	public: 
		enum class State
		{
			Disconnected,
			Connecting,
			Connected,
			Disconnecting,
			Hosting
		};

		void update(const Time& time);
		void connect(const Address& destination, const Time& time);
		bool listen(uint32_t port);
		void sendMessages(OutgoingMessage* messages, uint32_t numMessages,
			              const Address& destination);

		void sendPacket(const Address& destination, Packet* packet);

		/** @return true if attempting a connection */
		bool isConnecting() const;

		/** @return queue of incoming messages */
		std::queue<IncomingMessage>& getMessages();

		/** @return queue of incoming ordered messages */
		std::queue<IncomingMessage>& getOrderedMessages();

		void clearBuffers();
		void disconnect(const Address& address);
		SequenceBuffer<SentMessage>& getAcks();

	private:
		void receivePackets();
		void readAcks(Sequence baseSequence, uint32_t ackBits);
		void setState(State state);

		std::vector<Packet>            m_outgoingPackets;
		std::queue<IncomingMessage>    m_incomingMessages;
		std::queue<IncomingMessage>    m_incomingMessagesOrdered;
		SequenceBuffer<SentPacketData> m_sentPackets;
		SequenceBuffer<SentMessage>    m_acks;

		Socket*  m_socket;
		float    m_stateTime;
		State    m_state;
		int32_t  m_receivedMessageCount;
		Sequence m_sequenceCounter;
		Address  m_remoteAddress;
	};

} // namespace network
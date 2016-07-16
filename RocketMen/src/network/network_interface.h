
#pragma once

#include <network/address.h>
#include <game_time.h>
#include <network/packet.h>

#include <cstdint>
#include <vector>
#include <queue>

namespace network
{
	/** Common vars shared by Server and Client */
	static const uint32_t s_maxDuplicatePeers    = 4;
	static const uint32_t s_maxPlayersPerClient  = 4;
	static const float    s_snapshotCreationRate = 1/20.f;
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
			STATE_DISCONNECTED,
			STATE_CONNECTING,
			STATE_CONNECTED,
			STATE_DISCONNECTING,
			STATE_HOSTING
		};

		void update(float deltaTime);
		void connect(const Address& destination, const Time& time);
		void host(uint32_t port);
		
		/** Sends message directly, ignoring reliability */
		void sendMessage(const Address& destination, NetworkMessage& message);

		/** Sends a batch of messages */
		void sendMessages(const Address& destination,
						  std::vector<NetworkMessage>& messages);

		/** @return true if attempting a connection */
		bool isConnecting() const;

		/** Returns queue of incoming messages */
		std::queue<IncomingMessage>& getMessages();

		/** Returns queue of incoming messages */
		std::queue<IncomingMessage>& getOrderedMessages();

		void clearBuffers();
	private:
		/** Directly sends a packet */
		void sendPacket(const Address& destination, Packet* packet);

		void receivePackets();
		void setState(State state);

		std::vector<Packet>         m_outgoingPackets;
		std::queue<IncomingMessage> m_incomingMessages;
		std::queue<IncomingMessage> m_incomingMessagesOrdered;

		Socket* m_socket;
		float   m_stateTimer;
		State   m_state;
		//uint64_t m_packetSequence;
	};

} // namespace network
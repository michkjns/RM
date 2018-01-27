
#pragma once
#include <network/address.h>
#include <network/connection_callback.h>

class Time;

namespace network
{
	struct Message;
	struct Packet;
	class  MessageFactory;
	class  ReliableOrderedChannel;
	class  Socket;
	class  UnreliableChannel;
	
	class Connection
	{
	public:
		enum class State
		{
			Disconnected,
			Connecting,
			Connected,
			Closed
		};

	public:
		Connection(Socket* socket, const Address& address,
			ConnectionCallbackMethod callback, 
			MessageFactory& messageFactory);

		~Connection();

		void update(const Time& time);

		void sendMessage(Message* message);
		void sendPendingMessages(const Time& time);
		void receivePacket(Packet& packet);
		void close();

		Message* getNextMessage();
		
		const Address& getAddress() const;
		State getState() const;
		void setState(State state);
		void tryConnect();

		bool isClosed() const;

	private:
		Address  m_address;
		Socket*  m_socket;
		uint32_t m_connectionAttempt;
		float    m_timeSinceLastPacketReceived;
		State    m_state;
		float    m_connectionAttemptDuration;

		UnreliableChannel*      m_unreliableChannel;
		ReliableOrderedChannel* m_reliableOrderedChannel;

		ConnectionCallbackMethod m_connectionCallback;
		MessageFactory& m_messageFactory;
	};

}; // namespace network

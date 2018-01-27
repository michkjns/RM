
#pragma once

class Time;

namespace network 
{
	class  Address;
	struct Message;
	class  MessageFactory;
	struct Packet;
	class  Socket;
	
	class NetworkChannel
	{
	public:
		virtual void sendMessage(Message* message) = 0;

		virtual void sendPendingMessages(Socket* socket, 
			const Address& address, const Time& time, MessageFactory* messageFactory) = 0;

		virtual void receivePacket(Packet& packet) = 0;

		virtual Message* getNextMessage() = 0;

	protected:
		void sendPacket(Socket* socket, const Address& address, Packet* packet, MessageFactory* messageFactory);
		Message* readMessage(Packet& packet);
	};

}; // namespace network
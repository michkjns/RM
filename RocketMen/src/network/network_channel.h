
#pragma once

class Time;

namespace network 
{
	class  Address;
	struct IncomingMessage;
	struct Message;
	class  Packet;
	class  Socket;
	
	class NetworkChannel
	{
	public:
		virtual void sendMessage(const Message& message)       = 0;
		virtual void sendPendingMessages(Socket* socket, 
			const Address& address, const Time& time)    = 0;

		virtual void receivePacket(Packet& packet)       = 0;
		virtual IncomingMessage* getNextMessage()        = 0;

	protected:
		void sendPacket(Socket* socket, const Address& address, Packet* packet);
	};

}; // namespace network

#pragma once

#include <cstdint>

namespace network
{
	class Socket;
	class Address;

	enum EBroadcast
	{
		BROADCAST_SINGLE,
		BROADCAST_ALL,
	};

	enum EReliable
	{
		UNRELIABLE,
		RELIABLE,
	};
	
	class NetworkInterface
	{
	public:
		NetworkInterface();
		virtual	~NetworkInterface();

		void tick();

		void connect(const Address& destination);
		void host(uint32_t port);
		void disconnect();

	//	void Send(const Packet& packet, uint8_t channel = 0);

		void send(const void* data, int32_t dataLength,
				EBroadcast broadcast, int32_t recipientID,
				EReliable reliable = EReliable::RELIABLE,
				uint8_t channel = 0);
		

	private:
		Socket* m_socket;
	};

} // namespace network
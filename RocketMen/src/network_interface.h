
#pragma once

#include <stdint.h>

namespace network
{
	class Socket;

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

		void Send(const void* data, int32_t dataLength,
				  EBroadcast broadcast, int32_t recipientID,
				  EReliable reliable = EReliable::RELIABLE,
				  uint8_t channel = 0);


	private:
		Socket* m_socket;
	};

} // namespace network
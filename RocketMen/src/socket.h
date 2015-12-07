
#pragma once

#include "netadress.h"

#include <stdint.h>
#include <winsock2.h>

#define DEFAULT_PORT 4321
#define LOCAL_HOST "127.0.0.1"

namespace network 
{
	class Socket 
	{
	public:
		virtual ~Socket() {}
		virtual bool initialize(int port = DEFAULT_PORT, bool isHost = false) = 0;
		virtual bool isInitialized() = 0;

		virtual int receive() = 0;
		virtual bool send(Adress adress, char* buffer, int bufferLength) = 0;
	
		virtual int getPort() = 0;
		virtual uint64_t getBytesReceived() = 0;
		virtual uint64_t getBytesSent() = 0;
		virtual uint64_t getPacketsReceived() = 0;
		virtual uint64_t getPacketsSent() = 0;
	
		enum class NetProtocol
		{
			ESocket_UDP,
			ESocket_TCP
		};

		static Socket* create(NetProtocol type);
	};

}; // namespace network


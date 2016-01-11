
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
		virtual bool initialize(uint32_t port = DEFAULT_PORT, bool isHost = false) = 0;
		virtual bool isInitialized() = 0;

		virtual int receive() = 0;
		virtual bool send(Adress adress, const char* buffer, int bufferLength) = 0;
	
		virtual uint32_t getPort()				const = 0;
		virtual uint64_t getBytesReceived()		const = 0;
		virtual uint64_t getBytesSent()			const = 0;
		virtual uint64_t getPacketsReceived()	const = 0;
		virtual uint64_t getPacketsSent()		const = 0;
	
		enum class ENetProtocol
		{
			PROTOCOL_UDP,
			PROTOCOL_TCP
		};

		static Socket* create(ENetProtocol type);
	};

}; // namespace network



#pragma once

#include "address.h"

#include <stdint.h>

static const uint32_t	g_defaultPort	= 4321;
static const char*		g_localHost		= "127.0.0.1";

namespace network 
{
	class Socket 
	{
	public:
		virtual ~Socket() {}
		virtual bool initialize(uint32_t port = g_defaultPort, bool isHost = false) = 0;
		virtual bool isInitialized()			const = 0;

		virtual int receive() = 0;
		virtual bool send(Address adress, const void* buffer, int bufferLength) = 0;
	
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


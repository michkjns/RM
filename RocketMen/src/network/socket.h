
#pragma once

#include <bitstream.h>
#include <network/address.h>
#include <network/packet.h>

#include <cstdint>

namespace network 
{
	class Socket 
	{
	public:
		virtual ~Socket() {}
		virtual bool initialize(uint32_t port, bool isHost = false) = 0;
		virtual bool isInitialized() const = 0;

		/** Receive
		* @param Addres& address  Address of sender
		* @param char* buffer     Buffer for received data
		* @param int32_t& length  Length of received data
		* @return true when something is received
		*/
		virtual bool receive(Address& address, char* buffer, int32_t& length) = 0;

		/** Send
		* @param const Address adress  Destination adddress
		* @param const void*     Buffer  buffer to send
		* @param const size_t length   Length of data to read from buffer
		* @return true when no error occurred
		*/
		virtual bool send(const Address& address, const void* buffer, 
						  const size_t length) = 0;

		virtual uint32_t getPort()				const = 0;
		virtual uint64_t getBytesReceived()		const = 0;
		virtual uint64_t getBytesSent()			const = 0;
		virtual uint64_t getPacketsReceived()	const = 0;
		virtual uint64_t getPacketsSent()		const = 0;
	
		enum class NetProtocol
		{
			PROTOCOL_UDP,
			PROTOCOL_TCP
		};

		static Socket* create(NetProtocol type);
	};

}; // namespace network


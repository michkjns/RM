
#pragma once

#include "bitstream.h"
#include "network_message.h" 

#include <stdint.h>

namespace network
{
	struct PacketHeader
	{
		/** Number of messages in packet */
		int32_t  messageCount;
		int32_t  sequenceNumber;
		uint16_t dataLength;
	};

	static const int32_t g_maxBlockSize = 2048;
	static const int32_t g_packetHeaderSize = sizeof(PacketHeader);
	static const int32_t g_maxPacketSize = (g_maxBlockSize + g_packetHeaderSize);

	class Packet
	{
	public:
		Packet();
		~Packet() {}
		PacketHeader header;

		/** Writes a message to the buffer 
		* @param const NetworkMessage& message   Message to write
		*/
		void writeMessage(const NetworkMessage& message);

		/** Writes data to buffer
		* @param void* data          Pointer to data to write
		* @param const size_t length Length of the data
		*/
		void writeData(const void* data, const size_t length);

		/** Reads NetworkMessage from the packet 
		* @return NetworkMessage Message read
		*/
		IncomingMessage readMessage();

		/** Gets the packet data
		* @return char* Pointer to data buffer 
		*/
		char* getData() const;

	private:
		unsigned char m_data[g_maxBlockSize];
		int32_t m_read;
	};

}; // namespace network
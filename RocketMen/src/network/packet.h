
#pragma once

#include "bitstream.h"
#include "network_message.h" 

#include <cstdint>

namespace network
{
	static const int32_t g_maxMessagesPerPacket = 64;
	struct SentPacketData
	{
		uint16_t numMessages;
		Sequence messageIds[g_maxMessagesPerPacket];
	};

	struct PacketHeader
	{
		/** Number of messages in packet */
		int32_t  numMessages;
		uint32_t ackBits;
		uint32_t hash;
		Sequence sequence;
		uint16_t ackSequence;
		uint16_t dataLength;
	};

	static const int32_t g_protocolId = 1000;
	static const int32_t g_maxBlockSize = 2048;
	static const int32_t g_packetBufferSize = g_maxBlockSize + sizeof(g_protocolId);
	static const int32_t g_packetHeaderSize = sizeof(PacketHeader);
	static const int32_t g_maxPacketSize = (g_maxBlockSize + g_packetHeaderSize);
	static const int32_t g_messageOverhead = sizeof(MessageType) + sizeof(Sequence) + sizeof(int32_t);
// ============================================================================

	class Packet
	{
	public:
		Packet();
		Packet(ChannelType channel);
		~Packet() {}
		PacketHeader header;

		/** Writes a message to the buffer 
		* @param const NetworkMessage& message   Message to write
		*/
		void writeMessage(const OutgoingMessage& message);

		/** Writes data to buffer
		* @param void* data          Pointer to data to write
		* @param const size_t length Length of the data
		*/
		void writeData(const void* data, const size_t length);

		/** Reads NetworkMessage from the packet 
		* @return NetworkMessage Message read
		*/
		IncomingMessage* readNextMessage();

		void resetReading();

		/** Gets the packet data
		* @return char* Pointer to data buffer 
		*/
		char* getData() const;

		bool isEmpty() const;
		ChannelType getChannel() const;

		bool getError() const;

	private:
		unsigned char m_data[g_packetBufferSize];
		int32_t       m_read;
		Sequence      m_messageIDs[g_maxMessagesPerPacket];
		ChannelType   m_channel;
		bool          m_error;
	};

}; // namespace network

#pragma once

#include <network/network_message.h>
#include <utility/bitstream.h>

#include <cstdint>

namespace network
{
	static const int32_t g_maxMessagesPerPacket = 64;
	struct SentPacketEntry
	{
		uint16_t numMessages;
		Sequence messageIds[g_maxMessagesPerPacket];
	};

	static const int32_t g_protocolId = 1000;
	static const int32_t g_maxBlockSize = 2048;
	static const int32_t g_packetBufferSize = g_maxBlockSize + sizeof(g_protocolId);
	static const int32_t g_maxPacketSize = (g_maxBlockSize);
// ============================================================================

	struct Packet
	{
		Packet();
		~Packet() {}

		Address address;

		struct {
			int32_t  numMessages;
			uint32_t ackBits;
			Sequence sequence;
			Sequence ackSequence;
		} header;

		Sequence messageIds[g_maxMessagesPerPacket];
		Message* messages[g_maxMessagesPerPacket];

		template<typename Stream>
		bool serialize(Stream& stream)
		{
			assert(serializeCheck(stream, "packet_start"));

			serializeData(stream, (char*)&header, sizeof(header));

			for (int32_t i = 0; i < header.numMessages; i++)
			{
				serializeBits(stream, messageIds[i], 16);
	
				if (Stream::isWriting)
				{
					assert(messages[i]->type > MessageType::None);
					assert(messages[i]->type < MessageType::NUM_MESSAGE_TYPES);
					serializeBits(stream, messages[i]->type, 8);
					assert(messages[i]->data.getDataLength() < g_maxPacketSize);
					int32_t messageSize = messages[i]->data.getDataLength();

					serializeInt(stream, messageSize);
					serializeData(stream, messages[i]->data.getData(), messageSize);
				}
				else
				{
					MessageType messageType = MessageType::None;
					serializeBits(stream, messageType, 8);
					assert(messageType > MessageType::None);
					assert(messageType < MessageType::NUM_MESSAGE_TYPES);

					int32_t messageSize = 0;
					serializeInt(stream, messageSize);
					assert(messageSize < g_maxPacketSize);

					messages[i] = new Message(messageType, messageSize);
					serializeData(stream, messages[i]->data.getData(), messageSize);
				}
			}

			assert(serializeCheck(stream, "packet_end"));
			if (Stream::isWriting)
			{
				stream.flush();
			}
			return true;
		}

	};

}; // namespace network
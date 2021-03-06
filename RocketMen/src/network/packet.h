
#pragma once

#include <network/message.h>
#include <network/message_factory.h>
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
		Packet::Packet()
		{
			header = {};
		}

		Packet::~Packet()
		{
			for (int32_t i = 0; i < header.numMessages; i++)
			{
				messages[i]->releaseRef();
			}
		}
		
		struct {
			int32_t  numMessages;
			uint32_t ackBits;
			Sequence sequence;
			Sequence ackSequence;
		} header;

		Address     address;
		Sequence    messageIds[g_maxMessagesPerPacket];
		MessageType messageTypes[g_maxMessagesPerPacket];
		Message*    messages[g_maxMessagesPerPacket];

		template<typename Stream>
		bool serialize(Stream& stream, MessageFactory* messageFactory)
		{
			serializeCheck(stream, "packet_start");

			/** Serialize Header */
			if (Stream::isWriting)
			{
				ASSERT(header.numMessages >= 0 && header.numMessages < g_maxMessagesPerPacket,
					"Invalid number of messages specified in packet");
			}
			serializeData(stream, (char*)&header, sizeof(header));
			if (Stream::isReading)
			{
				if (header.numMessages <= 0 || header.numMessages >= g_maxMessagesPerPacket)
				{
					ASSERT(false, "Invalid value for numMessages");
					return false;
				}
			}

			serializeCheck(stream, "packet_test");

			/** Serialize Message Ids */
			for (int32_t i = 0; i < header.numMessages; i++)
				serializeBits(stream, messageIds[i], 16);
			
			// serializeData(stream, reinterpret_cast<char*>(messageIds), sizeof(messageIds[0]) * header.numMessages); // TODO - bug in serialization classes?
	
			/** Serialize Message Types */
			for (int32_t i = 0; i < header.numMessages; i++)
				serializeBits(stream, messageTypes[i], 8);

			/** Serialize Message Data*/
			for (int32_t i = 0; i < header.numMessages; i++)
			{
				if (Stream::isWriting)
				{
					ASSERT(messageTypes[i] > MessageType::None);
					ASSERT(messageTypes[i] < MessageType::NUM_MESSAGE_TYPES);
				}
			
				if (Stream::isReading)
				{
					ASSERT(messageFactory != nullptr);
					if (messageTypes[i] <= MessageType::None || messageTypes[i] >= MessageType::NUM_MESSAGE_TYPES)
					{
						return false;
					}

					messages[i] = messageFactory->createMessage(messageTypes[i]);

					if (messages[i] == nullptr)
					{
						return false;
					}
					messages[i]->assignId(messageIds[i]);
				}

				serializeCheck(stream, "begin_message");
				if (!messages[i]->serialize(stream))
				{
					return false;
				}
				serializeCheck(stream, "end_message");
			}

			serializeCheck(stream, "packet_end");
			if (Stream::isWriting)
			{
				stream.flush();
			}
			return true;
		}
	};

}; // namespace network

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
			assert(serializeCheck(stream, "packet_start"));

			/** Serialize Header */
			if (Stream::isWriting)
			{
				assert(header.numMessages >= 0 && header.numMessages < g_maxMessagesPerPacket);
			}
			serializeData(stream, (char*)&header, sizeof(header));
			if (Stream::isReading)
			{
//				int32_t asdc = 3;
				if (header.numMessages <= 0 || header.numMessages >= g_maxMessagesPerPacket)
				{
					return ensure(false);
				}
			}

			assert(serializeCheck(stream, "packet_test"));

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
					assert(messageTypes[i] > MessageType::None);
					assert(messageTypes[i] < MessageType::NUM_MESSAGE_TYPES);
				}
			
				if (Stream::isReading)
				{
					assert(messageFactory != nullptr);
					if (messageTypes[i] <= MessageType::None || messageTypes[i] >= MessageType::NUM_MESSAGE_TYPES)
					{
						return ensure(false);
					}

					messages[i] = messageFactory->createMessage(messageTypes[i]);

					if (messages[i] == nullptr)
					{
						return ensure(false);
					}
					messages[i]->assignId(messageIds[i]);
				}

				assert(serializeCheck(stream, "begin_message"));
				if (!messages[i]->serialize(stream))
				{
					return ensure(false);
				}
				assert(serializeCheck(stream, "end_message"));
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
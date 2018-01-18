
#pragma once

#include <utility/bitstream.h>
#include <common.h>
#include <network/address.h>
#include <network/message_type.h>

namespace network
{
	const int32_t s_defaultMessageBufferSize = 128;

	struct Message
	{
		Message(MessageType inType, int32_t bufferSize = s_defaultMessageBufferSize) :
			type(inType), data(bufferSize) {}

		Sequence    id;
		MessageType type;
		WriteStream data;
	};

	struct OutgoingMessageEntry
	{
		OutgoingMessageEntry() : 
			message(nullptr), timeLastSent(0.f) {}

		OutgoingMessageEntry(Message* msg) : 
			message(msg), timeLastSent(0.f) { assert(msg != nullptr); }

		Message* message;
		float    timeLastSent;
	};

	struct IncomingMessage
	{
		IncomingMessage(MessageType messageType, Sequence messageId, const char* sourceData, int32_t sourceDataSize) : 
			type(messageType), id(messageId), data(sourceData, sourceDataSize) {}

		void markAsRead() {	type = MessageType::None; }

		MessageType type;
		ReadStream  data;
		Sequence    id;
		Address     address;
	};

	struct IncomingMessageEntry
	{
		IncomingMessageEntry() : message(nullptr) {}
		IncomingMessage* message;
	};

	inline ChannelType getMessageChannel(struct Message* message)
	{
		return getMessageChannel(message->type);
	}

}; // namespace network
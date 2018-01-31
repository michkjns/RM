
#pragma once

#include <utility/bitstream.h>
#include <common.h>
#include <network/address.h>
#include <network/message_type.h>

#define DECLARE_MESSAGE( name, channel ) \
	MessageType getType() const override { return MessageType::name; } \
	ChannelType getChannel() const override { return ChannelType::channel; } \
	bool serialize(WriteStream& stream) override { return serialize_impl(stream); } \
	bool serialize(ReadStream& stream) override { return serialize_impl(stream); }

namespace network
{
	struct Message
	{
		Message();

		void assignId(Sequence id) { m_id = id;	}
		Sequence getId() const { return m_id; }
	
		Message* addRef();
		void releaseRef();

		int32_t getRefCount() const { return m_refCount; }
		virtual MessageType getType() const = 0;
		virtual ChannelType getChannel() const = 0;

		virtual bool serialize(WriteStream& stream) = 0;
		virtual bool serialize(ReadStream& stream) = 0;

	protected:
		virtual ~Message();
	private: 

		int32_t     m_refCount;
		Sequence    m_id;
	};

	struct IncomingMessageEntry
	{
		IncomingMessageEntry() : message(nullptr) {}

		Message* message;
	};

	struct OutgoingMessageEntry
	{
		OutgoingMessageEntry() : message(nullptr) {}

		Message* message;
		float timeLastSent;
	};

}; // namespace network
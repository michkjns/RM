
#pragma once
#pragma warning(default:4061)
#pragma warning(default:4062)

#include <cstdint>

namespace network
{
	enum class MessageType : int8_t
	{
		None = 0,

		ClockSync,
		KeepAlive,
		Disconnect,

		// Server to client
		AcceptClient,
		AcceptPlayer,
		Gamestate,
		SpawnEntity,
		AcceptEntity,
		DestroyEntity,
		GameEvent,

		// Client to server
		RequestConnection,
		IntroducePlayer,
		PlayerInput,
		RequestEntity,

		NUM_MESSAGE_TYPES
	};

	enum class ChannelType : uint8_t
	{
		Unreliable,
		ReliableOrdered
	};

	inline ChannelType getMessageChannel(MessageType type)
	{
		switch (type)
		{
			case MessageType::ClockSync:
			case MessageType::Gamestate:
			{
				return ChannelType::Unreliable;
			}
			case MessageType::AcceptClient:
			case MessageType::AcceptEntity:
			case MessageType::AcceptPlayer:
			case MessageType::DestroyEntity:
			case MessageType::Disconnect:
			case MessageType::GameEvent:
			case MessageType::IntroducePlayer:
			case MessageType::KeepAlive:
			case MessageType::PlayerInput:
			case MessageType::RequestConnection:
			case MessageType::RequestEntity:
			case MessageType::SpawnEntity:
			{
				return ChannelType::ReliableOrdered;
			}
			case MessageType::None:
			case MessageType::NUM_MESSAGE_TYPES:
			{
				assert(false);
				return ChannelType::Unreliable;
			};
		}
		assert(false);

		return ChannelType::Unreliable;
	};

#define CASE_RETURN_STRING(name) \
	case name: return #name

	static inline const char* messageTypeAsString(MessageType type)
	{
		switch (type)
		{
			CASE_RETURN_STRING(MessageType::None);
			CASE_RETURN_STRING(MessageType::ClockSync);
			CASE_RETURN_STRING(MessageType::KeepAlive);
			CASE_RETURN_STRING(MessageType::Disconnect);
			CASE_RETURN_STRING(MessageType::AcceptClient);
			CASE_RETURN_STRING(MessageType::AcceptPlayer);
			CASE_RETURN_STRING(MessageType::Gamestate);
			CASE_RETURN_STRING(MessageType::SpawnEntity);
			CASE_RETURN_STRING(MessageType::AcceptEntity);
			CASE_RETURN_STRING(MessageType::DestroyEntity);
			CASE_RETURN_STRING(MessageType::GameEvent);
			CASE_RETURN_STRING(MessageType::RequestConnection);
			CASE_RETURN_STRING(MessageType::IntroducePlayer);
			CASE_RETURN_STRING(MessageType::PlayerInput);
			CASE_RETURN_STRING(MessageType::RequestEntity);
			CASE_RETURN_STRING(MessageType::NUM_MESSAGE_TYPES);
		}

		assert(false);
		return "Invalid MessageType";
	}
#undef CASE_RETURN_STRING

};
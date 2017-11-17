
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

#define ReturnStringEnumCase(name) \
	case name: return #name

	static inline const char* messageTypeAsString(MessageType type)
	{
		switch (type)
		{
			ReturnStringEnumCase(MessageType::None);
			ReturnStringEnumCase(MessageType::ClockSync);
			ReturnStringEnumCase(MessageType::KeepAlive);
			ReturnStringEnumCase(MessageType::Disconnect);
			ReturnStringEnumCase(MessageType::AcceptClient);
			ReturnStringEnumCase(MessageType::AcceptPlayer);
			ReturnStringEnumCase(MessageType::Gamestate);
			ReturnStringEnumCase(MessageType::SpawnEntity);
			ReturnStringEnumCase(MessageType::AcceptEntity);
			ReturnStringEnumCase(MessageType::DestroyEntity);
			ReturnStringEnumCase(MessageType::GameEvent);
			ReturnStringEnumCase(MessageType::RequestConnection);
			ReturnStringEnumCase(MessageType::IntroducePlayer);
			ReturnStringEnumCase(MessageType::PlayerInput);
			ReturnStringEnumCase(MessageType::RequestEntity);
			ReturnStringEnumCase(MessageType::NUM_MESSAGE_TYPES);
		}

		assert(false);
		return "Invalid MessageType";
	}
#undef ReturnStringEnumCase

};
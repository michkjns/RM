
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
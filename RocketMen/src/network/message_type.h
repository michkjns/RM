
#pragma once
#pragma warning(default:4061)
#pragma warning(default:4062)

#include <cstdint>

namespace network
{
	enum class MessageType : int8_t
	{
		None = 0,

		KeepAlive,
		Disconnect,

		// Server to client
		AcceptConnection,
		AcceptPlayer,
		Snapshot,
		SpawnEntity,
		DestroyEntity,
		GameEvent,
		ServerTime,

		// Client to server
		RequestConnection,
		IntroducePlayer,
		PlayerInput,
		RequestEntity,
		RequestTime,

		NUM_MESSAGE_TYPES
	};

	enum class ChannelType : uint8_t
	{
		UnreliableUnordered,
		ReliableOrdered
	};

}; // namespace network
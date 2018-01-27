
#pragma once

#include <network/message.h>

namespace network {
namespace message {

		struct GameState : public Message
		{
			DECLARE_MESSAGE(GameState, ReliableOrdered);

			template<typename Stream>
			bool serialize_impl(Stream& stream)
			{
				serializeCheck(stream, "begin_game_state");

				serializeCheck(stream, "end_game_state");
				return true;
			}
		};

}; // namespace message
};// namespace network
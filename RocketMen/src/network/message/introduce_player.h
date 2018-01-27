
#pragma once

#include <network/message.h>

namespace network {
namespace message {

	struct IntroducePlayer : public Message
	{
		DECLARE_MESSAGE(IntroducePlayer, ReliableOrdered);

		template<typename Stream>
		bool serialize_impl(Stream& stream)
		{
			if (!serializeCheck(stream, "begin_introduce_player"))
			{
				return false;
			}

			if (Stream::isWriting)
			{
				assert(numPlayers >= 1 && numPlayers <= s_maxPlayersPerClient);
			}

			serializeInt(stream, numPlayers, 1, s_maxPlayersPerClient);

			if (Stream::isReading)
			{
				if (numPlayers < 1 || numPlayers > s_maxPlayersPerClient)
				{
					return false;
				}
			}

			if (!serializeCheck(stream, "end_introduce_player"))
			{
				return false;
			}

			return true;
		}

		int32_t numPlayers;

	};

}; // namespace message
};// namespace network
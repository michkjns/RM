
#pragma once

#include <network/message.h>
#include <network/common_network.h>

namespace network {
namespace message {

		struct AcceptPlayer : public Message
		{
			DECLARE_MESSAGE(AcceptPlayer, ReliableOrdered);

			template<typename Stream>
			bool serialize_impl(Stream& stream)
			{
				serializeCheck(stream, "begin_accept_player");

				if (Stream::isWriting)
				{
					ASSERT(numPlayers >= 1 && numPlayers <= s_maxPlayersPerClient);
				}

				serializeInt(stream, numPlayers, 1, s_maxPlayersPerClient);

				if (Stream::isReading)
				{
					if (numPlayers < 1 || numPlayers > s_maxPlayersPerClient)
					{
						return false;
					}
				}

				for (int32_t i = 0; i < numPlayers; i++)
				{
					if (Stream::isWriting)
					{
						ASSERT(playerIds[i] >= 0);
					}

					serializeBits(stream, playerIds[i], 16);

					if (Stream::isReading)
					{
						if (playerIds[i] < 0)
						{
							return false;
						}
					}
				}

				serializeCheck(stream, "end_accept_player");

				return true;
			}

			int32_t numPlayers;
			int16_t playerIds[s_maxPlayersPerClient];

		};

}; // namespace message
};// namespace network
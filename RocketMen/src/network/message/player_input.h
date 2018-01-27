
#pragma once

#include <network/message.h>

namespace network {
namespace message {

		struct PlayerInput : public Message
		{
			DECLARE_MESSAGE(PlayerInput, ReliableOrdered);
			static const int32_t maxDataLength = 512;

			template<typename Stream>
			bool serialize_impl(Stream& stream)
			{
				serializeCheck(stream, "begin_player_input");

				if (Stream::isWriting)
				{
					assert(numFrames > 0);
				}
				serializeInt(stream, numFrames);
				if (Stream::isReading)
				{
					if (numFrames <= 0)
					{
						return false;
					}
				}
				serializeInt(stream, startFrame);
				serializeInt(stream, numPlayers);
			
				serializeCheck(stream, "end_player_input");

				return true;
			}

			char    data[maxDataLength];
			int32_t dataLength;
			int32_t numFrames;
			int32_t startFrame;
			int32_t numPlayers;
		};

}; // namespace message
};// namespace network

#pragma once

#include <network/message_factory.h>

#include <network/message/disconnect.h>
#include <network/message/introduce_player.h>
#include <network/message/keep_alive.h>
#include <network/message/player_input.h>
#include <network/message/request_connection.h>
#include <network/message/request_entity.h>
#include <network/message/request_time.h>

namespace network {

	class MessageFactoryClient : public MessageFactory
	{
	public:
		MessageFactoryClient() {};
		~MessageFactoryClient() {};

		virtual Message* createMessage(MessageType type) override
		{
			switch (type)
			{
				case MessageType::Disconnect:
				{
					return new message::Disconnect();
				}
				case MessageType::IntroducePlayer:
				{
					return new message::IntroducePlayer();
				}
				case MessageType::PlayerInput:
				{
					return new message::PlayerInput();
				}
				case MessageType::RequestConnection:
				{
					return new message::RequestConnection();
				}
				case MessageType::RequestEntity:
				{
					return new message::RequestEntity();
				}
				case MessageType::KeepAlive:
				{
					return new message::KeepAlive();
				}
				case MessageType::RequestTime:
				{
					return new message::RequestTime();
				}

				case MessageType::None:
				case MessageType::AcceptConnection:
				case MessageType::AcceptPlayer:
				case MessageType::Snapshot:
				case MessageType::SpawnEntity:
				case MessageType::DestroyEntity:
				case MessageType::GameEvent:
				case MessageType::ServerTime:
				case MessageType::NUM_MESSAGE_TYPES:
				{
					ASSERT(false, "Illegal MessageType passed");
					return nullptr;
				}
			}

			return nullptr;
		}
	};
}; // namespace network
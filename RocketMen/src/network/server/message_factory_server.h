
#pragma once

#include <network/message_factory.h>

#include <core/debug.h>

#include <network/message/accept_connection.h>
#include <network/message/accept_player.h>
#include <network/message/destroy_entity.h>
#include <network/message/disconnect.h>
#include <network/message/keep_alive.h>
#include <network/message/server_time.h>
#include <network/message/snapshot.h>
#include <network/message/spawn_entity.h>

namespace network {

	class MessageFactoryServer : public MessageFactory
	{
	public:
		MessageFactoryServer() {};
		~MessageFactoryServer() {};

		virtual Message* createMessage(MessageType type) override
		{
			switch (type)
			{
				case MessageType::AcceptConnection:
				{
					return new message::AcceptConnection();
				}
				case MessageType::AcceptPlayer:
				{
					return new message::AcceptPlayer();
				}
				case MessageType::Disconnect:
				{
					return new message::Disconnect();
				}
				case MessageType::KeepAlive:
				{
					return new message::KeepAlive();
				}
				case MessageType::DestroyEntity:
				{
					return new message::DestroyEntity();
				}
				case MessageType::Snapshot:
				{
					return new message::Snapshot();
				}
				case MessageType::ServerTime:
				{
					return new message::ServerTime();
				}
				case MessageType::SpawnEntity:
				{
					return new message::SpawnEntity();
				}

				case MessageType::None:
				case MessageType::GameEvent:
				case MessageType::RequestConnection:
				case MessageType::IntroducePlayer:
				case MessageType::PlayerInput:
				case MessageType::RequestEntity:
				case MessageType::RequestTime:
				case MessageType::NUM_MESSAGE_TYPES:
				{
					LOG_ERROR("MessageFactoryServer::createMessage Message Type %d not allowed", (int32_t)type);
					assert(false);
					return nullptr;
				}
			}

			assert(false);
			return nullptr;
		}
	};
}; // namespace network
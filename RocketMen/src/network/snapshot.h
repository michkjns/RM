
#pragma once

#include <utility/bitstream.h>
#include <network/network_message.h>

#include <vector>

class Entity;

namespace network
{
	class Snapshot
	{
	public:
		static Message* createMessage(std::vector<Entity*>& entities);
	};
};
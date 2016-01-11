
#pragma once

#include "netadress.h"

#include <stdint.h>

namespace network
{
	struct Peer
	{
		Adress address;
		uint32_t peerID;
		uint32_t numPlayers;
	};
}; // namespace network
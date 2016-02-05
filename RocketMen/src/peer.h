
#pragma once

#include "address.h"

namespace network
{
	struct Peer
	{
		Peer() :
			peerID(-1),
			numPlayers(0)
		{}

		Address	 address;
		int32_t	 peerID;
		uint32_t numPlayers;


		operator bool() const { return peerID >= 0; }
	};
	
	

}; // namespace network
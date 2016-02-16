
#pragma once

#include "address.h"

namespace network
{
	struct Peer
	{
		Peer()
			: address((uint32_t)0, 0)
			, peerID(-1)
			, numPlayers(0)
			, duplicatePeers(0)
		{}

		Address	 address;
		int32_t	 peerID;
		uint32_t numPlayers;
		uint32_t duplicatePeers;


		operator bool() const { return peerID >= 0; }
	};
	
	

}; // namespace network
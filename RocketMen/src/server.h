
#pragma once

#include "networker.h"

namespace network
{
	class Server
	{
	public:
		virtual ~Server() {};
	
		virtual bool initialize() = 0;
		virtual void tick(uint64_t dt) = 0;
	
		virtual unsigned int getNumClients() const = 0;

		static Server* create();
	};
};
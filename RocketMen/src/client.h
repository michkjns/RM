
#pragma once

#include "networker.h"

namespace network
{
	class Client
	{
	public:
		virtual ~Client() {};
	
		virtual bool initialize() = 0;
		virtual void tick() = 0;
	
		virtual unsigned int getNumClients() const = 0;
	
		static Client* create();
	};
};
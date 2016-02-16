
#pragma once

#include "networker.h"

class Server
{
public:
	virtual ~Server() {};

	virtual bool initialize() = 0;
	virtual void tick(const Time& time) = 0;

	virtual void host(uint32_t port) = 0;
	virtual unsigned int getNumClients() const = 0;

	static Server* create();
};
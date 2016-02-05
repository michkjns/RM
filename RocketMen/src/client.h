
#pragma once

#include "networker.h"

class Client
{
public:
	virtual ~Client() {};

	virtual bool initialize() = 0;
	virtual void tick() = 0;

	static Client* create();
};

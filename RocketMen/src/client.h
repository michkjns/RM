
#pragma once

#include "networker.h"

class Time;
class Client
{
public:
	virtual ~Client() {};

	virtual bool initialize() = 0;
	virtual void tick() = 0;
	virtual void connect(const network::Address& address) = 0;
	static Client* create(Time& time);
};

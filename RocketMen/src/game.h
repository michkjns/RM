
#pragma once

#include <stdint.h>

class Game
{
public:

	virtual bool initialize() = 0;
	virtual void tick(uint64_t dt) = 0;
	virtual void terminate() = 0;
};
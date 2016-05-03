
#pragma once

#define GAMEVER 0

#include "game.h"

class RocketMenGame : public Game
{
public:
	bool initialize() override;
	void fixedUpdate(uint64_t timestep) override;
	void update(const Time& time) override;
	void terminate() override;

private:

};
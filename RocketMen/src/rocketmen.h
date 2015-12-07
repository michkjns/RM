
#pragma once

#define GAMEVER 0

#include "game.h"

class RocketMenGame : public Game
{
public:
	bool initialize() override;
	void tick(uint64_t dt) override;
	void terminate() override;

private:

};
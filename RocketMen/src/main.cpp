
#include "core.h"
#include "debug.h"
#include "rocketmen.h"

#include <iostream>
#include <cstring>

#include "address.h"

int main(int argc, char *argv[])
{

	network::Address("127.0.0.1", 1234);

	Debug::openLog("rm.log");
#ifdef _DEBUG
	Debug::setVerbosity(Debug::EVerbosity::LEVEL_DEBUG);
#else
	Debug::setVerbosity(Debug::EVerbosity::LEVEL_INFO);
#endif

	Core engine;
	Game* game = new RocketMenGame();

	engine.initialize(game, argc, argv);
	engine.run();
	engine.destroy();

	delete game;

	LOG_INFO("Application has ended succesfully, closing logfile..");
	Debug::closeLog();
	
	return 0;
}
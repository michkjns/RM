
#include "core.h"
#include "debug.h"
#include "rocketmen.h"

#include <iostream>
#include <cstring>

int main(int argc, char *argv[])
{
	Debug::openLog("rm.log");
#ifdef _DEBUG
	Debug::setVerbosity(Debug::EVerbosity::DEBUG);
#else
	Debug::setVerbosity(Debug::EVerbosity::INFO);
#endif

	Core* engine = Core::get();
	Game* game = new RocketMenGame();

	engine->initialize(game, argc, argv);
	engine->run();
	engine->destroy();

	delete game;
	delete engine;

	LOG_INFO("Application has ended succesfully, closing logfile..");
	Debug::closeLog();
	
	return 0;
}
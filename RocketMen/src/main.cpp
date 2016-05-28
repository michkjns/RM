
#include <core/core.h>
#include <core/debug.h>
#include <game/rocketmen.h>

#include <iostream>
#include <cstring>

#include <network/address.h>

#if 0
#include <C:\Program Files (x86)\Visual Leak Detector\include\vld.h>
#endif

int main(int argc, char *argv[])
{
	Debug::openLog("rm.log");
	Debug::print("GAME : %s v%f\n", GAME_NAME, GAME_VERSION);
#ifdef _DEBUG
	Debug::setVerbosity(Debug::Verbosity::LEVEL_DEBUG);
#else
	Debug::setVerbosity(Debug::Verbosity::LEVEL_INFO);
#endif

	Core engine;
	Game* game = new rm::RocketMenGame();

	engine.initialize(game, argc, argv);
	engine.run();
	engine.destroy();

	delete game;

	LOG_INFO("Application has ended succesfully, closing logfile..");
	Debug::closeLog();
	
	return 0;
}
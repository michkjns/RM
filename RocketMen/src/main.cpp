
#include <core/core.h>
#include <core/debug.h>
#include <game/rocketmen.h>

#include <iostream>
#include <cstring>

#include <bitstream.h>
#include <network/address.h>

#if 0
#include <C:\Program Files (x86)\Visual Leak Detector\include\vld.h>
#endif

extern bool streamTests();

std::string getOutputFile(int argc, char* argv[])
{
	for (int32_t i = 0; i < argc; i++)
	{
		if (i >= argc - 1)
			break;
		const char* arg = argv[i];
		if (strcmp(arg, "-o") == 0 || strcmp(arg, "--output") == 0)
		{
			return std::string(argv[i+1]);
		}
	}
	;
	return std::string("rm.log");
}

int main(int argc, char *argv[])
{
	Debug::openLog(getOutputFile(argc, argv).c_str());
	Debug::print("GAME : %s v%f\n", GAME_NAME, GAME_VERSION);

#ifdef _DEBUG
	Debug::setVerbosity(Debug::Verbosity::LEVEL_DEBUG);
#else
	Debug::setVerbosity(Debug::Verbosity::LEVEL_INFO);
#endif

	if (!streamTests())
	{
		LOG_ERROR("Bitstream Tests failed!");
		__debugbreak();
	}

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
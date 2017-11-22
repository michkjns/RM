																																			
#define UNIT_TESTING 0

#include <utility/bitstream.h>
#include <core/core.h>
#include <core/debug.h>
#include <game/rocketmen.h>
#include <network/address.h>
#include <utility/utility.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <utility>
#include <vector>

extern bool streamTests();

static std::string getOutputFile(int argc, char* argv[]);

int main(int argc, char *argv[])
{
	Debug::openLog(getOutputFile(argc, argv).c_str());
	
#ifdef _DEBUG
	Debug::setVerbosity(Debug::Verbosity::Debug);
#else
	Debug::setVerbosity(Debug::Verbosity::Info);
#endif

#if UNIT_TESTING
	if (!streamTests())
	{
		LOG_ERROR("Bitstream tests failed!");
	}
	else
	{
		LOG_INFO("Bitstream tests succesful!");
	}
#endif

	Core core;
	Game* game = new rm::RocketMenGame();

	core.initialize(game, argc, argv);
	core.run();
	core.destroy();

	delete game;

	LOG_INFO("Application has ended succesfully, closing logfile..");
	Debug::closeLog();
	
	return 0;
}

std::string getOutputFile(int argc, char* argv[])
{
	for (int32_t i = 0; i < argc; i++)
	{
		if (i >= argc - 1)
			break;

		const char* arg = argv[i];
		if (strcmp(arg, "-o") == 0 || strcmp(arg, "--output") == 0)
		{
			return std::string(argv[i + 1]);
		}
	}

	return std::string("rm.log");
}
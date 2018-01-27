																																			
#define RM_RUN_TESTS 1

#include <utility/bitstream.h>
#include <core/core.h>
#include <core/debug.h>
#include <game/rocketmen_game.h>
#include <network/address.h>
#include <utility/utility.h>
#include <utility/commandline_options.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <utility>

extern bool bitstreamTests();

static std::string getOutputFile(int argc, char* argv[]);

int main(int argc, char *argv[])
{
	Debug::openLog(getOutputFile(argc, argv).c_str());
	
#ifdef _DEBUG
	Debug::setVerbosity(Debug::Verbosity::Debug);
#else
	Debug::setVerbosity(Debug::Verbosity::Info);
#endif

#if RM_RUN_TESTS
	if (!bitstreamTests())
	{
		LOG_ERROR("Bitstream tests: FAIL");
	}
	else
	{
		LOG_INFO("Bitstream tests: SUCCES");
	}
#endif

	CommandLineOptions options;
	options.registerOption("-d", "--dedicated");
	options.registerOption("-l", "--listen");
	options.registerOption("-v", "--verbosity");
	options.parse(argc, argv);

	Core core;
	Game* game = new rm::RocketMenGame();

	core.initialize(game, options);
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
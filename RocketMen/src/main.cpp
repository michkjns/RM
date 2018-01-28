																																			
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

extern bool testSerialization();

static void initializeVerbosityLevel(const CommandLineOptions& options);
static void initializeLog(const CommandLineOptions& options);

int main(int argc, char *argv[])
{
	CommandLineOptions options;
	options.registerOption("-d", "--dedicated");
	options.registerOption("-l", "--listen");
	options.registerOption("-v", "--verbosity");
	options.registerOption("-o", "--output");
	options.parse(argc, argv);

	initializeLog(options);
	initializeVerbosityLevel(options);

#if RM_RUN_TESTS
	if (!testSerialization())
	{
		LOG_ERROR("SerializationStream tests: FAIL");
	}
	else
	{
		LOG_INFO("SerializationStream tests: SUCCES");
	}
#endif

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

void initializeLog(const CommandLineOptions& options)
{
	std::string fileName = "rm.log";
	if (options.isSet("--output"))
	{
		auto args = options.getArgs("--output");
		if(args.size() == 1)
		{
			fileName = args.at(0);
		}
		else
		{
			LOG_ERROR("--output requires and accepts only 1 argument");
		}
	}

	Debug::openLog(fileName.c_str());
}

#ifdef _DEBUG
void initializeVerbosityLevel(const CommandLineOptions& /*options*/)
#else
void initializeVerbosityLevel(const CommandLineOptions& options)
#endif
{
#ifdef _DEBUG
	Debug::setVerbosity(Debug::Verbosity::Debug);
#else
	if (options.isSet("--verbosity"))
	{
		auto args = options.getArgs("--verbosity");
		assert(args.size() == 1);

		int32_t verbosity = atoi(args[0].c_str());
		verbosity = std::max(0, std::min((int)Debug::Verbosity::Debug, verbosity));
		Debug::setVerbosity(static_cast<Debug::Verbosity>(verbosity));
	}
#endif
}

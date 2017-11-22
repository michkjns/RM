
#include <utility/commandline_options.h>
#include <core/debug.h>

#include <string>

inline bool isOption(const char* arg)
{
	return strncmp(arg, "-", 1) == 0;
}

CommandLineOptions::CommandLineOptions()
{
}

CommandLineOptions::~CommandLineOptions()
{
}

void CommandLineOptions::registerOption(const char* name, const char* altName)
{
	for (const CommandLineOption& option : m_options)
	{
		if (option.name == name || option.altName == altName)
		{
			return;
		}
	}

	CommandLineOption newOption =
	{
		std::string(name),
		std::string(altName),
		false,
		std::vector<std::string>(),
		0
	};

	m_options.push_back(newOption);
}

void CommandLineOptions::parse(int argc, char* argv[])
{
	for (int i = 1; i < argc; i++)
	{
		const char* arg = argv[i];
		if (isOption(arg))
		{
			for (CommandLineOption& option : m_options)
			{
				if (option.name == arg || option.altName == arg)
				{
					option.isSet = true;
					while (i + 1 < argc && !isOption(argv[i + 1]))
					{
						i++;
						option.numArgs++;
						option.args.push_back(argv[i]);
					}
				}
			}
		}
	}
}

bool CommandLineOptions::isSet(const char* optionName) const
{
	for (const CommandLineOption& option : m_options)
	{
		if (option.name == optionName || option.altName == optionName)
		{
			return option.isSet;
		}
	}

	return false;
}

std::vector<std::string> CommandLineOptions::getArgs(const char* optionName) const
{
	for (const CommandLineOption& option : m_options)
	{
		if (option.name == optionName || option.altName == optionName)
		{
			return option.args;
		}
	}

	assert(false);
	return std::vector<std::string>();
}

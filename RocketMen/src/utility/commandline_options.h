
#pragma once

#include <vector>
#include <string>

struct CommandLineOption
{
	std::string name;
	std::string altName;

	bool isSet;
	std::vector<std::string> args;
	int numArgs;
};

class CommandLineOptions
{
public:
	CommandLineOptions();
	~CommandLineOptions();

	void registerOption(const char* name, const char* altName);
	void parse(int argc, char** argv);

	bool isSet(const char* optionName) const;
	std::vector<std::string> getArgs(const char* optionName) const;

private:
	std::vector<CommandLineOption> m_options;
};
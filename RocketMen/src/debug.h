
#pragma once

class Debug
{
public:
	enum Verbosity : unsigned int
	{
		LEVEL_DEBUG = 5,
		LEVEL_INFO = 10,
		LEVEL_WARNING = 20,
		LEVEL_ERROR = 30,
	};

public:
	static void openLog(const char* file);
	static void closeLog();

	static void log(Verbosity verbosityLevel, const char* message , ... );

	static void setVerbosity(Verbosity verbosity);
private:

};


#define LOG_INFO(...)    Debug::log(Debug::Verbosity::LEVEL_INFO,    __VA_ARGS__);
#define LOG_ERROR(...)   Debug::log(Debug::Verbosity::LEVEL_ERROR,   __VA_ARGS__);
#define LOG_WARNING(...) Debug::log(Debug::Verbosity::LEVEL_WARNING, __VA_ARGS__);
#define LOG_DEBUG(...)   Debug::log(Debug::Verbosity::LEVEL_DEBUG,   __VA_ARGS__);

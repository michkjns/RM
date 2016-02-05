
#pragma once

class Debug
{
public:
	enum EVerbosity : unsigned int
	{
		LEVEL_DEBUG = 5,
		LEVEL_INFO = 10,
		LEVEL_WARNING = 20,
		LEVEL_ERROR = 30,
	};

public:
	static void openLog(const char* file);
	static void closeLog();

	static void log(EVerbosity verbosityLevel, const char* message , ... );

	static void setVerbosity(EVerbosity verbosity);
private:

};


#define LOG_DEBUG(...)		Debug::log(Debug::EVerbosity::LEVEL_DEBUG,		__VA_ARGS__);
#define LOG_INFO(...)		Debug::log(Debug::EVerbosity::LEVEL_INFO,		__VA_ARGS__);
#define LOG_ERROR(...)		Debug::log(Debug::EVerbosity::LEVEL_ERROR,		__VA_ARGS__);
#define LOG_WARNING(...)	Debug::log(Debug::EVerbosity::LEVEL_WARNING,	__VA_ARGS__);

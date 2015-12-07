#pragma once

class Debug
{
public:
	enum EVerbosity : unsigned int
	{
		DEBUG = 5,
		INFO = 10,
		WARNING = 20,
		ERROR = 30,
	};

public:
	static void openLog(const char* file);
	static void closeLog();

	static void log(EVerbosity verbosityLevel, const char* message , ... );

	static void setVerbosity(EVerbosity verbosity);
private:

};


#define LOG_DEBUG(...) Debug::log(Debug::EVerbosity::DEBUG,  __VA_ARGS__);
#define LOG_INFO(...) Debug::log(Debug::EVerbosity::INFO,  __VA_ARGS__);
#define LOG_ERROR(...) Debug::log(Debug::EVerbosity::ERROR, __VA_ARGS__);
#define LOG_WARNING(...) Debug::log(Debug::EVerbosity::WARNING, __VA_ARGS__);

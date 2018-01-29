
#pragma once

#include <common.h>

#define LOG_INFO(...)    Debug::log(Debug::Verbosity::Info,    __VA_ARGS__);
#define LOG_ERROR(...)   Debug::log(Debug::Verbosity::Error,   __VA_ARGS__), ensure(false);
#define LOG_WARNING(...) Debug::log(Debug::Verbosity::Warning, __VA_ARGS__);
#define LOG_DEBUG(...)   Debug::log(Debug::Verbosity::Debug,   __VA_ARGS__);

class Debug
{
public:
	enum class Verbosity : int32_t
	{
		Error = 0,
		Warning,
		Info,
		Debug,
	};

public:
	static void openLog(const char* file);
	static void closeLog();

	static void log(Verbosity verbosityLevel, const char* message, ... );
	static void print(const char* message, ...);
	static void setVerbosity(Verbosity verbosity);
	static Verbosity getVerbosity();
};

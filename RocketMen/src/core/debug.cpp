
#include <core/debug.h>

#include <ctime>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdarg.h>

static bool             s_fileOpened = false;
static std::ofstream    s_logFile;
static Debug::Verbosity	s_verbosityLevel = Debug::Verbosity::LEVEL_INFO;
//==============================================================================

inline std::string verbosityToString(Debug::Verbosity verbosity)
{
	return std::string(
		(verbosity <= Debug::Verbosity::LEVEL_DEBUG)   ? "DEBUG"   :
		(verbosity <= Debug::Verbosity::LEVEL_INFO)    ? "INFO"    :
		(verbosity <= Debug::Verbosity::LEVEL_WARNING) ? "WARNING" :
		(verbosity <= Debug::Verbosity::LEVEL_ERROR)   ? "ERROR"   :
		"");
}

void Debug::openLog(const char* file)
{
	std::time_t t = std::time(NULL);
	s_logFile.open(file, std::fstream::out);
	if (s_logFile.fail())
	{
		printf("Error opening %s", file);
	}
	s_fileOpened = true;

	time_t rawtime;
	struct tm timeinfo; 

	time(&rawtime);

#ifndef _WIN32
	#define localtime_s(x, y) localtime_r(y, x)
#endif
	localtime_s(&timeinfo, &rawtime); 

	char buffer[80];

	strftime(buffer, 80, "%c", &timeinfo);

	LOG_INFO("Log file opened - %s", buffer);
}

void Debug::closeLog()
{
	std::time_t t = std::time(NULL);
	time_t rawtime;
	struct tm timeinfo;
	time(&rawtime);
#ifndef _WIN32
#define localtime_s(x, y) localtime_r(y, x)
#endif
	localtime_s(&timeinfo, &rawtime);
	char buffer[80];
	strftime(buffer, 80, "%c", &timeinfo);
	LOG_INFO("%s", buffer);

	s_logFile.close();
	s_fileOpened = false;
}

void Debug::setVerbosity(Verbosity verbosity)
{
	s_verbosityLevel = verbosity;
	std::string verbosityStr = verbosityToString(verbosity);

	LOG_INFO("Logging Verbosity is now set to: %s", verbosityStr.c_str());
}

void Debug::log(Verbosity verbosityLevel, const char* format, ...)
{
	if (verbosityLevel < s_verbosityLevel) return;

	std::string verbosityStr = verbosityToString(verbosityLevel);
	char buffer[1024];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, 1024, format, args);
	std::stringstream stream;
	stream << verbosityStr << " : " << buffer << std::endl;
	std::cout << stream.str();
	va_end(args);

	if (s_fileOpened)
	{
		s_logFile << stream.str();
	}

	s_logFile.flush();
}

void Debug::print(const char* message, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, message);
	vsnprintf(buffer, 1024, message, args);
	std::stringstream stream;
	stream << buffer;
	std::cout << stream.str();
	va_end(args);

	if (s_fileOpened)
	{
		s_logFile << stream.str();
	}

	s_logFile.flush();
}

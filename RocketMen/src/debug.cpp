
#include "debug.h"

#include <ctime>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdarg.h>

static bool					s_fileOpened = false;
static std::ofstream		s_logFile;
static Debug::EVerbosity	s_verbosityLevel = Debug::EVerbosity::LEVEL_INFO;

void Debug::openLog(const char* file)
{
	std::time_t t = std::time(NULL);
	s_logFile.open(file, std::fstream::out);
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
	s_logFile.close();
	s_fileOpened = false;
}

void Debug::setVerbosity(EVerbosity verbosity)
{
	s_verbosityLevel = verbosity;
}
	

void Debug::log(EVerbosity verbosityLevel, const char* format, ...)
{

	if (verbosityLevel < s_verbosityLevel) return;

	std::string verbosity =
		(verbosityLevel <= Debug::EVerbosity::LEVEL_DEBUG)		? "DEBUG"	:
		(verbosityLevel <= Debug::EVerbosity::LEVEL_INFO)		? "INFO"	:
		(verbosityLevel <= Debug::EVerbosity::LEVEL_WARNING)	? "WARNING" :
		(verbosityLevel <= Debug::EVerbosity::LEVEL_ERROR)		? "ERROR"	: 
		"";

	char buffer[1024];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, 1024, format, args);
	std::stringstream stream;
	stream << verbosity << " : " << buffer << std::endl;
	std::cout << stream.str();
	va_end(args);

	if (s_fileOpened)
	{
		s_logFile << stream.str();
	}

}

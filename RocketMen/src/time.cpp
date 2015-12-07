
#include "time.h"

#ifdef _WIN32
#include <Windows.h> 
#endif

#include <assert.h>

Time::Time()
	: m_currentTime(0)
	, m_lastRealTime(0)
{
#ifdef _WIN32
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&m_frequency));
#else
	// TODO: Support more platforms
	assert(false);
#endif
	m_frequency /= 1000000;
	m_lastRealTime = getRealTime();
}

Time::~Time()
{
}

uint64_t Time::getRealTime()
{
	uint64_t time;
#ifdef _WIN32
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time));
#else
	// TODO: other platforms
#endif
	return (time / m_frequency);
}

void Time::update()
{
	uint64_t currentRealTime = getRealTime();
	m_deltaTime = (currentRealTime - m_lastRealTime);
	m_lastRealTime = currentRealTime;
	updateBy(m_deltaTime);
}

void Time::updateBy(uint64_t time)
{
	m_currentTime += time;
}

float Time::getSeconds() const
{
	return m_currentTime * 0.000001f;
}

uint64_t Time::getMilliSeconds() const
{
	return (uint64_t)(m_currentTime * 0.001f);
}

uint64_t Time::getMicroSeconds() const
{
	return m_currentTime;
}

float Time::getDeltaSeconds() const
{
	return m_deltaTime * 0.000001f;
}

uint64_t Time::getDeltaMilliSeconds() const
{
	return (uint64_t)(m_deltaTime * 0.001f);
}

uint64_t Time::getDeltaMicroSeconds() const
{
	return m_deltaTime;
}

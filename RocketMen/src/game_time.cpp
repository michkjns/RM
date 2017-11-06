
#include "game_time.h"
#include <assert.h>

Time::Time() :
	m_deltaTimeSeconds(0), 
	m_deltaTimeMicroSeconds(0),
	m_runTimeMicroSeconds(0),
	m_tickCount(0)
{
	m_lastTime    = m_clock.now();
	m_currentTime = m_clock.now();
}

Time::~Time()
{
}

void Time::update()
{
	m_lastTime = m_currentTime;
	m_currentTime = m_clock.now();
	m_deltaTimeMicroSeconds = std::chrono::duration_cast<std::chrono::microseconds>(m_currentTime - m_lastTime).count();
	m_deltaTimeSeconds = static_cast<float>(m_deltaTimeMicroSeconds * 0.000001f);
	updateBy(m_deltaTimeMicroSeconds);
	m_tickCount++;
}

void Time::updateBy(uint64_t time)
{
	m_runTimeMicroSeconds += time;
}

float Time::getSeconds() const
{
	return m_runTimeMicroSeconds * 0.000001f;
}

uint64_t Time::getMilliSeconds() const
{
	return m_runTimeMicroSeconds / 1000;
}

uint64_t Time::getMicroSeconds() const
{
	return m_runTimeMicroSeconds;
}

float Time::getDeltaSeconds() const
{
	return m_deltaTimeSeconds;
}

uint64_t Time::getDeltaMilliSeconds() const
{
	return (m_deltaTimeMicroSeconds / 1000);
}

uint64_t Time::getDeltaMicroSeconds() const
{
	return m_deltaTimeMicroSeconds;
}

uint64_t Time::getTickCount() const
{
	return m_tickCount;
}

#pragma once

#include <cstdint>

// Inspired by http://lspiroengine.com/?p=378

class Time
{
public:
	Time();
	~Time();

	void update();
	void updateBy(uint64_t time);

	float getSeconds() const;
	uint64_t getMilliSeconds() const;
	uint64_t getMicroSeconds() const;

	float getDeltaSeconds() const;
	uint64_t getDeltaMicroSeconds() const;
	uint64_t getDeltaMilliSeconds() const;

private:
	uint64_t getRealTime();

	float m_deltaMilliseconds;
	uint64_t m_lastRealTime;
	uint64_t m_currentTime;
	uint64_t m_frequency;
	uint64_t m_deltaTime;

};
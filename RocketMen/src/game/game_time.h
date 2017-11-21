
#pragma once

#include <common.h>

#include <cstdint>
#include <chrono>

// Inspired by http://lspiroengine.com/?p=378

class Time
{
public:
	Time();
	~Time();

	/** Update the game clock */
	void update();

	/** Returns the total elapsed time in seconds */
	float getSeconds() const;

	/** Returns the total elapsed time in milliseconds
	* @return	uint64_t	Time in milliseconds
	*/
	uint64_t getMilliSeconds() const;

	/** Returns the total elapsed time in microseconds
	* @return	uint64_t	Time in microseconds
	*/
	uint64_t getMicroSeconds() const;

	/** Returns the elapsed time between the current and previous frame in
	*	seconds
	*	@return	float	DeltaTime in seconds
	*/
	float	 getDeltaSeconds() const;
	
	/** Returns the elapsed time between the current and previous frame in
	*	microseconds
	*	@return	float	DeltaTime in microseconds
	*/
	uint64_t getDeltaMicroSeconds() const;

	/** Returns the elapsed time between the current and previous frame in
	*	milliseconds
	*	@return	float	DeltaTime in milliseconds
	*/
	uint64_t getDeltaMilliSeconds() const;

	/** Returns the total number of ticks
	* @return	uint64_t	Current count of ticks
	*/
	uint64_t getTickCount() const;

private:
	void updateBy(uint64_t time);

	std::chrono::steady_clock			  m_clock;
	std::chrono::steady_clock::time_point m_lastTime;
	std::chrono::steady_clock::time_point m_currentTime;

	float	 m_deltaTimeSeconds;
	uint64_t m_deltaTimeMicroSeconds;
	uint64_t m_runTimeMicroSeconds;
	uint64_t m_tickCount;
};
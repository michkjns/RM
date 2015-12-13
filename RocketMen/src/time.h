
#pragma once

#include <cstdint>

// Inspired by http://lspiroengine.com/?p=378

class Time
{
public:
	Time();
	~Time();

	/** Update the time by polling the CPU ticks. Call this only once per
	frame
	*/
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
	float getDeltaSeconds() const;
	uint64_t getDeltaMicroSeconds() const;
	uint64_t getDeltaMilliSeconds() const;

	/** Returns the total number of ticks
	* @return	uint64_t	Current count of ticks
	*/
	uint64_t getTickCount() const;

private:
	void updateBy(uint64_t time);
	uint64_t getRealTime();

	float m_deltaSeconds;
	uint64_t m_tickCount;
	uint64_t m_lastRealTime;

	/** Current time in microseconds */
	uint64_t m_currentTime;

	/** CPU Performance frequency */
	uint64_t m_frequency;

	/** Time difference in microseconds between last and current tick */
	uint64_t m_deltaTime;

};
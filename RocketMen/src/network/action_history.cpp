
#include "action_history.h"

using namespace network;

ActionHistory::ActionHistory(uint32_t size)
{
	m_historyBuffer.resize(static_cast<size_t>(size));
	m_newestFrame = m_historyBuffer.begin();
	m_oldestFrame = m_historyBuffer.begin();
}

ActionHistory::~ActionHistory()
{
}

void ActionHistory::add(const ClientFrame& frame)
{
	/** Delete old frame */
	std::list<ClientFrame>::iterator newFrame = next(m_newestFrame);

	/** Insert new frame */
	(*newFrame) = frame;
	if (newFrame == m_oldestFrame)
	{
		m_oldestFrame = next(m_oldestFrame);
	}
	m_newestFrame = newFrame;
}

std::list<ClientFrame>::iterator ActionHistory::next(std::list<ClientFrame>::iterator iter)
{
	if (iter != m_historyBuffer.end() && &*iter == &m_historyBuffer.back())
	{
		return m_historyBuffer.begin();
	}
	else
	{
		return ++iter;
	}
}

void ActionHistory::replayFrom(uint64_t sequenceNr)
{
	std::list<ClientFrame>::iterator replayFrame = m_oldestFrame;
	while (replayFrame != m_newestFrame)
	{
		/** Skip deprecated input */
		if (replayFrame->sequence < sequenceNr)
		{
			replayFrame = next(replayFrame);
		}
		else
		{
			//ClientFrame& frame = *(replayFrame);
			//InputMapper::ApplyInput(frame.actions);
			//frame.actions->resetReading();

			replayFrame = next(replayFrame);
		}
	}
}
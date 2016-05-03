
#include "client_history.h"

using namespace network;

ClientHistory::ClientHistory(uint32_t size)
{
	m_historyBuffer.resize(static_cast<size_t>(size));
	m_newestFrame = m_historyBuffer.begin();
	m_oldestFrame = m_historyBuffer.begin();
}

ClientHistory::~ClientHistory()
{
	for (auto frame : m_historyBuffer)
	{
		if (frame.actions != nullptr)
		{
			delete frame.actions;
		}
	}
}

void ClientHistory::add(const ClientFrame& frame)
{
	/** Delete old frame */
	std::list<ClientFrame>::iterator newFrame = next(m_newestFrame);
	if ((*newFrame).actions != nullptr)
	{
		delete (*newFrame).actions;
	}

	/** Insert new frame */
	(*newFrame) = frame;
	if (newFrame == m_oldestFrame)
	{
		m_oldestFrame = next(m_oldestFrame);
	}
	m_newestFrame = newFrame;

}

std::list<ClientFrame>::iterator ClientHistory::next(std::list<ClientFrame>::iterator iter)
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

void ClientHistory::replayFrom(uint64_t sequenceNr)
{
	std::list<ClientFrame>::iterator replayFrame = m_oldestFrame;
	while (replayFrame != m_newestFrame)
	{
		/** Skip deprecated input */
		if (replayFrame->sequenceNr < sequenceNr)
		{
			replayFrame = next(replayFrame);
		}
		else
		{
			ClientFrame& frame = *(replayFrame);
			if (frame.actions != nullptr)
			{
				//InputMapper::ApplyInput(frame.actions);
				frame.actions->resetReading();
			}

			replayFrame = next(replayFrame);
		}
	}
}
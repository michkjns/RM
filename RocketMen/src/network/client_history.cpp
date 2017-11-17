
#include "client_history.h"

using namespace network;

static const int32_t s_historySize = 512;

ClientHistory::ClientHistory() :
	m_frames(s_historySize)
{
}

ClientHistory::~ClientHistory()
{
}

Frame* ClientHistory::insertFrame(Sequence frameId)
{
	Frame* frame = m_frames.insert(frameId);
	for (int32_t i = 0; i < s_maxPlayersPerClient; i++)
	{
		frame->actions[i].clear();
	}
	return frame;
}

Frame* ClientHistory::getFrame(Sequence frameId)
{
	return m_frames.getEntry(frameId);
}

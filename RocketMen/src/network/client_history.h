
#pragma once

#include <core/input_buffer.h>
#include <network/common_network.h>
#include <network/sequence_buffer.h>

namespace network
{
	struct Frame
	{
		ActionBuffer actions[s_maxPlayersPerClient];
	};

	class ClientHistory
	{
	public:
		ClientHistory();
		~ClientHistory();

		Frame* insertFrame(Sequence frameId);
		Frame* getFrame(Sequence frameId);
		
	private:
		SequenceBuffer<Frame> m_frames;
	};

}; // namespace network
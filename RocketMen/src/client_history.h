
#pragma once

#include "bitstream.h"

#include <cstdint>
#include <list>

namespace network
{
	struct ClientFrame
	{
		uint64_t	sequenceNr;
		BitStream*	actions;
	};

	class ClientHistory
	{
	public:
		/** ClientHistory(uint32_t size)
		* @param uint32_t	size	Size of the history buffer in number of frame entries.
		*/
		ClientHistory(uint32_t size);
		~ClientHistory();

		/** Adds a ClientFrame to the historybuffer */
		void add(const ClientFrame& frame);

		/** Reads the input history and resimulate the inputs
		*	@param uint64_t	sequenceNr	The input ID from where to start, everything preceeding this is skipped.
		*/
		void replayFrom(uint64_t sequenceNr);

	private:
		/** Increment a history buffer iterator, automatically wraps around.
		* @param std::list<ClientFrame>::iterator	iter
		*		the iterator to increment
		*
		* @return	std::list<ClientFrame>::iterator
		*	The incremented result of the passed iterator.
		*/
		std::list<ClientFrame>::iterator next(std::list<ClientFrame>::iterator iter);

		std::list<ClientFrame>::iterator m_oldestFrame;
		std::list<ClientFrame>::iterator m_newestFrame;
		std::list<ClientFrame>			 m_historyBuffer;

	};

}; // namespace msh
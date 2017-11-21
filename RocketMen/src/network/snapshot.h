
#pragma once

#include <utility/bitstream.h>

#include <vector>

class Entity;

namespace network
{
	class Snapshot
	{
	public:
		Snapshot(std::vector<Entity*>& entities);

		const char* getBuffer() const;
		size_t getSize() const;

	private:
		WriteStream m_writeStream;
	};
};

#pragma once

#include <common.h>

namespace network
{
	static const uint32_t s_maxPlayersPerClient       = 4;
	static const float    s_snapshotCreationRate      = 1 / 20.f;
	static const uint32_t s_sentPacketsBufferSize     = 1024;
	static const uint32_t s_receivedPacketsBufferSize = 1024;
	static const uint32_t s_maxSnapshotSize           = 512;
}; // namespace network
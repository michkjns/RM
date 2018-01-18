
#include <network/packet.h>
#include <core/debug.h>

#include <assert.h>
#include <cstring>

using namespace network;
using std::memcpy;

Packet::Packet()
{
	header = {};
}
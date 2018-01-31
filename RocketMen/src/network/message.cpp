
#include "message.h"

using namespace network;

Message::Message() :
	m_refCount(1),
	m_id(0)
{
}

Message::~Message()
{
	ASSERT(m_refCount == 0);
}

Message* Message::addRef()
{
	m_refCount++;
	return this;
}

bool Message::releaseRef()
{
	ASSERT(m_refCount > 0);

	m_refCount--;
	if (m_refCount == 0)
	{
		delete this;
		return true;
	}

	return false;
}


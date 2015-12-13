
#include "networker.h"

using namespace network;

Networker::Networker()
	: m_isInitialized(false)
	, m_isConnected(false)
{
}

Networker::~Networker()
{
}

bool Networker::initialize()
{
	m_isInitialized = true;
	return true;
}

void Networker::tick()
{
}

bool Networker::isInitialized() const
{
	return m_isInitialized;
}

bool Networker::isConnected() const
{
	return m_isConnected;
}

#include "networker.h"
#include "address.h"

using namespace network;

Networker::Networker() :
	m_sequenceCounter(0),
	m_reliableSendRate(0.05f),
	m_unreliableSendRate(0.016f),
	m_isInitialized(false),
	m_isConnected(false)
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

void Networker::tick(float deltaTime)
{
	m_networkInterface.tick();
	handleIncomingPackets();
	handleOutgoingPackets(deltaTime);
}

bool Networker::isInitialized() const
{
	return m_isInitialized;
}

bool Networker::isConnected() const
{
	return m_isConnected;
}

void Networker::handleOutgoingPackets(float deltaTime)
{
	m_updatedTimeReliable += deltaTime;
	m_updatedTimeUnreliable += deltaTime;

	/* Reliable Packets */
	if (m_updatedTimeReliable >= m_reliableSendRate)
	{
		m_updatedTimeReliable -= m_reliableSendRate;
		while (!m_outgoingReliablePackets.empty())
		{
			Packet packet = m_outgoingReliablePackets.front();
			m_outgoingReliablePackets.pop();
			sendPacket(packet);
		}
	}

	/* Unreliable Packets */
	if (m_updatedTimeUnreliable >= m_unreliableSendRate)
	{
		m_updatedTimeUnreliable -= m_unreliableSendRate;
		while (!m_outgoingUnreliablePackets.empty())
		{
			Packet packet = m_outgoingUnreliablePackets.front();
			m_outgoingUnreliablePackets.pop();
			sendPacket(packet);
		}
	}
}

void Networker::handleIncomingPackets()
{
	bool queueCleared = false;
	Packet& packet = m_networkInterface.getPacket();
	while (packet.header.type != ECommand::EVENT_CLEAR)
	{
		handlePacket(packet);
		if (packet.data != nullptr)
		{
			delete packet.data;
		}

		packet.header.type = ECommand::EVENT_CLEAR;
		packet = m_networkInterface.getPacket();
	}
}

void Networker::sendPacket(Packet& packet)
{
	m_networkInterface.send(packet, m_sequenceCounter++);
}

void Networker::queuePacket(const Packet& packet)
{
	switch (packet.reliable)
	{
		case EReliable::RELIABLE:
		{
			m_outgoingReliablePackets.push(packet);
			break;
		}
		case EReliable::UNRELIABLE:
		{
			m_outgoingUnreliablePackets.push(packet);
			break;
		}
		default: break;
	}
}

void Networker::setReliableSendRate(uint32_t ratePerSecond)
{
	m_reliableSendRate = 1.0f / static_cast<float>(ratePerSecond);
}

void Networker::setUnreliableSendRate(uint32_t ratePerSecond)
{
	m_unreliableSendRate = 1.0f / static_cast<float>(ratePerSecond);
}

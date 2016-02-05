
#include "networker.h"

using namespace network;

Networker::Networker() :
	m_sequenceCounter(0),
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

void Networker::tick()
{
	m_networkInterface.tick();
}

bool Networker::isInitialized() const
{
	return m_isInitialized;
}

bool Networker::isConnected() const
{
	return m_isConnected;
}

void Networker::sendPackets()
{
	while (!m_outgoingPackets.empty())
	{
		Packet packet = m_outgoingPackets.front();
		m_outgoingPackets.pop();
		sendPacket(packet);
	}
}

void Networker::receivePackets()
{
	bool queueCleared = false;
	while (!m_incomingPackets.empty())
	{
		Packet packet = m_incomingPackets.front();
		m_incomingPackets.pop();
		handlePacket(packet);
		if (packet.data != nullptr)
		{
			delete packet.data;
		}
	}
}

void Networker::sendPacket(const Packet& packet)
{
	const uint32_t dataLength = (packet.data) ? 
		static_cast<uint32_t>(packet.data->getLength()) 
		: 0;

	const uint32_t packetSize = static_cast<uint32_t>(sizeof(PacketHeader) + dataLength);
	char* completePacket = new char[packetSize];
	char* packetContentPtr = completePacket + sizeof(PacketHeader);
	memcpy(completePacket, &packet.header, sizeof(PacketHeader));

	if (packet.data != nullptr)
	{
		if (packet.data->getLength() > 0)
		{
			memcpy(packetContentPtr, packet.data->getBuffer(), packet.data->getLength());
		}
		delete packet.data;
	}

	m_networkInterface.send(completePacket, packetSize, packet.broadcast, packet.reliable);

	delete completePacket;
}

void Networker::queuePacket(const Packet& packet)
{
	m_outgoingPackets.push(packet);
}

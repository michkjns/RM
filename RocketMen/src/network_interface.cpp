
#include "network_interface.h"

#include "debug.h"
#include "packet.h"
#include "socket.h"

using namespace network;

static const uint32_t s_maxPeers = 8;
static const uint32_t s_maxDuplicatePeers = 8; // Maximum peers allowed to have the same address
static Peer s_invalidPeer;

NetworkInterface::NetworkInterface()
	: m_peerIDCounter(0)
	, m_peerID(0)
	, m_state(EState::STATE_DISCONNECTED)
{
	m_socket = Socket::create(Socket::ENetProtocol::PROTOCOL_UDP);
	m_peers.resize(s_maxPeers);
}

Peer& NetworkInterface::getPeer(Address address)
{
	for (Peer& peer : m_peers)
	{
		if (peer.address == address)
		{
			return peer;
		}
	}

	// Peer not found, return an invalid peer
	return s_invalidPeer;
}

Peer& NetworkInterface::getPeer(int32_t peerID)
{
	for (Peer& peer : m_peers)
	{
		if (peer.peerID == peerID)
		{
			return peer;
		}
	}

	// Peer not found, return an invalid peer
	return s_invalidPeer;
}

int32_t NetworkInterface::getPeerID() const
{
	return m_peerID;
}

void NetworkInterface::handleIncomingPackets()
{
	BitStream* packetData = m_socket->getPacket();
	while (packetData != nullptr)
	{
		char* incomingAddr = new char[sizeof(Address)];
		packetData->readBytes(incomingAddr, sizeof(Address));
		Address peerAddress(reinterpret_cast<Address*>(incomingAddr)->getAddress(), reinterpret_cast<Address*>(incomingAddr)->getPort());
		delete[] incomingAddr;

		Packet packet;
		packetData->readBytes(reinterpret_cast<char*>(&packet.header), sizeof(PacketHeader));
		packet.data = packetData;

		switch (packet.header.type)
		{
			case ECommand::CLIENT_CONNECT:
			{
				if (m_state == EState::STATE_HOSTING)
				{
					Peer& peer = getPeer(peerAddress);
					if (peer)
					{
						if (peer.duplicatePeers < s_maxDuplicatePeers)
						{
							peer.duplicatePeers++;
							peer = newPeer(peerAddress);
						}
					}
					else
					{
						peer = newPeer(peerAddress);
					}
					packet.header.senderID = peer.peerID;
					m_incomingPackets.push(packet);
				}
				break;
			}
			case ECommand::CLIENT_DISCONNECT:
			{
				// A client disconnects from this server
				if (m_state == EState::STATE_HOSTING)
				{
					Peer& peer = getPeer(packet.header.senderID);
					if (peer)
					{
						for (Peer& dupPeer : m_peers)
						{
							if (dupPeer.address == peer.address)
							{
								dupPeer.duplicatePeers--;
							}
						}
						peer.address = Address((uint32_t)0, 0);
						peer.peerID = -1;
						peer.numPlayers = 0;
						peer.duplicatePeers = 0;
					}
				}
				else
				{
					// I am a client and my disconnect has been acknowledged by the server
					m_state = EState::STATE_DISCONNECTED;
					m_incomingPackets.push(packet);
				}
				break;
			}
			case ECommand::SERVER_HANDSHAKE:
			{
				if (m_state == EState::STATE_CONNECTING)
				{
					m_state = EState::STATE_CONNECTED;
					m_peerID = packet.data->readInt32();
					m_incomingPackets.push(packet);
				}
				break;
			}
			default:
			{
				m_incomingPackets.push(packet);
				break;
			}
		}
	}

	packetData = m_socket->getPacket();
}

Peer& NetworkInterface::newPeer(const Address& address)
{
	uint32_t slot = 0;
	for (slot; slot < s_maxPeers; slot++)
	{
		if (!m_peers[slot])
		{
			m_peers[slot].address = address;
			m_peers[slot].peerID = m_peerIDCounter++;
			return m_peers[slot];
		}
	}

	// No available slots, return an invalid peer
	return s_invalidPeer;
}

NetworkInterface::~NetworkInterface()
{
	if (m_socket) delete m_socket;
}

void NetworkInterface::tick()
{
	if (m_socket->isInitialized())
	{
		m_socket->receive();
	}
	
	handleIncomingPackets();
}

void NetworkInterface::connect(const Address& destination, const Time& time)
{
	if (!m_socket->isInitialized())
	{
		m_socket->initialize(destination.getPort());
	}

	LOG_INFO("Attempting to connect to %s", destination.getString().c_str());

	BitStream* stream = BitStream::create();
	stream->writeByte(ECommand::CLIENT_CONNECT);

	m_socket->send(destination, stream->getBuffer(), stream->getLength());
	delete stream;
	m_state = EState::STATE_CONNECTING;
}

void NetworkInterface::host(uint32_t port)
{
	if (m_socket->isInitialized())
	{
		delete m_socket;
	}
	
	if (m_socket->initialize(port, true))
	{
		m_state = EState::STATE_HOSTING;
	}
}

void NetworkInterface::disconnect()
{
	if (m_state != EState::STATE_HOSTING)
	{
		BitStream* stream = BitStream::create();
		stream->writeByte(ECommand::CLIENT_DISCONNECT);
		m_socket->send(m_peers[0].address, stream->getBuffer(), stream->getLength());
		delete stream;
		m_state = EState::STATE_DISCONNECTING;
	}
}

void NetworkInterface::send(Packet& packet, int32_t sequenceNumber, uint8_t channel /* = 0 */)
{
	const uint32_t dataLength = (packet.data) ?	static_cast<uint32_t>(packet.data->getLength())	: 0;
	const int32_t  recipientID = packet.header.recipientID;
	packet.header.sequenceNumber = (packet.reliable == EReliable::RELIABLE) ? -sequenceNumber : sequenceNumber;
	const size_t packetSize = sizeof(PacketHeader) + dataLength;
	char* completePacket = new char[packetSize];
	char* packetContentPtr = completePacket + sizeof(PacketHeader);
	packet.header.senderID = m_peerID;
	memcpy(completePacket, &packet.header, sizeof(PacketHeader));

	if (packet.data != nullptr)
	{
		if (packet.data->getLength() > 0)
		{
			memcpy(packetContentPtr, packet.data->getBuffer(), packet.data->getLength());
		}
		delete packet.data;
	}
	
	if (packet.broadcast == EBroadcast::BROADCAST_ALL)
	{
		for (Peer& peer : m_peers)
		{
			if (peer.peerID == recipientID)
			{
				continue;
			}
			if (peer)
			{
				m_socket->send(peer.address, completePacket, packetSize);
			}
		}
	}
	else /*if (packet.broadcast == EBroadcast::BROADCAST_SINGLE)*/
	{
		const Peer& peer = getPeer((uint32_t)recipientID);
		if (peer)
		{
			m_socket->send(peer.address, completePacket, packetSize);
		}
	}

	delete[] completePacket;
}

Packet& NetworkInterface::getPacket()
{
	if (m_incomingPackets.size() > 0)
	{
		if (m_incomingPackets.front().header.type == ECommand::EVENT_CLEAR && m_incomingPackets.size() > 1)
		{
			m_incomingPackets.pop();
		}
		return m_incomingPackets.front();
	}
	else
	{
		static Packet clearPacket;
		clearPacket.header.type = ECommand::EVENT_CLEAR;
		return clearPacket;
	}
}

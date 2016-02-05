
#include "network_interface.h"

#include "packet.h"
#include "peer.h"
#include "socket.h"

#include <vector>

using namespace network;

static std::vector<Peer>	s_peers;
static const uint32_t		s_maxPeers = 8;

NetworkInterface::NetworkInterface()
{
	m_socket = Socket::create(Socket::ENetProtocol::PROTOCOL_UDP);
	s_peers.resize(s_maxPeers);
}

Peer& getPeer(uint32_t peerID)
{
	for (Peer& peer : s_peers)
	{
		if (peer.peerID == peerID)
		{
			return peer;
		}
	}

	static Peer invalid;
	return invalid;
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
}

void NetworkInterface::connect(const Address& destination)
{
	if (!m_socket->isInitialized())
	{
		m_socket->initialize(destination.getPort());
	}

	BitStream* stream = BitStream::create();
	
	delete stream;
	//m_socket->connect( ?
}

void NetworkInterface::host(uint32_t port)
{
	if (m_socket->isInitialized())
	{
		delete m_socket;
	}
	
	m_socket->initialize(port, true);
}

void NetworkInterface::disconnect()
{
}

void NetworkInterface::send(const void* data, int32_t dataLength
							, EBroadcast broadcast, int32_t recipientID
							, EReliable reliable /* = EReliable::RELIABLE */
							, uint8_t channel /* = 0 */)
{
	if (dataLength > 65507)
	{
		//
	}
	const Peer& peer = getPeer(recipientID);
	if (peer)
	{
		m_socket->send(peer.address, data, dataLength);
	}
}
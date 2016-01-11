
#include "network_interface.h"
#include "socket.h"

using namespace network;

NetworkInterface::NetworkInterface()
{
	m_socket = Socket::create(Socket::ENetProtocol::PROTOCOL_UDP);
}

NetworkInterface::~NetworkInterface()
{
	if (m_socket) delete m_socket;
}

void NetworkInterface::tick()
{
	m_socket->receive();
}

void NetworkInterface::Send(const void* data, int32_t dataLength
							, EBroadcast broadcast, int32_t recipientID
							, EReliable reliable /* = EReliable::RELIABLE */
							, uint8_t channel /* = 0 */)
{
	//m_socket->send( peer.address, data, dataLength);
}
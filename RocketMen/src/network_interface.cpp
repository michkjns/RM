
#include "network_interface.h"
#include "socket.h"

using namespace network;

NetworkInterface::NetworkInterface()
{
	m_socket = Socket::create(Socket::NetProtocol::ESocket_UDP);
}

NetworkInterface::~NetworkInterface()
{
	if (m_socket) delete m_socket;
}

void NetworkInterface::Send(const void* data, int32_t dataLength,
							bool broadcast, int32_t recipientID,
							bool reliable /* = true */,
							uint8_t channel /* = 0 */)
{
	//
}
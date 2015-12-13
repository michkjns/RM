
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

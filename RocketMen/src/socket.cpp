
#include "socket.h"

#include "bitstream.h"
#include "debug.h"

#include <assert.h>
#include <stdio.h>
#include <WS2tcpip.h>
#include <queue>

#pragma comment(lib, "Ws2_32.lib")

using namespace network;
using std::queue;

static bool      s_initializeWSA = true;
static int       s_numSockets    = 0;
static const int s_defaultPort	 = 1234;
static WSADATA   s_wsa;

static bool initializeWSA();
static bool closeWSA();

// Win32 Impl
class Socket_impl : public Socket
{
public:
	Socket_impl(ENetProtocol type);
	~Socket_impl();

	bool initialize(uint32_t port, bool isHost)	override;
	bool isInitialized()                  const override;

	int	 receive() override;
	bool send(Address adress, const void* buffer, size_t bufferLength)	override;
	BitStream* getPacket() override;

	uint32_t getPort()            const	override;
	uint64_t getBytesSent()       const override;
	uint64_t getBytesReceived()	  const override;
	uint64_t getPacketsReceived() const override;
	uint64_t getPacketsSent()     const override;

private:
	bool initializeUDP(uint16_t port, bool isHost /* = false */);
	bool initializeTCP(uint16_t port, bool isHost /* = false */);

	bool sendUDP(Address address, const void* buffer, size_t bufferLength);
	bool sendTCP(Address address, const void* buffer, size_t bufferLength);

	int	receiveUDP();
	int	receiveTCP();

	bool         m_isInitialized;
	uint64_t     m_bytesReceived;
	uint64_t     m_bytesSent;
	uint64_t     m_packetsReceived;
	uint64_t     m_packetsSent;
	uint16_t     m_port;
	ENetProtocol m_type;

	queue<BitStream*> m_incomingPackets;

	SOCKET m_winSocket;
};

Socket_impl::Socket_impl(ENetProtocol type)
	: m_isInitialized(false)
	, m_bytesReceived(0)
	, m_port(s_defaultPort)
	, m_bytesSent(0)
	, m_type(type)
{
}

Socket_impl::~Socket_impl()
{
	if (m_isInitialized)
	{
		closesocket(m_winSocket);

		s_numSockets--;
		assert(s_numSockets >= 0);

		if (s_numSockets == 0)
		{
			if (WSACleanup() != 0)
			{
				LOG_ERROR("WSACleanup failed. Error Code : %d\n", WSAGetLastError());
				assert(false);
			}

			s_initializeWSA = true;
		}
	}
}

bool Socket_impl::initialize(uint32_t port, bool isHost)
{
	if (s_initializeWSA)
	{
		initializeWSA();
	}

	if (m_type == ENetProtocol::PROTOCOL_UDP)
	{
		m_isInitialized = initializeUDP(port, isHost);
	}
	else
	{
		m_isInitialized = initializeTCP(port, isHost);
	}

	s_numSockets++;
	return m_isInitialized;
}

bool Socket_impl::isInitialized() const
{
	return m_isInitialized;
}

bool Socket_impl::send(Address adress, const void* buffer, size_t bufferLength)
{
	if (m_type == ENetProtocol::PROTOCOL_UDP)
	{
		return sendUDP(adress, buffer, bufferLength);
	}
	else
	{
		return sendTCP(adress, buffer, bufferLength);
	}
}

BitStream* Socket_impl::getPacket()
{
	if (!m_incomingPackets.empty())
	{
		BitStream* packet = m_incomingPackets.front();
		m_incomingPackets.pop();
		return packet;
	}
	return nullptr;
}

uint32_t Socket_impl::getPort() const
{
	return m_port;
}

uint64_t Socket_impl::getBytesSent() const
{
	return m_bytesSent;
}

uint64_t Socket_impl::getBytesReceived() const
{
	return m_bytesReceived;
}

uint64_t Socket_impl::getPacketsReceived() const
{
	return m_packetsReceived;
}

uint64_t Socket_impl::getPacketsSent() const
{
	return m_packetsSent;
}

int Socket_impl::receive()
{
	if (m_isInitialized)
	{
		if (m_type == ENetProtocol::PROTOCOL_UDP)
		{
			return receiveUDP();
		}
		else
		{
			return receiveTCP();
		}
	}
	return 0;
}

bool Socket_impl::initializeUDP(uint16_t port, bool isHost)
{
	m_winSocket = socket(AF_INET, SOCK_DGRAM, 0);
	unsigned long nonBlocking = 1;
	ioctlsocket(m_winSocket, FIONBIO, &nonBlocking);
	m_port = port;

	if (isHost)
	{
		sockaddr_in localAddress;
		localAddress.sin_family = AF_INET;
		localAddress.sin_addr.s_addr = INADDR_ANY;
		localAddress.sin_port = htons(m_port);

		if (bind(m_winSocket, (sockaddr*)&localAddress, sizeof(localAddress)) != 0)
		{
			LOG_ERROR("WSA bind failed. Error Code : %d\n", WSAGetLastError());
			return false;
		}
	}

	return true;
}

bool Socket_impl::initializeTCP(uint16_t port, bool isHost)
{
	// TODO
	assert(false);
	return false;
}

bool Socket_impl::sendUDP(Address adress, const void* buffer, size_t bufferLength)
{
	sockaddr_in remoteAddress;
	remoteAddress.sin_family		= AF_INET;
	remoteAddress.sin_addr.s_addr	= htonl(adress.getAddress()	);
	remoteAddress.sin_port			= htons(adress.getPort()	);

	int dataSent = sendto(m_winSocket, static_cast<const char*>(buffer), (int)bufferLength, 0, reinterpret_cast<sockaddr*>(&remoteAddress), (int)sizeof(remoteAddress));
	if (dataSent == SOCKET_ERROR)
	{
		LOG_ERROR("Send failed. Error Code : %d\n", WSAGetLastError());
		return false;
	}
		
	m_bytesSent += dataSent;

	return true;
}

bool Socket_impl::sendTCP(Address adress, const void* buffer, size_t bufferLength)
{
	// TODO
	assert(false);
	return false;
}

int Socket_impl::receiveUDP()
{
	sockaddr_in remoteAddress;
	int remoteAddrSize = sizeof(remoteAddress);

	char buffer[2048];
	int error = 0;
	int receivedLength = recvfrom(m_winSocket, buffer, 32, 0, (sockaddr*)&remoteAddress, &remoteAddrSize);
	if (receivedLength == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		switch (error)
		{
			case WSAEWOULDBLOCK:
			case WSAECONNRESET:
			{
				return 0;
			}

			default:
			{
				LOG_ERROR("recvfrom failed. Error Code : %d\n", error);
				return -1;
			}
		}
	}
	else
	{
		BitStream* stream = BitStream::create();
		Address incomingAddress(ntohl(remoteAddress.sin_addr.s_addr), ntohs(remoteAddress.sin_port));
		
		stream->writeData(reinterpret_cast<char*>(&incomingAddress), sizeof(Address));
		stream->writeData(buffer, receivedLength);
		m_incomingPackets.push(stream);
	}

	m_bytesReceived += receivedLength;
	return receivedLength;
}

int Socket_impl::receiveTCP()
{
	// TODO
	return 0;
}

Socket* Socket::create(ENetProtocol type)
{
	return new Socket_impl(type);
}

bool initializeWSA()
{
	if (WSAStartup(MAKEWORD(2, 2), &s_wsa) != 0)
	{
		LOG_ERROR("WSAStartup failed. Error Code : %d\n", WSAGetLastError());
		return false;
	}

	s_initializeWSA = false;
	return true;
}

bool closeWSA()
{
	if (WSACleanup() != 0)
	{
		LOG_ERROR("WSACleanup failed. Error Code : %d\n", WSAGetLastError());
		return false;
	}
	return true;
}



#include <network/socket.h>

#include <utility/bitstream.h>
#include <core/debug.h>

#include <assert.h>
#include <stdio.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace network;

static bool    s_initializedWSA = false;
static int     s_numSockets = 0;
static WSADATA s_wsa;

static bool initializeWSA();
static bool closeWSA();

// Win32 Impl
class Socket_win32 : public Socket
{
public:
	Socket_win32();
	~Socket_win32();

	bool initialize(uint16_t port)	override;
	bool isInitialized()      const override;

	bool receive(Address& adress, char* buffer, int32_t& length) override;
	bool send(const Address& adress, const void* buffer, const size_t bufferLength)	override;
	
	uint32_t getPort()            const	override;
	uint64_t getBytesSent()       const override;
	uint64_t getBytesReceived()	  const override;
	uint64_t getPacketsReceived() const override;
	uint64_t getPacketsSent()     const override;

private:
	bool     m_isInitialized;
	uint64_t m_bytesReceived;
	uint64_t m_bytesSent;
	uint64_t m_packetsReceived;
	uint64_t m_packetsSent;
	uint16_t m_port;

	SOCKET m_winSocket;
};

Socket_win32::Socket_win32() : 
	m_isInitialized(false),
	m_bytesReceived(0),
	m_bytesSent(0),
	m_packetsReceived(0),
	m_packetsSent(0),
	m_port(0),
	m_winSocket(0)
{
}

Socket_win32::~Socket_win32()
{
	if (m_isInitialized)
	{
		closesocket(m_winSocket);

		s_numSockets--;
		assert(s_numSockets >= 0);

		if (s_numSockets == 0)
		{
			if (closeWSA())
			{
				s_initializedWSA = false;
			}
			else
			{
				assert(false);
			}
		}
	}

#ifdef _DEBUG
	LOG_DEBUG("~Socket_win32: bytes received: %d, bytes sent: %d, packets received: %d, packets sent: %d",
		m_bytesReceived, m_bytesSent, m_packetsReceived, m_packetsSent);
#endif

}

bool Socket_win32::initialize(uint16_t port)
{
	if (!s_initializedWSA)
	{
		initializeWSA();
	}
	
	m_winSocket = socket(AF_INET, SOCK_DGRAM, 0);
	unsigned long nonBlocking = 1;
	ioctlsocket(m_winSocket, FIONBIO, &nonBlocking);
	m_port = port;

	sockaddr_in localAddress;
	localAddress.sin_family = AF_INET;
	localAddress.sin_addr.s_addr = INADDR_ANY;
	localAddress.sin_port = htons(m_port);

	if (bind(m_winSocket, (sockaddr*)&localAddress, sizeof(localAddress)) != 0)
	{
		LOG_ERROR("WSA bind failed. Error Code : %d\n", WSAGetLastError());
		return false;
	}

	s_numSockets++;
	m_isInitialized = true;
	return m_isInitialized;
}

bool Socket_win32::isInitialized() const
{
	return m_isInitialized;
}

bool Socket_win32::send(const Address& adress, const void* buffer, size_t bufferLength)
{
	sockaddr_in remoteAddress;
	remoteAddress.sin_family = AF_INET;
	remoteAddress.sin_addr.s_addr = htonl(adress.getAddress());
	remoteAddress.sin_port = htons(adress.getPort());

	int dataSent = sendto(m_winSocket, static_cast<const char*>(buffer),
		static_cast<int>(bufferLength), 0,
		reinterpret_cast<sockaddr*>(&remoteAddress),
		static_cast<int>(sizeof(remoteAddress)));

	if (dataSent == SOCKET_ERROR)
	{
		LOG_ERROR("Socket: Send failed. Error Code : %d\n", WSAGetLastError());
		__debugbreak();
		return false;
	}
	m_bytesSent += dataSent;
	m_packetsSent++;
	return true;
}

uint32_t Socket_win32::getPort() const
{
	return m_port;
}

uint64_t Socket_win32::getBytesSent() const
{
	return m_bytesSent;
}

uint64_t Socket_win32::getBytesReceived() const
{
	return m_bytesReceived;
}

uint64_t Socket_win32::getPacketsReceived() const
{
	return m_packetsReceived;
}

uint64_t Socket_win32::getPacketsSent() const
{
	return m_packetsSent;
}

bool Socket_win32::receive(Address& address, char* buffer, int32_t& length)
{
	sockaddr_in remoteAddress;
	int32_t remoteAddrSize = sizeof(remoteAddress);
	int32_t receivedLength = recvfrom(m_winSocket, buffer, g_maxPacketSize, 0, 
									  reinterpret_cast<sockaddr*>(&remoteAddress),
									  &remoteAddrSize);

	if (receivedLength != SOCKET_ERROR)
	{
		address = Address(ntohl(remoteAddress.sin_addr.s_addr),
						  ntohs(remoteAddress.sin_port));
		m_bytesReceived += receivedLength;
		length = receivedLength;

		return true;
	}
	else
	{
		int32_t error = WSAGetLastError();
		switch (error)
		{
			case WSAEWOULDBLOCK:
			case WSAECONNRESET:
			{
				return false;
			}
			case WSAEINVAL:
			{
				LOG_ERROR("recvfrom failed. Error Code : Invalid argument (10022)");
				assert(false);
				return false;
			}
			default:
			{
				LOG_ERROR("recvfrom failed. Error Code : %d\n", error);
				return false;
			}
		}		
	}
}

Socket* Socket::create()
{
	return new Socket_win32();
}

bool initializeWSA()
{
	if (WSAStartup(MAKEWORD(2, 2), &s_wsa) != 0)
	{
		LOG_ERROR("WSAStartup failed. Error Code : %d\n", WSAGetLastError());
		return false;
	}

	s_initializedWSA = true;
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


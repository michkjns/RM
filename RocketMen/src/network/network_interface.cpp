
#include <network/network_interface.h>

#include <core/debug.h>
//#include <crc/crc.h>
#include <network/packet.h>
#include <network/network_message.h>
#include <network/socket.h>

#include <array>
#include <assert.h>
#include <algorithm>

using namespace network;

extern "C" unsigned long crcFast(unsigned char const message[], int nBytes);

static const uint32_t s_maxPeers          = 8;
static const uint32_t s_maxDuplicatePeers = 8; // Maximum peers allowed to have the same address
static const float    s_connectionTimeout = 30.0f; // Seconds to wait until timeout
//==============================================================================

NetworkInterface::NetworkInterface() : 
	m_stateTimer(0.0f),
	m_state(State::STATE_DISCONNECTED)
{
	m_socket = Socket::create(Socket::NetProtocol::PROTOCOL_UDP);
	assert(m_socket);
}

NetworkInterface::~NetworkInterface()
{
	if (m_socket) delete m_socket;
}

void NetworkInterface::clearBuffers()
{
	m_outgoingPackets.clear();
}

void NetworkInterface::receivePackets()
{
	if (!m_socket->isInitialized())
		return assert(false);

	Address address;
	char    buffer[g_maxPacketSize];
	int32_t length = 0;

	std::array<IncomingMessage, s_maxPendingMessages> orderedMsgs;
	uint32_t orderedMsgCount = 0;

	while (m_socket->receive(address, buffer, length))
	{
		assert(length <= g_maxPacketSize);

		Packet* packet = new Packet;
		uint32_t checksum;

		// Reconstruct packet
		BitStream stream;
		stream.writeBuffer(buffer, length);
		checksum = stream.readInt32();
		stream.readBytes((char*)&packet->header, sizeof(PacketHeader));
		stream.readBytes(packet->getData(), packet->header.dataLength);
		

		// Write protocol ID after packet to include in the checksum
		memcpy(packet->getData() + packet->header.dataLength, &s_protocolID, sizeof(s_protocolID));

		if (checksum != crcFast((const unsigned char*)packet->getData(), 
		                        packet->header.dataLength + sizeof(uint32_t)))
		{
			LOG_DEBUG("Checksum mismatch! Packet dicarded.");
			delete packet;
			continue;
		}

		// Dissect packet
		for (int32_t i = 0; i < packet->header.messageCount; i++)
		{
			IncomingMessage msg = packet->readMessage();
			msg.address = address;
			if (msg.isOrdered && orderedMsgCount < s_maxPendingMessages)
			{
				orderedMsgs[orderedMsgCount++] = msg;
			}
			else
			{
				m_incomingMessages.push(msg);
			}
		}
		
		delete packet;
	}

	if(orderedMsgCount > 0)
	{
		std::sort(orderedMsgs.begin(), orderedMsgs.begin() + orderedMsgCount-1,
				  [](IncomingMessage& a, IncomingMessage& b) {
			return (a.sequenceNr < b.sequenceNr);
		});
	}
	for (uint32_t i = 0; i < orderedMsgCount; i++)
	{
		m_incomingMessagesOrdered.push(orderedMsgs[i]);
	}
}

void NetworkInterface::sendPacket(const Address& destination, Packet* packet)
{
	assert(packet != nullptr);

	BitStream stream;
	assert(g_maxBlockSize - packet->header.dataLength > sizeof(uint32_t));

	// Write protocol ID after packet to include in the checksum
	memcpy(packet->getData() + packet->header.dataLength, &s_protocolID, sizeof(s_protocolID));
	uint32_t checksum = crcFast((const unsigned char*)packet->getData(), 
	                            packet->header.dataLength + sizeof(uint32_t));
	stream.writeInt32(checksum);
	stream.writeData(reinterpret_cast<char*>(&packet->header), sizeof(PacketHeader));
	stream.writeData(packet->getData(), packet->header.dataLength);
	m_socket->send(destination, stream.getBuffer(), stream.getLength());

//		LOG_DEBUG("Send packet: %i port", packet->header.messageCount, destination.getPort());
}

void NetworkInterface::setState(State state)
{
	m_state = state;
	m_stateTimer = 0.0f;
}

void NetworkInterface::update(float deltaTime)
{
	m_stateTimer += deltaTime;

	receivePackets();

	switch (m_state)
	{
		case State::STATE_CONNECTING:
		{
			if (m_stateTimer >= s_connectionTimeout)
			{
				return setState(State::STATE_DISCONNECTED);
			}
			break;
		}
		default: break;
	}
}

void NetworkInterface::connect(const Address& destination, const Time& time)
{
	if (!m_socket->isInitialized())
	{
		m_socket->initialize(destination.getPort());
	}

	NetworkMessage message = {};
	message.type           = MessageType::CLIENT_CONNECT_REQUEST;

	Packet* packet = new Packet;
		packet->header = {};
		packet->writeMessage(message);
		sendPacket(destination, packet);
	delete packet;

	setState(State::STATE_CONNECTING);
}

void NetworkInterface::host(uint32_t port)
{
	if (m_socket->isInitialized())
	{
		delete m_socket;
	}
	
	if (m_socket->initialize(port, true))
	{
		setState(State::STATE_HOSTING);
	}
}

//void NetworkInterface::disconnect()
//{
//	if (m_state != State::STATE_HOSTING)
//	{
//		NetworkMessage message = {};
//		message.type = MessageType::CLIENT_DISCONNECT;
//		message.isReliable = true;
//
//		sendMessage(m_serverAddress, message);
//
//		setState(State::STATE_DISCONNECTING);
//	}
//}

void NetworkInterface::sendMessage(const Address& destination, NetworkMessage& message)
{
	Packet* packet = new Packet;
	    packet->header = {};
	    packet->writeMessage(message);
	    sendPacket(destination, packet);
	delete packet;
}

void NetworkInterface::sendMessages(
	const Address& destination, 
	std::vector<NetworkMessage>& messages)
{
	Packet* packet = new Packet;
		packet->header = {};
		packet->header.sequenceNumber = static_cast<int32_t>(m_socket->getPacketsSent());
		for (auto msg : messages)
		{
			packet->writeMessage(msg);
		}
		sendPacket(destination, packet);
	delete packet;
}

bool NetworkInterface::isConnecting() const
{
	return (m_state == State::STATE_CONNECTING);
}

std::queue<IncomingMessage>& NetworkInterface::getMessages()
{
	return m_incomingMessages;
}

std::queue<IncomingMessage>& NetworkInterface::getOrderedMessages()
{
	return m_incomingMessagesOrdered;
}

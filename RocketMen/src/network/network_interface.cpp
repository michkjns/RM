
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
	m_state(State::Disconnected),
	m_receivedMessageCount(0)
{
	m_socket = Socket::create(Socket::NetProtocol::UDP);
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
			LOG_DEBUG("Checksum mismatch; Packet discarded.");
			delete packet;
			continue;
		}

		if (packet->header.ackBits != 0 && 
			packet->header.sequence > 0 && 
			packet->header.ackSequence >= 0)
		{
			IncomingMessage ackMsg = {};
			ackMsg.type     = MessageType::Ack;
			ackMsg.address  = address;
			ackMsg.sequence = packet->header.sequence;
			ackMsg.data.writeInt32(packet->header.ackSequence);
			ackMsg.data.writeInt32(packet->header.ackBits);
			m_incomingMessages.push(ackMsg);
		}
		// Dissect packet
		for (int32_t i = 0; i < packet->header.messageCount; i++)
		{
			IncomingMessage msg = packet->readMessage();

			msg.address = address;
			msg.sequence = m_receivedMessageCount++;
			//if (msg.isOrdered && orderedMsgCount < s_maxPendingMessages)
			//{
			//	orderedMsgs[orderedMsgCount++] = msg;
			//}
			//else
			{
				m_incomingMessages.push(msg);
			}
		}
		
		delete packet;
	}

	//if(orderedMsgCount > 0)
	//{
	//	std::sort(orderedMsgs.begin(), orderedMsgs.begin() + orderedMsgCount-1,
	//			  [](IncomingMessage& a, IncomingMessage& b) {
	//		return (a.sequence < b.sequence);
	//	});
	//}
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
		case State::Connecting:
		{
			if (m_stateTimer >= s_connectionTimeout)
			{
				return setState(State::Disconnected);
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
	
	setState(State::Connecting);
}

void NetworkInterface::host(uint32_t port)
{
	if (m_socket->isInitialized())
	{
		delete m_socket;
	}
	
	if (m_socket->initialize(port, true))
	{
		setState(State::Hosting);
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

bool NetworkInterface::isConnecting() const
{
	return (m_state == State::Connecting);
}

std::queue<IncomingMessage>& NetworkInterface::getMessages()
{
	return m_incomingMessages;
}

std::queue<IncomingMessage>& NetworkInterface::getOrderedMessages()
{
	return m_incomingMessagesOrdered;
}

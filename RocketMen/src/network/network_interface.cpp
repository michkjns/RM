
#include <network/network_interface.h>

#include <core/debug.h>
#include <network/packet.h>
#include <network/network_message.h>
#include <network/socket.h>

#include <array>
#include <assert.h>
#include <algorithm>
#include <map>

using namespace network;

extern "C" unsigned long crcFast(unsigned char const message[], int nBytes);

static const uint32_t s_maxPeers          = 8;
static const uint32_t s_maxDuplicatePeers = 8;     // Maximum peers allowed to have the same address
static const float    s_timeoutSeconds = 30.0f;

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(char)  \
  (char & 0x80 ? '1' : '0'), \
  (char & 0x40 ? '1' : '0'), \
  (char & 0x20 ? '1' : '0'), \
  (char & 0x10 ? '1' : '0'), \
  (char & 0x08 ? '1' : '0'), \
  (char & 0x04 ? '1' : '0'), \
  (char & 0x02 ? '1' : '0'), \
  (char & 0x01 ? '1' : '0')

// ============================================================================

NetworkInterface::NetworkInterface() :
	m_sentPackets(s_sentPacketsBufferSize),
	m_acks(s_maxPendingMessages),
	m_stateTime(0.0f),
	m_state(State::Disconnected),
	m_receivedMessageCount(0),
	m_sequenceCounter(0)
{
	m_socket = Socket::create(Socket::NetProtocol::UDP);
	assert(m_socket);
}

NetworkInterface::~NetworkInterface()
{
	assert(m_socket);
	delete m_socket;
}

void NetworkInterface::clearBuffers()
{
	m_outgoingPackets.clear();
}

void NetworkInterface::receivePackets()
{
	assert(m_socket->isInitialized());

	Address address;
	char    buffer[g_maxPacketSize];
	int32_t length = 0;

	std::array<IncomingMessage, s_maxPendingMessages> orderedMsgs;
	uint32_t orderedMsgCount = 0;

	while (m_socket->receive(address, buffer, length))
	{
		assert(length <= g_maxPacketSize);

		Packet packet;
		uint32_t checksum;

		// Reconstruct packet
		BitStream stream;
		stream.writeBuffer(buffer, length);
		checksum = stream.readInt32();
		stream.readBytes((char*)&packet.header, sizeof(PacketHeader));
		stream.readBytes(packet.getData(), packet.header.dataLength);
		
		//if (m_state == NetworkInterface::State::Connected)
		//{
		//	char* data = new char[packet.header.dataLength + 1];
		//	memcpy(data, packet.getData(), packet.header.dataLength);
		//	data[packet.header.dataLength] = '\0';
		//	std::string dataStr();
		//	for (int i = 0; i < packet.header.dataLength; i++)
		//	{
		//		char byteStr[8];
		//		sprintf_s(byteStr, 8, BYTE_TO_BINARY_PATTERN " ", BYTE_TO_BINARY(data[i]));
		//		dataStr += std::string(byteStr);
		//	}
		//	LOG_DEBUG("%d: %s", packet.header.sequence, dataStr);

		//	delete data;
		//	delete dataStr;
		//}
		// Write protocol ID after packet to include in the checksum
		memcpy(packet.getData() + packet.header.dataLength, &g_protocolID, sizeof(g_protocolID));

		if (checksum != crcFast((const unsigned char*)packet.getData(), 
		                        packet.header.dataLength + sizeof(uint32_t)))
		{
			LOG_DEBUG("NetworkInterface::receivePackets: Checksum mismatch; Packet discarded.");
			continue;
		}

		// Read acks
		if (packet.header.ackBits != 0 &&
			packet.header.sequence > 0 &&
			packet.header.ackSequence >= 0)
		{
			readAcks(packet.header.ackSequence, packet.header.ackBits);
		}

		// Read messages
		for (int32_t i = 0; i < packet.header.messageCount; i++)
		{
			IncomingMessage message = packet.readNextMessage();

			message.address = address;
			message.sequence = m_receivedMessageCount++;

			if (message.type == MessageType::AcceptClient 
				&& m_state == NetworkInterface::State::Connecting)
			{
				setState(NetworkInterface::State::Connected);
			}

			m_incomingMessages.push(message);
		}
	}

	for (uint32_t i = 0; i < orderedMsgCount; i++)
	{
		m_incomingMessagesOrdered.push(orderedMsgs[i]);
	}
}

void NetworkInterface::readAcks(Sequence baseSequence, uint32_t ackBits)
{
	ensure(ackBits != 0);
	ensure(sequenceLessThan(baseSequence, m_sequenceCounter));

	m_sentPackets.getEntry(baseSequence)->acked = true;	

	for (int32_t i = 0; i < 32; i++)
	{
		if ((ackBits & 1) << i)
		{
			const Sequence ackSequence = baseSequence - i;
			if (SentPacketData* packetData = m_sentPackets.getEntry(ackSequence))
			{
				packetData->acked = true;
				for (int16_t j = 0; j < packetData->numMessages; j++)
				{
					m_acks.getEntry(packetData->messageIDs[j])->acked = true;
				}
			}
		}
	}
}

void NetworkInterface::sendPacket(const Address& destination, Packet* packet)
{
	assert(packet != nullptr);
	assert(g_maxBlockSize - packet->header.dataLength > sizeof(uint32_t));

	// Write protocol ID after packet to include in the checksum
	memcpy(packet->getData() + packet->header.dataLength, &g_protocolID, sizeof(g_protocolID));
	uint32_t checksum = crcFast((const unsigned char*)packet->getData(), 
	                            packet->header.dataLength + sizeof(uint32_t));


	//if (m_state == NetworkInterface::State::Hosting)
	//{
	//	char* data = new char[packet->header.dataLength+1];
	//	memcpy(data, packet->getData(), packet->header.dataLength);
	//	data[packet->header.dataLength] = '\0';
	//	LOG_DEBUG("%d: %s", packet->header.sequence, data);
	//}

	BitStream stream;
	stream.writeInt32(checksum);
	stream.writeData(reinterpret_cast<char*>(&packet->header), sizeof(PacketHeader));
	stream.writeData(packet->getData(), packet->header.dataLength);
	m_socket->send(destination, stream.getBuffer(), stream.getLength());
	m_sentPackets.insert(packet->header.sequence);
}

void NetworkInterface::setState(State state)
{
	m_state = state;
	m_stateTime = 0.0f;
}

void NetworkInterface::update(const Time& time)
{
	const float deltaTime = time.getDeltaSeconds();
	m_stateTime += deltaTime;

	if (m_state != State::Disconnected)
	{
		receivePackets();
	}

	switch (m_state)
	{
		case State::Connecting:
		{
			if (m_stateTime >= s_timeoutSeconds)
			{
				return setState(State::Disconnected);
			}
			break;
		}
		case State::Connected:
		case State::Disconnected:
		case State::Disconnecting:
		case State::Hosting:
			break;
	}
}

void NetworkInterface::connect(const Address& destination, const Time& time)
{
	if (!m_socket->isInitialized())
	{
		m_socket->initialize(destination.getPort());
	}

	m_remoteAddress = destination;

	OutgoingMessage message = {};
	message.type = MessageType::RequestConnection;
	message.data.writeInt32(rand());

	Packet packet;
	packet.header = {};
	packet.writeMessage(message);

	sendPacket(destination, &packet);
	setState(State::Connecting);
}

bool NetworkInterface::listen(uint32_t port)
{
	assert(m_socket->isInitialized() == false);
	
	if (m_socket->initialize(port, true))
	{
		setState(State::Hosting);
		return true;
	}

	return false;
}

void NetworkInterface::sendMessages(OutgoingMessage* messages, uint32_t numMessages,
	const Address& destination)
{
	assert(m_state != State::Disconnected);

	Packet packet;
	packet.header = {};
	packet.header.sequence = m_sequenceCounter++;

	for (uint32_t i = 0; i < numMessages; i++)
	{
		OutgoingMessage& message = messages[i];
		if (message.type != MessageType::None)
		{
			packet.writeMessage(message);
			if (message.isReliable || message.isOrdered)
			{
				if (SentMessage* ack = m_acks.insert(message.sequence))
				{
					ack->acked = false;
				}
			}
			message.type = MessageType::None;
		}
	}

	if (!packet.isEmpty())
	{
		sendPacket(destination, &packet);
	}
}

void NetworkInterface::disconnect(const Address& address)
{
	if (m_state == State::Disconnected || m_state == State::Disconnecting)
	{
		ensure(false);
		return;
	}

	OutgoingMessage message = {};
	message.type = MessageType::Disconnect;
	message.isReliable = true;
	sendMessages(&message, 1, address);
	
	setState(State::Disconnecting);
}

SequenceBuffer<SentMessage>& network::NetworkInterface::getAcks() 
{
	return m_acks;
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


#include "client.h"

#include "debug.h"
#include "game_time.h"
#include <assert.h>

using namespace network;

class Client_impl : public Client, public Networker
{
public:
	Client_impl(Time& time);
	~Client_impl();
	
	bool initialize() override;
	void tick() override;
	void connect(const Address& address) override;

private:
	void onHandshake(const Packet& packet);

	void handlePacket(const Packet& packet) override;
	void sendInput();

	Time& m_gameTime;
	int32_t m_lastReceivedState;
};

Client_impl::Client_impl(Time& time) :
	m_gameTime(time),
	m_lastReceivedState(0)
{
}

Client_impl::~Client_impl()
{
}

bool Client_impl::initialize()
{
	if (!Networker::initialize())
	{
		return false;
	}

	return true;
}

void Client_impl::tick()
{
	Networker::tick(m_gameTime.getDeltaSeconds());
}

void Client_impl::connect(const Address& address)
{
	m_networkInterface.connect(address, m_gameTime);
}

void Client_impl::onHandshake(const Packet& packet)
{
	LOG_INFO("Client: Received handshake from the server! I have received ID %d", m_networkInterface.getPeerID());
}

Client* Client::create(Time& time)
{
	return new Client_impl(time);
}

void Client_impl::handlePacket(const Packet& packet)
{
	switch (packet.header.type)
	{
		/** Server to client */
		case ECommand::SERVER_GAMESTATE:
		{
			if (packet.header.sequenceNumber < m_lastReceivedState)
			{
				// discard packet
			}
			else
			{
				m_lastReceivedState = packet.header.sequenceNumber;
			}
			break;
		}
		case ECommand::SERVER_HANDSHAKE:
		{
			onHandshake(packet);
			break;
		}

		/** Connection */
		/*case ECommand::CLIENT_CONNECT:
		{
			break;
		}*/
		case ECommand::CLIENT_DISCONNECT:
		{
			break;
		}

		default: break;
	}
}

void Client_impl::sendInput()
{
	BitStream* stream = BitStream::create();


	Packet packet = createPacket(ECommand::PLAYER_INPUT, stream, -1, EBroadcast::BROADCAST_ALL, EReliable::UNRELIABLE);
	queuePacket(packet);
}


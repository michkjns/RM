
#pragma once

#include <core/state_machine.h>
#include <network/session.h>

#include <cstdint>
#include <functional>

#define DECLARE_GAME_INFO(GAME_NAME, GAME_VERSION) \
virtual const char* const getName()    const override { return GAME_NAME; } \
virtual const char* const getVersion() const override { return GAME_VERSION; }

namespace network {
	class Address;
	class LocalClient;
	class Server;
};

class CommandLineOptions;

class Game
{
public:
	Game();
	virtual ~Game();

	virtual void initialize(const CommandLineOptions& options) = 0;
	virtual void update(const class Time& time);
	virtual void tick(float fixedDeltaTime, Sequence frameCounter, class Physics* physics);
	virtual void terminate();
	virtual void onPlayerJoin(int16_t playerId) = 0;
	virtual void onPlayerLeave(int16_t playerId) = 0;

public:
	GameState* initialize(GameStateFactory* stateFactory, uint32_t initialStateId);
	virtual const char* const getName()    const;
	virtual const char* const getVersion() const;

	uint64_t getTimestep();
	void     setTimestep(uint64_t timestep);

	void processPlayerActions(class ActionBuffer& inputActions, int16_t playerId);
	GameState* pushState(uint32_t stateId);
	void popState();

	bool createSession(GameSessionType type);

	void joinSession(const network::Address& address,
		std::function<void(Game*, JoinSessionResult)> callback);

	void leaveSession();

	bool isSessionActive() const;
	GameSessionType getSessionType() const { return m_sessionType; };

protected:
	uint64_t m_timestep;
	StateMachine m_stateMachine;
	GameStateFactory* m_stateFactory;
	GameSessionType m_sessionType;

private:
	network::LocalClient* m_client;
	network::Server* m_server;
};

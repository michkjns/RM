
#pragma once

#include <buffer.h>
#include <network/address.h>

#include <array>
#include <vector>

namespace network
{
	class  Connection;
	struct Message;

	class RemoteClient
	{
	private:
		static const int32_t s_networkIdBufferSize = 16;

	public:
		RemoteClient();
		~RemoteClient();

		void initialize(int32_t id, Connection* connection);
		void clear();
		void addPlayer(int16_t playerId);

		bool        isUsed()                     const;
		bool        isAvailable()                const;
		bool        ownsPlayer(int16_t playerId) const;
		int32_t     getId()                      const;
		uint32_t    getNumPlayers()              const;
		Connection* getConnection()              const;

		Buffer<int16_t>& getPlayerIds();

	private:
		Connection* m_connection;
		int32_t	    m_id;
		int32_t     m_recentNetworkIds[s_networkIdBufferSize];
		int8_t      m_nextNetworkId;

		Buffer<int16_t> m_playerIds;
	
		friend bool operator== (const RemoteClient& a, const RemoteClient& b);
		friend bool operator!= (const RemoteClient& a, const RemoteClient& b);
	};	
}; // namespace network
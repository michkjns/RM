
#pragma once

#include "address.h"

#include <array>
#include <vector>

namespace network
{
	class  Connection;
	struct Message;

	class RemoteClient
	{
	private:
		static const int32_t  s_networkIdBufferSize = 16;

	public:
		RemoteClient();
		~RemoteClient();

		void initialize(int32_t id, Connection* connection);
		void clear();
		void setNumPlayers(uint32_t numPlayers);

		bool        isUsed()        const;
		bool        isAvailable()   const;
		int32_t     getId()         const;
		uint32_t    getNumPlayers() const;
		Connection* getConnection() const;

	private:
		Connection* m_connection;
		int32_t	    m_id;
		uint32_t    m_numPlayers;
		int32_t     m_recentNetworkIds[s_networkIdBufferSize];
		int8_t      m_nextNetworkId;
	
		friend bool operator== (const RemoteClient& a, const RemoteClient& b);
		friend bool operator!= (const RemoteClient& a, const RemoteClient& b);
	};	
}; // namespace network
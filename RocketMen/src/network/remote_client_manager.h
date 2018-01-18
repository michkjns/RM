
#pragma once

#include <common.h>

class Time;

namespace network
{
	class Address;
	class Connection;
	class RemoteClient;

	class RemoteClientManager
	{
	public:
		RemoteClientManager(int32_t size);
		~RemoteClientManager();

		RemoteClient* add(Connection* connection);
		void remove(RemoteClient* client);
		int32_t getMaxClients() const;
		int32_t count() const;
		void clear();

		void sendMessage(struct Message* message, bool skipLocalClient = false);
		void updateConnections(const Time& time);
		void sendPendingMessages(const Time& time);

		void setLocalClientId(int32_t id);
		int32_t getLocalClientId() const;

		RemoteClient* getClient(const Address& address) const;
		RemoteClient* getClient(const Connection* connection) const;

		RemoteClient* begin();
		RemoteClient* begin() const;
		RemoteClient* end();
		RemoteClient* end() const;
	private:
		RemoteClient* m_clients;
		int32_t m_size;
		int32_t m_localClientId;
		int32_t m_clientIdCounter;
	};

}; //namespace network
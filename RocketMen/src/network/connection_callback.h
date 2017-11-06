
#pragma once

#include <functional>

namespace network
{
	class Connection;

	enum class ConnectionCallback
	{
		ConnectionEstablished,
		ConnectionReceived,
		ConnectionFailed,
		ConnectionLost
	};

	using ConnectionCallbackMethod =
		std::function<void(ConnectionCallback, Connection*)>;

}; // namespace network
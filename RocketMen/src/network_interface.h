
#pragma once

namespace network
{
	class Socket;

	class NetworkInterface
	{
	public:
		NetworkInterface();
		virtual	~NetworkInterface();

	private:
		Socket* m_socket;
	};

} // namespace network
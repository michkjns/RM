
#pragma once

namespace network
{
	class Socket;

	class NetworkInterface
	{
	public:
		NetworkInterface();
		virtual	~NetworkInterface();

		void Send(const void* data, int32_t dataLength,
				  bool broadcast, int32_t recipientID,
				  bool reliable = true,
				  uint8_t channel = 0);


	private:
		Socket* m_socket;
	};

} // namespace network

#pragma once

namespace network
{
	struct Adress
	{

		Adress(char* address, int port)
			: ip4(address)
			, port(port) {}
	
		char* ip4;
		int port;
	};

}; // namespace network

#pragma once

#include <cstdint>

namespace network
{
	// http://gafferongames.com/networking-for-game-programmers/sending-and-receiving-packets/
	class Address
	{
	public:
		Address();
		Address(uint8_t a,
				uint8_t b,
				uint8_t c,
				uint8_t d,
				uint16_t port);

		Address(const char* address, uint16_t port);

		uint32_t	getAddress() 	const;
		uint8_t		getA()			const;
		uint8_t		getB()			const;
		uint8_t		getC()			const;
		uint8_t		getD()			const;
		uint16_t	getPort()		const;

		bool operator == (const Address & other) const;
		bool operator != (const Address & other) const;

	private:
		uint32_t m_address;
		uint16_t m_port;
	};

}; // namespace network
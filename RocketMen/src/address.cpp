
#include "address.h"

using namespace network;

Address::Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port)
{
	m_address = (a << 24) |	(b << 16) |	(c << 8) | d;
	m_port = port;
}

Address::Address(const char* address, uint16_t port)
{
	uint8_t a = 0;
	uint8_t b = 0;
	uint8_t c = 0;
	uint8_t d = 0;

	if (strcmp(address, "localhost") == 0)
	{
		a = 127;
		b = 0;
		c = 0;
		d = 1;
	}
	else
	{
		char* str = _strdup(address);
		char* context = nullptr;
		char* token = strtok_s(str, ".", &context);
		std::string parts[4];
		int i = 0;
		while (token != nullptr)
		{
			parts[i] = token;
			token = strtok_s(nullptr, ".", &context);
			i++;
		}
		free(str);
		a = std::stoi(parts[0]);
		b = std::stoi(parts[1]);
		c = std::stoi(parts[2]);
		d = std::stoi(parts[3]);
	}
	
	m_address = (a << 24) | (b << 16) | (c << 8) | d;
	m_port = port;

}

Address::Address(uint32_t address, uint16_t port)
	: m_address(address)
	, m_port(port)
{
}

uint32_t Address::getAddress() const
{
	return m_address;
}

uint8_t Address::getA() const
{
	return static_cast<uint8_t>(m_address >> 24);
}

uint8_t Address::getB() const
{
	return static_cast<uint8_t>(m_address >> 16);
}

uint8_t Address::getC() const
{
	return static_cast<uint8_t>(m_address >> 8);
}

uint8_t Address::getD() const
{
	return static_cast<uint8_t>(m_address);
}

uint16_t Address::getPort() const
{
	return m_port;
}

std::string Address::getString() const
{
	return std::to_string(getA()) + "." + std::to_string(getB()) + "." + std::to_string(getC()) + "." + std::to_string(getD()) + ":" + std::to_string(getPort());
}

bool Address::operator==(const Address& other) const
{
	return (m_address == other.getAddress()) && (m_port == other.getPort());
}

bool Address::operator!=(const Address& other) const
{
	return !(*this == other);
}

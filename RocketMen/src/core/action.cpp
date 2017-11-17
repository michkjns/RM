
#include <core/action.h>

#include <common.h>
#include <utility.h>

using namespace input;

Action::Action() :
	m_hashedName(0)
{
}

void Action::set(size_t hash, ButtonState inputEvent)
{
	m_hashedName = hash;
	m_inputEvent = inputEvent;
}

void Action::set(size_t hash, float value)
{
	m_hashedName  = hash;
	m_value = value;
}

void Action::set(const std::string& name, ButtonState inputEvent)
{
	m_hashedName = std::hash<std::string>()(name);
	m_inputEvent = inputEvent;
}

void Action::set(const std::string& name, float value)
{
	m_hashedName  = std::hash<std::string>()(name);
	m_value = value;
}

size_t Action::getHash() const
{
	return m_hashedName;
}

float Action::getValue() const
{
	return m_value;
}

bool input::operator==(const Action& a, const Action& b)
{
	return (a.getHash() == b.getHash() && a.getValue() == b.getValue());
}

bool input::operator!=(const Action& a, const Action& b)
{
	return !(a == b);
}

bool input::operator==(const Action& a, const std::string& b)
{
	return (a.m_hashedName == std::hash<std::string>()(toLower(b)));
}

bool input::operator!= (const Action &a, const std::string &b)
{
	return !(a == b);
}
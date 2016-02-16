/*
==============================================================================
String implementation

Author: Michiel
=============================================================================
*/
#include "String.h"
#include <assert.h>
#include <cstring>
#include <stdio.h>

using namespace msh;

String::String()
	: m_data(nullptr)
	, m_length(1)
{
	m_data = new char[1];
	m_data[0] = '\0';
}


String::String(const String& string)
{
	m_length = string.getLength() + 1;
	m_data = new char[m_length];

	strcpy_s(m_data, m_length, string.GetData());
}

String::String(const char* string)
{
	assert(string != nullptr);

	m_length = strlen(string) + 1;
	m_data = new char[m_length];
	strcpy_s(m_data, m_length, string);
}

String::String(const int value)
{
	char buffer[16];
	sprintf_s(buffer, 16, "%d", value);

	m_length = strnlen_s(buffer, 16) + 1;
	m_data = new char[m_length];
	strcpy_s(m_data, m_length, buffer);
}

String::~String()
{
	if (m_data != nullptr)
	{
		delete[] m_data;
	}
}

String& String::operator=(const String &a)
{
	Clear();
	m_length = a.getLength() + 1;
	m_data = new char[m_length];
	strcpy_s(m_data, m_length, a.GetData());
	return *this;
}

String& msh::String::operator=(const char* b)
{
	String str(b);
	*this = str;
	return *this;
}

String& String::operator+=(const String &a)
{
	if (a.IsEmpty())
	{
		return *this;
	}

	size_t length = m_length + a.getLength();
	char* buffer = new char[length];
	strcpy_s(buffer, length, m_data);
	strcpy_s(buffer + getLength(), length - getLength(), a.GetData());
	Clear();
	m_length = length;
	m_data = new char[m_length];
	strcpy_s(m_data, m_length, buffer);
	delete[] buffer;
	return *this;
}

String& msh::String::operator+=(const char* b)
{
	*this += String(b);
	return *this;
}

unsigned int String::getLength() const
{
	return static_cast<unsigned int>(m_length) - 1;
}

bool msh::String::IsEmpty() const
{
	return (getLength() == 0);
}

void String::Clear()
{
	if (m_data != nullptr)
	{
		delete[] m_data;
	}

	m_length = 0;
	m_data = nullptr;
}

char* String::GetData() const
{
	return m_data;
}

String msh::String::Substring(unsigned int begin, unsigned int length) const
{
	length = (begin + length < getLength()) ? length : getLength() - begin;

	char* substr = new char[length + 1];
	memcpy(substr, m_data + begin, length);
	substr[length] = '\0';
	String str(substr);
	delete[] substr;
	return str;
}

unsigned int String::Find(const String& string) const
{
	const unsigned int length = getLength();
	for (unsigned int i = 0; i < length; i++)
	{
		if (strncmp(m_data + i, string.GetData(), string.getLength()) == 0)
		{
			return i;
		}
	}

	return npos;
}

String msh::operator+(const String& a, const String& b)
{
	if (a.IsEmpty())
	{
		return b;
	}
	else if (b.IsEmpty())
	{
		return a;
	}

	String str(a);
	str += b;
	return str;
}

String msh::operator+(const String & a, const char* b)
{
	String str(b);
	return a + str;
}

/*
==============================================================================
Standard mutable String class

Author: Michiel
=============================================================================
*/
#pragma once

namespace msh
{

class String
{
	friend String operator+(const String& a, const String& b);
	friend String operator+(const String& a, const char* b);

public:
	String();
	String(const String &string);
	String(const char* string);
	String(const int value);

	~String();

	unsigned int getLength() const;
	bool IsEmpty() const;
	char* GetData() const;
	String Substring(unsigned int begin, unsigned int length) const;
	//unsigned int Find(const String& string) const;
	unsigned int Find(const String& string) const;

	String& operator=(const String&);
	String& operator=(const char*);
	String& operator+=(const String&);
	String& operator+=(const char*);

	static const unsigned int npos = 0xffffffff;
private:
	void Clear();
	char* m_data;
	size_t m_length;

};

}; // namespace msh
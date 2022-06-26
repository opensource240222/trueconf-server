#pragma once

#include <stdlib.h>
#include "src/VS_BaseBuffers.h"
#include "src/VS_AsnBuffers.h"
#include "std-generic/cpplib/string_view.h"

// Class, that represents string format, used in h323 messages and
// provide conversation between c-strings and h323-strings.
// For example:
// (c-string, ASCII)"ALIAS" => (h323-string)"\0A\0L\0I\0A\0S"
// Unicode strings are not supported yet.
class VS_H323String
{
	// Data of the string. Contains double zero symbols at the end.
	// E.g.: (c-string, ASCII)"ALIAS" => (h323-string)"\0A\0L\0I\0A\0S\0\0"
	unsigned char*	m_data;
	// Length of the string.
	// E.g.: length of "\0A\0L\0I\0A\0S\0\0" will be 5.
	size_t			m_length;
	// Service method to allocate memory and save the ASCII string.
	void SaveASCIIString(const char *str, std::size_t len)
	{
		assert(str);
		// Allocate memory;
		m_length = len;
		size_t bytesize = (m_length + 1) * 2;
		m_data = new unsigned char[bytesize];
		// Copy data.
		for(size_t i = 0; i < bytesize; i++ )
		m_data[i] = (i%2) ? *str++ : 0;
	}
	// Service method. Copy all data from raw bytes buffer <data> that have <length> bytes.
	// For example, buffer can be "\0A\0L\0I\0A\0S", length = 10.
	// Do not free previous buffers!
	bool CopyDataFrom(void* data, size_t length)
	{
		// Start conditions.
		if(data == NULL || length == 0 || length%2 == 1) return false;
		// Copy data.
		const auto p_data = static_cast<const char*>(data) + length - 2;
		const bool append_null = *(reinterpret_cast<const std::uint16_t*>(p_data)) != 0;
		m_data = new unsigned char[append_null ? length + 2 : length]; // + 2 zero bytes at the end
		m_length = length / 2 + (append_null ? 0 : -1);
		if (static_cast<void*>(m_data) != data)
		{
			::memcpy(m_data, data, length);
		}

		if (append_null)
		{
			// Add "\0\0" at the end.
			::memset(m_data + length, 0, 2);
		}
		return true;
	}
	// Service method. Free all allocated memory.
	void FreeMemory()
	{
		delete[] m_data;
		m_length = 0;
		m_data = 0;
	}
public:
	VS_H323String()
		:m_data(0), m_length(0)
	{
		SaveASCIIString("", 0);
	}

	explicit VS_H323String(string_view str)
		:m_data(0), m_length(0)
	{
		const char *c_str = "";
		std::size_t len = 0;

		if (!str.empty())
		{
			c_str = str.data();
			len = str.length();
		}
		SaveASCIIString(c_str, len);
	}

	VS_H323String(const VS_H323String& other)
	{
		if(!CopyDataFrom(other.m_data, other.m_length * 2))
			SaveASCIIString("", 0);
	}

	explicit VS_H323String(const VS_BitBuffer& buff)
	{
		if(!CopyDataFrom(buff.GetData(), buff.ByteSize()))
			SaveASCIIString("", 0);
	}

	~VS_H323String()
	{
		FreeMemory();
	}
	VS_H323String& operator=(const VS_H323String& other)
	{
		FreeMemory();
		if(!CopyDataFrom(other.m_data, other.m_length * 2))
			SaveASCIIString("", 0);
		return *this;
	}
	VS_H323String& operator=(const VS_BitBuffer& buff)
	{
		FreeMemory();
		if(!CopyDataFrom(buff.GetData(), buff.ByteSize()))
			SaveASCIIString("", 0);
		return *this;
	}
	// Makes VS_PerBuffer, that contains h323-string.
	// If <zero_end> parameter is true, will return VS_PerBuffer with
	// double zero symbols at the end.
	// For example:
	// (<zero_end> == false) => "\0A\0L\0I\0A\0S"
	// (<zero_end> == true)  => "\0A\0L\0I\0A\0S\0\0"
	VS_PerBuffer MakePerBuffer(bool zeroEnd = false) const
	{
		const size_t length = zeroEnd ? m_length + 1 : m_length;
		return VS_PerBuffer(m_data, length * 2 * 8);
	}

	std::string MakeString() const
	{
		std::string str;
		str.resize(m_length);
		for (std::size_t i = 0; i < m_length; i++)
		{
			str[i] = m_data[i * 2 + 1];
		}
		return str;
	}

	// Equal operator.
	bool operator==(const VS_H323String& other) const
	{
		if(m_length != other.m_length) return false;
		if(m_length == 0) return true;
		return memcmp(m_data, other.m_data, m_length * 2) == 0;
	}
	// Inequality operator.
	bool operator!=(const VS_H323String& other) const
	{
		return !(*this == other);
	}
	// Get length of inside data
	size_t Length() const
	{
		return m_length;
	}
	// Check if no data inside
	bool IsEmpty() const
	{
		return m_length == 0;
	}

};
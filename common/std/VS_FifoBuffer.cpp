#include "VS_FifoBuffer.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <vector>

VS_FifoBuffer::VS_FifoBuffer(int MaxLen)
{
	m_Buffer = (char*)malloc(MaxLen);
	m_MaxLen = MaxLen;
	m_StartPos = 0;
	m_EndPos = 0;
}

VS_FifoBuffer::~VS_FifoBuffer()
{
	if (m_Buffer) free(m_Buffer);
}

bool VS_FifoBuffer::AddData(void* data, int len)
{
	if (len > m_MaxLen - GetDataLen())
		return false;

	if (len <= 0)
		return true;

	int busyBytes = m_EndPos % m_MaxLen;
	int size = std::min((m_MaxLen - busyBytes), len);

	memcpy(m_Buffer + busyBytes, data, size);

	m_EndPos += len;
	len -= size;

	if (len > 0)
		memcpy(m_Buffer, (char*)data + size, len);

	return true;
}

bool VS_FifoBuffer::GetData(void* data, int len, bool discard)
{
	if (GetDataLen() < len || len <= 0)
		return false;

	int empty_bytes = m_StartPos % m_MaxLen;
	int size = std::min((m_MaxLen - empty_bytes), len);

	if (discard)
		m_StartPos += len;

	memcpy(data, m_Buffer + empty_bytes, size);

	len -= size;

	if (len > 0)
		memcpy((char*)data + size, m_Buffer, len);

	return true;
}

void* VS_FifoBuffer::GetData()
{
	return m_Buffer;
}

void VS_FifoBuffer::Discard(int len)
{
	m_StartPos += std::min(len, GetDataLen());
}

int VS_FifoBuffer::GetDataLen()
{
	return (int)(m_EndPos - m_StartPos);
}

std::pair<char*, size_t> VS_FifoBuffer::GetFirstArray()
{
	if (m_StartPos / m_MaxLen < m_EndPos / m_MaxLen
		&& m_EndPos % m_MaxLen != 0)
	{
		return { m_Buffer + m_StartPos % m_MaxLen, m_EndPos - m_StartPos - m_EndPos % m_MaxLen };
	}
	else
	{
		return { m_Buffer + m_StartPos % m_MaxLen, m_EndPos - m_StartPos };
	}
}

std::pair<char*, size_t> VS_FifoBuffer::GetSecondArray()
{
	if (m_StartPos / m_MaxLen < m_EndPos / m_MaxLen
		&& m_EndPos % m_MaxLen != 0)
	{
		return { m_Buffer, m_EndPos % m_MaxLen };
	}
	else
	{
		return { nullptr, 0 };
	}
}

void VS_FifoBuffer::Linearize()
{
	auto second = GetSecondArray();

	if (second.second)
	{
		std::vector<uint8_t> tmp(second.first, second.first + second.second);

		auto first = GetFirstArray();

		memmove(m_Buffer, first.first, first.second);
		memcpy(m_Buffer + first.second, tmp.data(), tmp.size());
	}

	m_EndPos -= m_StartPos;
	m_StartPos = 0;
}

void VS_FifoBuffer::Clear()
{
	m_StartPos = 0;
	m_EndPos = 0;
}

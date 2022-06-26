#pragma once

#include <cstdint>
#include <stddef.h>
#include <utility>

class VS_FifoBuffer
{
	char *		m_Buffer;			///< store buffer
	int			m_MaxLen;			///< max lenght of buffer
	int64_t		m_StartPos;			///< start byte position
	int64_t		m_EndPos;			///< end byte position
public:
	/// Constructor
	VS_FifoBuffer(int MaxLen);
	/// Destructor
	~VS_FifoBuffer();
	/// Add data to fifo buffer
	bool AddData(void* data, int len);
	/// Get data from fifo buffer
	bool GetData(void* data, int len, bool discard = true);

	void* GetData();

	void Discard(int len);

	void Linearize();

	std::pair<char*, size_t> GetFirstArray();
	std::pair<char*, size_t> GetSecondArray();

	int GetDataLen();
	/// Clear buffer by reset indexes
	void Clear();
};
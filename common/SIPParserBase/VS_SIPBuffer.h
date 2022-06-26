/**
 **************************************************************************
 * \file VS_SIPBuffer.h
 * (c) 2002-2006 Visicron Inc.  http://www.visicron.net/
 * \brief Буфер для парсинга протокола SIP (+SDP)
 *
 * \b Project ^SIPLib
 *
 * \author ktrushnikov
 * \date 11-03-2206
 ****************************************************************************/

#pragma once

#include "VS_SIPError.h"
#include "std-generic/cpplib/string_view.h"
#include <memory>

class VS_SIPBuffer: public VS_SIPError
{
public:
	VS_SIPBuffer();
	VS_SIPBuffer(const char* aPtr, const std::size_t aSize);
	explicit VS_SIPBuffer(string_view str);
	VS_SIPBuffer(const VS_SIPBuffer &rhs);
	~VS_SIPBuffer();

	TSIPErrorCodes AddData(const char* aPtr, std::size_t aSize);
	template <size_t N>
	TSIPErrorCodes AddData(const char (&str)[N])
	{
		static_assert(N > 1, "Non-empty string literal required");
		return AddData(str, N - 1);
	}
	TSIPErrorCodes AddData(string_view str)
	{
		return AddData(str.data(), str.size());
	}
	TSIPErrorCodes AddData(VS_SIPBuffer& aBuffer);

	TSIPErrorCodes GetData(char* aPtr, std::size_t aSize);
	TSIPErrorCodes GetDataConst(char* aPtr, std::size_t aSize);
	TSIPErrorCodes GetDataAlloc(std::unique_ptr<char []> &aPtr, std::size_t aSize);
	TSIPErrorCodes GetDataAllocConst(std::unique_ptr<char []> &aPtr, std::size_t aSize);
	TSIPErrorCodes GetAllDataAllocConst(std::unique_ptr<char []> &aPtr, std::size_t &aSize) const;
	TSIPErrorCodes GetAllDataAllocConst(std::unique_ptr<char []> &aPtr) const;

	TSIPErrorCodes GetNextBlock(char* aPtr, std::size_t &aSize);
	TSIPErrorCodes GetNextBlockConst(char* aPtr, std::size_t &aSize);
	TSIPErrorCodes GetNextBlockAlloc(std::unique_ptr<char []> &aPtr, std::size_t &aSize);
	TSIPErrorCodes GetNextBlockAllocConst(std::unique_ptr<char []> &aPtr, std::size_t &aSize);

	TSIPErrorCodes GetHeaderAllocConst(const char* aSeparator, std::unique_ptr<const char[]> &aPtr, std::size_t &aSize);
	TSIPErrorCodes SkipHeader();

	std::size_t GetReadIndex() const;
	std::size_t GetWriteIndex() const;
	std::size_t GetBufferSize() const;

	TSIPErrorCodes Skip(std::size_t aBytes);

	bool operator==(const VS_SIPBuffer& aBuffer);
	VS_SIPBuffer& operator=(const VS_SIPBuffer& aBuffer);

	void Clean() noexcept override;

protected:
	TSIPErrorCodes MultiLineToSingle(const char* aInput, std::size_t aInputSize,
							 const char* &aOutput, std::size_t &aOutputSize) const;

	void SetReadIndex(std::size_t aValue);
	void SetWriteIndex(std::size_t aValue);

private:
	void Init();

	char* iPtr;
	static const int BLOCK_SIZE = 256;
	std::size_t iReadIndex, iWriteIndex;
	std::size_t iSize;
};

#include "VS_SIPBuffer.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/utf8.h"
#include "std-generic/cpplib/scope_exit.h"

#include "std-generic/compat/memory.h"
#include <string.h>

typedef std::make_signed<std::size_t>::type s_size_t;

void VS_SIPBuffer::Init()
{
	iPtr = 0;
	iWriteIndex = 0;
	iReadIndex = 0;
	iSize = 0;
}

VS_SIPBuffer::VS_SIPBuffer()
{
	Init();
}

VS_SIPBuffer::VS_SIPBuffer(const char* aPtr, const std::size_t aSize)
{
	Init();

	this->AddData(aPtr, aSize);
}

VS_SIPBuffer::VS_SIPBuffer(string_view str)
{
	Init();

	this->AddData(str.data(), str.size());
}

VS_SIPBuffer::VS_SIPBuffer(const VS_SIPBuffer &rhs)
	: VS_SIPError(rhs), iPtr(0), iReadIndex(rhs.iReadIndex), iWriteIndex(rhs.iWriteIndex), iSize(rhs.iSize)
{
	SetError(rhs.GetLastClassError());
	SetValid(rhs.IsValid());

	if (iSize)
	{
		iPtr = new char[iSize];
		memcpy(iPtr, rhs.iPtr, iSize);
	}
}

VS_SIPBuffer::~VS_SIPBuffer()
{
	VS_SIPBuffer::Clean();
}

TSIPErrorCodes VS_SIPBuffer::MultiLineToSingle(const char* aInput, std::size_t aInputSize,
									const char* &aOutput, std::size_t &aOutputSize) const
{
	if ( !aInput )
		return TSIPErrorCodes::e_InputParam;

	const char* p = aInput;

	std::size_t i = 0;
	while( (i < aInputSize) && ((p = strstr(p, "\n\t")) != 0) )
	{
		i++;
		p += 2;
	}

	if ( !i || ((i * 2) >= aInputSize) )
	{
		aOutput = aInput;
		aOutputSize = aInputSize;
		return TSIPErrorCodes::e_ok;
	}

	aOutputSize = aInputSize - (i * 2);

	auto out = new char[aOutputSize];
	aOutput = out;

	const char* p1 = nullptr;
	const char* p2 = aInput;
	std::size_t l = 0;

	i = 0;
	while ( (i < aOutputSize) && ((p1 = strstr(p2, "\n\t")) != 0) )
	{
		auto l1 = strlen(p1);
		auto l2 = strlen(p2);
		l = l2 - l1;

		memcpy(out, p2, l);
		out += l;

		p2 = p1 + 2;
		i++;
	}

	l = strlen(p2);
	if ( l > 0)
		memcpy(out, p2, l);

	return TSIPErrorCodes::e_ok;


//	char* p;
//	unsigned int l;

// ������ ������ �� �����
	//while( (p = strstr(iPtr, "\n\t")) != 0)
	//{
	//	l = (unsigned int) strlen(p);

	//	strncpy(p, p+2, l-2);
	//	iSize -= 2;
	//}
}

/**
 ******************************************************************************
 * ��������� ������ � ����� ������� iWriteIndex
 * \return int - ��� ������ TSIPErrorCodes
 *
 * \param aPtr - ��������� �� ������, ������� ��������
 * \param aSize - ���-�� ���� ��� ���������� (����� ������ aPtr)
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
TSIPErrorCodes VS_SIPBuffer::AddData(const char* aPtr, std::size_t aSize)
{
	if ( !aPtr || (aSize <= 0) )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	const auto free = iSize - iWriteIndex;

	if (aSize < free)
	{
		// �� ����� malloc() - ������ �����������
		memcpy(iPtr+iWriteIndex, aPtr, aSize);
	}
	else
	{
		// ����� malloc()
		const std::size_t need_alloc = aSize - free;

		std::size_t nblock = need_alloc / BLOCK_SIZE;
		nblock++;

		char * ptr = new char[iSize + nblock*BLOCK_SIZE];

		// copy old buffer
		memcpy(ptr, iPtr, iWriteIndex);


		// copy new string
		memcpy(ptr+iWriteIndex, aPtr, aSize);
		// free old buffer
		delete[] iPtr;
		// save buffer in class
		iPtr = ptr;
		iSize += nblock*BLOCK_SIZE;
	}

	iWriteIndex += aSize;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);

	return TSIPErrorCodes::e_ok;
}

/**
 ******************************************************************************
 * ��������� ������ � ����� ������� iWriteIndex
 * \return int - ��� ������ TSIPErrorCodes
 *
 * \param aBuffer - �����, ������� �������� (����������� ������ ���������� [0..iWriteIndex])
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
TSIPErrorCodes VS_SIPBuffer::AddData(VS_SIPBuffer& aBuffer)
{
	if ( !aBuffer.IsValid() )
	{
		return aBuffer.GetLastClassError();
	}

	const std::size_t size = aBuffer.iWriteIndex/* + 1*/;

	// ����� ��� ���������� ����
	if (size < 1)
		return TSIPErrorCodes::e_ok;

	return this->AddData(aBuffer.iPtr, size);
}

/**
 ******************************************************************************
 * ��������� ������ �� ������ �� ������� iReadIndex (�� ������� ������)
 * \return int - ��� ������ TSIPErrorCodes
 *
 * \param aPtr - ��������� �� ���������� ������, ���� ���� ���������� ������
 * \param aSize - ���-�� ����, ������� ����� ������� (� ������ - ������ aPtr)
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
TSIPErrorCodes VS_SIPBuffer::GetDataConst(char* aPtr, std::size_t aSize)
{
	if ( !aPtr || (aSize <= 0) )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	if ( !this->IsValid() )
	{
		return this->GetLastClassError();
	}

	const std::size_t need = iReadIndex + aSize;

	// ������ ������� ������, ��� ����
	if (need > iWriteIndex)
	{
		SetError(TSIPErrorCodes::e_EndOfBuffer);
		return TSIPErrorCodes::e_EndOfBuffer;
	}

	memcpy(aPtr, iPtr + iReadIndex, aSize);
	aPtr[aSize] = '\0';

	return TSIPErrorCodes::e_ok;
}

/**
 ******************************************************************************
 * ��������� ������ �� ������ ������� iReadIndex (�� ������� ������)
 * \return int - ��� ������ TSIPErrorCodes
 *
 * \param aPtr - ��������� �� ���������� ������, ���� ����� ���������� ������
 * \param aSize - ���-�� ����, ������� ����� ������� (� ������ - ������ aPtr)
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
TSIPErrorCodes VS_SIPBuffer::GetData(char* aPtr, std::size_t aSize)
{
	if ( (aPtr == 0) || (aSize <= 0) )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	if ( !this->IsValid() )
	{
		return this->GetLastClassError();
	}
	TSIPErrorCodes ret = this->GetDataConst(aPtr, aSize);
	if (ret != TSIPErrorCodes::e_ok) return ret;

	iReadIndex += aSize;
//	iReadIndex++;			// skip '\n'

	return TSIPErrorCodes::e_ok;
}

/**
 ******************************************************************************
 * ��������� ������ �� ������ �� ������� iReadIndex (������� ������)
 * \return int - ��� ������ TSIPErrorCodes
 *
 * \param aPtr - ���������, ���� ���� ���������� ������
 * \param aSize - ���-�� ����, ������� ����� ������� (� ������ - ������ aPtr)
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
TSIPErrorCodes VS_SIPBuffer::GetDataAllocConst(std::unique_ptr<char []> &aPtr, std::size_t aSize)
{
	if ( aSize <= 0 )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	if ( !IsValid() )
	{
		return GetLastClassError();
	}

	auto s = std::make_unique<char []>(aSize + 1);		// added '\0'

	TSIPErrorCodes ret = this->GetDataConst(s.get(), aSize);
	if (ret != TSIPErrorCodes::e_ok)
	{
		return ret;
	}

	aPtr = std::move(s);

	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

/**
 ******************************************************************************
 * ��������� ������ �� ������ ������� iReadIndex (������� ������)
 * \return int - ��� ������ TSIPErrorCodes
 *
 * \param aPtr - ��������� �� ���������� ������, ���� ���� ���������� ������
 * \param aSize - ���-�� ����, ������� ����� ������� (� ������ - ������ aPtr)
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
TSIPErrorCodes VS_SIPBuffer::GetDataAlloc(std::unique_ptr<char []> &aPtr, std::size_t aSize)
{
	if ( aSize <= 0 )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	if ( !IsValid() )
	{
		return GetLastClassError();
	}

	TSIPErrorCodes ret = GetDataAllocConst(aPtr, aSize);
	if (ret != TSIPErrorCodes::e_ok) return ret;

	iReadIndex += aSize;

	return ret;
}

/**
 ******************************************************************************
 * ��������� ������ �� ������ ������� iReadIndex (�� ������� ������)
 * \return int - ��� ������ TSIPErrorCodes
 *
 * \param aPtr - ���������, ���� ���� ���������� ������
 * \param aSize - ���-�� ����, ������ ������
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
TSIPErrorCodes VS_SIPBuffer::GetNextBlock(char* aPtr, std::size_t &aSize)
{
	if ( !IsValid() )
	{
		return GetLastClassError();
	}

	TSIPErrorCodes ret = GetNextBlockConst(aPtr, aSize);
	if (ret != TSIPErrorCodes::e_ok) return ret;

	iReadIndex += aSize;
	iReadIndex += 2;			// skip '\r\n'

	SetError(ret);
	return ret;
}

/**
 ******************************************************************************
 * ��������� ������ �� ������ �� ������� iReadIndex (�� ������� ������)
 * \return int - ��� ������ TSIPErrorCodes
 *
 * \param aPtr - ��������� �� ���������� ������, ���� ���� ���������� ������
 * \param aSize - ���-�� ����, ������ ������
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
TSIPErrorCodes VS_SIPBuffer::GetNextBlockConst(char* aPtr, std::size_t &aSize)
{
	if ( !IsValid() )
	{
		return GetLastClassError();
	}

	char* s = iPtr + iReadIndex;
	const s_size_t sz = iWriteIndex - iReadIndex;

	if ( sz == 0)
		return TSIPErrorCodes::e_EndOfBuffer;

	if ( sz < 0)
		return TSIPErrorCodes::e_buffer;

	char* rn = 0;
	std::size_t i(0);
	while(i < static_cast<std::size_t>(sz - 1))
	{
		if (s[i]=='\r' && s[i+1]=='\n')
		{
			rn = s+i;
			break;
		}
		++i;
	}
	if ( !rn )
		return TSIPErrorCodes::e_EndOfBuffer;

	const std::size_t pos = rn - s;

	if ( !pos )							// ������ ������ - ������ ������� ������ "\r\n"
		return TSIPErrorCodes::e_EndOfBuffer;

	if (pos > static_cast<std::size_t>(sz))
		return TSIPErrorCodes::e_buffer;

	if (aSize < pos)
	{
		aSize = pos;
		return TSIPErrorCodes::e_buffer;
	}

	if ( !aPtr )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	TSIPErrorCodes ret = GetDataConst(aPtr, pos);
	if (ret != TSIPErrorCodes::e_ok) return ret;

	aSize = pos;

	SetError(ret);
	return ret;
}

/**
 ******************************************************************************
 * ��������� ������ �� ������ �� ������� iReadIndex (������� ������)
 * \return int - ��� ������ TSIPErrorCodes
 *
 * \param aPtr - ��������� �� ���������� ������, ���� ���� ���������� ������
 * \param aSize - ���-�� ����, ������ ������
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
TSIPErrorCodes VS_SIPBuffer::GetNextBlockAllocConst(std::unique_ptr<char[]> &aPtr, std::size_t &aSize)
{
	if ( !IsValid() )
		return GetLastClassError();

	if (iReadIndex > iWriteIndex)
		return TSIPErrorCodes::e_EndOfBuffer;

	aSize = 0;

	TSIPErrorCodes ret = this->GetNextBlockConst(aPtr.get(), aSize);
	if ( ret != TSIPErrorCodes::e_ok )
	{
		if ( aSize > 0 )
		{
			auto s = std::make_unique<char[]>(aSize + 1);

			ret = this->GetNextBlockConst(s.get(), aSize);
			if ( ret != TSIPErrorCodes::e_ok )
			{
				if ( aPtr ) {
					aPtr.reset();
					aSize = 0;
					return ret;
				}
			}
			else
			{
				aPtr = std::move(s);
			}
		}else{
			aPtr = 0;
			aSize = 0;

			return ret;
		}
	}

	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

/**
 ******************************************************************************
 * ��������� ������ �� ������ ������� iReadIndex (������� ������)
 * \return int - ��� ������ TSIPErrorCodes
 *
 * \param aPtr - ���������, ���� ���� ���������� ������
 * \param aSize - ���-�� ����, ������ ������
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
TSIPErrorCodes VS_SIPBuffer::GetNextBlockAlloc(std::unique_ptr<char[]> &aPtr, std::size_t &aSize)
{
	if ( !IsValid() )
	{
		return GetLastClassError();
	}

	TSIPErrorCodes ret = GetNextBlockAllocConst(aPtr, aSize);
	if (ret != TSIPErrorCodes::e_ok) return ret;

	iReadIndex += aSize;
	iReadIndex += 2;		// skip '\r\n'

	return ret;
}

TSIPErrorCodes VS_SIPBuffer::GetHeaderAllocConst(const char* aSeparator, std::unique_ptr<const char[]> &aPtr, std::size_t &aSize)
{
	if ( !aSeparator )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	std::unique_ptr<char[]> theBlock;
	std::size_t theSize = 0;

	TSIPErrorCodes res = GetNextBlockAllocConst(theBlock, theSize);
	if (TSIPErrorCodes::e_ok != res )
		return res;

	std::string str;
	string_view sv(theBlock.get(), theSize);
	auto pos = sv.find_first_of(aSeparator);
	if (pos != decltype(sv)::npos) {
		sv.remove_suffix(sv.length() - pos);
		str = vs::UTF8ToUpper(sv);
	}
	else
		str = "STARTLINE";

	if (str.empty())
		return TSIPErrorCodes::e_InputParam;

	auto ptr = vs::make_unique_default_init<char[]>(str.length() + 1);	// added '\0'
	memcpy(ptr.get(), str.c_str(), str.length());
	ptr[str.length()] = '\0';

	aPtr = std::move(ptr);
	aSize = str.length();
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPBuffer::SkipHeader()
{
	char* s = iPtr + iReadIndex;
	const s_size_t sz = iWriteIndex - iReadIndex;

	if ( sz == 0)
		return TSIPErrorCodes::e_EndOfBuffer;

	if ( sz < 0)
		return TSIPErrorCodes::e_buffer;

	char* rn = strstr(s, "\r\n");
	if ( !rn )
		return TSIPErrorCodes::e_buffer;

	const s_size_t pos = rn - s;

	if (pos > sz)
		return TSIPErrorCodes::e_buffer;

	iReadIndex += pos;		// Skip header
	iReadIndex += 2;		// Skip '\r\n'

	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPBuffer::Skip(std::size_t aBytes)
{
	if ( (iReadIndex + aBytes) > iWriteIndex )
		return TSIPErrorCodes::e_InputParam;

	iReadIndex += aBytes;

	return TSIPErrorCodes::e_ok;
}

/**
 ******************************************************************************
 * ���������� �������� ������� ������ (���������� �����)
 * \return unsigned int - ������ ������
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
std::size_t VS_SIPBuffer::GetReadIndex() const
{
	return iReadIndex;
}

/**
 ******************************************************************************
 * ���������� �������� ������� ������ (���������� �����)
 * \return unsigned int - ������ ������
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
std::size_t VS_SIPBuffer::GetWriteIndex() const
{
	return iWriteIndex;
}

/**
 ******************************************************************************
 * ����� �������� ������� ������ (���������� �����)
 * \return void
 *
 * \param aValue - ���������� ��������
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
void VS_SIPBuffer::SetReadIndex(std::size_t aValue)
{
	iReadIndex = aValue;
}

/**
 ******************************************************************************
 * ����� �������� ������� ������ (���������� �����)
 * \return void
 *
 * \param aValue - ���������� ��������
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
void VS_SIPBuffer::SetWriteIndex(std::size_t aValue)
{
	iWriteIndex = aValue;
}

/**
 ******************************************************************************
 * ���������� �������� ������� ������ (���������� �����)
 * \return unsigned int - ������ ������
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
std::size_t VS_SIPBuffer::GetBufferSize() const
{
	return iSize;
}

/**
 ******************************************************************************
 * ��-������ ���������� ��� ������ (��������� ����� � ��-������ ������)
 * \return bool - ������ ����� true, �� ����� - false
 *
 * \param aBuffer - ����� ��� ��������� (������ ����� ���������==)
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
bool VS_SIPBuffer::operator==(const VS_SIPBuffer& aBuffer)
{
	// ��������� �� ������ ����: if (a == a)
	if (this == &aBuffer)
		return true;
	if (this->VS_SIPError::operator!=(aBuffer))
	{
		return false;
	}
	// �������� �� ����������
	if ( !this->IsValid() || !aBuffer.IsValid() )
		return false;

	if (this->iWriteIndex != aBuffer.iWriteIndex)
		return false;

	//if ( (this->iPtr == 0) || (buffer.iPtr == 0) )
	//	return

	std::size_t sz = this->iWriteIndex;

	for(std::size_t i = 0; i < sz; i++)
	{
		if ( !(this->iPtr[i] == aBuffer.iPtr[i]) )
		{
			return false;
		}
	}

	return true;
}

/**
 ******************************************************************************
 * ��-������ �������� ������ ����� � ����� (��������� ����� � ��-������ ������)
 * \return VS_SIPBuffer& - ����� ����� ���������=
 *
 * \param aBuffer - ����� ��� ����������� (������ ����� ���������==)
 *
 * \date    11-03-2006
 * \author	ktrushnikov
 *****************************************************************************/
VS_SIPBuffer& VS_SIPBuffer::operator=(const VS_SIPBuffer& aBuffer)
{
	if (this == &aBuffer)
		return *this;
	if (!(*this == aBuffer))
	{
		this->VS_SIPError::operator=(aBuffer);
		// clean old buffer
		if (this->iPtr != 0) delete this->iPtr;

		// alloc new memory
		if (aBuffer.iSize > 0)
		{
			this->iPtr = new char[aBuffer.iSize];
			if (this->iPtr == 0) return *this;
		}
		else
		{
			this->iPtr = 0;
		}

		// copy buffer
		for (std::size_t i = 0; i < aBuffer.iSize; i++)
		{
			this->iPtr[i] = aBuffer.iPtr[i];
		}
		this->iSize = aBuffer.iSize;

		// copy index
		this->iReadIndex = aBuffer.iReadIndex;
		this->iWriteIndex = aBuffer.iWriteIndex;

		// copy VS_SIPError
		this->SetError(aBuffer.GetLastClassError());
		this->SetValid(aBuffer.IsValid());
	}
	return *this;
}

void VS_SIPBuffer::Clean() noexcept
{
	iReadIndex = 0;
	iWriteIndex = 0;
	iSize = 0;

	delete [] iPtr;
	iPtr = nullptr;

	VS_SIPError(TSIPErrorCodes::e_null, false);
}

TSIPErrorCodes VS_SIPBuffer::GetAllDataAllocConst(std::unique_ptr<char[]> &aPtr, std::size_t &aSize) const
{
	aSize = GetWriteIndex();
	if ( !aSize )
		return TSIPErrorCodes::e_buffer;

	aPtr = vs::make_unique_default_init<char[]>(aSize + 1);

	memcpy(aPtr.get(), iPtr, aSize);
	aPtr[aSize] = 0;
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPBuffer::GetAllDataAllocConst(std::unique_ptr<char[]> &aPtr) const
{
	const std::size_t sz = GetWriteIndex();
	aPtr = vs::make_unique_default_init<char[]>(sz + 1);

	memcpy(aPtr.get(), iPtr, sz);
	aPtr[sz] = 0;

	return TSIPErrorCodes::e_ok;
}

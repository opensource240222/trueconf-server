/**
**************************************************************************
* \file VS_Container.cpp
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
* \brief Container class implementation
*
* \b Project Standart Libraries
* \author PetrovichevD
* \date 22.11.02
*
* $Revision: 6 $
*
* $History: VS_Container.cpp $
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 14.03.11   Time: 12:10
 * Updated in $/VSNA/std/cpplib
 * - added VS_Container::IsEmpty()
 * - Gateway: WaitProps() after conneted to server, but before try to
 * login (bacuse we need Prop: authentication_method)
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 2.02.11    Time: 0:51
 * Updated in $/VSNA/std/cpplib
 * vcs 3.2:
 * - AddToAB in a PoolThreadTask
 * - AddToAB user with full reallogin in roaming
 * - VS_Container::AttachToCnt()
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 3.09.09    Time: 11:38
 * Updated in $/VSNA/std/cpplib
 * Save abooks at AS:
 *   - VS_Container: GetLongValueRef() added
 *   - TRANSPORT_SRCUSER_PARAM: pass user from Transport to Container
 *
 * *****************  Version 3  *****************
 * User: Dront78      Date: 14.06.09   Time: 10:49
 * Updated in $/VSNA/std/cpplib
 * - VZOchat7 merge. see VZOchat7.h for details
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 29.04.08   Time: 19:07
 * Updated in $/VSNA/std/cpplib
 * copy constructor
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 7.02.07    Time: 12:56
 * Updated in $/VS2005/std/cpplib
 * fixed warning
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 5.02.07    Time: 21:36
 * Updated in $/VS2005/std/cpplib
 * - project configuration
 * - depricated functions warnings suppressed
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 16.08.06   Time: 15:43
 * Updated in $/VS/std/cpplib
 * - AddVAlue speeding up
 *
 * *****************  Version 19  *****************
 * User: Stass        Date: 28.07.06   Time: 16:20
 * Updated in $/VS/std/cpplib
 * added __int64 pass
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 2.06.06    Time: 15:34
 * Updated in $/VS/std/cpplib
 * - added Nhp video and audio buffers
 *
 * *****************  Version 17  *****************
 * User: Stass        Date: 21.12.05   Time: 19:06
 * Updated in $/VS/std/cpplib
 * added get type for current cnt value
 *
 * *****************  Version 16  *****************
 * User: Stass        Date: 26.07.05   Time: 18:56
 * Updated in $/VS/std/cpplib
 * fixed copy constructor
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 19.07.05   Time: 19:41
 * Updated in $/VS/std/cpplib
 * error in constructor fixed
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
****************************************************************************/

/****************************************************************************
* Includes
****************************************************************************/
#include "VS_Container.h"
#include "std-generic/clib/rangecd.h"
#include "std-generic/clib/strcasecmp.h"
#include "std-generic/cpplib/utf8.h"

#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <memory>
#include <string>

#if !defined(__has_feature)
#	define __has_feature(x) 0
#endif

// Copy of SYSTEMTIME from WinAPI, to avoid including Windows.h
struct vs_SYSTEMTIME
{
	uint16_t wYear;
	uint16_t wMonth;
	uint16_t wDayOfWeek;
	uint16_t wDay;
	uint16_t wHour;
	uint16_t wMinute;
	uint16_t wSecond;
	uint16_t wMilliseconds;
};

////////////////////////////////////////////////////////////////////////////////
// Constructors
VS_Container::VS_Container() noexcept
	: m_currKey(nullptr)
	, m_currValue(nullptr)
	, m_value(nullptr)
	, m_valueSize(0)
	, m_MemorySize(0)
	, m_valueCnt(0)
{
}

VS_Container::VS_Container(const VS_Container &cnt)
	: m_currKey(nullptr)
	, m_currValue(nullptr)
	, m_value(nullptr)
	, m_valueSize(0)
	, m_MemorySize(0)
	, m_valueCnt(0)
{
	cnt.CopyTo(*this);
}

VS_Container::VS_Container(VS_Container&& src) noexcept
	: m_currKey(src.m_currKey)
	, m_currValue(src.m_currValue)
	, m_value(src.m_value)
	, m_valueSize(src.m_valueSize)
	, m_MemorySize(src.m_MemorySize)
	, m_valueCnt(src.m_valueCnt)
{
	src.m_currKey = nullptr;
	src.m_currValue = nullptr;
	src.m_value = nullptr;
	src.m_valueSize = 0;
	src.m_MemorySize = 0;
	src.m_valueCnt = 0;
}

VS_Container& VS_Container::operator=(VS_Container&& src) noexcept
{
	if (this == &src)
		return *this;

	m_currKey = src.m_currKey;
	src.m_currKey = nullptr;
	m_currValue = src.m_currValue;
	src.m_currValue = nullptr;
	::free(m_value);
	m_value = src.m_value;
	src.m_value = nullptr;
	m_valueSize = src.m_valueSize;
	src.m_valueSize = 0;
	m_valueCnt = src.m_valueCnt;
	src.m_valueCnt = 0;
	m_MemorySize = src.m_MemorySize;
	src.m_MemorySize = 0;
	return *this;
}

VS_Container& VS_Container::operator= (const VS_Container &cnt)
{
	if (this == &cnt)
		return *this;

	Clear();
	cnt.CopyTo(*this);
	return *this;
}

////////////////////////////////////////////////////////////////////////////////
// Requred
VS_Container::~VS_Container()
{
	Clear();
}

bool VS_Container::IsValid(void) const
{
	return m_valueCnt != 0 && m_value != nullptr;
}

bool VS_Container::CopyTo(VS_Container& cnt) const
{
	if (!IsValid()) return false;
	cnt.Clear();
	if ((cnt.m_value = cnt.GetMemory(m_valueSize)) != nullptr) {
		memcpy(cnt.m_value, m_value, m_valueSize);
		cnt.m_valueSize = m_valueSize;
		cnt.m_valueCnt = m_valueCnt;
		return true;
	}
	else return false;
}

void VS_Container::Clear()
{
	if (m_value != nullptr) {
		free(m_value);
		m_value = nullptr;
		m_valueSize = 0;
		m_valueCnt = 0;
		m_currKey = nullptr;
		m_currValue = nullptr;
		m_MemorySize = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Helpers
inline const void* AddrInc(const void* addr, ptrdiff_t off) { return static_cast<const char*>(addr) + off; }
inline       void* AddrInc(      void* addr, ptrdiff_t off) { return static_cast<      char*>(addr) + off; }

static PascalStr* c_max_key = reinterpret_cast<PascalStr*>(static_cast<intptr_t>(-1));

#define MIN_KEY_VALUE	sizeof(PascalStr)
#define MIN_BOOL_VALUE	sizeof(PascalStr)
#define MIN_INTEGER_VALUE	sizeof(PascalStr)
#define MIN_BOOL_VALUE	sizeof(PascalStr)

inline size_t SizeOfVal(const RawValue* val)
{
	switch (val->type) {
	case VS_CNT_BOOL_VT:
		return (sizeof(uint8_t/*type*/) + sizeof(unsigned char));
	case VS_CNT_INTEGER_VT:
		return (sizeof(uint8_t/*type*/) + sizeof(int32_t));
	case VS_CNT_DOUBLE_VT:
		return (sizeof(uint8_t/*type*/) + sizeof(double));
	case VS_CNT_STRING_VT:
	case VS_CNT_BINARY_VT:
		return (sizeof(uint8_t/*type*/) + sizeof(uint32_t/*size*/) + val->data.sVal.len);
  case VS_CNT_INT64_VT:
		return (sizeof(uint8_t/*type*/) + sizeof(int64_t));
	} // end switch
	return 0;
}

bool VS_Container::AddValue(string_view name, const ContainerVT type, const void* val, const size_t size)
{
	if (name.size() > 254)	// Too long name
		return false;
	size_t keyLen = name.size() + 1 + sizeof(uint8_t/*namelen*/);
	size_t valLen;
	//	valLen = sizeof(unsigned char/*type*/);
	switch (type) {
	case VS_CNT_BOOL_VT:
		valLen = sizeof(uint8_t/*type*/) + sizeof(unsigned char);
		break;
	case VS_CNT_INTEGER_VT:
		valLen = sizeof(uint8_t/*type*/) + sizeof(int32_t);
		break;
	case VS_CNT_DOUBLE_VT:
		valLen = sizeof(uint8_t/*type*/) + sizeof(double);
		break;
	case VS_CNT_STRING_VT:
		valLen = sizeof(uint8_t/*type*/) + sizeof(uint32_t/*size*/) + size + 1/*'\0'*/;
		break;
	case VS_CNT_BINARY_VT:
		valLen = sizeof(uint8_t/*type*/) + sizeof(uint32_t/*size*/) + size;
		break;
	case VS_CNT_INT64_VT:
		valLen = sizeof(uint8_t/*type*/) + sizeof(int64_t);
		break;
	default:	// Unknown type
		return false;
	} // end switch
	//void*	newValue = (m_value == nullptr) ? malloc(keyLen + valLen) : realloc(m_value, m_valueSize + keyLen + valLen);
	void*	newValue = GetMemory(keyLen + valLen);
	if (newValue != nullptr) {
		// Store Key
		auto key = static_cast<PascalStr*>(AddrInc(newValue, m_valueSize));
		key->len = static_cast<uint8_t>(name.size() + 1);
		memcpy(key->szStr, name.data(), name.size());
		key->szStr[name.size()] = '\0';
		// Store Value
		auto value = static_cast<RawValue*>(AddrInc(key, keyLen));
		value->type = type;
		switch (type) {
		case VS_CNT_BOOL_VT:
			value->data.bVal = *static_cast<const bool*>(val) ? 1 : 0;
			break;
		case VS_CNT_INTEGER_VT:
			value->data.i32Val = *static_cast<const int32_t*>(val);
			break;
		case VS_CNT_DOUBLE_VT:
			value->data.dVal = *static_cast<const double*>(val);
			break;
		case VS_CNT_STRING_VT:
			value->data.sVal.len = size + 1;
			memcpy(value->data.sVal.szStr, val, size);
			value->data.sVal.szStr[size] = '\0';
			break;
		case VS_CNT_BINARY_VT:
			value->data.sVal.len = size;
			memcpy(value->data.sVal.szStr, val, size);
			break;
		case VS_CNT_INT64_VT:
			value->data.i64Val= *static_cast<const int64_t*>(val);
			break;
		} // end switch
		m_value = newValue;
		m_valueSize += keyLen + valLen;
		m_valueCnt++;
		return true;
	} // end if
	return false;
}

void* VS_Container::GetMemory(size_t size)
{
	void* newValue = nullptr;
	if (m_value == nullptr) {
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
		// Allocate exactly what is needed and thus force reallocation on each call to AddValue to catch more use-after-free bugs.
		m_MemorySize = size;
#else
		// first memory allocation, allocate at least 1000 bytes
		m_MemorySize = size < 1000 ? 1000 : size;
#endif
		newValue = malloc(m_MemorySize);
	}
	else if (m_valueSize + size > m_MemorySize) {
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
		// Allocate exactly what is needed and thus force reallocation on each call to AddValue to catch more use-after-free bugs.
		m_MemorySize = m_valueSize + size;
#else
		// not enouth internal memoty, allocate at least doubled size
		m_MemorySize = m_valueSize + size + m_MemorySize;
#endif
		newValue = realloc(m_value, m_MemorySize);
	}
	else
		newValue = m_value;
	return newValue;
}


bool VS_Container::GetValue(string_view name, const ContainerVT type, void* val, size_t& size) const
{
	if (m_valueCnt == 0)
		return false;
	auto key = static_cast<const PascalStr*>(m_value);
	for (long i = m_valueCnt; i > 0; i--) {
		auto value = static_cast<const RawValue*>(AddrInc(key, key->len + sizeof(uint8_t/*namelen*/)));
		if (name.size() + 1 == key->len && strncasecmp(name.data(), key->szStr, name.size()) == 0 && type == value->type) { // Found
			switch (type) {
			case VS_CNT_BOOL_VT:
				*static_cast<bool*>(val) = value->data.bVal != 0;
				break;
			case VS_CNT_INTEGER_VT:
				*static_cast<int32_t*>(val) = value->data.i32Val;
				break;
			case VS_CNT_DOUBLE_VT:
				*static_cast<double*>(val) = value->data.dVal;
				break;
			case VS_CNT_STRING_VT:
			case VS_CNT_BINARY_VT:
				if (value->data.sVal.len > size) {
					size = value->data.sVal.len;
					return false;
				}
				size = value->data.sVal.len;
				if (val != nullptr)
					memcpy(val, value->data.sVal.szStr, size);
				break;
			case VS_CNT_INT64_VT:
				*static_cast<int64_t*>(val) = value->data.i64Val;
				break;
			} // end switch
			return true;
		} // end if
		key = static_cast<const PascalStr*>(AddrInc(value, SizeOfVal(value))); // Next Pair
	} // end for
	return false;
}
const void* VS_Container::GetValueRef(string_view name, const ContainerVT type, size_t& size) const
{
	if (m_valueCnt == 0)
		return nullptr;
	auto key = static_cast<const PascalStr*>(m_value);
	for (long i = m_valueCnt; i > 0; i--) {
		auto value = static_cast<const RawValue*>(AddrInc(key, key->len + sizeof(uint8_t/*namelen*/)));
		if (name.size() + 1 == key->len && strncasecmp(name.data(), key->szStr, name.size()) == 0 && type == value->type) { // Found
			switch (type) {
			case VS_CNT_BOOL_VT:
				return &value->data.bVal;
			case VS_CNT_INTEGER_VT:
				return &value->data.i32Val;
			case VS_CNT_DOUBLE_VT:
				return &value->data.dVal;
			case VS_CNT_STRING_VT:
			case VS_CNT_BINARY_VT:
				size = value->data.sVal.len;
				return value->data.sVal.szStr;
			case VS_CNT_INT64_VT:
				return &value->data.i64Val;
			} // end switch
			break;
		} // end if
		key = static_cast<const PascalStr*>(AddrInc(value, SizeOfVal(value))); // Next Pair
	} // end for
	return nullptr;
}


////////////////////////////////////////////////////////////////////////////////
// Sequence access
////////////////////////////////////////////////////////////////////////////////
void VS_Container::Reset() const
{
	m_currKey = nullptr;	// Set to start
}

bool VS_Container::Next() const
{
	if (m_valueCnt == 0)
		return false;
	if (m_currKey == c_max_key)
		return false;
	if (m_currKey == nullptr)
		m_currKey = static_cast<PascalStr*>(m_value);
	else {
		m_currKey = static_cast<PascalStr*>(AddrInc(m_currValue, SizeOfVal(m_currValue)));
		if (m_currKey == AddrInc(m_value, m_valueSize)) {
			m_currKey = c_max_key;	// Set to end
			return false;
		} // end if
	} // end if
	m_currValue = static_cast<RawValue*>(AddrInc(m_currKey, m_currKey->len + sizeof(uint8_t/*namelen*/)));
	return true;
}

bool VS_Container::AttachCurrentTo(VS_Container &cnt) const
{
	if ((m_currKey == nullptr) || (m_currKey == c_max_key))
		return false;
	switch (m_currValue->type) {
		case VS_CNT_BOOL_VT:
			return cnt.AddValue(m_currKey->szStr, m_currValue->data.bVal != 0);
		case VS_CNT_INTEGER_VT:
			return cnt.AddValue(m_currKey->szStr, m_currValue->data.i32Val);
		case VS_CNT_DOUBLE_VT:
			return cnt.AddValue(m_currKey->szStr, m_currValue->data.dVal);
		case VS_CNT_STRING_VT:
			return cnt.AddValue(m_currKey->szStr, m_currValue->data.sVal.szStr);
		case VS_CNT_BINARY_VT:
			return cnt.AddValue(m_currKey->szStr, m_currValue->data.sVal.szStr, m_currValue->data.sVal.len);
		case VS_CNT_INT64_VT:
			return cnt.AddValue(m_currKey->szStr, VS_CNT_INT64_VT, &m_currValue->data.i64Val, 0);
	}
	return false;
}

const char* VS_Container::GetName() const
{
	if ((m_currKey == nullptr) || (m_currKey == c_max_key))
		return nullptr;
	return m_currKey->szStr;
}

ContainerVT VS_Container::GetType() const
{
	if ((m_currKey == nullptr) || (m_currKey == c_max_key))
		return static_cast<ContainerVT>(255);
	return m_currValue->type;
}

bool VS_Container::GetValue(bool &val) const
{
	if ((m_currKey == nullptr) || (m_currKey == c_max_key))
		return false;
	if (m_currValue->type != VS_CNT_BOOL_VT)
		return false;
	val = (m_currValue->data.bVal != 0);
	return true;
}

bool VS_Container::GetValue(int32_t &val) const
{
	if ((m_currKey == nullptr) || (m_currKey == c_max_key))
		return false;
	if (m_currValue->type != VS_CNT_INTEGER_VT)
		return false;
	val = m_currValue->data.i32Val;
	return true;
}

bool VS_Container::GetValue(double &val) const
{
	if ((m_currKey == nullptr) || (m_currKey == c_max_key))
		return false;
	if (m_currValue->type != VS_CNT_DOUBLE_VT)
		return false;
	val = m_currValue->data.dVal;
	return true;
}

bool VS_Container::GetValue(int64_t &val) const
{
	if ((m_currKey == nullptr) || (m_currKey == c_max_key))
		return false;
  if (m_currValue->type != VS_CNT_INT64_VT)
		return false;
	val = m_currValue->data.i64Val;
	return true;
}

bool VS_Container::GetValue(VS_Container &val) const
{
	if ((m_currKey == nullptr) || (m_currKey == c_max_key))
		return false;
	if (m_currValue->type != VS_CNT_BINARY_VT)
		return false;
	return val.Deserialize(m_currValue->data.sVal.szStr, m_currValue->data.sVal.len);
}
bool VS_Container::GetValue(std::string &val) const
{
	auto p = GetStrValueRef();
	if (!p)
		return false;
	val = p;
	return true;
}
bool VS_Container::GetValue(std::chrono::system_clock::time_point &tp) const
{
	size_t sz = 0;
	const auto valid_until_ft = static_cast<const int64_t *>(GetBinValueRef(sz));
	if (valid_until_ft == nullptr || sz != sizeof(int64_t))
	{
		return false;
	}
	if ((*valid_until_ft) == 0)
	{
		tp = std::chrono::system_clock::time_point();
	}
	else
	{
		tp = tu::WindowsTickToUnixSeconds(*valid_until_ft);
	}
	return true;
}
std::string VS_Container::GetStrValue() const
{
	std::string ret;
	GetValue(ret);
	return ret;
}
////////////////////////////////////////////////////////////////////////////////
// Get reference to values
////////////////////////////////////////////////////////////////////////////////
const char* VS_Container::GetStrValueRef() const
{
	if ((m_currKey == nullptr) || (m_currKey == c_max_key))
		return nullptr;
	if (m_currValue->type != VS_CNT_STRING_VT)
		return nullptr;
	return m_currValue->data.sVal.szStr;
}

const void* VS_Container::GetBinValueRef(size_t &size) const
{
	if ((m_currKey == nullptr) || (m_currKey == c_max_key))
		return nullptr;
	if (m_currValue->type != VS_CNT_BINARY_VT)
		return nullptr;
	size = m_currValue->data.sVal.len;
	return m_currValue->data.sVal.szStr;
}

cstring_view VS_Container::GetStrValueView() const
{
	if ((m_currKey == nullptr) || (m_currKey == c_max_key))
		return {};
	if (m_currValue->type != VS_CNT_STRING_VT)
		return {};
	return { null_terminated, m_currValue->data.sVal.szStr, m_currValue->data.sVal.len - 1 };
}

////////////////////////////////////////////////////////////////////////////////
// Add value
bool VS_Container::AddValue(string_view name, std::tm& time)
{
	// Normalize uninitialized fields
	if (std::mktime(&time) == static_cast<std::time_t>(-1))
		return false;

	vs_SYSTEMTIME st;
	st.wYear = static_cast<uint16_t>(time.tm_year + 1900);
	st.wMonth = static_cast<uint16_t>(time.tm_mon + 1);
	st.wDayOfWeek = static_cast<uint16_t>(time.tm_wday);
	st.wDay = static_cast<uint16_t>(time.tm_mday);
	st.wHour = static_cast<uint16_t>(time.tm_hour);
	st.wMinute = static_cast<uint16_t>(time.tm_min);
	st.wSecond = static_cast<uint16_t>(time.tm_sec);
	st.wMilliseconds = 0;

	return AddValue(name, VS_CNT_BINARY_VT, &st, sizeof(st));
}

bool VS_Container::AddValue(string_view name, const VS_Container &val)
{
	size_t size = 0;
	val.Serialize(nullptr, size);
	std::unique_ptr<char[]> data(new char[size]);
	if (!val.Serialize(data.get(), size))
		return false;
	return AddValue(name, VS_CNT_BINARY_VT, data.get(), size);
}

bool VS_Container::DeleteValue(string_view name)
{
	if (m_valueCnt == 0)
		return false;
	auto key = static_cast<PascalStr*>(m_value);
	for (auto i = m_valueCnt; i > 0; i--) {
		auto value = static_cast<RawValue*>(AddrInc(key, key->len + sizeof(uint8_t/*namelen*/)));
		auto next_key = static_cast<PascalStr*>(AddrInc(value, SizeOfVal(value)));
		if (name.size() + 1 == key->len && strncasecmp(name.data(), key->szStr, name.size()) == 0) { // Found
			memmove(key, next_key, m_valueSize - (reinterpret_cast<char*>(next_key) - static_cast<char*>(m_value)));
			m_valueCnt--;
			m_valueSize -= reinterpret_cast<char*>(next_key) - reinterpret_cast<char*>(key);
			return true;
		}
		key = next_key;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// Get value
bool VS_Container::GetValue(string_view name, std::tm& time) const
{
	size_t size;
	auto data = GetValueRef(name, VS_CNT_BINARY_VT, size);
	if (!data || size != sizeof(vs_SYSTEMTIME))
		return false;

	const auto st = static_cast<const vs_SYSTEMTIME*>(data);
	time.tm_isdst = -1; // mktime() will compute whether this is during Standard or Daylight time.
	time.tm_sec = static_cast<int>(st->wSecond);
	time.tm_min = static_cast<int>(st->wMinute);
	time.tm_hour = static_cast<int>(st->wHour);
	time.tm_mday = static_cast<int>(st->wDay);
	time.tm_mon = static_cast<int>(st->wMonth - 1);
	time.tm_year = static_cast<int>(st->wYear - 1900);

	// Normalize uninitialized fields
	if (std::mktime(&time) == static_cast<std::time_t>(-1))
		return false;

	return true;
}

bool VS_Container::GetValue(string_view name, VS_Container& val) const
{
	size_t size;
	auto data = GetValueRef(name, VS_CNT_BINARY_VT, size);
	if (!data)
		return false;
	return val.Deserialize(data, size);
}
bool VS_Container::GetValue(string_view name, std::string& val) const
{
	auto p = GetStrValueRef(name);
	if (!p)
		return false;
	val = p;
	return true;
}
////////////////////////////////////////////////////////////////////////////////
// Serialization/deserialization
bool VS_Container::Serialize(void* buff, size_t &size) const
{
	if (buff == nullptr)
	{
		size = m_valueSize * 2 + sizeof(uint32_t);
		return true;
	} // end if
	if (size < (m_valueSize * 2 + sizeof(uint32_t)))
		return false;
	*static_cast<uint32_t*>(buff) = m_valueSize;
	size = sizeof(uint32_t);
	if (m_valueSize != 0)	// Check empty case
		size += RCDV_Encode(m_value, m_valueSize, AddrInc(buff, sizeof(uint32_t)));
	return true;
}

bool VS_Container::SerializeAlloc(void* &buff, size_t &size) const
{
	size = m_valueSize * 2 + sizeof(uint32_t);
	buff = malloc(size);
	if (buff == nullptr)
		return false;
	*static_cast<uint32_t*>(buff) = m_valueSize;
	size = sizeof(uint32_t);
	if (m_valueSize != 0)	// Check empty case
		size += RCDV_Encode(m_value, m_valueSize, AddrInc(buff, sizeof(uint32_t)));
	return true;
}

bool VS_Container::Deserialize(const void* buff, const size_t size)
{
	if (buff == nullptr || size < sizeof(uint32_t))
		return false;
	size_t newValueSize = *static_cast<const uint32_t*>(buff);
	unsigned long newValueCnt = 0;
	// Check empty case
	if (newValueSize == 0) {
		Clear();
		return true;
	}
	// Normal
	void* newValue = malloc(newValueSize);
	if (newValue != nullptr)
	{
		size_t vsize = size - sizeof(uint32_t);
		if (RCDV_DecodeSafe(AddrInc(buff, sizeof(uint32_t)), newValue, newValueSize,vsize) == vsize )
		{
			// Verify Content
			bool verified = false;
			auto key = static_cast<const PascalStr*>(newValue);
			auto end = AddrInc(newValue, newValueSize);
			while (key < end)
			{
				// Check Key length
				verified = AddrInc(key, sizeof(PascalStr)) <= end;
				if (!verified)
					break;
				// Check Value pointer
				auto value = static_cast<const RawValue*>(AddrInc(key, key->len + sizeof(uint8_t/*namelen*/)));
				verified = (value < end);
				if (!verified)
					break;
				// Check Key value
				verified = (key->szStr[key->len - 1] == '\0');
				if (!verified)
					break;
				// Check Value length
				if (value->type == VS_CNT_BINARY_VT || value->type == VS_CNT_STRING_VT)
				{
					verified = AddrInc(value, sizeof(uint8_t/*type*/) + sizeof(uint32_t/*size*/)) <= end;
					if (!verified)
						break;
				}
				key = static_cast<const PascalStr*>(AddrInc(value, SizeOfVal(value))); // Next Pair
				verified = (key <= end);
				if (!verified)
					break;
				// Check Value content
				if (value->type == VS_CNT_STRING_VT) {
					verified = value->data.sVal.szStr[static_cast<ptrdiff_t>(value->data.sVal.len) - 1] == '\0';
					if (!verified)
						break;
				} // end if
				newValueCnt++;
			} // end while
			if (verified) {
				free(m_value);
				m_value = newValue;
				m_valueSize = newValueSize;
				m_valueCnt = newValueCnt;
				m_MemorySize = newValueSize;
				return true;
			} // end if
		} // end if
		free(newValue);
	} // end if
	return false;
}
uint32_t VS_Container::GetAllocSize() const {
	return m_valueSize * 2 + sizeof(uint32_t);
}
//by stass: special method for sending wchar as unicode
#if defined(_WIN32) // Not ported yet
bool VS_Container::AddValue(string_view name, const wchar_t* val)
{
	if (val == nullptr)
		return false;

	if (name.size() > 254)	// Too long name
		return false;

	const auto val_utf8 = vs::UTF16toUTF8Convert(val);
	if (val_utf8.empty() && val[0])
		return false;

	size_t keyLen = name.size() + 1 + sizeof(uint8_t/*namelen*/);
	size_t valLen = sizeof(uint8_t/*type*/) + sizeof(uint32_t/*size*/) + val_utf8.size() + 1/*'\0'*/;

	//void*	newValue = (m_value == nullptr) ? malloc(keyLen + valLen) : realloc(m_value, m_valueSize + keyLen + valLen);
	void*	newValue = GetMemory(keyLen + valLen);

	if (newValue == nullptr)
		return false;

	// Store Key
	auto key = static_cast<PascalStr*>(AddrInc(newValue, m_valueSize));
	key->len = static_cast<uint8_t>(name.size() + 1);
	memcpy(key->szStr, name.data(), name.size());
	key->szStr[name.size()] = '\0';

	// Store Value
	auto value = static_cast<RawValue*>(AddrInc(key, keyLen));
	value->type = VS_CNT_STRING_VT;
	value->data.sVal.len = static_cast<uint32_t>(val_utf8.size() + 1);
	memcpy(value->data.sVal.szStr, val_utf8.data(), val_utf8.size() + 1);

	m_value = newValue;
	m_valueSize += keyLen + valLen;
	m_valueCnt++;
	return true;
}
#endif

void VS_Container::AttachToCnt(VS_Container& cnt) const
{
	if (m_valueCnt == 0) return;

	auto key = static_cast<const PascalStr*>(m_value);
	for (long i = m_valueCnt; i > 0; i--) {
		auto value = static_cast<const RawValue*>(AddrInc(key, key->len + sizeof(uint8_t/*namelen*/)));
		switch (value->type) {
		case VS_CNT_BOOL_VT:
			cnt.AddValue(key->szStr, value->data.bVal==1);
			break;
		case VS_CNT_INTEGER_VT:
			cnt.AddValue(key->szStr, value->data.i32Val);
			break;
		case VS_CNT_DOUBLE_VT:
			cnt.AddValue(key->szStr, value->data.dVal);
			break;
		case VS_CNT_STRING_VT:
			cnt.AddValue(key->szStr, value->data.sVal.szStr);
			break;
		case VS_CNT_BINARY_VT:
			cnt.AddValue(key->szStr, value->data.sVal.szStr, value->data.sVal.len);
			break;
		case VS_CNT_INT64_VT:
			cnt.AddValue(key->szStr, VS_CNT_INT64_VT, &value->data.i64Val, 0);
			break;
		} // end switch
		key = static_cast<const PascalStr*>(AddrInc(value, SizeOfVal(value))); // Next Pair
	} // end for
}

bool VS_Container::IsEmpty() const
{
	return m_valueCnt==0;
}
/****************************************************************************/
VS_BinBuff::VS_BinBuff() noexcept
	: m_buff(nullptr)
	, m_size(0)
{
}

VS_BinBuff::VS_BinBuff(const void* buff, size_t size)
	: m_buff(nullptr)
	, m_size(0)
{
	Set(buff, size);
}

VS_BinBuff::VS_BinBuff(const VS_BinBuff &src)
	: m_buff(nullptr)
	, m_size(0)
{
	Set(src.m_buff, src.m_size);
}

VS_BinBuff::VS_BinBuff(VS_BinBuff&& src) noexcept
	: m_buff(src.m_buff)
	, m_size(src.m_size)
{
	src.m_buff = nullptr;
	src.m_size = 0;
}

VS_BinBuff::~VS_BinBuff()
{
	::free(m_buff);
}

void VS_BinBuff::Empty()
{
	::free(m_buff);
	m_buff = nullptr;
	m_size = 0;
}

void VS_BinBuff::Set(const void* buff, size_t size)
{
	if (!buff)
		size = 0;
	SetSize(size);
	if (size > 0)
		::memcpy(m_buff, buff, m_size);
}

void VS_BinBuff::Set(const VS_Container & cnt)
{
	Empty();
	cnt.SerializeAlloc(m_buff, m_size);
}

void VS_BinBuff::SetSize(size_t size)
{
	if (m_size == size)
		return;
	::free(m_buff);
	m_buff = size > 0 ? ::malloc(size) : nullptr;
	m_size = size;
}

VS_BinBuff &VS_BinBuff::operator=(const VS_BinBuff &src)
{
	if (this == &src)
		return *this;

	Set(src.m_buff, src.m_size);
	return *this;
}

VS_BinBuff& VS_BinBuff::operator=(VS_BinBuff&& src) noexcept
{
	if (this == &src)
		return *this;

	::free(m_buff);
	m_buff = src.m_buff; src.m_buff = nullptr;
	m_size = src.m_size; src.m_size = 0;
	return *this;
}

bool operator==(const VS_BinBuff& l, const VS_BinBuff& r)
{
	return l.m_size == r.m_size && 0 == memcmp(l.m_buff, r.m_buff, l.m_size);
}

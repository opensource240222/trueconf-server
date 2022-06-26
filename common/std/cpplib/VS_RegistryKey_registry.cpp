#if defined(_WIN32)

#include "VS_RegistryKey_registry.h"
#include "VS_RegistryKey_internal.h"
#include "std-generic/cpplib/utf8.h"
#include "std-generic/cpplib/deleters.h"

#include <Windows.h>
#include <Shlwapi.h>

#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <limits>
#include <memory>

#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Shlwapi.lib")

static_assert(sizeof(wchar_t) == sizeof(char16_t), "wchar_t != char16_t. This shouldn't happen on Windows.");

namespace regkey {

static const char softwareKey[] = "Software";
static const char keySeparator	= '\\';
static const char16_t keySeparator_utf16 = L'\\';

RegistryKey::RegistryKey(string_view root, bool user, string_view name, bool read_only, bool create)
	: m_ValuesIdx(0)
	, m_keyIdx(0)
	, m_user(user)
	, m_keyReadOnly(read_only)
	, m_keyReset(false)
{
	if (!IsKeyNameValid(root) || !IsKeyNameValid(name))
	{
		m_key = nullptr;
		return;
	}

	size_t sz = sizeof(softwareKey)/sizeof(softwareKey[0])
		+ (root.empty() ? 0 : (1/*keySeparator*/ + root.size()))
		+ (name.empty() ? 0 : (1/*keySeparator*/ + name.size()));
	m_keyName.reserve(sz);
	m_keyName += softwareKey;
	if (!root.empty())
	{
		m_keyName += keySeparator;
		m_keyName += root;
	}
	if (!name.empty())
	{
		m_keyName += keySeparator;
		m_keyName += name;
	}
	m_keyNameW = vs::UTF8toUTF16Convert(m_keyName);
	if (m_keyName[0] && m_keyNameW.empty())
	{
		m_key = nullptr;
		return;
	}
	OpenKey(create);
}

RegistryKey::RegistryKey(const RegistryKey& rk)
	: m_key(0)
	, m_ValuesIdx(0)
	, m_keyIdx(0)
	, m_user(false)
	, m_keyReadOnly(false)
	, m_keyReset(false)
{
	if (rk.m_key != 0) {
		m_user = rk.m_user;
		m_keyReadOnly = rk.m_keyReadOnly;
		m_keyName = rk.m_keyName;
		m_keyNameW = rk.m_keyNameW;
		OpenKey(false);
	} // end if
}

key_ptr RegistryKey::Clone() const
{
	return std::make_unique<RegistryKey>(*this);
}

RegistryKey::RegistryKey(std::u16string&& keyNameW, bool user, bool read_only)
	: m_keyNameW(std::move(keyNameW))
	, m_ValuesIdx(0)
	, m_keyIdx(0)
	, m_user(user)
	, m_keyReadOnly(read_only)
	, m_keyReset(false)
{
	m_keyName = vs::UTF16toUTF8Convert(m_keyNameW);
	if (m_keyNameW[0] && m_keyName.empty())
	{
		m_key = nullptr;
		return;
	}
	OpenKey(false);
}

RegistryKey::~RegistryKey()
{
	if (m_key != 0) RegCloseKey((HKEY)m_key);
	m_key = 0;
}

void RegistryKey::OpenKey(bool create)
{
	HKEY key;
	auto result = RegOpenKeyExW(m_user ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE, reinterpret_cast<const wchar_t*>(m_keyNameW.c_str()), 0, (m_keyReadOnly ? KEY_READ : (KEY_READ | KEY_WRITE)) | KEY_WOW64_32KEY, &key);
	if (result == ERROR_FILE_NOT_FOUND && !m_keyReadOnly && create)
		result = RegCreateKeyExW(m_user ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE, reinterpret_cast<const wchar_t*>(m_keyNameW.c_str()), 0, nullptr, REG_OPTION_NON_VOLATILE, (KEY_READ | KEY_WRITE) | KEY_WOW64_32KEY, nullptr, &key, nullptr);
	m_key = result == ERROR_SUCCESS ? key : nullptr;
}


////////////////////////////////////////////////////////////////////////////////
// Key Enumerator
void RegistryKey::ResetKey()
{
	m_keyIdx = 0;
	m_keyReset = true;
}

key_ptr RegistryKey::NextKey(bool readOnly)
{
	if (m_key == 0)
		return nullptr;
	if ((!m_keyReset) && (m_keyIdx == 0))
		return nullptr;
    FILETIME lastWriteTime;
	wchar_t keyName[MAX_PATH];
	DWORD keyNameSize = sizeof(keyName)/sizeof(keyName[0]);
	auto retCode = RegEnumKeyExW((HKEY)m_key, m_keyIdx, keyName, &keyNameSize, 0, 0, 0, &lastWriteTime);
	if (retCode != ERROR_SUCCESS)
		return nullptr;

	std::u16string newKeyName;
	newKeyName.reserve(m_keyNameW.size() + sizeof(keySeparator_utf16) + keyNameSize);
	newKeyName += m_keyNameW;
	newKeyName += keySeparator_utf16;
	newKeyName += reinterpret_cast<char16_t*>(keyName);
	key_ptr key = key_ptr(new RegistryKey(std::move(newKeyName), m_user, readOnly));
	if (!key->IsValid())
		return nullptr;

	m_keyReset = false;
	m_keyIdx++;
	return key;
}

bool RegistryKey::RemoveKey(string_view name)
{
	if (m_keyReadOnly)
		return false;

	auto name_utf16 = vs::UTF8toUTF16Convert(name);
	if (!name.empty() && name_utf16.empty())
		return false;

	DWORD retCode = SHDeleteKeyW((HKEY)m_key, reinterpret_cast<const wchar_t*>(name_utf16.c_str()));
	return retCode == ERROR_SUCCESS || retCode == ERROR_FILE_NOT_FOUND;
}

bool RegistryKey::RenameKey(string_view from, string_view to)
{
	if (m_keyReadOnly)
		return false;
	switch (IsSubKey(from, to))
	{
	case 1: case 2:
		return false; // Moving to a or from a subkey, SHCopyKey can't handle moving to subkey, and moving from subkey fails because we delete old key first.
	case 3:
		return true; // Moving to itself, nothing to do.
	}

	auto from_utf16 = vs::UTF8toUTF16Convert(from);
	if (!from.empty() && from_utf16.empty())
		return false;
	auto to_utf16 = vs::UTF8toUTF16Convert(to);
	if (!to.empty() && to_utf16.empty())
		return false;

	HKEY	key;
	DWORD	err;
	// Remove destination key to avoid merging contents of the keys
	err = SHDeleteKeyW((HKEY)m_key, reinterpret_cast<const wchar_t*>(to_utf16.c_str()));
	if (err != ERROR_SUCCESS && err != ERROR_FILE_NOT_FOUND)
		return false;
	err = RegCreateKeyExW((HKEY)m_key, reinterpret_cast<const wchar_t*>(to_utf16.c_str()), 0, nullptr, REG_OPTION_NON_VOLATILE, (KEY_READ | KEY_WRITE), nullptr, &key, nullptr);
	if (err != ERROR_SUCCESS)
		return false;
	// Copy contents of old key to new key
	err = SHCopyKeyW((HKEY)m_key, reinterpret_cast<const wchar_t*>(from_utf16.c_str()), key, 0);
	if (err != ERROR_SUCCESS)
		return false;
	// Remove old key
	err = SHDeleteKeyW((HKEY)m_key, reinterpret_cast<const wchar_t*>(from_utf16.c_str()));
	if (err != ERROR_SUCCESS)
		return false;
	return true;
}

bool RegistryKey::CopyKey(string_view from, string_view to)
{
	if (m_keyReadOnly)
		return false;
	switch (IsSubKey(from, to))
	{
	case 1: case 2:
		return false; // Moving to a or from a subkey, SHCopyKey can't handle moving to subkey, and moving from subkey disabled for consistency with RenameKey.
	case 3:
		return true; // Moving to itself, nothing to do.
	}

	auto from_utf16 = vs::UTF8toUTF16Convert(from);
	if (!from.empty() && from_utf16.empty())
		return false;
	auto to_utf16 = vs::UTF8toUTF16Convert(to);
	if (!to.empty() && to_utf16.empty())
		return false;

	HKEY	key;
	DWORD	err;
	err = RegCreateKeyExW((HKEY)m_key, reinterpret_cast<const wchar_t*>(to_utf16.c_str()), 0, nullptr, REG_OPTION_NON_VOLATILE, (KEY_READ | KEY_WRITE), nullptr, &key, nullptr);
	if (err != ERROR_SUCCESS)
		return false;
	// Copy contents of old key to new key
	err = SHCopyKeyW((HKEY)m_key, reinterpret_cast<const wchar_t*>(from_utf16.c_str()), key, 0);
	if (err != ERROR_SUCCESS)
		return false;
	return true;
}

bool RegistryKey::HasValue(string_view name) const
{
	if (m_key == 0)
		return false;

	auto name_utf16 = vs::UTF8toUTF16Convert(name);
	if (!name.empty() && name_utf16.empty())
		return false;
	DWORD regSize = 0;
	auto result = RegQueryValueExW((HKEY)m_key, reinterpret_cast<const wchar_t*>(name_utf16.c_str()), 0, nullptr, nullptr, &regSize);
	if (result != ERROR_SUCCESS && result != ERROR_MORE_DATA)
		return false;
	if (regSize > INT_MAX)
		return false;
	return true;
}

bool RegistryKey::RemoveValue(string_view name)
{
	if (m_key == 0)
		return false;

	auto name_utf16 = vs::UTF8toUTF16Convert(name);
	if (!name.empty() && name_utf16.empty())
		return false;
	auto result = RegDeleteValueW((HKEY)m_key, reinterpret_cast<const wchar_t*>(name_utf16.c_str()));
	return result == ERROR_SUCCESS;
}

/**
 **************************************************************************
 * \brief Conversion table
 *
 *|-------------------------------------+---------------+---------------+---------------+---------------+---------------+
 *|                                     | VS_INTEGER_VT |  VS_INT64_VT  | VS_STRING_VT  | VS_WSTRING_VT | VS_BINARY_VT  |
 *|-------------------------------------+---------------+---------------+---------------+---------------+---------------+
 *|REG_SZ                               |   wcstol()    |   wcstol()    |   u16to8()    |   memcpy()    |   memcpy()    |
 *|REG_EXPAND_SZ                        |   -           |   -           |   u16to8()    |   memcpy()    |   memcpy()    |
 *|REG_BINARY                           |   memcpy()    |   memcpy()    |   -           |   -           |   memcpy()    |
 *|REG_DWORD                            |   memcpy()    |   memcpy()    |   ltoa()      |   ?           |   memcpy()    |
 *|REG_DWORD_LITTLE_ENDIAN              |   -           |   -           |   -           |   -           |   -           |
 *|REG_DWORD_BIG_ENDIAN                 |   -           |   -           |   -           |   -           |   -           |
 *|REG_LINK                             |   -           |   -           |   -           |   -           |   -           |
 *|REG_MULTI_SZ                         |   wcstol()    |   wcstol()    |   u16to8()    |   memcpy()    |   memcpy()    |
 *|REG_RESOURCE_LIST                    |   -           |   -           |   u16to8()    |   memcpy()    |   memcpy()    |
 *|REG_FULL_RESOURCE_DESCRIPTOR         |   -           |   -           |   -           |   -           |   -           |
 *|REG_RESOURCE_REQUIREMENTS_LIST       |   -           |   -           |   -           |   -           |   -           |
 *|REG_QWORD, REG_QWORD_LITTLE_ENDIAN   |   -           |   -           |   -           |   -           |   -           |
 *|-------------------------------------+---------------+---------------+---------------+---------------+---------------+
 ****************************************************************************/
static const char TRANSL_TBL[REG_QWORD_LITTLE_ENDIAN + 1][VS_REG_BINARY_VT + 1] =
{
	{  0,     0,     0,     0,     0},
	{'s',   's',   'u',   'm',   'm'},
	{  0,     0,   'u',   'm',   'm'},
	{'m',   'm',     0,     0,   'm'},
	{'m',   'm',   'l',     0,   'm'},
	{  0,     0,     0,     0,     0},
	{  0,     0,     0,     0,     0},
	{'s',   's',   'u',   'm',   'm'},
	{  0,     0,   'u',   'm',   'm'},
	{  0,     0,     0,     0,     0},
	{  0,     0,     0,     0,     0},
	{  0,   'm',     0,     0,     0}
};

bool GetBestType_Registry(DWORD regType, RegistryVT& type)
{
	switch (regType)
	{
	case REG_SZ:
	case REG_EXPAND_SZ:
	case REG_MULTI_SZ:
	case REG_RESOURCE_LIST:
		type = VS_REG_STRING_VT;
		return true;
	case REG_BINARY:
		type = VS_REG_BINARY_VT;
		return true;
	case REG_DWORD:
		type = VS_REG_INTEGER_VT;
		return true;
	case REG_QWORD:
		type = VS_REG_INT64_VT;
		return true;
	}
	return false;
}

// Parses resistry data of type 'regType' to type 'type' and stores it into 'buffer',
// Data is loaded from registry by LoadFunctor.
// LoadFunctor should have signature compatible to: bool (void* buffer, size_t size)
// 'size' specifies maximum size of the result that can be stored into 'buffer'.
// If size of the result is greater that 'size' returns -(size of the result).
// If buffer isn't provided (buffer == null) it will be allocated via malloc.
// If parsing fails returns 0, othewise returns size of the result.
template <class LoadFunctor>
int32_t ParseValue_Registry(void*& buffer, size_t size, RegistryVT type, DWORD regType, DWORD regSize, LoadFunctor&& load_value)
{
	switch (TRANSL_TBL[regType][type]) {
	case 'l':	// ltoa
	{
		int32_t value;
		if (!load_value(&value, sizeof(int32_t)))
			return 0;
		char value_str[1/*sign*/ + (std::numeric_limits<int32_t>::digits10 + 1)/*digits*/ + 1/*'\0'*/] = {};
		_ltoa(value, value_str, 10);
		const size_t result_size = std::strlen(value_str) + 1;
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		std::memcpy(buffer, value_str, result_size);
		return result_size;
	}
	case 's':	// wcstol
	{
		const size_t result_size = sizeof(int32_t);
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		auto value = std::make_unique<wchar_t[]>(regSize / sizeof(wchar_t));
		if (!load_value(value.get(), regSize))
			return 0;
		int32_t value_int;
		if (!ParseToInteger(value.get(), value_int))
			return 0;
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		*static_cast<int32_t*>(buffer) = value_int;
		return result_size;
	}
	case 'm':	// memcpy
	{
		const size_t result_size = regSize;
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
		{
			std::unique_ptr<void, free_deleter> buffer_holder(::malloc(result_size));
			if (!load_value(buffer_holder.get(), result_size))
				return 0;
			buffer = buffer_holder.release();
		}
		else
		{
			if (!load_value(buffer, result_size))
				return 0;
		}
		return result_size;
	}
	case 'u':	// UTF-16 -> UTF-8
	{
		auto value = std::make_unique<wchar_t[]>(regSize / sizeof(wchar_t));
		if (!load_value(value.get(), regSize))
			return 0;
		const auto value_utf8 = vs::UTF16toUTF8Convert(value.get());
		if (value[0] && value_utf8.empty())
			return 0;
		const size_t result_size = value_utf8.size() + 1;
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		std::memcpy(buffer, value_utf8.data(), result_size);
		return result_size;
	}
	} // end switch
	return 0;	// No possible conversion exists.
}

template <class LoadFunctor>
bool ParseValue_Registry(std::string& result, DWORD regType, DWORD regSize, LoadFunctor&& load_value)
{
	switch (TRANSL_TBL[regType][VS_REG_STRING_VT]) {
	case 'l': // ltoa
	{
		int32_t value;
		if (!load_value(&value, sizeof(int32_t)))
			return false;
		char value_str[1/*sign*/ + (std::numeric_limits<int32_t>::digits10 + 1)/*digits*/ + 1/*'\0'*/] = {};
		_ltoa(value, value_str, 10);
		result = value_str;
		return true;
	}
	case 'u': // UTF-16 -> UTF-8
	{
		auto value = std::make_unique<wchar_t[]>(regSize / sizeof(wchar_t));
		if (!load_value(value.get(), regSize))
			return false;
		auto value_utf8 = vs::UTF16toUTF8Convert(value.get());
		if (value[0] && value_utf8.empty())
			return false;
		result = std::move(value_utf8);
		return true;
	}
	}
	return false;
}

int32_t RegistryKey::GetValue(void* buff, size_t size, RegistryVT type, const char* name) const
{
	assert(!(buff == nullptr && size != 0)); // This combination doesn't make sense, also we use it internally in allocating GetValue.

	if (m_key == 0)
		return 0;

	auto name_utf16 = vs::UTF8toUTF16Convert(name);
	if (name[0] && name_utf16.empty())
		return 0;

	DWORD regType;
	DWORD regSize = 0;
	auto result = RegQueryValueExW((HKEY)m_key, reinterpret_cast<const wchar_t*>(name_utf16.c_str()), 0, &regType, nullptr, &regSize);
	if (result != ERROR_SUCCESS && result != ERROR_MORE_DATA)
		return 0;
	if (regSize > INT_MAX)
		return 0;

	return ParseValue_Registry(buff, size, type, regType, regSize, [&](void* buffer, size_t size) {
		DWORD regSize = size;
		return RegQueryValueExW((HKEY)m_key, reinterpret_cast<const wchar_t*>(name_utf16.c_str()), 0, nullptr, static_cast<BYTE*>(buffer), &regSize) == ERROR_SUCCESS;
	});
}

int32_t RegistryKey::GetValue(std::unique_ptr<void, free_deleter>& buff, RegistryVT type, const char* name) const
{
	if (m_key == 0)
		return 0;

	auto name_utf16 = vs::UTF8toUTF16Convert(name);
	if (name[0] && name_utf16.empty())
		return 0;

	DWORD regType;
	DWORD regSize = 0;
	auto result = RegQueryValueExW((HKEY)m_key, reinterpret_cast<const wchar_t*>(name_utf16.c_str()), 0, &regType, nullptr, &regSize);
	if (result != ERROR_SUCCESS && result != ERROR_MORE_DATA)
		return 0;
	if (regSize > INT_MAX)
		return 0;

	void* p_data = nullptr;
	const auto res = ParseValue_Registry(p_data, std::numeric_limits<int32_t>::max(), type, regType, regSize, [&](void* buffer, size_t size) {
		DWORD regSize = size;
		return RegQueryValueExW((HKEY)m_key, reinterpret_cast<const wchar_t*>(name_utf16.c_str()), 0, nullptr, static_cast<BYTE*>(buffer), &regSize) == ERROR_SUCCESS;
	});
	if (p_data)
		buff.reset(p_data);
	return res;
}

int32_t RegistryKey::GetValueAndType(void* buff, size_t size, RegistryVT& type, const char* name) const
{
	assert(!(buff == nullptr && size != 0)); // This combination doesn't make sense, also we use it internally in allocating GetValue.

	if (m_key == 0)
		return 0;

	auto name_utf16 = vs::UTF8toUTF16Convert(name);
	if (name[0] && name_utf16.empty())
		return 0;

	DWORD regType;
	DWORD regSize = 0;
	auto result = RegQueryValueExW((HKEY)m_key, reinterpret_cast<const wchar_t*>(name_utf16.c_str()), 0, &regType, nullptr, &regSize);
	if (result != ERROR_SUCCESS && result != ERROR_MORE_DATA)
		return 0;
	if (regSize > INT_MAX)
		return 0;
	if (!GetBestType_Registry(regType, type))
		return 0;

	return ParseValue_Registry(buff, size, type, regType, regSize, [&](void* buffer, size_t size) {
		DWORD regSize = size;
		return RegQueryValueExW((HKEY)m_key, reinterpret_cast<const wchar_t*>(name_utf16.c_str()), 0, nullptr, static_cast<BYTE*>(buffer), &regSize) == ERROR_SUCCESS;
	});
}

int32_t RegistryKey::GetValueAndType(std::unique_ptr<void, free_deleter>& buff, RegistryVT& type, const char* name) const
{
	if (m_key == 0)
		return 0;

	auto name_utf16 = vs::UTF8toUTF16Convert(name);
	if (name[0] && name_utf16.empty())
		return 0;

	DWORD regType;
	DWORD regSize = 0;
	auto result = RegQueryValueExW((HKEY)m_key, reinterpret_cast<const wchar_t*>(name_utf16.c_str()), 0, &regType, nullptr, &regSize);
	if (result != ERROR_SUCCESS && result != ERROR_MORE_DATA)
		return 0;
	if (regSize > INT_MAX)
		return 0;
	if (!GetBestType_Registry(regType, type))
		return 0;

	void* p_data = nullptr;
	const auto res = ParseValue_Registry(p_data, std::numeric_limits<int32_t>::max(), type, regType, regSize, [&](void* buffer, size_t size) {
		DWORD regSize = size;
		return RegQueryValueExW((HKEY)m_key, reinterpret_cast<const wchar_t*>(name_utf16.c_str()), 0, nullptr, static_cast<BYTE*>(buffer), &regSize) == ERROR_SUCCESS;
	});
	if (p_data)
		buff.reset(p_data);
	return res;
}

bool RegistryKey::GetString(std::string& data, const char* name) const
{
	if (m_key == 0)
		return false;

	auto name_utf16 = vs::UTF8toUTF16Convert(name);
	if (name[0] && name_utf16.empty())
		return false;

	DWORD regType;
	DWORD regSize = 0;
	auto result = RegQueryValueExW((HKEY)m_key, reinterpret_cast<const wchar_t*>(name_utf16.c_str()), 0, &regType, nullptr, &regSize);
	if (result != ERROR_SUCCESS && result != ERROR_MORE_DATA)
		return false;
	if (regSize > INT_MAX)
		return false;

	return ParseValue_Registry(data, regType, regSize, [&](void* buffer, size_t size) {
		DWORD regSize = size;
		return RegQueryValueExW((HKEY)m_key, reinterpret_cast<const wchar_t*>(name_utf16.c_str()), 0, nullptr, static_cast<BYTE*>(buffer), &regSize) == ERROR_SUCCESS;
	});
}

unsigned RegistryKey::GetValueCount() const
{
	if (m_key == 0)
		return 0;

	DWORD values;
	if (RegQueryInfoKeyW((HKEY)m_key, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &values, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
		return 0;
	return values;
}

unsigned RegistryKey::GetKeyCount() const
{
	if (m_key == 0)
		return 0;

	DWORD sub_keys;
	if (RegQueryInfoKeyW((HKEY)m_key, nullptr, nullptr, nullptr, &sub_keys, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
		return 0;
	return sub_keys;
}

void RegistryKey::ResetValues()
{
	m_ValuesIdx = 0;
}

int32_t RegistryKey::NextValue(std::unique_ptr<void, free_deleter>& buff, RegistryVT type, std::string& name)
{
	wchar_t name_utf16[1024];
	DWORD name_size = sizeof(name_utf16) / sizeof(name_utf16[0]);
	DWORD regType;
	DWORD regSize = 0;
	auto result = RegEnumValueW((HKEY)m_key, m_ValuesIdx, name_utf16, &name_size, 0, &regType, 0, &regSize);
	if (result != ERROR_SUCCESS && result != ERROR_MORE_DATA)
		return 0;
	if (name_size >= sizeof(name_utf16) / sizeof(name_utf16[0]))
		return 0;
	if (regSize > INT_MAX)
		return 0;

	void* p_data = nullptr;
	const auto res = ParseValue_Registry(p_data, std::numeric_limits<int32_t>::max(), type, regType, regSize, [&](void* buffer, size_t size) {
		DWORD regSize = size;
		return RegQueryValueExW((HKEY)m_key, name_utf16, 0, nullptr, static_cast<BYTE*>(buffer), &regSize) == ERROR_SUCCESS;
	});
	if (p_data)
		buff.reset(p_data);
	if (res > 0)
	{
		name = vs::UTF16toUTF8Convert(name_utf16);
		if (name_utf16[0] && name.empty())
			return 0;
	}
	m_ValuesIdx++;
	return res;
}

int32_t RegistryKey::NextValueAndType(std::unique_ptr<void, free_deleter>& buff, RegistryVT& type, std::string& name)
{
	wchar_t name_utf16[1024];
	DWORD name_size = sizeof(name_utf16) / sizeof(name_utf16[0]);
	DWORD regType;
	DWORD regSize = 0;
	auto result = RegEnumValueW((HKEY)m_key, m_ValuesIdx, name_utf16, &name_size, 0, &regType, 0, &regSize);
	if (result != ERROR_SUCCESS && result != ERROR_MORE_DATA)
		return 0;
	if (name_size >= sizeof(name_utf16) / sizeof(name_utf16[0]))
		return 0;
	if (regSize > INT_MAX)
		return 0;

	if (!GetBestType_Registry(regType, type))
		return 0;

	void* p_data = nullptr;
	const auto res = ParseValue_Registry(p_data, std::numeric_limits<int32_t>::max(), type, regType, regSize, [&](void* buffer, size_t size) {
		DWORD regSize = size;
		return RegQueryValueExW((HKEY)m_key, name_utf16, 0, nullptr, static_cast<BYTE*>(buffer), &regSize) == ERROR_SUCCESS;
	});
	if (p_data)
		buff.reset(p_data);
	if (res > 0)
	{
		name = vs::UTF16toUTF8Convert(name_utf16);
		if (name_utf16[0] && name.empty())
			return 0;
	}
	m_ValuesIdx++;
	return res;
}

bool RegistryKey::NextString(std::string& data, std::string& name)
{
	DWORD type;
	wchar_t name_utf16[1024];
	DWORD name_size = sizeof(name_utf16) / sizeof(name_utf16[0]);
	wchar_t value_utf16[1024];
	DWORD value_size = sizeof(value_utf16);
	auto result = RegEnumValueW((HKEY)m_key, m_ValuesIdx, name_utf16, &name_size, 0, &type, reinterpret_cast<BYTE*>(value_utf16), &value_size);

  m_ValuesIdx++;
  if(type!=REG_SZ)
		return false;

	if (result != ERROR_SUCCESS)
		return false;

	name = vs::UTF16toUTF8Convert(name_utf16);
	if (name_utf16[0] && name.empty())
		return 0;
	data = vs::UTF16toUTF8Convert(value_utf16);
	if (value_utf16[0] && data.empty())
		return 0;

	return true;
}

bool RegistryKey::SetValue(const void* buff, size_t size, RegistryVT type, const char* name)
{
	if (m_key == 0)
		return false;
	if (buff == nullptr)
		return false;

	DWORD regType;
	std::u16string value_utf16;
	switch (type) {
	case VS_REG_INTEGER_VT:
		if (size != sizeof(int32_t))
			return false;
		regType = REG_DWORD;
		break;
	case VS_REG_INT64_VT:
		if(size != sizeof(int64_t))
			return false;
		regType = REG_QWORD;
		break;
	case VS_REG_STRING_VT:
		regType = REG_SZ;
		value_utf16 = vs::UTF8toUTF16Convert(static_cast<const char*>(buff));
		if (static_cast<const char*>(buff)[0] && value_utf16.empty())
			return false;
		buff = value_utf16.data();
		size = (value_utf16.size() + 1) * sizeof(wchar_t);
		break;
	case VS_REG_BINARY_VT:
		regType = REG_BINARY;
		break;
	case VS_REG_WSTRING_VT:
		regType = REG_SZ;
		size = (wcslen(static_cast<const wchar_t*>(buff)) + 1) * sizeof(wchar_t);
		break;
	} // end switch

	auto name_utf16 = vs::UTF8toUTF16Convert(name);
	if (name[0] && name_utf16.empty())
		return 0;
	return RegSetValueExW((HKEY)m_key, reinterpret_cast<const wchar_t*>(name_utf16.c_str()), 0, regType, reinterpret_cast<const BYTE*>(buff), size) == ERROR_SUCCESS;
}

const char* RegistryKey::GetName() const
{
	if (IsValid()) {
		auto pos = m_keyName.find_last_of('\\');
		if (pos == std::string::npos)
			return 0;
		return m_keyName.data() + pos + 1;
	} // end if
	return 0;
}

std::string RegistryKey::GetLastWriteTime() const
{
	FILETIME last_write_time;
	if (RegQueryInfoKey((HKEY)m_key, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &last_write_time) == ERROR_SUCCESS)
		return std::to_string(last_write_time.dwHighDateTime) + std::to_string(last_write_time.dwLowDateTime);
	return {};
}

bool RegistryKey::IsValid() const
{
	return (m_key != 0);
}

RegistryBackend::RegistryBackend()
	: m_force_lm(false)
{
}

bool RegistryBackend::Init(string_view configuration)
{
	const auto param_sep_pos = configuration.find(':');
	if (param_sep_pos == configuration.npos)
		return false;

	string_view parameters(configuration);
	parameters.remove_prefix(param_sep_pos + 1);
	typedef boost::algorithm::find_iterator<string_view::const_iterator> find_iterator;
	for (find_iterator param_it(parameters, boost::algorithm::token_finder([](char x) { return x != ','; }, boost::algorithm::token_compress_on)); !param_it.eof(); ++param_it)
	{
		if (boost::iequals(*param_it, string_view("force_lm=true")))
			m_force_lm = true;
		else if (boost::iequals(*param_it, string_view("force_lm=false")))
			m_force_lm = false;
	}
	return true;
}

key_ptr RegistryBackend::Create(string_view root, bool user, string_view name, bool read_only, bool create)
{
	return std::make_unique<RegistryKey>(root, user && !m_force_lm, name, read_only, create);
}

}

int VS_RegistryKey::CopyKey(bool s_user, const char *src, bool d_user, const char *dst)
{
	HKEY skey, dkey;
	DWORD err = RegOpenKeyExA(s_user ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE, src, 0, KEY_READ | KEY_WOW64_32KEY, &skey);
	if (err == ERROR_SUCCESS) {
		err = RegCreateKeyEx(d_user ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE, dst, 0, 0, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE | KEY_WOW64_32KEY, 0, &dkey, 0);
		if (err == ERROR_SUCCESS) {
			err = SHCopyKey(skey, "", dkey, 0);
			RegCloseKey(dkey);
		}
		RegCloseKey(skey);
	}
	return err;
}

#endif
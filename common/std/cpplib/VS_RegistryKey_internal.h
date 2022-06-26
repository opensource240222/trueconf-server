#pragma once

#include "VS_RegistryKey.h"
#include "std-generic/clib/strcasecmp.h"
#include "std-generic/clib/wcscasecmp.h"
#include "std/cpplib/numerical.h"

#include "std-generic/compat/algorithm.h"
#include "std-generic/compat/memory.h"
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cwchar>
#include <cwctype>

namespace regkey {

// No-op implementation, used in default-constructed and moved-from instances of VS_RegistryKey.
class KeyStub final : public Key
{
public:
	virtual key_ptr Clone() const override { return vs::make_unique<KeyStub>(); }
	virtual bool IsValid() const override { return false; }
	virtual int32_t GetValue(void*, size_t, RegistryVT, const char*) const override { return 0; }
	virtual int32_t GetValue(std::unique_ptr<void, free_deleter>&, RegistryVT, const char*) const override { return 0; }
	virtual int32_t GetValueAndType(void*, size_t, RegistryVT&, const char*) const override { return 0; }
	virtual int32_t GetValueAndType(std::unique_ptr<void, free_deleter>&, RegistryVT&, const char*) const override { return 0; }
	virtual bool GetString(std::string&, const char*) const override { return false; }
	virtual bool SetValue(const void*, size_t, RegistryVT, const char*) override { return false; }
	virtual bool HasValue(string_view) const override { return false; }
	virtual bool RemoveValue(string_view) override { return false; }
	virtual bool RemoveKey(string_view) override { return false; }
	virtual bool RenameKey(string_view, string_view) override { return false; }
	virtual bool CopyKey(string_view, string_view) override { return false; }
	virtual unsigned GetValueCount() const override { return 0; }
	virtual unsigned GetKeyCount() const override { return 0; }
	virtual void ResetValues() override {}
	virtual int32_t NextValue(std::unique_ptr<void, free_deleter>&, RegistryVT, std::string&) override { return 0; }
	virtual int32_t NextValueAndType(std::unique_ptr<void, free_deleter>&, RegistryVT&, std::string&) override { return 0; }
	virtual bool NextString(std::string&, std::string&) override { return false; }
	virtual void ResetKey() override {}
	virtual key_ptr NextKey(bool) override { return nullptr; }
	virtual const char* GetName() const override { return nullptr; }
	virtual std::string GetLastWriteTime() const override { return {}; }
};

inline bool IsKeyNameValid(string_view name)
{
	return name.empty() || (name.front() != '\\' && name.back() != '\\');
}

// Checks is either key is a subkey of the other key.
// Returned value is a bitmask:
//   * first bit is set if key1 is a subkey of key2.
//   * second bit is set if key2 is a subkey of key1.
inline unsigned IsSubKey(string_view key1, string_view key2)
{
	const auto difference = vs::mismatch(key1.begin(), key1.end(), key2.begin(), key2.end(), [](char c1, char c2) {
		if (c1 >= 'A' && c1 <= 'Z')
			c1 = c1 - 'A' + 'a';
		if (c2 >= 'A' && c2 <= 'Z')
			c2 = c2 - 'A' + 'a';
		return c1 == c2;
	});
	unsigned result = 0;
	if (difference.first == key1.end())
		result |= 0x1;
	if (difference.second == key2.end())
		result |= 0x2;
	return result;
}

template <class T>
inline bool ParseInteger_SpecialValues(const char* str, T& value)
{
	if (strcasecmp(str, "true") == 0 || strcasecmp(str, "yes")  == 0)
	{
		value = 1;
		return true;
	}
	if (strcasecmp(str, "false") == 0 || strcasecmp(str, "no")  == 0)
	{
		value = 0;
		return true;
	}
	return false;
}

inline bool ParseToInteger(const char* str, int32_t& value)
{
	// We need to check if str starts with a minus sign, this will determine if we should use strtol or strtoul.
	// First we need o skip white-space as defined by strto* functions.
	while (std::isspace(*str))
		++str;

	// To ensure similar behaviour on all platforms (not depending on the size of long) we need to clamp the value to the range of the result type.
	char* end = nullptr;
	if (*str == '-')
		value = clamp_cast<int32_t>(std::strtol(str, &end, 10));
	else
		value = clamp_cast<uint32_t>(std::strtoul(str, &end, 10));
	if (end != str)
		return true;

	// String is not a number, try another way to convert
	return ParseInteger_SpecialValues(str, value);
}

inline bool ParseToInteger(const char* str, int64_t& value)
{
	// We need to check if str starts with a minus sign, this will determine if we should use strtoll or strtoull.
	// First we need o skip white-space as defined by strto* functions.
	while (std::isspace(*str))
		++str;

	// To ensure similar behaviour on all platforms (not depending on the size of long) we need to clamp the value to the range of the result type.
	char* end = nullptr;
	if (*str == '-')
		value = clamp_cast<int64_t>(std::strtoll(str, &end, 10));
	else
		value = clamp_cast<uint64_t>(std::strtoull(str, &end, 10));
	if (end != str)
		return true;

	// String is not a number, try another way to convert
	return ParseInteger_SpecialValues(str, value);
}

#if defined(_WIN32) // Not ported yet
template <class T>
inline bool ParseInteger_SpecialValues(const wchar_t* str, T& value)
{
	if (wcscasecmp(str, L"true") == 0 || wcscasecmp(str, L"yes")  == 0)
	{
		value = 1;
		return true;
	}
	if (wcscasecmp(str, L"false") == 0 || wcscasecmp(str, L"no")  == 0)
	{
		value = 0;
		return true;
	}
	return false;
}

inline bool ParseToInteger(const wchar_t* str, int32_t& value)
{
	// We need to check if str starts with a minus sign, this will determine if we should use wcstol or wcstoul.
	// First we need o skip white-space as defined by wcsto* functions.
	while (std::iswspace(*str))
		++str;

	// To ensure similar behaviour on all platforms (not depending on the size of long) we need to clamp the value to the range of the result type.
	wchar_t* end = nullptr;
	if (*str == L'-')
		value = clamp_cast<int32_t>(std::wcstol(str, &end, 10));
	else
		value = clamp_cast<uint32_t>(std::wcstoul(str, &end, 10));
	if (end != str)
		return true;

	// String is not a number, try another way to convert
	return ParseInteger_SpecialValues(str, value);
}

inline bool ParseToInteger(const wchar_t* str, int64_t& value)
{
	// We need to check if str starts with a minus sign, this will determine if we should use wcstoll or wcstoull.
	// First we need o skip white-space as defined by wcsto* functions.
	while (std::iswspace(*str))
		++str;

	// To ensure similar behaviour on all platforms (not depending on the size of long) we need to clamp the value to the range of the result type.
	wchar_t* end = nullptr;
	if (*str == L'-')
		value = clamp_cast<int64_t>(std::wcstoll(str, &end, 10));
	else
		value = clamp_cast<uint64_t>(std::wcstoull(str, &end, 10));
	if (end != str)
		return true;

	// String is not a number, try another way to convert
	return ParseInteger_SpecialValues(str, value);
}
#endif

}

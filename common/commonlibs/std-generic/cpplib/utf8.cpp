#include "utf8.h"
#include "std-generic/cpplib/AtomicCache.h"
#include "std-generic/clang_version.h"
#include "std-generic/gcc_version.h"

#if defined(_WIN32)
#	include <Windows.h>
#elif defined(__linux__)
#	include <iconv.h>
#elif defined(__APPLE__)
#    include <iconv.h>
#else
#	error Unknown platform
#endif

#include <algorithm>
#include <cassert>
#include <cstdint>

namespace vs {

template <class String>
struct string_traits;

template <class charT>
struct string_traits<std::basic_string<charT>>
{
#if defined(_LIBCPP_VERSION) // libc++
	// x86: 10*char, 4*char16_t, 1*char32_t; x86_64: 22*char, 10*char16_t, 4*char32_t
	static constexpr size_t max_inplace_size = (sizeof(std::basic_string<charT>) - 1) / sizeof(charT) - 1;
#elif defined(__GLIBCXX__) || defined(_CPPLIB_VER) // stdlibc++ or MSVC STL
	static constexpr size_t max_inplace_size = 16 / sizeof(charT) - 1;
#endif
};

#if (defined(GCC_VERSION) && GCC_VERSION < 70000) \
 || (defined(CLANG_VERSION) && CLANG_VERSION < 30900)
// When building with GCC 4.7 or Clang 3.4 linking fails with message: undefined reference to string_traits<>::max_inplace_size
template <class charT>
constexpr size_t string_traits<std::basic_string<charT>>::max_inplace_size;
#endif

#if defined(_WIN32)

template <class FromChar, class ToChar>
UnicodeConverter<FromChar, ToChar>::UnicodeConverter()
{
}

template <class FromChar, class ToChar>
UnicodeConverter<FromChar, ToChar>::UnicodeConverter(UnicodeConverter&&) noexcept
{
}

template <class FromChar, class ToChar>
auto UnicodeConverter<FromChar, ToChar>::operator=(UnicodeConverter&&) noexcept -> UnicodeConverter&
{
	return *this;
}

template <class FromChar, class ToChar>
UnicodeConverter<FromChar, ToChar>::~UnicodeConverter()
{
}

template <class FromChar, class ToChar>
struct ConvertImpl {};

template <class ToChar>
struct ConvertImpl<char, ToChar>
{
	static std::basic_string<ToChar> Convert(string_view s)
	{
		static_assert(std::is_same<ToChar, wchar_t>::value || std::is_same<ToChar, char16_t>::value, "ToChar can be either wchar_t or char16_t");
		assert(s.size() <= INT_MAX);
		std::basic_string<ToChar> result;
		auto ret = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), static_cast<int>(s.size()), NULL, 0);
		if (ret <= 0)
			return result;
		result.resize(ret);
		if (ret != ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), static_cast<int>(s.size()), reinterpret_cast<wchar_t*>(&result[0]), ret))
			result.clear();
		return result;
	}
};

template <class FromChar>
struct ConvertImpl<FromChar, char>
{
	static std::string Convert(basic_string_view<FromChar> s)
	{
		static_assert(std::is_same<FromChar, wchar_t>::value || std::is_same<FromChar, char16_t>::value, "FromChar can be either wchar_t or char16_t");
		assert(s.size() <= INT_MAX);
		std::string result;
		unsigned flags = 0;
#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
		flags |= WC_ERR_INVALID_CHARS;
		// WinXP doesn't have WC_ERR_INVALID_CHARS.
		// So instead of an error we will get a string with invalid UTF-16 sequences replaced by "Replacement Character" U+FFFD.
#endif
		auto ret = ::WideCharToMultiByte(CP_UTF8, flags, reinterpret_cast<const wchar_t*>(s.data()), static_cast<int>(s.size()), NULL, 0, NULL, NULL);
		if (ret <= 0)
			return result;
		result.resize(ret);
		if (ret != ::WideCharToMultiByte(CP_UTF8, flags, reinterpret_cast<const wchar_t*>(s.data()), static_cast<int>(s.size()), &result[0], ret, NULL, NULL))
			result.clear();
		return result;
	}
};

template <>
struct ConvertImpl<char, char32_t>
{
	static std::u32string Convert(string_view s)
	{
		// WinAPI limits UTF-32 support to managed applications only.
		// So use custom conversion instead.
		// Note: There are no checks for incorrect byte sequences.

		std::u32string result;
		result.reserve(s.size());

		auto p = reinterpret_cast<const unsigned char*>(s.data());
		const auto p_end = p + s.size();
		while (p < p_end)
		{
			if ((*p & 0x80) == 0)
			{
				// ASCII
				result += static_cast<char32_t>(*p);
				++p;
				continue;
			}

			switch (*p & 0xf0)
			{
			case 0x80:
			case 0x90:
			case 0xa0:
			case 0xb0:
				// Continuation byte without a leading byte, skip it.
				p += 1;
				break;
			case 0xc0:
			case 0xd0:
				// 2 bytes
				if (p + 1 >= p_end)
				{
					// Truncated string
					p = p_end;
					break;
				}
				result += (static_cast<char32_t>(p[0] & 0x1f) << 6) | static_cast<char32_t>(p[1] & 0x3f);
				p += 2;
				break;
			case 0xe0:
				// 3 bytes
				if (p + 2 >= p_end)
				{
					// Truncated string
					p = p_end;
					break;
				}
				result += (static_cast<char32_t>(p[0] & 0x0f) << 12) | (static_cast<char32_t>(p[1] & 0x3f) << 6) | static_cast<char32_t>(p[2] & 0x3f);
				p += 3;
				break;
			case 0xf0:
				// 4 bytes
				if (p + 3 >= p_end)
				{
					// Truncated string
					p = p_end;
					break;
				}
				result += (static_cast<char32_t>(p[0] & 0x07) << 18) | (static_cast<char32_t>(p[1] & 0x3f) << 12) | (static_cast<char32_t>(p[2] & 0x3f) << 6) | static_cast<char32_t>(p[3] & 0x3f);
				p += 4;
				break;
			}
		}

		return result;
	}
};

template <class FromChar, class ToChar>
std::basic_string<ToChar> UnicodeConverter<FromChar, ToChar>::Convert(basic_string_view<FromChar> s)
{
	return ConvertImpl<FromChar, ToChar>::Convert(s);
}

#elif (defined(__linux__)| defined(__APPLE__))

static_assert(std::is_same<iconv_t, void*>::value, "");
static const ::iconv_t invalid_iconv_descriptor = (::iconv_t)(-1);
struct iconv_deleter { void operator()(::iconv_t x) const { ::iconv_close(x); }};

template <class FromChar, class ToChar>
struct UnicodeConverterStorage
{
	static vs::AtomicCache< ::iconv_t, iconv_deleter, 8> cache;
};
template <class FromChar, class ToChar>
vs::AtomicCache< ::iconv_t, iconv_deleter, 8> UnicodeConverterStorage<FromChar, ToChar>::cache(8, invalid_iconv_descriptor);

template <class charT> struct iconv_traits { static const char encoding_name[]; };
template <> const char iconv_traits<char>::encoding_name[]     = "UTF-8";
template <> const char iconv_traits<char16_t>::encoding_name[] = "UTF-16LE";
template <> const char iconv_traits<char32_t>::encoding_name[] = "UTF-32LE";
template <> const char iconv_traits<wchar_t>::encoding_name[]  = "WCHAR_T";

template <class FromChar, class ToChar>
UnicodeConverter<FromChar, ToChar>::UnicodeConverter()
	: m_cd(UnicodeConverterStorage<FromChar, ToChar>::cache.Get())
{
	if (m_cd == invalid_iconv_descriptor)
		m_cd = ::iconv_open(iconv_traits<ToChar>::encoding_name, iconv_traits<FromChar>::encoding_name);
}

template <class FromChar, class ToChar>
UnicodeConverter<FromChar, ToChar>::UnicodeConverter(UnicodeConverter&& x) noexcept
	: m_cd(x.m_cd)
{
	x.m_cd = invalid_iconv_descriptor;
}

template <class FromChar, class ToChar>
auto UnicodeConverter<FromChar, ToChar>::operator=(UnicodeConverter&& x) noexcept -> UnicodeConverter&
{
	// This will leave moved from object (x) in a valid state, and that is all that is required.
	// On the other hand this allows to omit self assignment check.
	std::swap(m_cd, x.m_cd);
	return *this;
}

template <class FromChar, class ToChar>
UnicodeConverter<FromChar, ToChar>::~UnicodeConverter()
{
	UnicodeConverterStorage<FromChar, ToChar>::cache.Put(m_cd);
}

template <class FromChar, class ToChar>
std::basic_string<ToChar> UnicodeConverter<FromChar, ToChar>::Convert(basic_string_view<FromChar> s)
{
	std::basic_string<ToChar> result;
	char* in = reinterpret_cast<char*>(const_cast<FromChar*>(s.data()));
	size_t in_bytes_left = s.size() * sizeof(FromChar);
	// Allocate minimal amount of memory at first, but not less than SSO size.
    const size_t _max_inplace_size = string_traits<decltype(result)>::max_inplace_size;
	result.resize(std::max(s.size() / (std::is_same<FromChar, char>::value ? sizeof(ToChar) : 1u), _max_inplace_size));
	size_t out_bytes_left = result.size() * sizeof(ToChar);
	while (true)
	{
		assert(out_bytes_left <= result.size() * sizeof(ToChar));
		char* out = reinterpret_cast<char*>(&result[result.size()]) - out_bytes_left;
		auto ret = ::iconv(m_cd, &in, &in_bytes_left, &out, &out_bytes_left);
		if (ret == static_cast<size_t>(-1) && errno != E2BIG)
		{
			result.clear();
			return result;
		}
		if (in_bytes_left == 0)
			break;
		const auto grow_size = result.size();
		assert(grow_size > 0);
		result.resize(result.size() + grow_size);
		out_bytes_left += grow_size * sizeof(ToChar);
	}
	assert(out_bytes_left % sizeof(ToChar) == 0);
	assert(out_bytes_left / sizeof(ToChar) <= result.size());
	result.resize(result.size() - out_bytes_left / sizeof(ToChar));
	return result;
}

#endif

template class UnicodeConverter<char, char16_t>;
template class UnicodeConverter<char16_t, char>;
template class UnicodeConverter<char, wchar_t>;
template class UnicodeConverter<wchar_t, char>;
template class UnicodeConverter<char, char32_t>;

bool IsASCIIString(string_view s)
{
	auto p = s.data();
	size_t i = s.size();

	uint64_t x = 0;
	while (i >= 8)
	{
		x = x | *reinterpret_cast<const uint64_t*>(p);
		i -= 8;
		p += 8;
	}
	if ((x & 0x8080808080808080) != 0)
		return false;

	uint32_t xx = 0;
	if (i >= 4)
	{
		xx = xx | *reinterpret_cast<const uint32_t*>(p);
		i -= 4;
		p += 4;
	}
	if (i >= 2)
	{
		xx = xx | *reinterpret_cast<const uint16_t*>(p);
		i -= 2;
		p += 2;
	}
	if (i >= 1)
	{
		xx = xx | *reinterpret_cast<const uint8_t*>(p);
		i -= 1;
		p += 1;
	}
	return (xx & 0x80808080) == 0;
}

bool IsASCIIString(u16string_view s)
{
	auto p = s.data();
	size_t i = s.size();

	uint64_t x = 0;
	while (i >= 4)
	{
		x = x | *reinterpret_cast<const uint64_t*>(p);
		i -= 4;
		p += 4;
	}
	if ((x & 0xff80ff80ff80ff80) != 0)
		return false;

	uint32_t xx = 0;
	if (i >= 2)
	{
		xx = xx | *reinterpret_cast<const uint32_t*>(p);
		i -= 2;
		p += 2;
	}
	if (i >= 1)
	{
		xx = xx | *reinterpret_cast<const uint16_t*>(p);
		i -= 1;
		p += 1;
	}
	return (xx & 0xff80ff80) == 0;
}

std::u16string ASCIItoUTF16Convert(string_view in)
{
	std::u16string out(in.size(), static_cast<char16_t>(0));
	auto dst = &out[0];
	auto src = &in[0];
	for (size_t i = 0, i_end = in.size(); i < i_end; ++i)
		*dst++ = static_cast<char16_t>(*src++);
	return out;
}

std::string UTF16toASCIIConvert(u16string_view in)
{
	std::string out(in.size(), '\0');
	auto dst = &out[0];
	auto src = &in[0];
	for (size_t i = 0, i_end = in.size(); i < i_end; ++i)
		*dst++ = static_cast<char>(*src++);
	return out;
}

const std::locale& GetUnicodeLocale()
{
	static const auto x = []() {
#if defined(_WIN32)
		try { return std::locale("en_US.UTF-8"); } catch (...) {}
		try { return std::locale("en_US.65001"); } catch (...) {}
		try { return std::locale("ru_RU.UTF-8"); } catch (...) {}
		try { return std::locale("ru_RU.65001"); } catch (...) {}
		try { return std::locale("en-US"); } catch (...) {}
#elif defined(__linux__)
		try { return std::locale("C.UTF-8"); } catch (...) {}
		try { return std::locale("C.utf8"); } catch (...) {}
		try { return std::locale("en_US.UTF-8"); } catch (...) {}
		try { return std::locale("en_US.utf8"); } catch (...) {}
		try { return std::locale("en.UTF-8"); } catch (...) {}
		try { return std::locale("en.utf8"); } catch (...) {}
		try { return std::locale("en_US"); } catch (...) {}
		try { return std::locale("en"); } catch (...) {}
#endif
		return std::locale(""); // Fallback to default system locale and hope that it is a Unicode one.
	}();
	return x;
}

std::string UTF8ToLower(string_view in)
{
	std::string result;
	if (IsASCIIString(in))
	{
		result = std::string(in);
		// We are dealing with ASCII and can use a custom tolower version to avoid function call overhead.
		std::transform(result.begin(), result.end(), result.begin(), [](char c) { return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c; });
	}
	else
	{
		auto wstr = UnicodeConverter<char, wchar_t>().Convert(in);
		const auto& loc = GetUnicodeLocale();
		for (auto& c : wstr)
			c = std::tolower(c, loc);
		result = UnicodeConverter<wchar_t, char>().Convert(wstr);
	}
	return result;
}

std::string UTF8ToUpper(string_view in)
{
	std::string result;
	if (IsASCIIString(in))
	{
		result = std::string(in);
		// We are dealing with ASCII and can use a custom toupper version to avoid function call overhead.
		std::transform(result.begin(), result.end(), result.begin(), [](char c) { return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c; });
	}
	else
	{
		auto wstr = UnicodeConverter<char, wchar_t>().Convert(in);
		const auto& loc = GetUnicodeLocale();
		for (auto& c : wstr)
			c = std::toupper(c, loc);
		result = UnicodeConverter<wchar_t, char>().Convert(wstr);
	}
	return result;
}

size_t CountCodePoints(string_view s)
{
	// Each code point either is a ASCII character or starts with a byte that have 2 high bits set.
	static const uint8_t sz_tbl[4] = {
		1,1, // 0xxxxxxx - ASCII characters
		0,   // 10xxxxxx - Continuation bytes
		1,   // 11xxxxxx - Start of multi-byte sequence
	};

	size_t result = 0;
	for (auto c : s)
		result += sz_tbl[static_cast<uint8_t>(c) >> 6];
	return result;
}

}

#pragma once

#include "std-generic/attributes.h"
#include "std-generic/cpplib/string_view.h"

#include <cstddef>
#include <locale>
#include <string>

namespace vs {

template <class FromChar, class ToChar>
class UnicodeConverter
{
public:
	UnicodeConverter();
	UnicodeConverter(const UnicodeConverter&) = delete;
	UnicodeConverter(UnicodeConverter&&) noexcept;
	UnicodeConverter& operator=(const UnicodeConverter&) = delete;
	UnicodeConverter& operator=(UnicodeConverter&&) noexcept;
	~UnicodeConverter();

	VS_NODISCARD std::basic_string<ToChar> Convert(basic_string_view<FromChar> s);

private:
#if (defined(__linux__)|defined(__APPLE__))
	void* m_cd;
#endif
};
extern template class UnicodeConverter<char, char16_t>; // UTF8 -> UTF16 converter
extern template class UnicodeConverter<char16_t, char>; // UTF16 -> UTF8 converter
extern template class UnicodeConverter<char, wchar_t>;  // UTF8 -> WideChar converter
extern template class UnicodeConverter<wchar_t, char>;  // WideChar -> UTF8 converter
extern template class UnicodeConverter<char, char32_t>; // UTF8 -> UTF32 converter
// TODO: Implement UTF32 -> UTF8
// extern template class UnicodeConverter<char32_t, char>; // UTF32 -> UTF8 converter

VS_NODISCARD bool IsASCIIString(string_view s);
VS_NODISCARD bool IsASCIIString(u16string_view s);
#if defined(_WIN32) // Not ported yet
VS_NODISCARD inline bool IsASCIIString(wstring_view s)
{
	static_assert(sizeof(wchar_t) == sizeof(char16_t), "Conversion is not correct. wchar_t != char16_t");
	return IsASCIIString({ reinterpret_cast<const char16_t*>(s.data()), s.size() });
}
#endif

VS_NODISCARD std::u16string ASCIItoUTF16Convert(string_view in);
VS_NODISCARD std::string UTF16toASCIIConvert(u16string_view in);

template<class charT>
/* will work even with char16_t on unix*/
VS_NODISCARD typename std::enable_if<sizeof(charT) >= sizeof(char16_t), std::basic_string<charT>>::type
ASCIItoWideCharConvert(string_view in)	// to UTF16 or to UTF32
{
	std::basic_string<charT> out(in.size(), static_cast<charT>(0));
	auto dst = &out[0];
	auto src = &in[0];
	for (size_t i = 0, i_end = in.size(); i < i_end; ++i)
		*dst++ = static_cast<charT>(*src++);
	return out;
}

VS_NODISCARD inline std::u16string UTF8toUTF16Convert(string_view in)
{
	if (IsASCIIString(in))
		return ASCIItoUTF16Convert(in);
	return UnicodeConverter<char, char16_t>().Convert(in);
}
VS_NODISCARD inline std::string UTF16toUTF8Convert(u16string_view in)
{
	if (IsASCIIString(in))
		return UTF16toASCIIConvert(in);
	return UnicodeConverter<char16_t, char>().Convert(in);
}
#if defined(_WIN32) // Not ported yet
VS_NODISCARD inline std::string UTF16toUTF8Convert(wstring_view in)
{
	static_assert(sizeof(wchar_t) == sizeof(char16_t), "Conversion is not correct. wchar_t != char16_t");
	return UTF16toUTF8Convert({ reinterpret_cast<const char16_t*>(in.data()), in.size() });
}
#endif

VS_NODISCARD inline std::wstring UTF8ToWideCharConvert(string_view in)
{
	if (IsASCIIString(in))
		return ASCIItoWideCharConvert<wchar_t>(in);
	return UnicodeConverter<char, wchar_t>().Convert(in);
}
VS_NODISCARD inline std::string WideCharToUTF8Convert(wstring_view in)
{
	return UnicodeConverter<wchar_t, char>().Convert(in);
}

VS_NODISCARD inline std::u32string UTF8ToUTF32Convert(string_view in)
{
	return UnicodeConverter<char, char32_t>().Convert(in);
}

// Returns locale capable of converting Unicode characters stored in wchar_t.
VS_NODISCARD const std::locale& GetUnicodeLocale();

VS_NODISCARD std::string UTF8ToLower(string_view in);
VS_NODISCARD std::string UTF8ToUpper(string_view in);

// Returns number of Unicode code points (in most cases a character is
// represented by a single code point).
// Requires input string to be a valid UTF-8 string.
VS_NODISCARD size_t CountCodePoints(string_view s);

}

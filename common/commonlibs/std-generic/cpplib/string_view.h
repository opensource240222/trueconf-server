#pragma once

#include "std-generic/compat/type_traits.h"
#include <ios>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>

#if __cplusplus >= 201402L || (defined(_MSC_VER) && _MSC_VER >= 1910)
#	define constexpr_14 constexpr
#else
#	define constexpr_14
#endif
#if __cplusplus >= 201703L || (defined(_MSC_VER) && _MSC_VER >= 1910)
#	define constexpr_17 constexpr
#else
#	define constexpr_17
#endif

// C++17 <string_view>

template <class charT, class traits = std::char_traits<charT>>
class basic_string_view
{
public:
	typedef traits traits_type;
	typedef charT value_type;
	typedef charT* pointer;
	typedef const charT* const_pointer;
	typedef charT& reference;
	typedef const charT& const_reference;
	typedef const_pointer const_iterator;
	typedef const_iterator iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	typedef const_reverse_iterator reverse_iterator;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	static constexpr size_type npos = size_type(-1);

	constexpr basic_string_view() noexcept
		: data_(nullptr)
		, size_(0)
	{}
	constexpr basic_string_view(const basic_string_view&) noexcept = default;
	basic_string_view& operator=(const basic_string_view&) noexcept = default;
	// cppcheck-suppress noExplicitConstructor
	constexpr_17 basic_string_view(const charT* str)
		: data_(str)
		, size_(traits::length(str))
	{}
	constexpr basic_string_view(const charT* str, size_type len)
		: data_(str)
		, size_(len)
	{}

	constexpr const_iterator begin()  const noexcept { return data_; }
	constexpr const_iterator end()    const noexcept { return data_ + size_; }
	constexpr const_iterator cbegin() const noexcept { return data_; }
	constexpr const_iterator cend()   const noexcept { return data_ + size_; }

	constexpr const_reverse_iterator rbegin()  const noexcept { return const_reverse_iterator(end()); }
	constexpr const_reverse_iterator rend()    const noexcept { return const_reverse_iterator(begin()); }
	constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
	constexpr const_reverse_iterator crend()   const noexcept { return const_reverse_iterator(cbegin()); }

	constexpr size_type size()     const noexcept { return size_; }
	constexpr size_type length()   const noexcept { return size_; }
	// cppcheck-suppress functionStatic
	constexpr size_type max_size() const noexcept { return (std::numeric_limits<size_type>::max)() - 1; }
	constexpr bool empty() const noexcept { return size_ == 0; }

	constexpr const_reference operator[](size_type pos) const
	{
		return data_[pos];
	}
	constexpr const_reference at(size_type pos) const
	{
		return pos < size_ ? data_[pos] : throw std::out_of_range("");
	}

	constexpr const_reference front() const { return data_[0]; }
	constexpr const_reference back()  const { return data_[size_ - 1]; }
	constexpr const_pointer   data()  const { return data_; }

	constexpr_14 void remove_prefix(size_type n)
	{
		data_ += n;
		size_ -= n;
	}
	constexpr_14 void remove_suffix(size_type n)
	{
		size_ -= n;
	}
	constexpr_14 void swap(basic_string_view& s) noexcept
	{
		using std::swap;
		swap(data_, s.data_);
		swap(size_, s.size_);
	}

	size_type copy(charT* s, size_type n, size_type pos = 0) const
	{
		if (pos > size_)
			throw std::out_of_range("");
		if (n > size_ - pos)
			n = size_ - pos;
		traits::copy(s, data_ + pos, n);
		return n;
	}

	constexpr basic_string_view substr(size_type pos = 0, size_type n = npos) const
	{
		return pos <= size_
			? basic_string_view(data_ + pos, n <= size_ - pos ? n : size_ - pos)
			: throw std::out_of_range("");
	}

	constexpr_17 int compare(basic_string_view str) const noexcept
	{
		int res = traits::compare(data_, str.data_, size_ < str.size_ ? size_ : str.size_);
		if (res == 0)
		{
			if (size_ < str.size_) res = -1;
			else if (size_ > str.size_) res = 1;
		}
		return res;
	}
	constexpr_17 int compare(size_type pos1, size_type n1, basic_string_view str) const
	{
		return substr(pos1, n1).compare(str);
	}
	constexpr_17 int compare(size_type pos1, size_type n1, basic_string_view str, size_type pos2, size_type n2) const
	{
		return substr(pos1, n1).compare(str.substr(pos2, n2));
	}
	constexpr_17 int compare(const charT* s) const
	{
		return compare(basic_string_view(s));
	}
	constexpr_17 int compare(size_type pos1, size_type n1, const charT* s) const
	{
		return compare(pos1, n1, basic_string_view(s));
	}
	constexpr_17 int compare(size_type pos1, size_type n1, const charT* s, size_type n2) const
	{
		return compare(pos1, n1, basic_string_view(s, n2));
	}

	constexpr_17 size_type find(basic_string_view str, size_type pos = 0) const noexcept
	{
		if (str.size_ > size_)
			return npos;
		for (; pos <= size_ - str.size_; ++pos)
			if (traits::compare(data_ + pos, str.data_, str.size_) == 0)
				return pos;
		return npos;
	}
	constexpr_17 size_type find(const charT* s, size_type pos, size_type n) const
	{
		return find(basic_string_view(s, n), pos);
	}
	constexpr_17 size_type find(const charT* s, size_type pos = 0) const
	{
		return find(basic_string_view(s), pos);
	}
	constexpr_17 size_type find(charT c, size_type pos = 0) const noexcept
	{
		return find_first_of(c, pos);
	}

	constexpr_17 size_type rfind(basic_string_view str, size_type pos = npos) const noexcept
	{
		if (str.size_ > size_)
			return npos;
		if (pos > size_ - str.size_)
			pos = size_ - str.size_;
		do {
			if (traits::compare(data_ + pos, str.data_, str.size_) == 0)
				return pos;
		} while (pos-- != 0);
		return npos;
	}
	constexpr_17 size_type rfind(const charT* s, size_type pos, size_type n) const
	{
		return rfind(basic_string_view(s, n), pos);
	}
	constexpr_17 size_type rfind(const charT* s, size_type pos = npos) const
	{
		return rfind(basic_string_view(s), pos);
	}
	constexpr_17 size_type rfind(charT c, size_type pos = npos) const noexcept
	{
		return find_last_of(c, pos);
	}

	constexpr_17 size_type find_first_of(basic_string_view str, size_type pos = 0) const noexcept
	{
		for (; pos < size_; ++pos)
			if (str.find(data_[pos]) != npos)
				return pos;
		return npos;
	}
	constexpr_17 size_type find_first_of(const charT* s, size_type pos, size_type n) const
	{
		return find_first_of(basic_string_view(s, n), pos);
	}
	constexpr_17 size_type find_first_of(const charT* s, size_type pos = 0) const
	{
		return find_first_of(basic_string_view(s), pos);
	}
	constexpr_17 size_type find_first_of(charT c, size_type pos = 0) const noexcept
	{
		for (; pos < size_; ++pos)
			if (traits::eq(data_[pos], c))
				return pos;
		return npos;
	}

	constexpr_17 size_type find_last_of(basic_string_view str, size_type pos = npos) const noexcept
	{
		if (size_ == 0)
			return npos;
		if (pos >= size_)
			pos = size_ - 1;
		do {
			if (str.find(data_[pos]) != npos)
				return pos;
		} while (pos-- > 0);
		return npos;
	}
	constexpr_17 size_type find_last_of(const charT* s, size_type pos, size_type n) const
	{
		return find_last_of(basic_string_view(s, n), pos);
	}
	constexpr_17 size_type find_last_of(const charT* s, size_type pos = npos) const
	{
		return find_last_of(basic_string_view(s), pos);
	}
	constexpr_17 size_type find_last_of(charT c, size_type pos = npos) const noexcept
	{
		if (size_ == 0)
			return npos;
		if (pos >= size_)
			pos = size_ - 1;
		do {
			if (traits::eq(data_[pos], c))
				return pos;
		} while (pos-- > 0);
		return npos;
	}

	constexpr_17 size_type find_first_not_of(basic_string_view str, size_type pos = 0) const noexcept
	{
		for (; pos < size_; ++pos)
			if (str.find(data_[pos]) == npos)
				return pos;
		return npos;
	}
	constexpr_17 size_type find_first_not_of(const charT* s, size_type pos, size_type n) const
	{
		return find_first_not_of(basic_string_view(s, n), pos);
	}
	constexpr_17 size_type find_first_not_of(const charT* s, size_type pos = 0) const
	{
		return find_first_not_of(basic_string_view(s), pos);
	}
	constexpr_17 size_type find_first_not_of(charT c, size_type pos = 0) const noexcept
	{
		for (; pos < size_; ++pos)
			if (!traits::eq(data_[pos], c))
				return pos;
		return npos;
	}

	constexpr_17 size_type find_last_not_of(basic_string_view str, size_type pos = npos) const noexcept
	{
		if (size_ == 0)
			return npos;
		if (pos >= size_)
			pos = size_ - 1;
		do {
			if (str.find(data_[pos]) == npos)
				return pos;
		} while (pos-- > 0);
		return npos;
	}
	constexpr_17 size_type find_last_not_of(const charT* s, size_type pos, size_type n) const
	{
		return find_last_not_of(basic_string_view(s, n), pos);
	}
	constexpr_17 size_type find_last_not_of(const charT* s, size_type pos = npos) const
	{
		return find_last_not_of(basic_string_view(s), pos);
	}
	constexpr_17 size_type find_last_not_of(charT c, size_type pos = npos) const noexcept
	{
		if (size_ == 0)
			return npos;
		if (pos >= size_)
			pos = size_ - 1;
		do {
			if (!traits::eq(data_[pos], c))
				return pos;
		} while (pos-- > 0);
		return npos;
	}

	// Non-standard, workarounds for missing support in std::string
	template <class Allocator>
	// cppcheck-suppress noExplicitConstructor
	basic_string_view(const std::basic_string<charT, traits, Allocator>& str) noexcept
		: data_(str.data())
		, size_(str.size())
	{}
	template <class Allocator>
	explicit operator std::basic_string<charT, traits, Allocator>() const
	{
		return { begin(), end() };
	}

private:
	const_pointer data_;
	size_type size_;
};

template <class charT, class traits>
constexpr_17 bool operator==(basic_string_view<charT, traits> lhs, basic_string_view<charT, traits> rhs) noexcept { return lhs.compare(rhs) == 0; }
template <class charT, class traits>
constexpr_17 bool operator!=(basic_string_view<charT, traits> lhs, basic_string_view<charT, traits> rhs) noexcept { return lhs.compare(rhs) != 0; }
template <class charT, class traits>
constexpr_17 bool operator< (basic_string_view<charT, traits> lhs, basic_string_view<charT, traits> rhs) noexcept { return lhs.compare(rhs) <  0; }
template <class charT, class traits>
constexpr_17 bool operator> (basic_string_view<charT, traits> lhs, basic_string_view<charT, traits> rhs) noexcept { return lhs.compare(rhs) >  0; }
template <class charT, class traits>
constexpr_17 bool operator<=(basic_string_view<charT, traits> lhs, basic_string_view<charT, traits> rhs) noexcept { return lhs.compare(rhs) <= 0; }
template <class charT, class traits>
constexpr_17 bool operator>=(basic_string_view<charT, traits> lhs, basic_string_view<charT, traits> rhs) noexcept { return lhs.compare(rhs) >= 0; }

// Extra template argument is added to work around bug in MSVC141, similar to std::basic_string_view from MS STL.
template <class charT, class traits, int = 1>
constexpr_17 bool operator==(basic_string_view<charT, traits> lhs, vs::type_identity_t<basic_string_view<charT, traits>> rhs) noexcept { return lhs.compare(rhs) == 0; }
template <class charT, class traits, int = 2>
constexpr_17 bool operator==(vs::type_identity_t<basic_string_view<charT, traits>> lhs, basic_string_view<charT, traits> rhs) noexcept { return lhs.compare(rhs) == 0; }
template <class charT, class traits, int = 1>
constexpr_17 bool operator!=(basic_string_view<charT, traits> lhs, vs::type_identity_t<basic_string_view<charT, traits>> rhs) noexcept { return lhs.compare(rhs) != 0; }
template <class charT, class traits, int = 2>
constexpr_17 bool operator!=(vs::type_identity_t<basic_string_view<charT, traits>> lhs, basic_string_view<charT, traits> rhs) noexcept { return lhs.compare(rhs) != 0; }
template <class charT, class traits, int = 1>
constexpr_17 bool operator< (basic_string_view<charT, traits> lhs, vs::type_identity_t<basic_string_view<charT, traits>> rhs) noexcept { return lhs.compare(rhs) <  0; }
template <class charT, class traits, int = 2>
constexpr_17 bool operator< (vs::type_identity_t<basic_string_view<charT, traits>> lhs, basic_string_view<charT, traits> rhs) noexcept { return lhs.compare(rhs) <  0; }
template <class charT, class traits, int = 1>
constexpr_17 bool operator> (basic_string_view<charT, traits> lhs, vs::type_identity_t<basic_string_view<charT, traits>> rhs) noexcept { return lhs.compare(rhs) >  0; }
template <class charT, class traits, int = 2>
constexpr_17 bool operator> (vs::type_identity_t<basic_string_view<charT, traits>> lhs, basic_string_view<charT, traits> rhs) noexcept { return lhs.compare(rhs) >  0; }
template <class charT, class traits, int = 1>
constexpr_17 bool operator<=(basic_string_view<charT, traits> lhs, vs::type_identity_t<basic_string_view<charT, traits>> rhs) noexcept { return lhs.compare(rhs) <= 0; }
template <class charT, class traits, int = 2>
constexpr_17 bool operator<=(vs::type_identity_t<basic_string_view<charT, traits>> lhs, basic_string_view<charT, traits> rhs) noexcept { return lhs.compare(rhs) <= 0; }
template <class charT, class traits, int = 1>
constexpr_17 bool operator>=(basic_string_view<charT, traits> lhs, vs::type_identity_t<basic_string_view<charT, traits>> rhs) noexcept { return lhs.compare(rhs) >= 0; }
template <class charT, class traits, int = 2>
constexpr_17 bool operator>=(vs::type_identity_t<basic_string_view<charT, traits>> lhs, basic_string_view<charT, traits> rhs) noexcept { return lhs.compare(rhs) >= 0; }

template <class charT, class traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& os, basic_string_view<charT, traits> str)
{
	if (static_cast<size_t>(os.width()) > str.size())
	{
		auto padding = os.width() - str.size();
		if (os.flags() & std::ios_base::right)
			do { os.put(os.fill()); } while (--padding != 0);
		os.write(str.data(), str.size());
		if (os.flags() & std::ios_base::left)
			do { os.put(os.fill()); } while (--padding != 0);
	}
	else
		os.write(str.data(), str.size());
	os.width(0);
	return os;
}

// Non-standard, workaround for missing support in std::string
template <class charT, class traits, class Allocator>
std::basic_string<charT, traits, Allocator>& operator+=(std::basic_string<charT, traits, Allocator>& s, basic_string_view<charT, traits> sv)
{
	return s.append(sv.begin(), sv.end());
}

typedef basic_string_view<char> string_view;
typedef basic_string_view<char16_t> u16string_view;
typedef basic_string_view<char32_t> u32string_view;
typedef basic_string_view<wchar_t> wstring_view;

// A string_view variant that maintains an additional invariant that the
// referenced character sequence is null-terminated.
// Inspired by p1402r0.

static constexpr struct null_terminated_t {} null_terminated {};

template <class charT, class traits = std::char_traits<charT>>
class basic_cstring_view : public basic_string_view<charT, traits>
{
public:
	using string_view_type       = basic_string_view<charT, traits>;
	using traits_type            = typename string_view_type::traits_type;
	using value_type             = typename string_view_type::value_type;
	using pointer                = typename string_view_type::pointer;
	using const_pointer          = typename string_view_type::const_pointer;
	using reference              = typename string_view_type::reference;
	using const_reference        = typename string_view_type::const_reference;
	using const_iterator         = typename string_view_type::const_iterator;
	using iterator               = typename string_view_type::iterator;
	using const_reverse_iterator = typename string_view_type::const_reverse_iterator;
	using reverse_iterator       = typename string_view_type::reverse_iterator;
	using size_type              = typename string_view_type::size_type;
	using difference_type        = typename string_view_type::difference_type;
	using string_view_type::npos;

	constexpr basic_cstring_view() noexcept
		: string_view_type(&null, 0)
	{}
	constexpr basic_cstring_view(const basic_cstring_view&) noexcept = default;
	basic_cstring_view& operator=(const basic_cstring_view&) noexcept = default;
	// cppcheck-suppress noExplicitConstructor
	constexpr_17 basic_cstring_view(const charT* str)
		: string_view_type(str)
	{}
	constexpr basic_cstring_view(null_terminated_t, const charT* str, size_type len)
		: string_view_type(str, len)
	{}
	constexpr basic_cstring_view(null_terminated_t, string_view_type sv)
		: string_view_type(sv.data(), sv.size())
	{}

	void remove_suffix(size_type n) = delete;

	constexpr_14 void swap(basic_cstring_view& s) noexcept
	{
		string_view_type::swap(*this, s);
	}

	using string_view_type::substr;
	constexpr basic_cstring_view substr(size_type pos = 0) const
	{
		return pos <= string_view_type::size()
			? basic_cstring_view(null_terminated, string_view_type::data() + pos, string_view_type::size() - pos)
			: throw std::out_of_range("");
	}

	constexpr const charT* c_str() const noexcept
	{
		return string_view_type::data();
	}

	// Non-standard, workaround for missing support in std::string
	template <class Allocator>
	// cppcheck-suppress noExplicitConstructor
	basic_cstring_view(const std::basic_string<charT, traits, Allocator>& str) noexcept
		: string_view_type(str.data(), str.size())
	{}

private:
	static const charT null { 0 };
};

template <class charT, class traits>
const charT basic_cstring_view<charT, traits>::null;

typedef basic_cstring_view<char> cstring_view;
typedef basic_cstring_view<char16_t> u16cstring_view;
typedef basic_cstring_view<char32_t> u32cstring_view;
typedef basic_cstring_view<wchar_t> wcstring_view;

#undef constexpr_14
#undef constexpr_17

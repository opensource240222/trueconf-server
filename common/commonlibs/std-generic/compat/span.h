#pragma once

#include "std-generic/libstdcxx_version.h"

// std::span from C++20 is available since libc++ 7, libstdc++ 10 and not yet alaivable in MSVC.
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 7000 && _LIBCPP_STD_VER > 17) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 100000 && __cplusplus > 201703L) \
 || 0

#include <span>

namespace vs {
using std::dynamic_extent;
using std::span;
using std::get;
using std::tuple_size;
using std::tuple_element;
// Not importing std::as_bytes and std::as_writable_bytes because we don't have std::byte.
}

#else

#include "std-generic/attributes.h"
#include "std-generic/compat/iterator.h"
#include "std-generic/compat/memory.h"
#include <array>
#include <cstddef>
#include <limits>
#include <type_traits>

// Uncomment to enable runtime precondition checks in vs::span. They are expensive and thus are not enabled by default even in debug builds.
// #define VS_ENABLE_SPAN_CHECKS

#if defined(VS_ENABLE_SPAN_CHECKS)
#	include <cassert>
#	define VS_SPAN_EXPECT(...) assert(__VA_ARGS__)
#else
#	define VS_SPAN_EXPECT(...) (void)0
#endif

#if __cplusplus >= 201402L
#	define constexpr_14 constexpr
#else
#	define constexpr_14
#endif

namespace vs {

static constexpr std::size_t dynamic_extent = std::numeric_limits<std::size_t>::max();

template <class ElementType, std::size_t Extent = dynamic_extent>
class span;

namespace detail {

// False when T is a non-contiguous iterator, true otherwise.
// We use this to catch obvious errors (like trying to make a span from std::list or std::set).
template <class T, class = void> struct maybe_contiguous_iterator : std::true_type {};
template <class T> struct maybe_contiguous_iterator<T, void_t<typename std::iterator_traits<T>::iterator_category>>
	: std::is_base_of<std::random_access_iterator_tag, typename std::iterator_traits<T>::iterator_category> {};

template <class T> struct is_span : std::false_type {};
template <class T, std::size_t N> struct is_span<span<T, N>> : std::true_type {};
template <class T> struct is_span<const T> : is_span<T> {};
template <class T> struct is_span<volatile T> : is_span<T> {};
template <class T> struct is_span<const volatile T> : is_span<T> {};

template <class T> struct is_std_array : std::false_type {};
template <class T, std::size_t N> struct is_std_array<std::array<T, N>> : std::true_type {};
template <class T> struct is_std_array<const T> : is_std_array<T> {};
template <class T> struct is_std_array<volatile T> : is_std_array<T> {};
template <class T> struct is_std_array<const volatile T> : is_std_array<T> {};

// In some places we need to make unqualified calls to data() and size() to allow finding matching functions via ADL.
// We don't want to pollute vs::detail namespace, so we create dedicated namespaces for that.
namespace adl_data {
	using vs::data;
	template <class T>
	constexpr auto impl(T&& x) -> decltype(data(std::forward<T>(x)))
	{
		return data(std::forward<T>(x));
	}
}
namespace adl_size {
	using vs::size;
	template <class T>
	constexpr auto impl(T&& x) -> decltype(size(std::forward<T>(x)))
	{
		return size(std::forward<T>(x));
	}
}

template <class ElementType, bool dynamic = false>
struct span_base
{
	ElementType* data_;
	constexpr span_base(ElementType* data, std::size_t) : data_(data) {}
};
template <class ElementType>
struct span_base<ElementType, true>
{
	ElementType* data_;
	std::size_t size_;
};

}

// std::span implementation based on N4849.
template <class ElementType, std::size_t Extent>
class span : private detail::span_base<ElementType, Extent == dynamic_extent>
{
	using base_t = detail::span_base<ElementType, Extent == dynamic_extent>;

	template <class T>
	using is_compatible_element_type = std::is_convertible<T (*)[], ElementType (*)[]>;
	template <class T>
	using is_compatible_iterator = is_compatible_element_type<typename std::remove_reference<iter_reference_t<T>>::type>;

public:
	using element_type = ElementType;
	using value_type = typename std::remove_cv<ElementType>::type;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using pointer = element_type*;
	using const_pointer = const element_type*;
	using reference = element_type&;
	using const_reference = const element_type&;
	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	static constexpr size_type extent = Extent;

	template <int Dummy = 0, class = typename std::enable_if<(Dummy, Extent == 0 || Extent == dynamic_extent)>::type>
	constexpr span() noexcept
		: base_t { nullptr, 0 } {}

	template <class It, class = typename std::enable_if<
		detail::maybe_contiguous_iterator<It>::value // Instead of "It satisfies contiguous_iterator"
		&& is_compatible_iterator<It>::value
	>::type>
	constexpr span(It first, size_type count)
		: base_t { vs::to_address(first), (VS_SPAN_EXPECT((Extent == dynamic_extent || count == Extent) && "Invalid size"), count) } {}

	template <class It, class End, class = typename std::enable_if<
		detail::maybe_contiguous_iterator<It>::value // Instead of "It satisfies contiguous_iterator"
		&& is_compatible_iterator<It>::value
		// Constraint "End satisfies sized_sentinel_for<It>" expands into 6 following conditions, we only care about one of them.
		// && std::is_convertible<decltype(std::declval<const It&> () == std::declval<const End&>()), bool>::value
		// && std::is_convertible<decltype(std::declval<const It&> () != std::declval<const End&>()), bool>::value
		// && std::is_convertible<decltype(std::declval<const End&>() == std::declval<const It&> ()), bool>::value
		// && std::is_convertible<decltype(std::declval<const End&>() != std::declval<const It&> ()), bool>::value
		&& std::is_same<decltype(std::declval<const End&>() - std::declval<const It&> ()), iter_difference_t<It>>::value
		// && std::is_same<decltype(std::declval<const It&> () - std::declval<const End&>()), iter_difference_t<It>>::value
		&& !std::is_convertible<End, std::size_t>::value
	>::type>
	constexpr span(It first, End last)
		: base_t { vs::to_address(first),
			(
				VS_SPAN_EXPECT((Extent == dynamic_extent || static_cast<size_type>(last - first) == Extent) && "Invalid size"),
				static_cast<size_type>(last - first)
			)
		} {}

	template <std::size_t N, class = typename std::enable_if<
		(Extent == dynamic_extent || N == Extent)
	>::type>
	constexpr span(element_type (&arr)[N]) noexcept
		: base_t { arr, N } {}
	template <std::size_t N, class = typename std::enable_if<
		(Extent == dynamic_extent || N == Extent)
	>::type>
	constexpr span(std::array<value_type, N>& arr) noexcept
		: base_t { arr.data(), N } {}
	template <std::size_t N, class = typename std::enable_if<
		(Extent == dynamic_extent || N == Extent)
		&& std::is_const<ElementType>::value
	>::type>
	constexpr span(const std::array<value_type, N>& arr) noexcept
		: base_t { arr.data(), N } {}

	// This is the old constructor (from N4835) that supports containers instead of ranges.
	// Since we don't have ranges yet there is no point in having the version that supports them.
	// Implementing the
	template <class Container, class = typename std::enable_if<
		Extent == dynamic_extent
		&& !detail::is_span<typename std::remove_reference<Container>::type>::value
		&& !detail::is_std_array<typename std::remove_reference<Container>::type>::value
		&& !std::is_array<typename std::remove_reference<Container>::type>::value
		&& is_compatible_element_type<typename std::remove_pointer<decltype(detail::adl_data::impl(std::declval<Container&>()))>::type>::value
		&& std::is_integral<decltype(detail::adl_size::impl(std::declval<Container&>()))>::value // Strengthened from "size(cont) in well-formed"
	>::type>
	constexpr span(Container&& r)
		: base_t { detail::adl_data::impl(r), detail::adl_size::impl(r) } {}

	constexpr span(const span& other) noexcept = default;

	template <class OtherElementType, std::size_t OtherExtent, class = typename std::enable_if<
		(Extent == dynamic_extent || Extent == OtherExtent)
		&& is_compatible_element_type<OtherElementType>::value
	>::type>
	constexpr span(const span<OtherElementType, OtherExtent>& s) noexcept
		: base_t { s.data(), s.size() } {}

	~span() noexcept = default;
	constexpr_14 span& operator=(const span& other) noexcept = default;

	template <std::size_t Count, typename std::enable_if<(Count, Extent != dynamic_extent), int>::type = 0>
	constexpr span<element_type, Count> first() const
	{
		static_assert(Count <= Extent, "Can't increase span size");
		return { this->data_, Count };
	}
	template <std::size_t Count, typename std::enable_if<(Count, Extent == dynamic_extent), int>::type = 0>
	constexpr span<element_type, Count> first() const
	{
		return { this->data_, (VS_SPAN_EXPECT(Count <= size() && "Can't increase span size"), Count) };
	}

	template <std::size_t Count, typename std::enable_if<(Count, Extent != dynamic_extent), int>::type = 0>
	constexpr span<element_type, Count> last() const
	{
		static_assert(Count <= Extent, "Can't increase span size");
		return { this->data_ + (size() - Count), Count };
	}
	template <std::size_t Count, typename std::enable_if<(Count, Extent == dynamic_extent), int>::type = 0>
	constexpr span<element_type, Count> last() const
	{
		return { this->data_ + (size() - Count), (VS_SPAN_EXPECT(Count <= size() && "Can't increase span size"), Count) };
	}

	template <std::size_t Offset, std::size_t Count = dynamic_extent, typename std::enable_if<(Count, Extent != dynamic_extent), int>::type = 0>
	constexpr span<element_type, (Count != dynamic_extent ? Count : Extent - Offset)> subspan() const
	{
		static_assert(Offset <= Extent, "Offset is out of bounds");
		static_assert(Count == dynamic_extent || Offset + Count <= Extent, "Can't increase span size");
		return { this->data_ + Offset, Count != dynamic_extent ? Count : size() - Offset };
	}
	template <std::size_t Offset, std::size_t Count = dynamic_extent, typename std::enable_if<(Count, Extent == dynamic_extent), int>::type = 0>
	constexpr span<element_type, (Count != dynamic_extent ? Count : dynamic_extent)> subspan() const
	{
		return { this->data_ + Offset,
			(
				VS_SPAN_EXPECT(Offset <= size() && "Offset is out of bounds"),
				VS_SPAN_EXPECT((Count == dynamic_extent || Offset + Count <= size()) && "Can't increase span size"),
				Count != dynamic_extent ? Count : size() - Offset
			)
		};
	}

	constexpr span<element_type, dynamic_extent> first(size_type count) const
	{
		return { this->data_, (VS_SPAN_EXPECT(count <= size() && "Can't increase span size"), count) };
	}
	constexpr span<element_type, dynamic_extent> last(size_type count) const
	{
		return { this->data_ + (size() - count), (VS_SPAN_EXPECT(count <= size() && "Can't increase span size"), count) };
	}
	constexpr span<element_type, dynamic_extent> subspan(size_type offset, size_type count = dynamic_extent) const
	{
		return { this->data_ + offset,
			(
				VS_SPAN_EXPECT(offset <= size() && "Offset is out of bounds"),
				VS_SPAN_EXPECT((count == dynamic_extent || offset + count <= size()) && "Can't increase span size"),
				count != dynamic_extent ? count : size() - offset
			)
		};
	}

	template <int Dummy = 0, typename std::enable_if<(Dummy, Extent != dynamic_extent), int>::type = 0>
	constexpr size_type size() const noexcept { return Extent; }
	template <int Dummy = 0, typename std::enable_if<(Dummy, Extent == dynamic_extent), int>::type = 0>
	constexpr size_type size() const noexcept { return this->size_; }

	constexpr size_type size_bytes() const noexcept { return sizeof(element_type) * size(); }
	VS_NODISCARD constexpr bool empty() const noexcept { return size() == 0; }

	constexpr reference operator[](size_type idx) const
	{
		return (VS_SPAN_EXPECT(idx < size() && "Index if out of range"), this->data_[idx]);
	}
	template <int Dummy = 0, typename std::enable_if<(Dummy, Extent != dynamic_extent), int>::type = 0>
	constexpr reference front() const
	{
		static_assert(Extent != 0, "Span is empty");
		return this->data_[0];
	}
	template <int Dummy = 0, typename std::enable_if<(Dummy, Extent == dynamic_extent), int>::type = 0>
	constexpr reference front() const
	{
		return (VS_SPAN_EXPECT(size() != 0 && "Span is empty"), this->data_[0]);
	}
	template <int Dummy = 0, typename std::enable_if<(Dummy, Extent != dynamic_extent), int>::type = 0>
	constexpr reference back() const
	{
		static_assert(Extent != 0, "Span is empty");
		return this->data_[size() - 1];
	}
	template <int Dummy = 0, typename std::enable_if<(Dummy, Extent == dynamic_extent), int>::type = 0>
	constexpr reference back() const
	{
		return (VS_SPAN_EXPECT(size() != 0 && "Span is empty"), this->data_[size() - 1]);
	}
	constexpr pointer data() const noexcept { return this->data_; }

	constexpr       iterator  begin() const noexcept { return this->data_; }
	constexpr       iterator  end()   const noexcept { return this->data_ + size(); }
	constexpr const_iterator cbegin() const noexcept { return this->data_; }
	constexpr const_iterator cend()   const noexcept { return this->data_ + size(); }

	constexpr       reverse_iterator  rbegin() const noexcept { return { this->data_ + size() }; }
	constexpr       reverse_iterator  rend()   const noexcept { return { this->data_ }; }
	constexpr const_reverse_iterator crbegin() const noexcept { return { this->data_ + size() }; }
	constexpr const_reverse_iterator crend()   const noexcept { return { this->data_ }; }
};

// Implementation of as_bytes and as_writable_bytes. We can't enable it because we don't have std::byte.
#if 0
template <class ElementType, std::size_t Extent>
span<const std::byte, Extent == dynamic_extent ? dynamic_extent : sizeof(ElementType) * Extent>
as_bytes(span<ElementType, Extent> s) noexcept
{
	return { reinterpret_cast<const std::byte*>(s.data()), s.size_bytes() };
}

template <class ElementType, std::size_t Extent, class = typename std::enable_if<!std::is_const<ElementType>::value>::type>
span<std::byte, Extent == dynamic_extent ? dynamic_extent : sizeof(ElementType) * Extent>
as_writable_bytes(span<ElementType, Extent> s) noexcept
{
	return { reinterpret_cast<std::byte*>(s.data()), s.size_bytes() };
}
#endif

template <std::size_t I, class ElementType, std::size_t Extent>
constexpr ElementType& get(vs::span<ElementType, Extent> s) noexcept
{
	return s[I];
}

}

namespace std {

template <class ElementType, size_t Extent>
struct tuple_size<vs::span<ElementType, Extent>> : std::integral_constant<std::size_t, Extent> {};
template <class ElementType>
struct tuple_size<vs::span<ElementType, vs::dynamic_extent>>; // not defined

template <size_t I, class ElementType, size_t Extent>
struct tuple_element<I, vs::span<ElementType, Extent>>
{
	static_assert(Extent != vs::dynamic_extent, "std::tuple_element is not defined for dynamically sized span");
	static_assert(I < Extent, "Index is out of range");
	using type = ElementType;
};

}

#undef VS_SPAN_EXPECT
#undef constexpr_14

#endif

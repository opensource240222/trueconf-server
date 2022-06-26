#include "std-generic/compat/span.h"

#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 7000 && _LIBCPP_STD_VER > 17) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 100000 && __cplusplus > 201703L) \
 || 0
#else

#include <gtest/gtest.h>

#include <list>
#include <type_traits>
#include <vector>

namespace span_tests {

struct Type
{
	Type();
	Type(const Type&);
	Type(Type&&);
	Type& operator=(const Type&);
	Type& operator=(Type&&);
	~Type() noexcept(false);

	bool operator==(const Type&) const;
};
struct Derived : Type {};

template <class Iterator>
struct Sentinel
{
	using value_type = typename std::iterator_traits<Iterator>::value_type;

	constexpr Sentinel() : end_() {};
	constexpr explicit Sentinel(value_type end) : end_(end) {};

	friend constexpr bool operator==(Sentinel<Iterator> s, Iterator it)
	{
		return s.end_ == *it;
	}
	friend constexpr bool operator==(Iterator it, Sentinel<Iterator> s)
	{
		return *it == s.end_;
	}
	friend constexpr bool operator!=(Sentinel<Iterator> s, Iterator it)
	{
		return !(s == it);
	}
	friend constexpr bool operator!=(Iterator it, Sentinel<Iterator> s)
	{
		return !(it == s);
	}

	friend constexpr typename std::iterator_traits<Iterator>::difference_type operator-(Sentinel<Iterator> s, Iterator it)
	{
		return *it == s.end_ ? 0 : 1 + (s - (it + 1));
	}

private:
	value_type end_;
};


// Default constructor is present only if Extent == 0 || Extent == dynamic_extent
static_assert( std::is_nothrow_default_constructible<vs::span<Type, 0>>::value, "");
static_assert(!std::is_default_constructible        <vs::span<Type, 1>>::value, "");
static_assert( std::is_nothrow_default_constructible<vs::span<Type   >>::value, "");

// Triviality of special member functions
static_assert( std::is_trivially_copy_constructible<vs::span<Type, 1>>::value, "");
static_assert( std::is_trivially_copy_assignable<vs::span<Type, 1>>::value, "");
static_assert( std::is_trivially_destructible<vs::span<Type, 1>>::value, "");

static_assert( std::is_trivially_copy_constructible<vs::span<Type>>::value, "");
static_assert( std::is_trivially_copy_assignable<vs::span<Type>>::value, "");
static_assert( std::is_trivially_destructible<vs::span<Type>>::value, "");

// Construction from C array
static_assert( std::is_nothrow_constructible<vs::span<      Type, 1>,       Type (&)[1]>::value, "");
static_assert(!std::is_constructible        <vs::span<      Type, 1>, const Type (&)[1]>::value, "");
static_assert( std::is_nothrow_constructible<vs::span<const Type, 1>,       Type (&)[1]>::value, "");
static_assert( std::is_nothrow_constructible<vs::span<const Type, 1>, const Type (&)[1]>::value, "");

static_assert( std::is_nothrow_constructible<vs::span<      Type   >,       Type (&)[1]>::value, "");
static_assert(!std::is_constructible        <vs::span<      Type   >, const Type (&)[1]>::value, "");
static_assert( std::is_nothrow_constructible<vs::span<const Type   >,       Type (&)[1]>::value, "");
static_assert( std::is_nothrow_constructible<vs::span<const Type   >, const Type (&)[1]>::value, "");

// Construction from std::array
static_assert( std::is_nothrow_constructible<vs::span<      Type, 1>,       std::array<Type, 1>&>::value, "");
static_assert(!std::is_constructible        <vs::span<      Type, 1>, const std::array<Type, 1>&>::value, "");
static_assert( std::is_nothrow_constructible<vs::span<const Type, 1>,       std::array<Type, 1>&>::value, "");
static_assert( std::is_nothrow_constructible<vs::span<const Type, 1>, const std::array<Type, 1>&>::value, "");

static_assert( std::is_nothrow_constructible<vs::span<      Type   >,       std::array<Type, 1>&>::value, "");
static_assert(!std::is_constructible        <vs::span<      Type   >, const std::array<Type, 1>&>::value, "");
static_assert( std::is_nothrow_constructible<vs::span<const Type   >,       std::array<Type, 1>&>::value, "");
static_assert( std::is_nothrow_constructible<vs::span<const Type   >, const std::array<Type, 1>&>::value, "");

// Can't construct from array of a different type
static_assert(!std::is_constructible<vs::span<Type, 1>, int (&)[1]>::value, "");
static_assert(!std::is_constructible<vs::span<Type, 1>, Derived (&)[1]>::value, "");
static_assert(!std::is_constructible<vs::span<Type, 1>, std::array<int, 1>&>::value, "");
static_assert(!std::is_constructible<vs::span<Type, 1>, std::array<Derived, 1>&>::value, "");

static_assert(!std::is_constructible<vs::span<Type   >, int (&)[1]>::value, "");
static_assert(!std::is_constructible<vs::span<Type   >, Derived (&)[1]>::value, "");
static_assert(!std::is_constructible<vs::span<Type   >, std::array<int, 1>&>::value, "");
static_assert(!std::is_constructible<vs::span<Type   >, std::array<Derived, 1>&>::value, "");

// Can't construct from array of a different size
static_assert(!std::is_constructible<vs::span<Type, 1>, Type (&)[2]>::value, "");
static_assert(!std::is_constructible<vs::span<Type, 1>, std::array<Type, 2>&>::value, "");

// Construction from span
// Cases with matching const qualifiers are handled by the copy constructor.
static_assert(!std::is_constructible        <vs::span<      Type, 1>, vs::span<const Type, 1>>::value, "");
static_assert( std::is_nothrow_constructible<vs::span<const Type, 1>, vs::span<      Type, 1>>::value, "");

static_assert(!std::is_constructible        <vs::span<      Type   >, vs::span<const Type   >>::value, "");
static_assert( std::is_nothrow_constructible<vs::span<const Type   >, vs::span<      Type   >>::value, "");

// Can't construct from span of a different type
static_assert(!std::is_constructible<vs::span<Type, 1>, vs::span<int, 1>>::value, "");
static_assert(!std::is_constructible<vs::span<Type, 1>, vs::span<Derived, 1>>::value, "");

static_assert(!std::is_constructible<vs::span<Type   >, vs::span<int, 1>>::value, "");
static_assert(!std::is_constructible<vs::span<Type   >, vs::span<Derived, 1>>::value, "");

// Can't construct from span of a different size
static_assert(!std::is_constructible<vs::span<Type, 1>, vs::span<Type, 2>>::value, "");

// Can construct a span with dynamic size from a span with a fixed size, but not vice versa.
static_assert(!std::is_constructible        <vs::span<Type, 1>, vs::span<Type   >>::value, "");
static_assert( std::is_nothrow_constructible<vs::span<Type   >, vs::span<Type, 1>>::value, "");

// Construction from pointer and size
static_assert( std::is_constructible<vs::span<      Type, 1>,       Type*, size_t>::value, "");
static_assert(!std::is_constructible<vs::span<      Type, 1>, const Type*, size_t>::value, "");
static_assert( std::is_constructible<vs::span<const Type, 1>,       Type*, size_t>::value, "");
static_assert( std::is_constructible<vs::span<const Type, 1>, const Type*, size_t>::value, "");

static_assert( std::is_constructible<vs::span<      Type   >,       Type*, size_t>::value, "");
static_assert(!std::is_constructible<vs::span<      Type   >, const Type*, size_t>::value, "");
static_assert( std::is_constructible<vs::span<const Type   >,       Type*, size_t>::value, "");
static_assert( std::is_constructible<vs::span<const Type   >, const Type*, size_t>::value, "");

// Construction from contiguous iterator and size
static_assert( std::is_constructible<vs::span<      Type, 1>, std::vector<Type>::      iterator, size_t>::value, "");
static_assert(!std::is_constructible<vs::span<      Type, 1>, std::vector<Type>::const_iterator, size_t>::value, "");
static_assert( std::is_constructible<vs::span<const Type, 1>, std::vector<Type>::      iterator, size_t>::value, "");
static_assert( std::is_constructible<vs::span<const Type, 1>, std::vector<Type>::const_iterator, size_t>::value, "");

static_assert( std::is_constructible<vs::span<      Type   >, std::vector<Type>::      iterator, size_t>::value, "");
static_assert(!std::is_constructible<vs::span<      Type   >, std::vector<Type>::const_iterator, size_t>::value, "");
static_assert( std::is_constructible<vs::span<const Type   >, std::vector<Type>::      iterator, size_t>::value, "");
static_assert( std::is_constructible<vs::span<const Type   >, std::vector<Type>::const_iterator, size_t>::value, "");

// Construction from a pair of pointers
static_assert( std::is_constructible<vs::span<      Type, 1>,       Type*, const Type*>::value, "");
static_assert(!std::is_constructible<vs::span<      Type, 1>, const Type*, const Type*>::value, "");
static_assert( std::is_constructible<vs::span<const Type, 1>,       Type*, const Type*>::value, "");
static_assert( std::is_constructible<vs::span<const Type, 1>, const Type*, const Type*>::value, "");

static_assert( std::is_constructible<vs::span<      Type   >,       Type*, const Type*>::value, "");
static_assert(!std::is_constructible<vs::span<      Type   >, const Type*, const Type*>::value, "");
static_assert( std::is_constructible<vs::span<const Type   >,       Type*, const Type*>::value, "");
static_assert( std::is_constructible<vs::span<const Type   >, const Type*, const Type*>::value, "");

// Construction from contiguous iterator and a sentinel
static_assert( std::is_constructible<vs::span<      Type, 1>,       Type*, Sentinel<const Type*>>::value, "");
static_assert(!std::is_constructible<vs::span<      Type, 1>, const Type*, Sentinel<const Type*>>::value, "");
static_assert( std::is_constructible<vs::span<const Type, 1>,       Type*, Sentinel<const Type*>>::value, "");
static_assert( std::is_constructible<vs::span<const Type, 1>, const Type*, Sentinel<const Type*>>::value, "");

static_assert( std::is_constructible<vs::span<      Type   >,       Type*, Sentinel<const Type*>>::value, "");
static_assert(!std::is_constructible<vs::span<      Type   >, const Type*, Sentinel<const Type*>>::value, "");
static_assert( std::is_constructible<vs::span<const Type   >,       Type*, Sentinel<const Type*>>::value, "");
static_assert( std::is_constructible<vs::span<const Type   >, const Type*, Sentinel<const Type*>>::value, "");

// Construction from a pair of iterators
static_assert( std::is_constructible<vs::span<      Type, 1>, std::vector<Type>::      iterator, std::vector<Type>::const_iterator>::value, "");
static_assert(!std::is_constructible<vs::span<      Type, 1>, std::vector<Type>::const_iterator, std::vector<Type>::const_iterator>::value, "");
static_assert( std::is_constructible<vs::span<const Type, 1>, std::vector<Type>::      iterator, std::vector<Type>::const_iterator>::value, "");
static_assert( std::is_constructible<vs::span<const Type, 1>, std::vector<Type>::const_iterator, std::vector<Type>::const_iterator>::value, "");

static_assert( std::is_constructible<vs::span<      Type   >, std::vector<Type>::      iterator, std::vector<Type>::const_iterator>::value, "");
static_assert(!std::is_constructible<vs::span<      Type   >, std::vector<Type>::const_iterator, std::vector<Type>::const_iterator>::value, "");
static_assert( std::is_constructible<vs::span<const Type   >, std::vector<Type>::      iterator, std::vector<Type>::const_iterator>::value, "");
static_assert( std::is_constructible<vs::span<const Type   >, std::vector<Type>::const_iterator, std::vector<Type>::const_iterator>::value, "");

// Can't construct from non-contiguous iterator
static_assert(!std::is_constructible<vs::span<Type, 1>, std::list<Type>::iterator, size_t>::value, "");
static_assert(!std::is_constructible<vs::span<Type, 1>, std::list<Type>::iterator, std::list<Type>::iterator>::value, "");

static_assert(!std::is_constructible<vs::span<Type   >, std::list<Type>::iterator, size_t>::value, "");
static_assert(!std::is_constructible<vs::span<Type   >, std::list<Type>::iterator, std::list<Type>::iterator>::value, "");

// Can't construct from non-matching iterators
static_assert(!std::is_constructible<vs::span<Type, 1>, std::vector<Type>::iterator, std::vector<int>::iterator>::value, "");
static_assert(!std::is_constructible<vs::span<Type, 1>, std::vector<Type>::iterator, Type*>::value, "");

static_assert(!std::is_constructible<vs::span<Type   >, std::vector<Type>::iterator, std::vector<int>::iterator>::value, "");
static_assert(!std::is_constructible<vs::span<Type   >, std::vector<Type>::iterator, Type*>::value, "");

// Can't construct from iterator with different value_type
static_assert(!std::is_constructible<vs::span<Type, 1>, Derived*, size_t>::value, "");
static_assert(!std::is_constructible<vs::span<Type, 1>, std::vector<Derived>::iterator, size_t>::value, "");
static_assert(!std::is_constructible<vs::span<Type, 1>, Derived*, Derived*>::value, "");
static_assert(!std::is_constructible<vs::span<Type, 1>, std::vector<Derived>::iterator, std::vector<Derived>::iterator>::value, "");

static_assert(!std::is_constructible<vs::span<Type   >, Derived*, size_t>::value, "");
static_assert(!std::is_constructible<vs::span<Type   >, std::vector<Derived>::iterator, size_t>::value, "");
static_assert(!std::is_constructible<vs::span<Type   >, Derived*, Derived*>::value, "");
static_assert(!std::is_constructible<vs::span<Type   >, std::vector<Derived>::iterator, std::vector<Derived>::iterator>::value, "");

// Construction from contiguous range
static_assert( std::is_constructible<vs::span<      Type>,       std::vector<Type>>::value, "");
static_assert(!std::is_constructible<vs::span<      Type>, const std::vector<Type>>::value, "");
static_assert( std::is_constructible<vs::span<const Type>,       std::vector<Type>>::value, "");
static_assert( std::is_constructible<vs::span<const Type>, const std::vector<Type>>::value, "");

static_assert(!std::is_constructible<vs::span<      Type>,       std::initializer_list<Type>>::value, "");
static_assert(!std::is_constructible<vs::span<      Type>, const std::initializer_list<Type>>::value, "");
static_assert( std::is_constructible<vs::span<const Type>,       std::initializer_list<Type>>::value, "");
static_assert( std::is_constructible<vs::span<const Type>, const std::initializer_list<Type>>::value, "");

// Can't construct from non-contiguous range
static_assert(!std::is_constructible<vs::span<Type>, std::list<Type>>::value, "");

// Sub-span with static length
static_assert(std::is_same<vs::span<Type, 2>, decltype(std::declval<vs::span<Type, 5>>().first<2>())>::value, "");
static_assert(std::is_same<vs::span<Type, 2>, decltype(std::declval<vs::span<Type, 5>>().last<2>())>::value, "");
static_assert(std::is_same<vs::span<Type, 2>, decltype(std::declval<vs::span<Type, 5>>().subspan<3>())>::value, "");
static_assert(std::is_same<vs::span<Type, 2>, decltype(std::declval<vs::span<Type, 5>>().subspan<1, 2>())>::value, "");

int values[] = { 2, 3, 5 };

TEST(Span, DefaultConstructed)
{
	constexpr vs::span<int> s;
	EXPECT_EQ(s.size(), 0u);
}

TEST(Span, DefaultConstructed_Fixed)
{
	constexpr vs::span<int, 0> s;
	EXPECT_EQ(s.size(), 0u);
}

TEST(Span, FromIteratorAndSize)
{
	constexpr vs::span<int> s(values + 1, 2);
	EXPECT_EQ(s.size(), 2u);
	ASSERT_EQ(s.data(), values + 1);
	EXPECT_EQ(s[0], 3);
	EXPECT_EQ(s[1], 5);
}

TEST(Span, FromIteratorAndSize_Fixed)
{
	constexpr vs::span<int, 2> s(values + 1, 2);
	EXPECT_EQ(s.size(), 2u);
	ASSERT_EQ(s.data(), values + 1);
	EXPECT_EQ(s[0], 3);
	EXPECT_EQ(s[1], 5);
}

TEST(Span, FromIteratorPair)
{
	constexpr vs::span<int> s(values + 1, values + (sizeof(values)/sizeof(values[0])));
	EXPECT_EQ(s.size(), 2u);
	ASSERT_EQ(s.data(), values + 1);
	EXPECT_EQ(s[0], 3);
	EXPECT_EQ(s[1], 5);
}

TEST(Span, FromIteratorPair_Fixed)
{
	constexpr vs::span<int, 2> s(values + 1, values + (sizeof(values)/sizeof(values[0])));
	EXPECT_EQ(s.size(), 2u);
	ASSERT_EQ(s.data(), values + 1);
	EXPECT_EQ(s[0], 3);
	EXPECT_EQ(s[1], 5);
}

TEST(Span, FromIteratorAndSentinel)
{
	vs::span<int> s(values, Sentinel<const int*>(5));
	EXPECT_EQ(s.size(), 2u);
	ASSERT_EQ(s.data(), values);
	EXPECT_EQ(s[0], 2);
	EXPECT_EQ(s[1], 3);
}

TEST(Span, FromIteratorAndSentinel_Fixed)
{
	vs::span<int, 2> s(values, Sentinel<const int*>(5));
	EXPECT_EQ(s.size(), 2u);
	ASSERT_EQ(s.data(), values);
	EXPECT_EQ(s[0], 2);
	EXPECT_EQ(s[1], 3);
}

TEST(Span, FromCArray)
{
	constexpr vs::span<int> s(values);
	EXPECT_EQ(s.size(), 3u);
	ASSERT_EQ(s.data(), values);
	EXPECT_EQ(s[0], 2);
	EXPECT_EQ(s[1], 3);
	EXPECT_EQ(s[2], 5);
}

TEST(Span, FromCArray_Fixed)
{
	constexpr vs::span<int, 3> s(values);
	EXPECT_EQ(s.size(), 3u);
	ASSERT_EQ(s.data(), values);
	EXPECT_EQ(s[0], 2);
	EXPECT_EQ(s[1], 3);
	EXPECT_EQ(s[2], 5);
}

TEST(Span, FromStdArray)
{
	std::array<int, 3> values { 11, 13, 17 };
	vs::span<int> s(values);
	EXPECT_EQ(s.size(), 3u);
	ASSERT_EQ(s.data(), values.data());
	EXPECT_EQ(s[0], 11);
	EXPECT_EQ(s[1], 13);
	EXPECT_EQ(s[2], 17);
}

TEST(Span, FromStdArray_Fixed)
{
	std::array<int, 3> values { 11, 13, 17 };
	vs::span<int, 3> s(values);
	EXPECT_EQ(s.size(), 3u);
	ASSERT_EQ(s.data(), values.data());
	EXPECT_EQ(s[0], 11);
	EXPECT_EQ(s[1], 13);
	EXPECT_EQ(s[2], 17);
}

TEST(Span, FromVector)
{
	std::vector<int> values { 23, 29, 31 };
	vs::span<int> s(values);
	EXPECT_EQ(s.size(), 3u);
	ASSERT_EQ(s.data(), values.data());
	EXPECT_EQ(s[0], 23);
	EXPECT_EQ(s[1], 29);
	EXPECT_EQ(s[2], 31);
}

TEST(Span, FromInitializerList)
{
	std::initializer_list<int> values { 41, 43, 47 };
	vs::span<const int> s(values);
	EXPECT_EQ(s.size(), 3u);
	ASSERT_EQ(s.data(), values.begin());
	EXPECT_EQ(s[0], 41);
	EXPECT_EQ(s[1], 43);
	EXPECT_EQ(s[2], 47);
}

TEST(Span, First)
{
	constexpr auto s = vs::span<int>(values).first(2);
	EXPECT_EQ(s.size(), 2u);
	ASSERT_EQ(s.data(), values);
	EXPECT_EQ(s[0], 2);
	EXPECT_EQ(s[1], 3);
}

TEST(Span, First_Fixed)
{
	constexpr auto s = vs::span<int>(values).first<2>();
	EXPECT_EQ(s.size(), 2u);
	ASSERT_EQ(s.data(), values);
	EXPECT_EQ(s[0], 2);
	EXPECT_EQ(s[1], 3);
}

TEST(Span, Last)
{
	constexpr auto s = vs::span<int>(values).last(2);
	EXPECT_EQ(s.size(), 2u);
	ASSERT_EQ(s.data(), values + 1);
	EXPECT_EQ(s[0], 3);
	EXPECT_EQ(s[1], 5);
}

TEST(Span, Last_Fixed)
{
	constexpr auto s = vs::span<int>(values).last<2>();
	EXPECT_EQ(s.size(), 2u);
	ASSERT_EQ(s.data(), values + 1);
	EXPECT_EQ(s[0], 3);
	EXPECT_EQ(s[1], 5);
}

TEST(Span, Subspan)
{
	constexpr auto s = vs::span<int>(values).subspan(1, 1);
	EXPECT_EQ(s.size(), 1u);
	ASSERT_EQ(s.data(), values + 1);
	EXPECT_EQ(s[0], 3);
}

TEST(Span, Subspan_Fixed)
{
	constexpr auto s = vs::span<int>(values).subspan<1, 1>();
	EXPECT_EQ(s.size(), 1u);
	ASSERT_EQ(s.data(), values + 1);
	EXPECT_EQ(s[0], 3);
}

namespace constexpr_test {

	constexpr int values[] = { 53, 59, 61 };
	constexpr vs::span<const int> s(values);
	static_assert(s.size() == 3, "");
	static_assert(s.empty() == false, "");
	static_assert(s.data() == values, "");
	static_assert(s[0] == 53, "");
	static_assert(s[1] == 59, "");
	static_assert(s[2] == 61, "");
	static_assert(s.front() == 53, "");
	static_assert(s.back() == 61, "");
	static_assert(s.first<2>()[1] == 59, "");
	static_assert(s.first(2)[1] == 59, "");
	static_assert(s.last<2>()[1] == 61, "");
	static_assert(s.last(2)[1] == 61, "");
	static_assert(s.subspan<1, 1>()[0] == 59, "");
	static_assert(s.subspan(1, 1)[0] == 59, "");

}

namespace constexpr_test_fixed {

	constexpr int values[] = { 53, 59, 61 };
	constexpr vs::span<const int, 3> s(values);
	static_assert(s.size() == 3, "");
	static_assert(s.empty() == false, "");
	static_assert(s.data() == values, "");
	static_assert(s[0] == 53, "");
	static_assert(s[1] == 59, "");
	static_assert(s[2] == 61, "");
	static_assert(s.front() == 53, "");
	static_assert(s.back() == 61, "");
	static_assert(s.first<2>()[1] == 59, "");
	static_assert(s.first(2)[1] == 59, "");
	static_assert(s.last<2>()[1] == 61, "");
	static_assert(s.last(2)[1] == 61, "");
	static_assert(s.subspan<1, 1>()[0] == 59, "");
	static_assert(s.subspan(1, 1)[0] == 59, "");

}

}

#endif

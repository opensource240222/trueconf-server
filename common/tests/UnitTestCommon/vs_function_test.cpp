#include "std/cpplib/function.h"

#include <functional>
#include <iostream>

#include <gtest/gtest.h>

namespace vs_function_test {

template <size_t size>
class traced_object
{
	char payload[size];
public:
	static unsigned default_constructed;
	static unsigned copy_constructed;
	static unsigned move_constructed;
	static unsigned destroyed;
	static unsigned copy_assigned;
	static unsigned move_assigned;
	static void reset_counters()
	{
		default_constructed = 0;
		copy_constructed = 0;
		move_constructed = 0;
		destroyed = 0;
		copy_assigned = 0;
		move_assigned = 0;
	}
	static int active_instances()
	{
		return int(default_constructed + copy_constructed + move_constructed) - int(destroyed);
	}

	traced_object() { ++default_constructed; }
	traced_object(const traced_object&) { ++copy_constructed; }
	traced_object(traced_object&&) { ++move_constructed; }
	~traced_object() { ++destroyed; }
	traced_object& operator=(const traced_object&) { ++copy_assigned; return *this; }
	traced_object& operator=(traced_object&&) { ++move_assigned; return *this; }
};

template <size_t size> unsigned traced_object<size>::default_constructed;
template <size_t size> unsigned traced_object<size>::copy_constructed;
template <size_t size> unsigned traced_object<size>::move_constructed;
template <size_t size> unsigned traced_object<size>::destroyed;
template <size_t size> unsigned traced_object<size>::copy_assigned;
template <size_t size> unsigned traced_object<size>::move_assigned;

template <size_t size>
class traced_callable : public traced_object<size>
{
	struct pmd_helper
	{
		operator int() { ++pmd_calls; return 42; }
	};
public:
	static unsigned functor_calls;
	static unsigned pmf_calls;
	static unsigned pmd_calls;
	static void reset_counters()
	{
		traced_object<size>::reset_counters();
		functor_calls = 0;
		pmf_calls = 0;
		pmd_calls = 0;
	}

	int operator()(int x) { ++functor_calls; return x*2; }
	int function(int x) { ++pmf_calls; return x*2;}
	pmd_helper variable;

#if defined(_MSC_VER) && _MSC_VER < 1900
	// MSVC12 doesn't generate move ctor/operator= for some stupid reason, nor does it allow to =default it.
	// So I have to specify all this stuff explicitly...
	traced_callable() {}
	traced_callable(traced_callable&& x) : traced_object(std::move(x)) {}
	traced_callable& operator=(traced_callable&& x) { static_cast<traced_object&>(*this) = std::move(x); return *this; }
#endif
};
template <size_t size> unsigned traced_callable<size>::functor_calls;
template <size_t size> unsigned traced_callable<size>::pmf_calls;
template <size_t size> unsigned traced_callable<size>::pmd_calls;

template <class T> using function = vs::function<T>;
static const size_t small_callable_size = 16;
static const size_t large_callable_size = 1000;

TEST(Function, DefaultConstruct)
{
	typedef function<int(int)> function_type;
	function_type f;
	ASSERT_FALSE(bool(f));
}
TEST(Function, CopyInitialize)
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		callable_type c;
		function_type f{c};
		ASSERT_TRUE(bool(f));
		EXPECT_EQ(14, f(7));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(1u, callable_type::copy_constructed);
	EXPECT_EQ(0u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(1u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, CopyInitialize_LargeObject)
{
	typedef traced_callable<large_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		callable_type c;
		function_type f{c};
		ASSERT_TRUE(bool(f));
		EXPECT_EQ(14, f(7));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(1u, callable_type::copy_constructed);
	EXPECT_EQ(0u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(1u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, MoveInitialize)
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		function_type f{callable_type()};
		ASSERT_TRUE(bool(f));
		EXPECT_EQ(14, f(7));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_EQ(1u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(1u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, MoveInitialize_LargeObject)
{
	typedef traced_callable<large_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		function_type f{callable_type()};
		ASSERT_TRUE(bool(f));
		EXPECT_EQ(14, f(7));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_EQ(1u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(1u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, CopyConstruct)
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		function_type f1{callable_type()};
		ASSERT_TRUE(bool(f1));
		function_type f2{f1};
		ASSERT_TRUE(bool(f1));
		ASSERT_TRUE(bool(f2));
		EXPECT_EQ(14, f1(7));
		EXPECT_EQ(26, f2(13));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(1u, callable_type::copy_constructed);
	EXPECT_EQ(1u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(2u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, CopyConstruct_LargeObject)
{
	typedef traced_callable<large_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		function_type f1{callable_type()};
		ASSERT_TRUE(bool(f1));
		function_type f2{f1};
		ASSERT_TRUE(bool(f1));
		ASSERT_TRUE(bool(f2));
		EXPECT_EQ(14, f1(7));
		EXPECT_EQ(26, f2(13));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(1u, callable_type::copy_constructed);
	EXPECT_EQ(1u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(2u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, MoveConstruct)
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		function_type f1{callable_type()};
		ASSERT_TRUE(bool(f1));
		function_type f2{std::move(f1)};
		ASSERT_FALSE(bool(f1));
		ASSERT_TRUE(bool(f2));
		EXPECT_EQ(14, f2(7));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_GE(2u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(1u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, MoveConstruct_LargeObject)
{
	typedef traced_callable<large_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		function_type f1{callable_type()};
		ASSERT_TRUE(bool(f1));
		function_type f2{std::move(f1)};
		ASSERT_FALSE(bool(f1));
		ASSERT_TRUE(bool(f2));
		EXPECT_EQ(14, f2(7));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_EQ(1u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(1u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, CopyAssign)
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		function_type f1{callable_type()};
		ASSERT_TRUE(bool(f1));
		function_type f2;
		f2 = f1;
		ASSERT_TRUE(bool(f1));
		ASSERT_TRUE(bool(f2));
		EXPECT_EQ(14, f1(7));
		EXPECT_EQ(26, f2(13));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(1u, callable_type::copy_constructed);
	EXPECT_EQ(1u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(2u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, CopyAssign_LargeObject)
{
	typedef traced_callable<large_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		function_type f1{callable_type()};
		ASSERT_TRUE(bool(f1));
		function_type f2;
		f2 = f1;
		ASSERT_TRUE(bool(f1));
		ASSERT_TRUE(bool(f2));
		EXPECT_EQ(14, f1(7));
		EXPECT_EQ(26, f2(13));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(1u, callable_type::copy_constructed);
	EXPECT_EQ(1u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(2u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, MoveAssign)
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		function_type f1{callable_type()};
		ASSERT_TRUE(bool(f1));
		function_type f2;
		f2 = std::move(f1);
		ASSERT_FALSE(bool(f1));
		ASSERT_TRUE(bool(f2));
		EXPECT_EQ(14, f2(7));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_GE(2u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(1u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, MoveAssign_LargeObject)
{
	typedef traced_callable<large_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		function_type f1{callable_type()};
		ASSERT_TRUE(bool(f1));
		function_type f2;
		f2 = std::move(f1);
		ASSERT_FALSE(bool(f1));
		ASSERT_TRUE(bool(f2));
		EXPECT_EQ(14, f2(7));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_EQ(1u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(1u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, Construct_RefWrap)
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		callable_type c;
		function_type f{std::ref(c)};
		ASSERT_TRUE(bool(f));
		EXPECT_EQ(14, f(7));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_EQ(0u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(1u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, Assign_NullPtr)
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		function_type f{callable_type()};
		ASSERT_TRUE(bool(f));
		f = nullptr;
		ASSERT_FALSE(bool(f));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_EQ(1u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(0u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, Assign_RefWrap)
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		callable_type c;
		function_type f;
		ASSERT_FALSE(bool(f));
		f = std::ref(c);
		ASSERT_TRUE(bool(f));
		EXPECT_EQ(14, f(7));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_EQ(0u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(1u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, Swap)
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		function_type f1{callable_type()};
		function_type f2{callable_type()};
		ASSERT_TRUE(bool(f1));
		ASSERT_TRUE(bool(f2));
		EXPECT_EQ(14, f1(7));
		EXPECT_EQ(26, f2(13));
		f1.swap(f2);
		ASSERT_TRUE(bool(f1));
		ASSERT_TRUE(bool(f2));
		EXPECT_EQ(14, f1(7));
		EXPECT_EQ(26, f2(13));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(2u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_GE(5u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(4u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, Swap_LargeObject)
{
	typedef traced_callable<large_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		function_type f1{callable_type()};
		function_type f2{callable_type()};
		ASSERT_TRUE(bool(f1));
		ASSERT_TRUE(bool(f2));
		EXPECT_EQ(14, f1(7));
		EXPECT_EQ(26, f2(13));
		f1.swap(f2);
		ASSERT_TRUE(bool(f1));
		ASSERT_TRUE(bool(f2));
		EXPECT_EQ(14, f1(7));
		EXPECT_EQ(26, f2(13));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(2u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_EQ(2u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(4u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, PMFCall) // Pointer to Member Function call
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(callable_type&, int)> function_type;
	callable_type::reset_counters();
	{
		callable_type c;
		function_type f{&callable_type::function};
		ASSERT_TRUE(bool(f));
		EXPECT_EQ(14, f(c, 7));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_EQ(0u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(0u, callable_type::functor_calls);
	EXPECT_EQ(1u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, PMFCall_RefWrap)
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(callable_type&, int)> function_type;
	callable_type::reset_counters();
	{
		callable_type c;
		function_type f{&callable_type::function};
		ASSERT_TRUE(bool(f));
		EXPECT_EQ(14, f(std::ref(c), 7));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_EQ(0u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(0u, callable_type::functor_calls);
	EXPECT_EQ(1u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, PMDCall) // Pointer to Member Data call
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(callable_type&)> function_type;
	callable_type::reset_counters();
	{
		callable_type c;
		function_type f{&callable_type::variable};
		ASSERT_TRUE(bool(f));
		EXPECT_EQ(42, f(c));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_EQ(0u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(0u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(1u, callable_type::pmd_calls);
}
TEST(Function, PMDCall_RefWrap)
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(callable_type&)> function_type;
	callable_type::reset_counters();
	{
		callable_type c;
		function_type f{&callable_type::variable};
		ASSERT_TRUE(bool(f));
		EXPECT_EQ(42, f(std::ref(c)));
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_EQ(0u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(0u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(1u, callable_type::pmd_calls);
}
TEST(Function, TargetType)
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(int)> function_type;
	callable_type::reset_counters();
	{
		function_type f{callable_type()};
		EXPECT_EQ(typeid(callable_type), f.target_type());
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_EQ(1u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(0u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}
TEST(Function, Target)
{
	typedef traced_callable<small_callable_size> callable_type;
	typedef function<int(callable_type&, int)> function_type;
	callable_type::reset_counters();
	{
		callable_type c;
		function_type f{&callable_type::function};
		EXPECT_EQ(&callable_type::function, *f.target<int(callable_type::*)(int)>());
	}
	EXPECT_EQ(0u, callable_type::active_instances());
	EXPECT_EQ(1u, callable_type::default_constructed);
	EXPECT_EQ(0u, callable_type::copy_constructed);
	EXPECT_EQ(0u, callable_type::move_constructed);
	EXPECT_EQ(0u, callable_type::copy_assigned);
	EXPECT_EQ(0u, callable_type::move_assigned);
	EXPECT_EQ(0u, callable_type::functor_calls);
	EXPECT_EQ(0u, callable_type::pmf_calls);
	EXPECT_EQ(0u, callable_type::pmd_calls);
}

} // namespace vs_function_test

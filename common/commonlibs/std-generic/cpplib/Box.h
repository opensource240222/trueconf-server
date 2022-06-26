#pragma once

#include <new>
#include <type_traits>
#include <utility>

namespace vs {

namespace detail { struct BoxTagBase {}; }

/*
 * vs::Box allows to type-safely store a type without providing its declaration and even without forward declaring it.
 *
 * It achieves this by using 2 techniques:
 * 1. Postponing instantiation of all member function to the point of their use.
 *    This is why most member functions are templates with an argument that doesn't matter.
 * 2. Making all places where name the real type is required template argument dependent.
 *    This achieved by using a template class (inner) to store a typedef to the real type.
 *
 * To use vs::Box with a type T that can be named (eg. forward declared type):
 * 1. Add vs::Box<T, ...> member with pre-calculated size and alignment for T.
 *
 * To use vs::Box with a type T that can not be named (eg. type nested in other type):
 * 1. Forward declare a struct which will be used as a tag:
 *        struct T_tag;
 * 2. Add vs::Box<T_tag, ...> member with pre-calculated size and alignment for T.
 * 3. Put this line in the .cpp file:
 *        struct T_tag : vs::BoxTag<T> {};
 *
 * If vs::Box will be a part of another class C then you need to ensure that
 * compiler won't try to create special member functions for class C when it
 * parses the header, to do that:
 * 1. Move definitions of existing constructors and the destructor to .cpp file,
 *    leave only declarations in the header.
 * 2. If destructor is not declared then declare it and put C::~C() = default;
 *    is the .cpp file.
 * 3. If copy constructor is not declared then declare it and either
 *    mark it with =delete or put C::C(const C&) = default; is the .cpp file.
 *    This is required only if anybody tries to copy class C.
 * 4. If move constructor is not declared then declare it and either
 *    mark it with =delete or put C::C(C&&) = default; is the .cpp file.
 *    This is required only if anybody tries to move class C.
 */

template <class T, unsigned size, unsigned alignment = alignof(void*)>
class Box
{
	template <class...>
	struct inner
	{
		using type = typename std::conditional<
			std::is_base_of<detail::BoxTagBase, T>::value,
			typename T::type,
			T
		>::type;
	};
	template <class X = void> using inner_t = typename inner<X>::type;

public:
	template <class... Args>
	Box(Args&&... args)
#if !defined(_MSC_VER)
		// For some reason MSVC (14.1*) wants to evaluate noexcept specifications on constructors early.
		// Moreover it isn't consistent - in some use cases it accepts this constructor (but not copy/move ones).
		// And it fails to do so because at that point T is only forward declared.
		// Interestingly this doesn't happen with copy/move assignment.
		noexcept(std::is_nothrow_constructible<typename inner<Args...>::type, Args&&...>::value)
#endif
	{
		static_assert(sizeof(inner_t<>) <= size, "vs::Box is smaller then the wrapped type.");
		static_assert(sizeof(inner_t<>) >= size, "vs::Box is larger than the wrapped type, reduce size to avoid wasting space.");
		static_assert(alignof(inner_t<>) <= alignment, "Alignment of vs::Box is smaller then alignment of the wrapped type.");
		static_assert(alignof(inner_t<>) >= alignment, "Alignment of vs::Box is larger then alignment of the wrapped type, reduce alignment to avoid wasting space.");
		new (m_storage) inner_t<>{ std::forward<Args>(args)... };
	}
	Box(const Box& x)
#if !defined(_MSC_VER)
		noexcept(std::is_nothrow_copy_constructible<inner_t<>>::value)
#endif
	{
		new (m_storage) inner_t<>{ x.get() };
	}
	Box(Box&& x)
#if !defined(_MSC_VER)
		noexcept(std::is_nothrow_move_constructible<inner_t<>>::value)
#endif
	{
		new (m_storage) inner_t<>{ std::move(x).get() };
	}
	~Box()
	{
		using TT = inner_t<>; // We have to do this because "~inner_t<>" is not parsed as destructor name by GCC and Clang.
		reinterpret_cast<TT*>(m_storage)->~TT();
	}

	template <class U>
	Box& operator=(U&& x) noexcept(std::is_nothrow_assignable<inner_t<>, U&&>::value)
	{
		get() = std::forward<U>(x);
		return *this;
	}
	Box& operator=(const Box& x) noexcept(std::is_nothrow_copy_assignable<inner_t<>>::value)
	{
		get() = x.get();
		return *this;
	}
	Box& operator=(Box&& x) noexcept(std::is_nothrow_move_assignable<inner_t<>>::value)
	{
		get() = std::move(x).get();
		return *this;
	}

	template <class X = void>       inner_t<X>&  get()       &  noexcept { return reinterpret_cast<      inner_t<>& >(m_storage); }
	template <class X = void>       inner_t<X>&& get()       && noexcept { return reinterpret_cast<      inner_t<>&&>(m_storage); }
	template <class X = void> const inner_t<X>&  get() const &  noexcept { return reinterpret_cast<const inner_t<>& >(m_storage); }
	template <class X = void> const inner_t<X>&& get() const && noexcept { return reinterpret_cast<const inner_t<>&&>(m_storage); }

	template <class X = void> operator       inner_t<X>& ()       &  noexcept { return get(); }
	template <class X = void> operator       inner_t<X>&&()       && noexcept { return get(); }
	template <class X = void> operator const inner_t<X>& () const &  noexcept { return get(); }
	template <class X = void> operator const inner_t<X>&&() const && noexcept { return get(); }

	template <class X = void>       inner_t<X>* operator->()       noexcept { return reinterpret_cast<      inner_t<>*>(m_storage); }
	template <class X = void> const inner_t<X>* operator->() const noexcept { return reinterpret_cast<const inner_t<>*>(m_storage); }

	/* C++14 version of get() and operator->()
	decltype(auto) get()       &  noexcept { return reinterpret_cast<      inner_t<>& >(m_storage); }
	decltype(auto) get()       && noexcept { return reinterpret_cast<      inner_t<>&&>(m_storage); }
	decltype(auto) get() const &  noexcept { return reinterpret_cast<const inner_t<>& >(m_storage); }
	decltype(auto) get() const && noexcept { return reinterpret_cast<const inner_t<>&&>(m_storage); }

	decltype(auto) operator->()       noexcept { return reinterpret_cast<      inner_t<>*>(m_storage); }
	decltype(auto) operator->() const noexcept { return reinterpret_cast<const inner_t<>*>(m_storage); }
	*/

private:
	alignas(alignment) unsigned char m_storage[size];
};

template <class T>
struct BoxTag : detail::BoxTagBase { using type = T; };

}

#pragma once

#include <type_traits>

// TODO: Remove when MSVC adds support for this.
// This is a workaround for missing support for non-static data members initializers (NSDMI)
// with aggregate initialization in MSVC 12.0 and MSVC 14.0.
//
// This code is valid C++14, but it is rejected by MSVC:
//    struct S { string a; int b = 123; };
//    S s { "hello" };
// So to work around that we can write:
//    struct S { string a; initialized<int, 123> b; };
//    S s { "hello" };
//
// default_initialized can be used for types which values can't be a non-type
// template parameters (like float).

template <class T, T init>
struct initialized
{
	initialized() = default;
	explicit initialized(T x) : value(x) {}
	initialized& operator=(T x) { value = x; return *this; }
	operator T&() { return value; }
	operator const T&() const { return value; }
	T&& move() { return static_cast<T&&>(value); }
private:
	T value = init;
};
static_assert(std::is_default_constructible<initialized<int, 1>>::value, "default construction");
static_assert(std::is_copy_constructible<initialized<int, 1>>::value, "copy construction");
static_assert(std::is_move_constructible<initialized<int, 1>>::value, "move construction");
static_assert(std::is_copy_assignable<initialized<int, 1>>::value, "copy assignment");
static_assert(std::is_move_assignable<initialized<int, 1>>::value, "move assignment");

template <class T>
struct default_initialized
{
	default_initialized() = default;
	explicit default_initialized(T x) : value(x) {}
	default_initialized& operator=(T x) { value = x; return *this; }
	operator T&() { return value; }
	operator const T&() const { return value; }
	T&& move() { return static_cast<T&&>(value); }
private:
	T value = T();
};
static_assert(std::is_default_constructible<default_initialized<float>>::value, "default construction");
static_assert(std::is_copy_constructible<default_initialized<float>>::value, "copy construction");
static_assert(std::is_move_constructible<default_initialized<float>>::value, "move construction");
static_assert(std::is_copy_assignable<default_initialized<float>>::value, "copy assignment");
static_assert(std::is_move_assignable<default_initialized<float>>::value, "move assignment");

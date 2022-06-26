#pragma once

namespace vs {

// This pointer wrapper can be used to work around array to pointer decay
// happening before template argument deduction. In practice this results in
// apparent inability to overload function for pointer to T and array of T.
template <class T>
struct ptr_arg
{
	// cppcheck-suppress noExplicitConstructor
	constexpr ptr_arg(T* v) noexcept : v_(v) {}
	constexpr operator T*() const noexcept { return v_; }
	T* v_;
};

}


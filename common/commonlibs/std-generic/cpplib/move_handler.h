#pragma once

#include <type_traits>

// move_handler wraps a handler declaring (but not defining) copy operations for it.
// Main use case is tricking Boost.Asio checks into accepting move-only handlers (eg. lamdbas capturing::unique_ptr).

namespace vs {

template <class F>
struct move_wrapper : F
{
    move_wrapper(F&& f) : F(std::move(f)) {}

    move_wrapper(move_wrapper&&) = default;
    move_wrapper& operator=(move_wrapper&&) = default;

    move_wrapper(const move_wrapper&);
    move_wrapper& operator=(const move_wrapper&);
};

template <class F>
move_wrapper<typename std::decay<F>::type> move_handler(F&& f)
{
    return std::move(f);
}

}

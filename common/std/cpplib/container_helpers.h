#pragma once

#include <utility>

#define INTO_CONTAINER_HELPER_IMPL(func) \
	template <class Container> \
	void func##_into(Container&) {} \
	template <class Container, class First, class... Rest> \
	void func##_into(Container& c, First&& first, Rest&&... rest) \
	{ \
		c.func(std::forward<First>(first)); \
		func##_into(c, std::forward<Rest>(rest)...); \
	}

INTO_CONTAINER_HELPER_IMPL(insert)
INTO_CONTAINER_HELPER_IMPL(emplace)
INTO_CONTAINER_HELPER_IMPL(emplace_back)
INTO_CONTAINER_HELPER_IMPL(push)
INTO_CONTAINER_HELPER_IMPL(push_back)
INTO_CONTAINER_HELPER_IMPL(push_front)

#undef INTO_CONTAINER_HELPER_IMPL

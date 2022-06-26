#pragma once

extern "C"
{
#ifndef __STDC_CONSTANT_MACROS
# define __STDC_CONSTANT_MACROS
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
}

#include <new>

namespace vs {

template <class T>
class av_allocator
{
public:
	using value_type = T;
	using pointer = T*;
	using size_type = ::size_t;

	av_allocator() = default;
	template <class U>
	av_allocator(const av_allocator<U>&) {}
	pointer allocate(size_type n)
	{
		if (n > static_cast<size_type>(-1) / sizeof(T))
			throw std::bad_alloc();
		if (auto p = static_cast<pointer>(::av_malloc(n * sizeof(T))))
			return p;
		throw std::bad_alloc();
	}
	void deallocate(pointer p, size_type) noexcept
	{
		::av_free(p);
	}
};

template <class T, class U>
bool operator==(const av_allocator<T>&, const av_allocator<U>&) { return true; }
template <class T, class U>
bool operator!=(const av_allocator<T>&, const av_allocator<U>&) { return false; }

}

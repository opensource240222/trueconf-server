#pragma once

#include "deleters.h"

#include <cassert>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <memory>
#include <type_traits>

namespace vs
{
// SharedBuffer acts as a reference to a memory block, and holds arbitrary resource via shared_ptr.
// Said resource is supposed to keep the memory block valid while it is alive.
//
// Due to lack of weak_use_count() in shared_ptr interface creating shared_ptr from weak_ptr for the same resource
// that were previously supplied to SharedBuffer will trick SharedBuffer into thinking that it has exclusive
// access to the memory buffer and you will end up with accessing data that is being modified in another thread.
class SharedBuffer
{
public:
	typedef size_t size_type;
	typedef char* iterator;
	typedef const char* const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	SharedBuffer()
		: size_(0)
	{
	}
	SharedBuffer(const SharedBuffer& x)
		: data_(x.data_)
		, size_(x.size_)
	{
	}
	SharedBuffer(SharedBuffer&& x) noexcept
		: data_(std::move(x.data_))
		, size_(x.size_)
	{
		x.size_ = 0;
	}
	SharedBuffer& operator=(const SharedBuffer& x)
	{
		if (&x == this)
			return *this;
		data_ = x.data_;
		size_ = x.size_;
		return *this;
	}
	SharedBuffer& operator=(SharedBuffer&& x) noexcept
	{
		if (&x == this)
			return *this;
		data_ = std::move(x.data_);
		size_ = x.size_;
		x.size_ = 0;
		return *this;
	}

	// Creates shared buffer for block [data, data+size) which is expected to be valid until supplied resource is destroyed
	template <class T>
	SharedBuffer(const void* data, size_type size, const std::shared_ptr<T>& resource)
		: data_(resource, const_cast<void*>(data))
		, size_(size)
	{
	}
	// Creates shared buffer for block [data, data+size) which is expected to be valid until last copy of this shared buffer is destroyed
	SharedBuffer(const void* data, size_type size)
		: data_(const_cast<void*>(data), noop_deleter())
		, size_(size)
	{
	}
	// Creates shared buffer for block [data, data+size) which is expected to be valid until supplied data resource is destroyed
	template <class T>
	SharedBuffer(const std::shared_ptr<T>& data, size_type size)
		: data_(data)
		, size_(size)
	{
	}
	// Move-enabled version of previous constructor
	template <class T>
	SharedBuffer(std::shared_ptr<T>&& data, size_type size)
		: data_(std::move(data))
		, size_(size)
	{
	}
	// Creates shared buffer for block [data, data+size)
	template <class T, class Deleter>
	SharedBuffer(std::unique_ptr<T, Deleter>&& data, size_type size)
		: data_(data.release(), data.get_deleter())
		, size_(size)
	{
	}
	// Creates shared buffer for block [data, data+sizeof(T)) which is expected to be valid until supplied data resource is destroyed
	template <class T>
	explicit SharedBuffer(const std::shared_ptr<T>& data)
		: data_(data)
		, size_(sizeof(T))
	{
	}
	// Move-enabled version of previous constructor
	template <class T>
	explicit SharedBuffer(std::shared_ptr<T>&& data)
		: data_(std::move(data))
		, size_(sizeof(T))
	{
	}
	// Creates shared buffer for block [data, data+sizeof(T))
	template <class T, class Deleter>
	explicit SharedBuffer(std::unique_ptr<T, Deleter>&& data)
		: data_(std::move(data))
		, size_(sizeof(T))
	{
	}
	// Creates shared buffer for newly allocated block of specified size
	explicit SharedBuffer(size_type size)
		: data_(new char[size], array_deleter<char>{})
		, size_(size)
	{
	}

	bool exclusive() const { return data_.use_count() == 1; }

	SharedBuffer& make_exclusive()
	{
		if (exclusive())
			return *this;
		auto new_data = std::shared_ptr<void>(new char[size_], array_deleter<char>{});
		std::memcpy(new_data.get(), data_.get(), size_);
		data_ = std::move(new_data);
		return *this;
	}

	SharedBuffer& shrink(size_type offset, size_type size)
	{
		if (offset >= size_ || size == 0)
		{
			data_.reset();
			size_ = 0;
			return *this;
		}

		if (offset > 0)
			data_ = std::shared_ptr<void>(data_, static_cast<char*>(data_.get()) + offset);
		size_ = size;
		return *this;
	}

	size_type size() const { return size_; }
	bool empty() const { return size_ == 0; }

	template <class T = const void>
	typename std::enable_if< std::is_const<T>::value, T*>::type data() const { return static_cast<T*>(data_.get()); }
	template <class T = void>
	typename std::enable_if<!std::is_const<T>::value, T*>::type data()
	{
		assert(exclusive());
		if (!exclusive())
			return nullptr;
		return static_cast<T*>(data_.get());
	}

	const_iterator begin()  const { return data<const char>(); }
	const_iterator end()    const { return data<const char>() + size(); }
	const_iterator cbegin() const { return begin(); }
	const_iterator cend()   const { return end(); }
	      iterator begin()        { return data<char>(); }
	      iterator end()          { return data<char>() + size(); }

	const_reverse_iterator rbegin()  const { return const_reverse_iterator(end()); }
	const_reverse_iterator rend()    const { return const_reverse_iterator(begin()); }
	const_reverse_iterator crbegin() const { return rbegin(); }
	const_reverse_iterator crend()   const { return rend(); }
	      reverse_iterator rbegin()        { return reverse_iterator(end()); }
	      reverse_iterator rend()          { return reverse_iterator(begin()); }

private:
	std::shared_ptr<void> data_;
	size_type size_;
};
}

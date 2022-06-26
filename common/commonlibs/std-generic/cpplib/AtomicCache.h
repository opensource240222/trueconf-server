#pragma once

#include <atomic>

namespace vs {

// Wait-free container that stores up to max_size elements of trivial type T.
// When an element leaves the cache it is passed to an instance of Deleter object which may perform necessary cleanup.
// Preferred number of elements to keep in the cache can be specified dynamically.
template <class T, class Deleter, unsigned max_size>
class AtomicCache
{
	static_assert(max_size >= 2, "Trying to create a cache for 1 or 0 elements, this looks useless.");

public:
	// size - preferred number of elements to keep in the cache, can be changed later.
	explicit AtomicCache(unsigned size, T null = T(), Deleter deleter = Deleter());
	~AtomicCache();

	// Removes an element from the cache and returns it, if the cache is empty returns 'null' value.
	T Get();
	// Adds the element to the cache, if the cache is full deletes the element instead.
	void Put(T x);

	unsigned Size() const { return m_size.load(std::memory_order_acquire); }
	void SetSize(unsigned new_size);
	void IncreaseSize(unsigned inc);
	void DecreaseSize(unsigned dec);

private:
	void MoveInaccessible(unsigned old_size, unsigned new_size);

	std::atomic<unsigned> m_size; // Preferred number of elements to keep in the cache, can be larger that max_size or smaller than min_size.
	const T m_null;
	Deleter m_deleter; // No empty base optimization because we are already wasting space on array alignment.
	alignas(64) std::atomic<T> m_data[max_size]; // Alignment to the cache line size is here to prevent "false sharing" with previous members.
};

template <class T, class Deleter, unsigned max_size>
inline AtomicCache<T, Deleter, max_size>::AtomicCache(unsigned size, T null, Deleter deleter)
	: m_size(size)
	, m_null(null)
	, m_deleter(static_cast<Deleter&&>(deleter))
{
	for (unsigned i = 0; i < max_size; ++i)
		m_data[i].store(m_null, std::memory_order_relaxed);
}

template <class T, class Deleter, unsigned max_size>
inline AtomicCache<T, Deleter, max_size>::~AtomicCache()
{
	m_size.store(0, std::memory_order_release);
	for (unsigned i = 0; i < max_size; ++i)
	{
		auto x = m_data[i].exchange(m_null, std::memory_order_relaxed);
		if (x != m_null)
			m_deleter(x);
	}
}

template <class T, class Deleter, unsigned max_size>
inline T AtomicCache<T, Deleter, max_size>::Get()
{
	// Try the first slot without loading and checking the size
	T x;
	if ((x = m_data[0].exchange(m_null, std::memory_order_relaxed)) != m_null)
		return x;

	auto size = m_size.load(std::memory_order_acquire);
	if (size > max_size)
		size = max_size;
	for (unsigned i = 1; i < size; ++i)
		if ((x = m_data[i].exchange(m_null, std::memory_order_relaxed)) != m_null)
			return x;

	return m_null;
}

template <class T, class Deleter, unsigned max_size>
inline void AtomicCache<T, Deleter, max_size>::Put(T x)
{
	if (x == m_null)
		return;

	// Try the first slot without loading and checking the size
	if ((x = m_data[0].exchange(x, std::memory_order_relaxed)) == m_null)
		return;

	auto size = m_size.load(std::memory_order_acquire);
	if (size > max_size)
		size = max_size;
	for (unsigned i = 1; i < size; ++i)
		if ((x = m_data[i].exchange(x, std::memory_order_relaxed)) == m_null)
			return;

	if (x != m_null)
		m_deleter(x);
}

template <class T, class Deleter, unsigned max_size>
inline void AtomicCache<T, Deleter, max_size>::SetSize(unsigned new_size)
{
	auto size = m_size.exchange(new_size, std::memory_order_release);
	MoveInaccessible(size, new_size);
}

template <class T, class Deleter, unsigned max_size>
inline void AtomicCache<T, Deleter, max_size>::IncreaseSize(unsigned inc)
{
	m_size.fetch_add(inc, std::memory_order_release);
}

template <class T, class Deleter, unsigned max_size>
inline void AtomicCache<T, Deleter, max_size>::DecreaseSize(unsigned dec)
{
	auto size = m_size.load(std::memory_order_relaxed);
	while (!m_size.compare_exchange_weak(size, size >= dec ? size - dec : 0, std::memory_order_release, std::memory_order_relaxed))
		size = m_size.load(std::memory_order_relaxed);
	MoveInaccessible(size, size >= dec ? size - dec : 0);
}

template <class T, class Deleter, unsigned max_size>
inline void AtomicCache<T, Deleter, max_size>::MoveInaccessible(unsigned old_size, unsigned new_size)
{
	if (new_size > max_size)
		return;
	if (old_size > max_size)
		old_size = max_size;

	// Reinsert elements that are now in the inaccessible part of the cache.
	for (unsigned i = new_size; i < old_size; ++i)
		Put(m_data[i].exchange(m_null, std::memory_order_relaxed));
}

}

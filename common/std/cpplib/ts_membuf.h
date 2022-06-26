#pragma once

#include "std-generic/compat/memory.h"
#include <cassert>
#include <mutex>
#include <streambuf>

namespace vs {

// A thread-safe, non-seekable, output-only std::streambuf that stores data in a single memory buffer.
template <class charT, class traits = std::char_traits<charT>, class Mutex = std::mutex>
class basic_ts_membuf : public std::basic_streambuf<charT, traits>
{
public:
	typedef std::basic_streambuf<charT, traits> base_t;
	typedef typename base_t::char_type char_type;
	typedef typename base_t::traits_type traits_type;
	typedef typename base_t::int_type int_type;
	typedef typename base_t::pos_type pos_type;
	typedef typename base_t::off_type off_type;

	basic_ts_membuf()
		: m_pptr(nullptr)
		, m_epptr(nullptr)
	{
	}

	std::unique_ptr<char_type[]> release(size_t& size)
	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		assert(m_buffer.get() <= m_pptr);
		assert(m_pptr <= m_epptr);
		size = m_pptr - m_buffer.get();
		m_pptr = nullptr;
		m_epptr = nullptr;
		return std::move(m_buffer);
	}

	void clear()
	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		m_buffer = nullptr;
		m_pptr = nullptr;
		m_epptr = nullptr;
	}

private:
	std::streamsize xsputn(const char_type* s, std::streamsize n) override
	{
		if (n <= 0)
			return 0;

		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		assert(m_buffer.get() <= m_pptr);
		assert(m_pptr <= m_epptr);
		if (m_pptr + n >= m_epptr)
			grow(static_cast<size_t>(n)); // We have already checked that n > 0
		traits_type::copy(m_pptr, s, static_cast<size_t>(n));
		m_pptr += n;
		return n;
	}

	int_type overflow(int_type c = traits_type::eof()) override
	{
		if (traits_type::eq_int_type(c, traits_type::eof()))
			return traits_type::not_eof(c);

		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		assert(m_buffer.get() <= m_pptr);
		assert(m_pptr <= m_epptr);
		if (m_pptr == m_epptr) // Equivalent to: m_pptr + 1 > m_epptr
			grow(1);
		*m_pptr++ = traits_type::to_char_type(c);
		return c;
	}

	void grow(size_t n)
	{
		assert(n > 0);
		const size_t old_size = m_epptr - m_buffer.get();
		const size_t new_size = n > old_size ? old_size + n : 2 * old_size;
		assert(new_size > old_size);
		auto new_buffer = vs::make_unique_default_init<char_type[]>(new_size);
		const size_t data_size = m_pptr - m_buffer.get();
		traits_type::copy(new_buffer.get(), m_buffer.get(), data_size);
		m_buffer = std::move(new_buffer);
		m_pptr = m_buffer.get() + data_size;
		m_epptr = m_buffer.get() + new_size;
	}

	Mutex m_mutex;
	std::unique_ptr<char_type[]> m_buffer;
	// We maintain our own values for pbase(), pptr() and epptr() because we have to ensure that access to them is protected by the mutex.
	char_type* m_pptr;
	char_type* m_epptr;
};

typedef basic_ts_membuf<char> ts_membuf;
typedef basic_ts_membuf<wchar_t> ts_wmembuf;

}

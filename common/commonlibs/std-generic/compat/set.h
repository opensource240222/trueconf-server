#pragma once

#include "std-generic/libstdcxx_version.h"

#include <set>

// All new C++14 std::set functions are available since libc++ 3.4, libstdc++ 5 and MSVC 14.0 (VS 2015).
// Note: libc++ versions prior to 3.7 had the same value for _LIBCPP_VERSION, so the check is imprecise.
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3700 && _LIBCPP_STD_VER > 11) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 50000 && __cplusplus > 201103L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1900)

namespace vs {
template <class Key, class Compare = std::less<Key>, class Allocator = std::allocator<Key>>
using set = std::set<Key, Compare, Allocator>;
}

#else

#include "std-generic/compat/type_traits.h"

namespace vs {

template <class Key, class Compare = std::less<Key>, class Allocator = std::allocator<Key>>
class set : public std::set<Key, Compare, Allocator>
{
	using base_t = std::set<Key, Compare, Allocator>;
public:
	typedef typename base_t::key_type key_type;
	typedef typename base_t::value_type value_type;
	typedef typename base_t::size_type size_type;
	typedef typename base_t::difference_type difference_type;
	typedef typename base_t::key_compare key_compare;
	typedef typename base_t::value_compare value_compare;
	typedef typename base_t::allocator_type allocator_type;
	typedef typename base_t::reference reference;
	typedef typename base_t::const_reference const_reference;
	typedef typename base_t::pointer pointer;
	typedef typename base_t::const_pointer const_pointer;
	typedef typename base_t::iterator iterator;
	typedef typename base_t::const_iterator const_iterator;
	typedef typename base_t::reverse_iterator reverse_iterator;
	typedef typename base_t::const_reverse_iterator const_reverse_iterator;

	// Support for inheriting constructors is buggy in many old compilers, so instead we manually forward all constructors to the base class.
	set() : base_t() {}
	explicit set(const Allocator& a) : base_t(a) {}
	explicit set(const Compare& comp, const Allocator& a = Allocator()) : base_t(comp, a) {}

	set(const base_t& x, const Allocator& a) : base_t(x, a) {}
	set(base_t&& x, const Allocator& a) : base_t(std::move(x), a) {}

	template <class InputIterator>
	set(InputIterator first, InputIterator last, const Compare& comp = Compare(), const Allocator& a = Allocator()) : base_t(first, last, comp, a) {}
	template <class InputIterator>
	set(InputIterator first, InputIterator last, const Allocator& a) : base_t(first, last, Compare(), a) {}

	set(std::initializer_list<value_type> il, const Compare& comp = Compare(), const Allocator& a = Allocator()) : base_t(il, comp, a) {};
	set(std::initializer_list<value_type> il, const Allocator& a) : base_t(il, Compare(), a) {}

	set(const set&) = default;
	set(set&&) = default;
	set& operator=(const set&) = default;
	set& operator=(set&&) = default;

	// Allow implicit conversion from real std::set
	// cppcheck-suppress noExplicitConstructor
	set(const base_t& x) : base_t(x) {};
	// cppcheck-suppress noExplicitConstructor
	set(base_t&& x) : base_t(std::move(x)) {}

	// emplace is available since libstdc++ 4.8
#if  defined(_LIBCPP_VERSION) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 40800) \
 ||  defined(_MSC_VER)
#else
	template <class... Args>
	std::pair<iterator, bool> emplace(Args&&... args)
	{
		return this->insert(value_type(std::forward<Args>(args)...));
	}
	template <class... Args>
	iterator emplace_hint(const_iterator hint, Args&&... args)
	{
		return this->insert(hint, value_type(std::forward<Args>(args)...));
	}
#endif

	// C++14 lookup functions that support transparent comparators are available since libc++ 3.4, libstdc++ 5 and MSVC 14.0 (VS 2015).
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3700 && _LIBCPP_STD_VER > 11) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 50000 && __cplusplus > 201103L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1900)
#else
	// Emulation of the lookup functions that support transparent comparators.
	// All of them have to construct a key_type object to perform the lookup.

	using base_t::find;
	template <class K>
	typename std::enable_if<detail::is_transparent<key_compare, K>::value, iterator>::type find(const K& x)
	{
		return this->find(static_cast<key_type>(x));
	}
	template <class K>
	typename std::enable_if<detail::is_transparent<key_compare, K>::value, const_iterator>::type find(const K& x) const
	{
		return this->find(static_cast<key_type>(x));
	}

	using base_t::count;
	template <class K>
	typename std::enable_if<detail::is_transparent<key_compare, K>::value, size_type>::type count(const K& x) const
	{
		return this->count(static_cast<key_type>(x));
	}

	using base_t::lower_bound;
	template <class K>
	typename std::enable_if<detail::is_transparent<key_compare, K>::value, iterator>::type lower_bound(const K& x)
	{
		return this->lower_bound(static_cast<key_type>(x));
	}
	template <class K>
	typename std::enable_if<detail::is_transparent<key_compare, K>::value, const_iterator>::type lower_bound(const K& x) const
	{
		return this->lower_bound(static_cast<key_type>(x));
	}

	using base_t::upper_bound;
	template <class K>
	typename std::enable_if<detail::is_transparent<key_compare, K>::value, iterator>::type upper_bound(const K& x)
	{
		return this->upper_bound(static_cast<key_type>(x));
	}
	template <class K>
	typename std::enable_if<detail::is_transparent<key_compare, K>::value, const_iterator>::type upper_bound(const K& x) const
	{
		return this->upper_bound(static_cast<key_type>(x));
	}

	using base_t::equal_range;
	template <class K>
	typename std::enable_if<detail::is_transparent<key_compare, K>::value, std::pair<iterator, iterator>>::type equal_range(const K& x)
	{
		return this->equal_range(static_cast<key_type>(x));
	}
	template <class K>
	typename std::enable_if<detail::is_transparent<key_compare, K>::value, std::pair<const_iterator, const_iterator>>::type equal_range(const K& x) const
	{
		return this->equal_range(static_cast<key_type>(x));
	}
#endif
};
static_assert(sizeof(vs::set<int>) == sizeof(std::set<int>), "vs::set shouldn't add extra members to allow slicing");

}

#endif

// std::erase_if from C++20 is available since libc++ 8.0, libstdc++ 9 and not yet alaivable in MSVC.
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 8000 && _LIBCPP_STD_VER > 17) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 90000 && __cplusplus > 201703L) \
 || 0

namespace vs {
using std::erase_if;
}

#else

namespace vs {

template <class Key, class Compare, class Allocator, class Predicate>
void erase_if(std::set<Key, Compare, Allocator>& c, Predicate pred)
{
	for (auto it = c.begin(), last = c.end(); it != last; )
		if (pred(*it))
			it = c.erase(it);
		else
			++it;
}

template <class Key, class Compare, class Allocator, class Predicate>
void erase_if(std::multiset<Key, Compare, Allocator>& c, Predicate pred)
{
	for (auto it = c.begin(), last = c.end(); it != last; )
		if (pred(*it))
			it = c.erase(it);
		else
			++it;
}

}

#endif

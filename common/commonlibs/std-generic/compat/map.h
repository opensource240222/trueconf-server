#pragma once

#include "std-generic/libstdcxx_version.h"

#include <map>

// All new C++14/C++17 std::map functions are available since libc++ 3.7, libstdc++ 6 and MSVC 14.1 (VS 2017).
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3700 && _LIBCPP_STD_VER > 14) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 60000 && __cplusplus > 201402L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1910)

namespace vs {
template <class Key, class T, class Compare = std::less<Key>, class Allocator = std::allocator<std::pair<const Key, T>>>
using map = std::map<Key, T, Compare, Allocator>;
}

#else

#include "std-generic/compat/type_traits.h"

namespace vs {

template <class Key, class T, class Compare = std::less<Key>, class Allocator = std::allocator<std::pair<const Key, T>>>
class map : public std::map<Key, T, Compare, Allocator>
{
	using base_t = std::map<Key, T, Compare, Allocator>;
public:
	typedef typename base_t::key_type key_type;
	typedef typename base_t::mapped_type mapped_type;
	typedef typename base_t::value_type value_type;
	typedef typename base_t::size_type size_type;
	typedef typename base_t::difference_type difference_type;
	typedef typename base_t::key_compare key_compare;
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
	map() : base_t() {}
	explicit map(const Allocator& a) : base_t(a) {}
	explicit map(const Compare& comp, const Allocator& a = Allocator()) : base_t(comp, a) {}

	map(const base_t& x, const Allocator& a) : base_t(x, a) {}
	map(base_t&& x, const Allocator& a) : base_t(std::move(x), a) {}

	template <class InputIterator>
	map(InputIterator first, InputIterator last, const Compare& comp = Compare(), const Allocator& a = Allocator()) : base_t(first, last, comp, a) {}
	template <class InputIterator>
	map(InputIterator first, InputIterator last, const Allocator& a) : base_t(first, last, Compare(), a) {}

	map(std::initializer_list<value_type> il, const Compare& comp = Compare(), const Allocator& a = Allocator()) : base_t(il, comp, a) {}
	map(std::initializer_list<value_type> il, const Allocator& a) : base_t(il, Compare(), a) {}

	map(const map&) = default;
	map(map&&) = default;
	map& operator=(const map&) = default;
	map& operator=(map&&) = default;

	// Allow implicit conversion from real std::map
	// cppcheck-suppress noExplicitConstructor
	map(const base_t& x) : base_t(x) {};
	// cppcheck-suppress noExplicitConstructor
	map(base_t&& x) : base_t(std::move(x)) {}

	// emplace is available since libstdc++ 4.8
#if  defined(_LIBCPP_VERSION) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 40800) \
 ||  defined(_MSC_VER)
	using base_t::emplace;
	using base_t::emplace_hint;
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

	// This is workaround for changes to std::pair converting constructor in C++17.
	// Before C++17 overload 'template <class U1, class U2> pair(U1&&, U2&&));'
	// existed only when both arguments were implicitly convertible to
	// corresponding pair member types.
	// In C++17 it exists when both pair member types are constructible from
	// corresponding arguments.
	// In the end without this workaround it is impossible to emplace
	// string_view into map<std::string, ...>.
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 4000 && _LIBCPP_STD_VER > 14) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 60000 && __cplusplus > 201402L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1910)
#else
	template <class A1, class A2>
	typename std::enable_if<std::is_constructible<const key_type, A1&&>::value && std::is_constructible<mapped_type, A2&&>::value, std::pair<iterator, bool>>::type
	emplace(A1&& a1, A2&& a2)
	{
		return this->insert(value_type(key_type(std::forward<A1>(a1)), mapped_type(std::forward<A2>(a2))));
	}
	template <class A1, class A2>
	typename std::enable_if<std::is_constructible<const key_type, A1&&>::value && std::is_constructible<mapped_type, A2&&>::value, iterator>::type
	emplace_hint(const_iterator hint, A1&& a1, A2&& a2)
	{
		return this->insert(hint, value_type(key_type(std::forward<A1>(a1)), mapped_type(std::forward<A2>(a2))));
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

	// New functions from C++17 are available since libc++ 3.7, libstdc++ 6 and MSVC 14.1 (VS 2017).
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3700 && _LIBCPP_STD_VER > 14) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 60000 && __cplusplus > 201402L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1910)
#else
	template <class... Args>
	std::pair<iterator, bool> try_emplace(const key_type& k, Args&&... args)
	{
		auto it = this->lower_bound(k);
		if (it == this->end() || key_compare()(k, it->first)) // No elements that are not less than k OR k is less than first such element
			return { this->emplace_hint(it, std::piecewise_construct, std::forward_as_tuple(k), std::forward_as_tuple(std::forward<Args>(args)...)), true };
		else
			return { std::move(it), false };
	}
	template <class... Args>
	std::pair<iterator, bool> try_emplace(key_type&& k, Args&&... args)
	{
		auto it = this->lower_bound(k);
		if (it == this->end() || key_compare()(k, it->first)) // No elements that are not less than k OR k is less than first such element
			return { this->emplace_hint(it, std::piecewise_construct, std::forward_as_tuple(std::move(k)), std::forward_as_tuple(std::forward<Args>(args)...)), true };
		else
			return { std::move(it), false };
	}
	template <class... Args>
	iterator try_emplace(const_iterator /*hint*/, const key_type& k, Args&&... args)
	{
		auto it = this->lower_bound(k);
		if (it == this->end() || key_compare()(k, it->first)) // No elements that are not less than k OR k is less than first such element
			return this->emplace_hint(it, std::piecewise_construct, std::forward_as_tuple(k), std::forward_as_tuple(std::forward<Args>(args)...));
		else
			return it;
	}
	template <class... Args>
	iterator try_emplace(const_iterator /*hint*/, key_type&& k, Args&&... args)
	{
		auto it = this->lower_bound(k);
		if (it == this->end() || key_compare()(k, it->first)) // No elements that are not less than k OR k is less than first such element
			return this->emplace_hint(it, std::piecewise_construct, std::forward_as_tuple(std::move(k)), std::forward_as_tuple(std::forward<Args>(args)...));
		else
			return it;
	}

	template <class M>
	std::pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj)
	{
		auto it = this->lower_bound(k);
		if (it == this->end() || key_compare()(k, it->first)) // No elements that are not less than k OR k is less than first such element
			return { this->emplace_hint(it, k, std::forward<M>(obj)), true };
		else
		{
			it->second = std::forward<M>(obj);
			return { std::move(it), false };
		}
	}
	template <class M>
	std::pair<iterator, bool> insert_or_assign(key_type&& k, M&& obj)
	{
		auto it = this->lower_bound(k);
		if (it == this->end() || key_compare()(k, it->first)) // No elements that are not less than k OR k is less than first such element
			return { this->emplace_hint(it, std::move(k), std::forward<M>(obj)), true };
		else
		{
			it->second = std::forward<M>(obj);
			return { std::move(it), false };
		}
	}
	template <class M>
	iterator insert_or_assign(const_iterator /*hint*/, const key_type& k, M&& obj)
	{
		auto it = this->lower_bound(k);
		if (it == this->end() || key_compare()(k, it->first)) // No elements that are not less than k OR k is less than first such element
			return this->emplace_hint(it, k, std::forward<M>(obj));
		else
		{
			it->second = std::forward<M>(obj);
			return it;
		}
	}
	template <class M>
	iterator insert_or_assign(const_iterator /*hint*/, key_type&& k, M&& obj)
	{
		auto it = this->lower_bound(k);
		if (it == this->end() || key_compare()(k, it->first)) // No elements that are not less than k OR k is less than first such element
			return this->emplace_hint(it, std::move(k), std::forward<M>(obj));
		else
		{
			it->second = std::forward<M>(obj);
			return it;
		}
	}
#endif

};
static_assert(sizeof(vs::map<int, int>) == sizeof(std::map<int, int>), "vs::map shouldn't add extra members to allow slicing");

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

template <class Key, class T, class Compare, class Allocator, class Predicate>
void erase_if(std::map<Key, T, Compare, Allocator>& c, Predicate pred)
{
	for (auto it = c.begin(), last = c.end(); it != last; )
		if (pred(*it))
			it = c.erase(it);
		else
			++it;
}

template <class Key, class T, class Compare, class Allocator, class Predicate>
void erase_if(std::multimap<Key, T, Compare, Allocator>& c, Predicate pred)
{
	for (auto it = c.begin(), last = c.end(); it != last; )
		if (pred(*it))
			it = c.erase(it);
		else
			++it;
}

}

#endif

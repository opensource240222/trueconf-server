#pragma once

#include <boost/range/const_iterator.hpp>
#include <boost/range/mutable_iterator.hpp>

#include <regex>

namespace boost
{
	template <class BidirIt>
	inline BidirIt range_begin(std::sub_match<BidirIt>& x)
	{
		return x.first;
	}
	template <class BidirIt>
	inline BidirIt range_begin(const std::sub_match<BidirIt>& x)
	{
		return x.first;
	}

	template <class BidirIt>
		inline BidirIt range_end(std::sub_match<BidirIt>& x)
	{
		return x.second;
	}
	template <class BidirIt>
	inline BidirIt range_end(const std::sub_match<BidirIt>& x)
	{
		return x.second;
	}

	template <class BidirIt>
	struct range_const_iterator<std::sub_match<BidirIt>>
	{
		typedef typename std::sub_match<BidirIt>::iterator type;
	};
	template <class BidirIt>
	struct range_mutable_iterator<std::sub_match<BidirIt>>
	{
		typedef typename std::sub_match<BidirIt>::iterator type;
	};
}

#pragma once

#include <boost/range/const_iterator.hpp>
#include <boost/range/mutable_iterator.hpp>
#include <boost/regex.hpp>

namespace boost
{
	template <class BidirIt>
	inline BidirIt range_begin(boost::sub_match<BidirIt>& x)
	{
		return x.first;
	}
	template <class BidirIt>
	inline BidirIt range_begin(const boost::sub_match<BidirIt>& x)
	{
		return x.first;
	}

	template <class BidirIt>
		inline BidirIt range_end(boost::sub_match<BidirIt>& x)
	{
		return x.second;
	}
	template <class BidirIt>
	inline BidirIt range_end(const boost::sub_match<BidirIt>& x)
	{
		return x.second;
	}

	template <class BidirIt>
	struct range_const_iterator<boost::sub_match<BidirIt>>
	{
		typedef typename boost::sub_match<BidirIt>::iterator type;
	};
	template <class BidirIt>
	struct range_mutable_iterator<boost::sub_match<BidirIt>>
	{
		typedef typename boost::sub_match<BidirIt>::iterator type;
	};
}

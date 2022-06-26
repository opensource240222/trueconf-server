#include "adjust_msg_order.h"
#include <algorithm>
namespace chat
{
namespace storage_utils
{
static size_t s_gap_size = 0xffff;
void set_gap(size_t gap)
{
	s_gap_size = gap;
}
size_t get_gap()
{
	return s_gap_size;
}
adjust_result adjust_msg_order(
	std::vector<order_info> &v,
	/*if == -1 => insert to the beginning*/
	const uint64_t insert_after,
	const order_info &value,
	const get_more_element& get_more,
	bool modify_only_last_if_possible)
{
	auto upper = insert_after == UINT64_MAX
		? v.begin()
		: std::upper_bound(
			v.begin(), v.end(), insert_after,
			[] (uint64_t val, const order_info&item)
			{return val < item.order;});
	auto where = v.insert(upper, value);
	if (v.size() == 1)
	{
		auto prev_vec = get_more(next_or_prec::preceding);
		if (prev_vec.empty())
		{
			auto next_vec = get_more(next_or_prec::next);
			if (!next_vec.empty())
			{
				v.insert(v.end(), next_vec.begin(), next_vec.end());
				where = v.begin();
				where->order = 0;
			}
			else
			{
				where->order = 0;
				return adjust_result( v.begin(), v.end(),where - v.begin() );
			}
		}
		else
		{
			auto where_index = where - v.begin() + prev_vec.size();
			v.insert(v.begin(), prev_vec.begin(), prev_vec.end());
			where = v.begin() + where_index;
			where->order = (where - 1)->order;
		}
	}
	else
		where->order = insert_after == UINT64_MAX ? 0 : insert_after;
	auto inserted_item = where;
	if (where == v.begin())
		++where;
	auto after_where = where + 1;
	/**
	if after_where == end => try get more(true);
	*/
	if (after_where == v.end())
	{
		auto new_v = get_more(next_or_prec::next);
		if (!new_v.empty())
		{
			auto inserted_item_index = inserted_item - v.begin();
			v.insert(v.end(), new_v.begin(), new_v.end());
			inserted_item = v.begin() + inserted_item_index;
			where = v.end() - new_v.size() - 1;
			after_where = where + 1;
		}
		else if (modify_only_last_if_possible)
		{
			where->order += s_gap_size;
			return adjust_result(
				inserted_item,
				after_where,
				inserted_item - v.begin());
		}
	}
	if (after_where == v.end()
		|| after_where->order - where->order == 1)
	{
		auto res = std::make_pair(v.end(), v.end());
		auto left = where - 1;
		auto right = after_where;
		auto loop_condition = [&]()
		{
			if (left == v.begin())
			{
				/**
					1 get previous;
					2 reinit iterators;
				*/
				auto prev_vec = get_more(next_or_prec::preceding);
				if (!prev_vec.empty())
				{
					auto right_index = right - v.begin() + prev_vec.size();
					auto where_index = where - v.begin() + prev_vec.size();
					auto inserted_item_index = inserted_item - v.begin()
						+ prev_vec.size();
					v.insert(v.begin(), prev_vec.begin(), prev_vec.end());
					left = v.begin() + prev_vec.size();
					right = v.begin() + right_index;
					where = v.begin() + where_index;
					inserted_item = v.begin() + inserted_item_index;
					res = std::make_pair(v.end(), v.end());
					return true;
				}
			}
			if (right == v.end())
			{
				/**
					1. get next
					2 reinit iterators;
				*/
				auto next_vec = get_more(next_or_prec::next);
				if (!next_vec.empty())
				{
					auto left_index = left - v.begin();
					auto where_index = where - v.begin();
					auto inserted_item_index = inserted_item - v.begin();
					v.insert(v.end(), next_vec.begin(), next_vec.end());
					right = v.end() - next_vec.size();
					left = v.begin() + left_index;
					where = v.begin() + where_index;
					inserted_item = v.begin() + inserted_item_index;
					res = std::make_pair(v.end(), v.end());
				}
			}
			return left != v.begin() || right != v.end();
		};
		while (loop_condition())
		{
			left == v.begin() ? left : --left;;
			right == v.end() ? right : ++right;
			auto next = left + 1;
			if (next != v.end())
			{
				if (next->order - left->order > 1)
				{
					res.first = std::move(left);
					break;
				}
			}
			if (right != v.end())
			{
				auto prev = right - 1;
				if (right->order - prev->order > 1)
				{
					res.second = std::move(right);
					break;
				}
			}
		}
		if (res.first != v.end())
		{
			auto next = res.first + 1;
			auto shift = (next->order - res.first->order) / 2;
			for (auto i = next;i != where;++i)
				i->order -= shift;
			return adjust_result(next, inserted_item + 1,
				inserted_item - v.begin());
		}
		if (res.second != v.end())
		{
			auto prev = res.second - 1;
			auto shift = (res.second->order - prev->order) / 2;
			for (auto i = where;i != res.second;++i)
				i->order += shift;
			return adjust_result(inserted_item, res.second,
								inserted_item - v.begin());
		}
		else
		{
			auto prev_order = where->order;
			for (auto i = where;i != v.end();prev_order = i->order, ++i)
				i->order = prev_order + s_gap_size;
			return adjust_result(inserted_item, v.end(),
								inserted_item - v.begin());
		}
	}
	else
	{
		where->order += (after_where->order - where->order) / 2;
		return adjust_result(inserted_item, where + 1,
							inserted_item - v.begin());
	}
}
}
}
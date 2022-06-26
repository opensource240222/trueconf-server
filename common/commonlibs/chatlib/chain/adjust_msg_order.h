#pragma once
#include "chatlib/chat_defs.h"

#include "std-generic/cpplib/macro_utils.h"

#include <functional>
#include <vector>
namespace chat
{
namespace storage_utils
{
struct order_info
{
	VS_FORWARDING_CTOR2(order_info, id, order) {}
	order_info()
		: order(UINT64_MAX) {}
	order_info(order_info&&src) = default;
	order_info(const order_info &src) = default;
	order_info&operator=(const order_info&src) = default;
	order_info&operator=(order_info&&src) = default;

	ChatMessageID id;
	uint64_t order;
};
//changed elements and index of inserted element
struct adjust_result
{
	adjust_result(std::vector<order_info>::const_iterator first,
		std::vector<order_info>::const_iterator last,
		std::vector<order_info>::size_type where_inserted)
		: first(first)
		, last(last)
		, where_inserted(where_inserted)
	{}
	std::vector<order_info>::const_iterator first;
	std::vector<order_info>::const_iterator last;
	std::vector<order_info>::size_type where_inserted;
};
/**true => get next, false => get preceding */
enum class next_or_prec
{
	next,
	preceding
};
using get_more_element = std::function<std::vector<order_info>(next_or_prec)>;
adjust_result adjust_msg_order(
		std::vector<order_info> &v,
		/*if == -1 => insert to the beginning*/
		const uint64_t insert_after,
		const order_info &value,
		const get_more_element& get_more,
		bool modify_only_last_if_possible = true);
void set_gap(size_t gap);
size_t get_gap();
}
}
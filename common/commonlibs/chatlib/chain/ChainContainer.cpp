#include "ChainContainer.h"

#include "std-generic/compat/memory.h"
#include <cassert>
#include <queue>

namespace chat
{
namespace chain
{
namespace detail
{
auto is_node_valid(detail::Node *node) -> bool
{
	if (node == nullptr)
		return false;
	else if (node->next == nullptr)
		return false;
	else if (node->data == nullptr) // end node
		return false;
	return true;
}
auto cut_by_parrent(detail::Node *node)->detail::Node* // return pointer to next after break
{
	/**
	find next node,  that sorted in no by parent and cut that
	*/
	for (auto &i = node; is_node_valid(i);i = i->next)
	{
		if (i->next->data == nullptr
			|| i->data->id != i->next->data->parent)
		{
			//cut
			auto ret = i->next;
			i->next = nullptr;
			ret->prev = nullptr;
			return ret;
		}
	}
	return nullptr;
}
void fixup_order(detail::Node *start_node)
{
	if (!is_node_valid(start_node))
		return;
	for(auto &prev = start_node, next = start_node->next;
		is_node_valid(next);
		prev = next, next = next->next)
	{
		if (next->order_num > prev->order_num)
			return;
		next->order_num = prev->order_num + 1;
	}
}
chain_iterator::chain_iterator(Node*node) :itr_(node)
{}
chain_iterator::chain_iterator(chain_iterator&&src)
{
	itr_ = src.itr_;
	src.itr_ = nullptr;
}
chain_iterator::pointer chain_iterator::operator->()
{
	return itr_->data.get();
}
chain_iterator::const_pointer chain_iterator::operator->() const
{
	return itr_->data.get();
}
chain_iterator::reference chain_iterator::operator*()
{
	return *itr_->data;
}
chain_iterator::const_reference chain_iterator::operator*() const
{
	return *itr_->data;
}
bool chain_iterator::operator==(const chain_iterator&it) const
{
	return itr_ == it.itr_;
}
chain_iterator& chain_iterator::operator--()
{
	if (itr_ == nullptr || itr_->prev == nullptr)
		return *this;
	itr_ = itr_->prev;
	return *this;
}
chain_iterator& chain_iterator::operator++()
{
	if (itr_ != nullptr && itr_->next!= nullptr && !is_last())
		itr_ = itr_->next;
	return *this;
}
chain_iterator chain_iterator::operator++(int)
{
	chain_iterator res(itr_);
	++*this;
	return res;
}
const_chain_iterator::const_chain_iterator(const chain_iterator &it) :it_(it)
{}
const_chain_iterator::reference const_chain_iterator::operator*() const
{
	return *it_;
}
const_chain_iterator::pointer const_chain_iterator::operator->() const
{
	return it_.operator->();
}
bool const_chain_iterator::operator==(const const_chain_iterator&src) const
{
	return src.it_ == it_;
}
bool const_chain_iterator::operator!=(const const_chain_iterator&src) const
{
	return !operator==(src);
}
const_chain_iterator& const_chain_iterator::operator--()
{
	--it_;
	return *this;
}
const_chain_iterator const_chain_iterator::operator--(int)
{
	const_chain_iterator res(it_);
	--it_;
	return res;
}
const_chain_iterator& const_chain_iterator::operator++()
{
	++it_;
	return *this;
}
const_chain_iterator const_chain_iterator::operator++(int )
{
	const_chain_iterator res(it_);
	++it_;
	return res;
}
}
ChainContainer::ChainContainer()
	: first_(&last_node_)
	, end_node_(&last_node_)
{
}
ChainContainer::ChainContainer(ChainContainer&&other)
	: last_node_(std::move(other.last_node_))
	, first_(other.first_ == other.end_node_
			? &last_node_
			: other.first_)
	, end_node_(&last_node_)
	, nodes_with_changed_order_(std::move(other.nodes_with_changed_order_))
	, search_links_(std::move(other.search_links_))
	, lost_idx_(std::move(other.lost_idx_))
	, nodes_(std::move(other.nodes_))
{
	last_node_.next = &last_node_;
	if (last_node_.prev != nullptr)
		last_node_.prev->next = &last_node_;
}
ChainContainer & ChainContainer::operator=(ChainContainer &&other)
{
	last_node_ = std::move(other.last_node_);
	if(other.first_ != other.end_node_)
		first_ = other.first_;
	nodes_with_changed_order_ = std::move(other.nodes_with_changed_order_);
	search_links_ = std::move(other.search_links_);
	lost_idx_ = std::move(other.lost_idx_);
	nodes_ = std::move(other.nodes_);
	last_node_.next = &last_node_;
	if (last_node_.prev != nullptr)
		last_node_.prev->next = &last_node_;
	return *this;
}
void ChainContainer::SaveNode(std::unique_ptr<detail::Node>&& node)
{
	detail::Node* p(node.get());
	nodes_.emplace(p,std::move(node));
	search_links_.emplace(p->data->id,p);
	//create_order_.back().second = p;
}
void ChainContainer::push_back(
	ChatMessageIDRef id,
	ChatMessageTimestamp stamp)
{
	if (search_links_.find(id) != search_links_.end())
		return;
	auto last = end_node_->prev;
	ChatMessageID previous = last == nullptr
		? ChatMessageID()
		: last->data->id;
	auto node = vs::make_unique<detail::Node>(
		vs::make_unique<detail::Data>(id, previous, stamp));
	InsertBefore(end_node_, node.get());
	//create_order_.push_back(std::make_pair("push_back", nullptr));
	SaveNode(std::move(node));
	nodes_with_changed_order_.clear();// always insert to the end
	//assert(integrity_check());
}
const_iterator ChainContainer::erase(ChatMessageIDRef id)
{
	/**
		1. search node;
		2. Get next next->prev = nullptr;
		3. erase node from any where
		4. start with next
			insert to queue;
			cut_by_parent and continue
	*/
	auto link_iter = search_links_.find(id);
	if (link_iter == search_links_.end())
		return end();
	auto node_iter = nodes_.find(link_iter->second);
	if (node_iter == nodes_.end())
	{
		assert(false);
		return end();
	}
	auto node = std::move(node_iter->second);
	//// debug
	//create_order_.push_back(std::make_pair("erase", node.get()));
	////
	search_links_.erase(link_iter);
	nodes_.erase(node_iter);
	auto &parrent_id = node->data->parent;
	if(!parrent_id.empty())
	{
		auto lost = lost_idx_.find(parrent_id);
		if (lost != lost_idx_.end())
		{
			lost->second.erase(node.get());
			if (lost->second.empty())
				lost_idx_.erase(lost);
		}
	}
	/**
		if node == first_ => first = end_
	*/
	if (node.get() == first_)
	{
		first_ = end_node_;
		end_node_->prev = nullptr;
		node->next = nullptr;
	}
	else
		CutTail(node->prev);
	vs::set<detail::Node*> queue_for_insert;
	for(auto next = node->next;nullptr != next;next = cut_by_parrent(next))
		queue_for_insert.insert(next);
	while (!queue_for_insert.empty())
	{
		auto node_i = queue_for_insert.begin();
		auto node_p = *node_i;
		queue_for_insert.erase(node_i);
		Insert(node_p);
	}
	if (nodes_with_changed_order_.empty())
		return end();
	auto res = const_iterator(iterator(
		*std::min_element(
			nodes_with_changed_order_.begin(),
			nodes_with_changed_order_.end(),
			[](const detail::Node *a, const detail::Node *b)
				{return a->order_num < b->order_num;})));
	nodes_with_changed_order_.clear();
	//assert(integrity_check());
	return res;
}
void ChainContainer::CutTail(detail::Node*node)
{
	if (node == nullptr || end_node_ == node)
		return;
	auto old_next = node->next;
	auto old_last = end_node_->prev;
	node->next = end_node_;
	end_node_->prev = node;
	if(nullptr != old_next && end_node_ != old_next)
		old_next->prev = nullptr;
	if (old_last!=node && old_last != first_)
		old_last->next = nullptr;
}
void ChainContainer::InsertBefore(
	detail::Node* before_node,
	detail::Node*node,
	bool notify_order_change)
{
	assert(before_node != node);
	node->prev = before_node->prev;
	node->next = before_node;
	before_node->prev = node;
	if (node->prev != nullptr)
	{
		node->prev->next = node;
		node->order_num = node->prev->order_num + 1;
		assert(node->order_num != UINT64_MAX);
	}
	if (before_node == first_)
	{
		first_ = node;
		first_->order_num = 0;
	}
	if(notify_order_change)
		nodes_with_changed_order_.insert(node);
	fixup_order(node);
}
void ChainContainer::InsertAfter(
	detail::Node*after,
	detail::Node*node,
	bool notify_order_change)
{
	assert(after != node);
	node->next = after->next;
	if (node->next != nullptr)
		node->next->prev = node;
	after->next = node;
	node->prev = after;
	node->order_num = after->order_num + 1;
	if(notify_order_change)
		nodes_with_changed_order_.insert(node);
	fixup_order(node);
}
const_iterator ChainContainer::insert(
	ChatMessageIDRef id,
	ChatMessageIDRef parent,
	ChatMessageTimestamp timestamp)
{
	if (search_links_.count(id) > 0)
		return end();
	auto node = vs::make_unique<detail::Node>(
		vs::make_unique<detail::Data>(id, parent, timestamp));
	auto node_ptr = node.get();
	//create_order_.push_back(std::make_pair("insert", nullptr));
	SaveNode(std::move(node));
	Insert(node_ptr);
	assert(!nodes_with_changed_order_.empty());
	auto res = const_iterator(iterator(
		*std::min_element(
			nodes_with_changed_order_.begin(),
			nodes_with_changed_order_.end(),
			[](const detail::Node *a, const detail::Node *b)
				{return a->order_num < b->order_num;})));
	nodes_with_changed_order_.clear();
	/**
		debug
	*/
	//assert(integrity_check());

	return res;
}
const_iterator ChainContainer::insert(
	const_iterator parent_it,
	ChatMessageIDRef id,
	ChatMessageTimestamp timestamp)
{
	if (parent_it == end()
		|| search_links_.count(id) > 0
		|| parent_it->id.empty())
		return end();
	auto node = vs::make_unique<detail::Node>(
		vs::make_unique<detail::Data>(id, parent_it->id, timestamp));
	auto node_ptr = node.get();
	//create_order_.push_back(std::make_pair("insert", nullptr));
	SaveNode(std::move(node));
	Insert(node_ptr,parent_it.it_.itr_);
	assert(!nodes_with_changed_order_.empty());
	auto res = const_iterator(iterator(
		*std::min_element(
			nodes_with_changed_order_.begin(),
			nodes_with_changed_order_.end(),
			[](const detail::Node *a, const detail::Node *b)
	{return a->order_num < b->order_num; })));
	nodes_with_changed_order_.clear();
	/**
	debug
	*/
	//assert(integrity_check());
	return res;
}
detail::Node * ChainContainer::create_node_before_force(
	ChatMessageIDRef id,
	ChatMessageIDRef parent,
	ChatMessageTimestamp timestamp)
{
	if (search_links_.count(id) > 0)
		return nullptr;
	auto node = vs::make_unique<detail::Node>(
		vs::make_unique < detail::Data>(id, parent, timestamp));
	auto node_ptr = node.get();
	//create_order_.push_back(std::make_pair("insert", nullptr));
	SaveNode(std::move(node));
	// save lost parent
	if (!node_ptr->data->parent.empty())
	{
		auto it = search_links_.find(node_ptr->data->parent);
		if (it == search_links_.end())
		{
			auto wich_lost_it = lost_idx_.find(node_ptr->data->parent);
			if (lost_idx_.end() == wich_lost_it)
				lost_idx_.emplace(
					node_ptr->data->parent,
					vs::set<detail::Node*>{node_ptr});
			else
				wich_lost_it->second.insert(node_ptr);
		}
	}
	// erase current from lost
	lost_idx_.erase(node_ptr->data->id);
	return node_ptr;
}
detail::Node* ChainContainer::force_insert_before(
	detail::Node* before,
	ChatMessageIDRef id,
	ChatMessageIDRef parent,
	ChatMessageTimestamp timestamp)
{
	auto node_ptr = create_node_before_force(id, parent, timestamp);
	if (nullptr == node_ptr)
		return nullptr;
	InsertBefore(before, node_ptr, false);
	return node_ptr;
}
void ChainContainer::Insert(detail::Node*node, detail::Node*parent)
{
	detail::Node* start = parent == nullptr ? first_ : parent->next;
	// parent is empty - candidate to be first
	if (parent == nullptr && !node->data->parent.empty())
	{
		auto it = search_links_.find(node->data->parent);
		if (it == search_links_.end())
		{
			auto wich_lost_it = lost_idx_.find(node->data->parent);
			if (lost_idx_.end() == wich_lost_it)
				lost_idx_.emplace(
						node->data->parent,
						vs::set<detail::Node*>{node});
			else
				wich_lost_it->second.insert(node);
		}
		else
		{
			parent = it->second;
			start = parent->next;
		}
	}
	if (lost_idx_.find(node->data->id) != lost_idx_.end())
	{
		MergeWithLostNode(node, parent);
	}
	else if (start == end_node_)
	{
		while (node != nullptr && node!=end_node_)
		{
			auto next = node->next;
			InsertBefore(end_node_, node);
			node = next;
		}
	}
	else if (start == nullptr)
	{
		while (node != nullptr && node != end_node_)
		{
			auto next = node->next;
			InsertAfter(parent, node);
			parent = node;
			node = next;
		}
	}
	else
		Merge(parent, start, node);
}
void ChainContainer::Merge(
	detail::Node* parent,
	detail::Node *start,
	detail::Node* node)
{
	static auto isInsertBefore =
		[](const detail::Node* before, const detail::Node* node) -> bool
	{
		if ((before->data->timestamp > node->data->timestamp) // node is candidate to be first
			|| ((before->data->timestamp == node->data->timestamp) // node was created early
				&& (before->data->id > node->data->id))		// compare by id;
			)
			return true;
		return false;
	};

	for (; start!=nullptr && start != end_node_ && node!=nullptr;
		parent = start,start = start->next)
	{
		auto next_node = node->next;
		if (isInsertBefore(start, node))
		{
			InsertBefore(start, node);
			start = start->prev;
			node = next_node;
		}
	}

	if (node == nullptr)
		return;

	if (start == nullptr)
	{
		while (!!node)
		{
			auto next = node->next;
			InsertAfter(parent, node);
			parent = node;
			node = next;
		}
	}
	else if (start == end_node_)
	{
		while (!!node)
		{
			auto next = node->next;
			InsertBefore(end_node_, node);
			node = next;
		}
	}
	else
		assert(0);
}
void ChainContainer::MergeWithLostNode(detail::Node *lost_node, detail::Node *parent_node)
{
	vs::set<detail::Node*> queue_for_merge;
	queue_for_merge.insert(lost_node);
	auto which_lost = lost_idx_.find(lost_node->data->id);
	for (const auto &i : which_lost->second)
	{
		auto prev = i->prev;
		auto next = i;
		do
		{
			if (next == end_node_)
			{
				if (i==first_)
					first_ = end_node_;
				else
				{
					end_node_->prev = prev;
					prev->next = end_node_;
				}
			}
			else
				queue_for_merge.insert(next);
		} while ((next = cut_by_parrent(next))!=nullptr);
	}
	lost_idx_.erase(which_lost);
	while (!queue_for_merge.empty())
	{
		auto node = queue_for_merge.begin();
		auto node_p = *node;
		queue_for_merge.erase(node);
		Insert(node_p, node_p == lost_node ? parent_node : nullptr);
	}
}
const_iterator ChainContainer::find(ChatMessageIDRef id) const
{
	auto iter = search_links_.find(id);
	if (iter == search_links_.end())
		return end();
	else
		return const_iterator(iterator(iter->second));
}
const_reference ChainContainer::front() const
{
	assert(!empty());
	return *first_->data;
}
const_iterator ChainContainer::end() const
{
	return const_iterator(iterator(end_node_));
}
const_iterator ChainContainer::begin() const
{
	return const_iterator(iterator(first_));
}
const_reference ChainContainer::back() const
{
	return *--end();
}
bool ChainContainer::integrity_check() const
{
	if (empty())
		return true;
	for (auto current = first_, next = first_->next;
		next != end_node_;
		current = next, next = next->next)
	{
		if (current->order_num >= next->order_num)
			return false;
		if (next->data->parent != current->data->id)
		{
			if (next->data->timestamp < current->data->timestamp
				|| (next->data->timestamp == current->data->timestamp
					&& next->data->id < current->data->id))
				return false;
		}
	}
	return true;
}
}
}
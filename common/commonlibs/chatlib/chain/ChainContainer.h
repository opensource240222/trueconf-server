#pragma once
#include "chatlib/chat_defs.h"

#include "std-generic/compat/map.h"
#include "std-generic/compat/set.h"
#include "std-generic/cpplib/macro_utils.h"

#include <vector>

namespace chat
{
namespace chain
{
class ChainContainer;
namespace detail
{
struct Data
{
	VS_FORWARDING_CTOR3(Data, id, parent, timestamp) {}
	Data() = default;

	bool operator==(const Data&other) const
	{
		return id == other.id
			&& parent == other.parent
			&& timestamp == other.timestamp;
	}
	ChatMessageID id;
	ChatMessageID parent;
	ChatMessageTimestamp timestamp;
};
struct Node
{
	explicit Node(std::unique_ptr<Data>&&data) : data(std::move(data))
	{}
	std::unique_ptr<Data> data;
	Node * next = nullptr;
	Node * prev = nullptr;
	uint64_t order_num = 0;
};
struct LastNode : Node
{
	explicit LastNode() : Node(nullptr)
	{
		next = this;
		order_num = UINT64_MAX;
	}
};

class chain_iterator
{
	friend ChainContainer;

	Node * itr_;
protected:
	bool is_last() const
	{
		return itr_->next == itr_;
	}
public:
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = Data;
	using difference_type = std::ptrdiff_t;
	using pointer = Data*;
	using const_pointer = const Data*;
	using reference = Data&;
	using const_reference = const Data&;

	chain_iterator& operator=(const chain_iterator&) = default;
	chain_iterator(const chain_iterator&) = default;
	chain_iterator(chain_iterator&&);
	explicit chain_iterator(Node*);
	chain_iterator()
		: itr_(nullptr)
	{}
	reference operator*();
	const_reference operator*() const;
	pointer operator->();
	const_pointer operator->() const;
	bool operator==(const chain_iterator&) const;
	chain_iterator& operator--();
	chain_iterator& operator++();
	chain_iterator operator++(int);
};
class const_chain_iterator
{
	friend ChainContainer;

	chain_iterator it_;
public:
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = Data;
	using difference_type = std::ptrdiff_t;
	using pointer = const Data*;
	using reference = const Data&;

	explicit const_chain_iterator(const chain_iterator&);
	const_chain_iterator() = default;
	reference operator*() const;
	pointer operator->() const;
	bool operator!=(const const_chain_iterator&) const;
	bool operator==(const const_chain_iterator&) const;
	const_chain_iterator& operator--();
	const_chain_iterator operator--(int);
	const_chain_iterator& operator++();
	const_chain_iterator operator++(int);
};
}
using iterator = detail::chain_iterator;
using const_iterator = detail::const_chain_iterator;
using const_reference = detail::chain_iterator::const_reference;
class ChainContainer
{
public:
	ChainContainer(ChainContainer&& other);
	ChainContainer();
	template<class InputIt>
	ChainContainer(InputIt first, InputIt last)
		: ChainContainer()
	{
		while (first != last)
		{
			force_insert_before(
				end_node_,
				first->msg_id,
				first->prev_id,
				first->timestamp);
			++first;
		}
	}
	ChainContainer(const ChainContainer&src) = delete;
	ChainContainer & operator=(ChainContainer&& other);
	ChainContainer& operator=(const ChainContainer&other) = delete;
	const_iterator insert(
		ChatMessageIDRef id,
		ChatMessageIDRef parent,
		ChatMessageTimestamp timestamp); // return item from whom the order has changed
	// insert if parent already found
	const_iterator insert(
		const_iterator parent_it,
		ChatMessageIDRef id,
		ChatMessageTimestamp timestamp
	);
	template<class InputIt>
	const_iterator force_insert(const_iterator position, InputIt first, InputIt last)
	{
		auto end_ = iterator(end_node_);
		auto res = end_;
		auto new_position_node = position.it_.itr_;
		while (first != last)
		{
			new_position_node = force_insert_before(
				new_position_node,
				first->msg_id_,
				first->prev_id_,
				first->timestamp_);
			assert(new_position_node != nullptr);
			if (new_position_node == nullptr)
				return end();
			new_position_node = new_position_node->next;
			++first;
			if (res == end_)
				res = iterator(new_position_node);
		}
		return const_iterator(res);
	}
	void push_back(
		ChatMessageIDRef,
		ChatMessageTimestamp);

	const_iterator find(ChatMessageIDRef id) const;
	const_reference front() const;
	const_reference back() const;
	const_iterator end() const;
	const_iterator begin() const;
	const_iterator erase(ChatMessageIDRef id); // return item from whom the order has changed
	size_t size() const
	{
		return nodes_.size();
	}
	bool empty() const
	{
		return first_ == end_node_;
	}

	//TODO: for debug
	bool integrity_check()const;
private:
	//return iterator to inserted element
	detail::Node * create_node_before_force(
		ChatMessageIDRef id,
		ChatMessageIDRef parent,
		ChatMessageTimestamp timestamp
	);
	// return new node
	detail::Node* force_insert_before(
		detail::Node*  before,
		ChatMessageIDRef id,
		ChatMessageIDRef parent,
		ChatMessageTimestamp timestamp
	);

	void InsertBefore(
		detail::Node*,
		detail::Node*,
		bool notify_order_change = true);
	void InsertAfter(
		detail::Node* after,
		detail::Node* node,
		bool notify_order_change = true);
	void SaveNode(std::unique_ptr<detail::Node>&&);
	void Insert(detail::Node *node, detail::Node *parent = nullptr);
	void Merge(detail::Node*parent,detail::Node* start, detail::Node* node);
	void MergeWithLostNode(detail::Node *, detail::Node* /*parent*/);
	void CutTail(detail::Node *);

	detail::LastNode last_node_;
	detail::Node *first_;
	detail::Node *end_node_;

	vs::set<detail::Node*> nodes_with_changed_order_;
	vs::map<ChatMessageIDRef, detail::Node*> search_links_;
	vs::map <ChatMessageIDRef, vs::set<detail::Node*>>	lost_idx_;
	vs::map<detail::Node*, std::unique_ptr<detail::Node>> nodes_;

	// TODO: for debug
	//std::vector<std::pair<std::string,detail::Node*>> create_order_;
};
}
}
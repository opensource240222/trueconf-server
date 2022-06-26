#pragma once

#include "chatlib/chatinfo/info_types.h"
#include "chatlib/chain/ChainContainer.h"
#include "chatlib/chat_defs.h"
#include "chatlib/storage/helpers.h"

#include "std-generic/compat/functional.h"
#include "std-generic/compat/map.h"
#include "std-generic/cpplib/StrCompare.h"

namespace chat
{
namespace detail
{
enum class PutToChainErrorCode
{
	undef,
	success,
	out_of_range
};
struct chain_item_ref
{
	VS_FORWARDING_CTOR3(chain_item_ref, msg_id, prev_id, timestamp) {}
	explicit chain_item_ref(const chain_item &src)
		: msg_id(src.msg_id)
		, prev_id(src.prev_id)
		, timestamp(src.timestamp)

	{}
	ChatMessageIDRef msg_id;
	ChatMessageIDRef prev_id;
	ChatMessageTimestamp timestamp;
};
struct chain_item_range_t
{
	VS_FORWARDING_CTOR3(chain_item_range_t, first, last, parent_is_lost) {}
	chain_item_range_t()
		: parent_is_lost(false)
	{}
	chain::const_iterator first;
	chain::const_iterator last;
	bool parent_is_lost;
};
struct PutToChainResult
{
	VS_FORWARDING_CTOR2(PutToChainResult,err, range)
	{}
	PutToChainResult()
		: err(PutToChainErrorCode::undef)
	{}
	PutToChainErrorCode err;
	chain_item_range_t range;
};
class ChainInfo
{
	ChatID chat_id_;
	std::weak_ptr<GlobalConfigInterface> global_cfg_;
	chain::ChainContainer chain_;
	uint32_t default_chain_len_;
	bool all_in_memory_ = false;

	void update_chain_from_db();
	PutToChainResult Insert(
		chain_item_ref item);
public:
	ChainInfo & operator=(const ChainInfo&) = delete;
	ChainInfo(const ChainInfo&) = delete;
	ChainInfo(ChatIDRef chat_id, const GlobalConfigPtr &);
	ChainInfo & operator=(ChainInfo&&other) = default;
	ChainInfo(ChainInfo&&) = default;

	PutToChainResult PutMessage(
		const msg::ChatMessagePtr&msg,
		bool is_outgoing);
	bool IsInMemory() const
	{
		return all_in_memory_;
	}
	std::pair<bool, ChatMessageID> GetPrevious(ChatMessageIDRef id) const;
	void EraseMsg(ChatMessageIDRef id);
	template<class OutputIt>
	void GetMsgIdxInOrder(OutputIt dst, const int32_t tail_max_len) const
	{
		auto counter = tail_max_len;
		for (const auto &i : chain_)
		{
			if (counter == 0)
				break;
			*dst++ = i.id;
		}
	}
	template<class OutputIt >
	OutputIt CopyFromContainer(OutputIt d_first) const
	{
		return std::copy(chain_.begin(), chain_.end(), d_first);
	}
	const chain::ChainContainer & GetChain() const
	{
		return chain_;
	}
};
}
class ChainOfMessages
{
	std::weak_ptr<GlobalConfigInterface> global_config_;
	vs::map<ChatID, detail::ChainInfo, vs::str_less> chains_by_chat_;
public:
	ChainOfMessages() = default;
	ChainOfMessages(ChainOfMessages&&) = default;
	void Init(const GlobalConfigPtr &);
	detail::PutToChainResult
		PutMessage(
			const msg::ChatMessagePtr &msg,
			bool is_outgoing);
	std::pair<bool,ChatMessageID> PreviousMessage(
		ChatIDRef,
		ChatMessageIDRef id) const;
	bool IsChainInMemory(ChatIDRef id) const;
	// return true if chain after updating is not empty, false otherwise
	bool UpdateChainFromDB(ChatIDRef chat_id);
	void Erase(const msg::ChatMessagePtr &m);
	template<class OutputIt>
	void GetMsgIdxInOrder(
		ChatIDRef id,
		OutputIt out,
		const int32_t tail_max_len = -1) const
	{
		auto iter = chains_by_chat_.find(id);
		if (iter == chains_by_chat_.end())
			return;
		iter->second.GetMsgIdxInOrder(out, tail_max_len);
	}
	template<class OutputIt >
	OutputIt CopyFromContainer(ChatIDRef id, OutputIt d_first) const
	{
		auto it = chains_by_chat_.find(id);
		if (it == chains_by_chat_.end())
			return d_first;
		return it->second.CopyFromContainer(d_first);
	}
	const chain::ChainContainer &GetChain(ChatIDRef id) const;
};
}
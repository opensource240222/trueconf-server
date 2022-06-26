#include "ChainOfMessages.h"

#include "chatlib/interface/GlobalConfigInterface.h"
#include "chatlib/msg/ChatMessage.h"
#include "chatlib/msg/attr.h"
#include "chatlib/storage/ChatStorage.h"
#include "chatlib/utils/chat_utils.h"

#include <cassert>
#include <iterator>

#define DEBUG_CURRENT_MODULE VS_DM_CHATS
namespace chat
{
namespace detail
{
ChainInfo::ChainInfo(ChatIDRef chat_id, const GlobalConfigPtr&cfg)
	: chat_id_(std::move(chat_id))
	, global_cfg_(cfg)
	, default_chain_len_(cfg->GetMaxChainLen())
{
	update_chain_from_db();
}
void ChainInfo::update_chain_from_db()
{
	auto storage = global_cfg_.lock()->GetChatStorage();
	if (!storage)
		return;
	auto tail_from_db = storage->GetLastMessages(chat_id_, default_chain_len_);
	if (!tail_from_db.empty())
	{
		chain_ = chain::ChainContainer(
			tail_from_db.begin(),
			tail_from_db.end());
	}
	all_in_memory_ = storage->CountMessagesInChat(chat_id_) == chain_.size();
}
PutToChainResult ChainInfo::PutMessage(
	const msg::ChatMessagePtr&msg,
	bool is_outgoing)
{
	auto msg_id = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	ChatMessageTimestamp timestamp;
	if (msg_id.empty()
		|| !msg->GetParam(attr::TIMESTAMP_paramName, timestamp))
	{
		return {};
	}
	if (is_outgoing)
	{
		chain_.push_back(msg_id, timestamp);
		auto &last = chain_.back();
		msg->SetParam(attr::PREVIOUS_MESSAGE_ID_paramName, last.parent);
		return {PutToChainErrorCode::success,
			chain_item_range_t{--chain_.end(),chain_.end(), false}};
	}
	else
	{
		auto previous_id = msg->GetParamStrRef(attr::PREVIOUS_MESSAGE_ID_paramName);
		/**
			1. if previous_id in chain or all chat is in memory => insert;
			2. if previous_id isn't in chain => return error out_of_range;
		*/
		return Insert({msg_id, previous_id, timestamp });
	}
}
PutToChainResult ChainInfo::Insert(chain_item_ref item)
{
	if(item.msg_id.empty())
		return {};
	if (all_in_memory_) // all chain in memory
	{
		/**
		 * if all chain in memory insert directly.
		 * */
		auto where_it = chain_.insert(item.msg_id, item.prev_id, item.timestamp);
		auto prev_is_lost = !item.prev_id.empty()
			&& chain_.find(item.prev_id) == chain_.end();
		return {PutToChainErrorCode::success,
			chain_item_range_t{ where_it,chain_.end(), prev_is_lost }};
	}
	auto it = item.prev_id.empty()
		? chain_.end()
		: chain_.find(item.prev_id);
	if (it != chain_.end())
	{
		auto where_it = chain_.insert(it, item.msg_id, item.timestamp);
		return {PutToChainErrorCode::success,
			chain_item_range_t{ where_it , chain_.end(), false }};
	}
	else
		return {PutToChainErrorCode::out_of_range,
			chain_item_range_t{}};
}
std::pair<bool, ChatMessageID> ChainInfo::GetPrevious(ChatMessageIDRef id) const
{
	auto iter = chain_.find(id);
	if(iter == chain_.end())
		return {false, {}};
	if(iter != chain_.begin())
		return {true, (--iter)->id};
	return {true, {}};
}
void ChainInfo::EraseMsg(ChatMessageIDRef id)
{
	chain_.erase(id);
}
}
void ChainOfMessages::Init(const GlobalConfigPtr &cfg)
{
	global_config_ = cfg;
}
detail::PutToChainResult ChainOfMessages::PutMessage(
	const msg::ChatMessagePtr& msg,
	bool is_outgoing)
{
	auto id = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	if (id.empty())
		return {};
	auto iter = chains_by_chat_.find(id);
	if (iter == chains_by_chat_.end())
	{
		auto cfg = global_config_.lock();
		if (!cfg)
			return {};
		iter = chains_by_chat_.emplace(id, detail::ChainInfo(id, cfg)).first;
	}
	return iter->second.PutMessage(msg, is_outgoing);
}
std::pair<bool, ChatMessageID> ChainOfMessages::PreviousMessage(
	ChatIDRef chat_id,
	ChatMessageIDRef msg_id) const
{
	auto iter = chains_by_chat_.find(chat_id);
	if (iter != chains_by_chat_.end())
		return iter->second.GetPrevious(msg_id);
	return {false, ChatMessageID()};
}
bool ChainOfMessages::IsChainInMemory(ChatIDRef id) const
{
	auto it = chains_by_chat_.find(id);
	if (it == chains_by_chat_.end())
		return false;
	return it->second.IsInMemory();
}
bool ChainOfMessages::UpdateChainFromDB(ChatIDRef chat_id)
{
	auto cfg = global_config_.lock();
	if (!cfg)
		return false;
	auto chain_it = chains_by_chat_.try_emplace(ChatID(chat_id), chat_id, cfg);
	if (!chain_it.second)
		chain_it.first->second = detail::ChainInfo(chat_id, cfg);
	return !chain_it.first->second.GetChain().empty();
}
void ChainOfMessages::Erase(const msg::ChatMessagePtr &msg)
{
	auto chat_id = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	assert(!chat_id.empty());
	auto msg_id = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	assert(!msg_id.empty());
	if (chat_id.empty() || msg_id.empty())
		return;
	auto chain_iter = chains_by_chat_.find(chat_id);
	if (chain_iter == chains_by_chat_.end())
		return;
	chain_iter->second.EraseMsg(msg_id);
}
const chain::ChainContainer &ChainOfMessages::GetChain(ChatIDRef id) const
{
	static const chain::ChainContainer dummy_obj;
	auto chain = chains_by_chat_.find(id);
	if (chain == chains_by_chat_.end())
		return dummy_obj;
	else
		return chain->second.GetChain();
}
}
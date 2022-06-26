#include "MainStorageLayer.h"

#include "chatlib/helpers/AccountInfo.h"
#include "chatlib/interface/GlobalConfigInterface.h"
#include "chatlib/msg/attr.h"
#include "chatlib/notify/GlobalChatEvents.h"
#include "chatlib/storage/ChatStorage.h"
#include "chatlib/utils/chat_utils.h"
#include "chatlib/utils/msg_utils.h"

#include <cassert>

#define DEBUG_CURRENT_MODULE VS_DM_CHATS
namespace chat
{
template<typename LayersTraits>
const std::string MainStorageLayer<LayersTraits>::main_storage_layer_id_ = "MainStorageLayer";

template<typename LayersTraits>
template<typename ...Args>
MainStorageLayer<LayersTraits>::MainStorageLayer(
	const ChatStoragePtr& impl,
	Args&& ...args)
	: LayerHelper<LayersTraits>(std::forward<Args>(args)...)
	, storage_backend_(std::move(impl))
{
}
template<typename LayersTraits>
void MainStorageLayer<LayersTraits>::Init(const GlobalConfigPtr &cfg)
{
	bucket_.Init(cfg);
	chain_.Init(cfg);
	events_notifier_ = cfg->GetEventsNotifier();
	current_account_ = cfg->GetCurrentAccount();
	cfg_ = cfg;
}
template<typename LayersTraits>
void MainStorageLayer<LayersTraits>::CreateP2PChatByFirstMessage(msg::ChatMessagePtr&& msg)
{
	std::string to = GetParamStrFromMsgContent(msg, msg::toKeyName);
	GetP2PChat(
		current_account_->GetCallID(), to,
		[
			this,
			msg_ptr = msg.release()
		](GlobalContext&& chat_info)
	{
		auto init_msg = msg::ChatMessagePtr(msg_ptr);
		if (chat_info.chatId.empty())
			return;
		// In case when messages send very often chat could already be created
		// while we waited async function result (GetP2PChat).
		auto create_chat_msg = msg::CreateP2PChatMessage(
			chat_info.chatId, current_chat_version,
			chat_info.participants,
			current_account_->GetCallID(),
			current_account_->GetExactCallID(),
			current_account_->GetCallIDType(),
			[
				this,
				init_msg_ref = init_msg.release()
			](
				cb::ProcessingResult res,
				ChatMessageIDRef,
				const cb::MsgIdAndOrderInChain&)
		{
			auto init_msg = msg::ChatMessagePtr(init_msg_ref);
			if(res == cb::ProcessingResult::ok)
				ForwardBelowMessage(std::move(init_msg), {});
		}
		).AcquireMessage();
		ForwardBelowMessage(std::move(create_chat_msg), {});
	});
}
template<typename LayersTraits>
void MainStorageLayer<LayersTraits>::GetP2PChat(
	CallIDRef peer1, CallIDRef peer2, const cb::GetGlobalContextCallBack& cb)
{
	auto id = GetP2PChatID(peer1, peer2);
	auto ctx = storage_backend_->GetGlobalContext(id);
	if (!ctx.chatId.empty())
	{
		cb(std::move(ctx));
		return;
	}
	auto resolver = GetCfg()->GetResolver();
	std::vector<CallID> idxForResolve{ CallID(peer1), CallID(peer2) };
	resolver->ResolveList(std::move(idxForResolve),
		[
			this,
			cb,
			chatId = std::move(id)
		](vs::ResolverInterface::ResolveListResult && res)
	{
		auto incorrect = std::find_if(res.begin(), res.end(),
			[](const std::pair<bool, vs::ResolverInterface::ResolveInfo> & info)
		{
			return !info.first || info.second.type == vs::CallIDType::undef;
		});
		if (incorrect != res.end())
		{
			cb({});
			return;
		}
		vs::set<ParticipantDescr, vs::less<>> parts;
		parts.emplace(res[0].second.callId,
			res[0].second.type == vs::CallIDType::client
			? ParticipantType::client
			: ParticipantType::server);
		parts.emplace(res[0].second.bsInfo, ParticipantType::server);
		parts.emplace(res[1].second.callId,
			res[1].second.type == vs::CallIDType::client
			? ParticipantType::client
			: ParticipantType::server);
		parts.emplace(res[1].second.bsInfo, ParticipantType::server);
		auto ctx = GlobalContext(
			std::move(chatId),
			std::string(),
			ChatType::p2p,
			current_chat_version,
			CallID(),
			ChatMessageTimestamp(),
			ChatMessageTimestamp(),
			ChatMessageID(),
			std::move(parts),
			vs::set<ParticipantDescr, vs::less<>>()
		);
		cb(std::move(ctx));
	});
}
template<typename LayersTraits>
GlobalConfigPtr MainStorageLayer<LayersTraits>::GetCfg() const
{
	auto cfg = cfg_.lock();
	assert(cfg);
	return cfg;
}
template<typename LayersTraits>
void MainStorageLayer<LayersTraits>::ForwardBelowMessage(msg::ChatMessagePtr&& msg,
	std::vector<ParticipantDescr>&& /*dstParts*/)
{
	// DB returns dstParts
	this->PostCall(vs::move_handler(
		[
			this,
			msg = std::move(msg)
		]() mutable
	{
		if (IsChatP2P(msg))
		{
			//	for outgoing message
			//	if message is for p2p chat and not CreateChat(p2p) check existence messages in storage if dosn't
			//	make CreateChat(p2p), AddPart(peer1), AddPart(peer2), AddPart(bs peer1), AddPart(bs peer2)
			// and send all, then send initial message
			auto chat_id = msg->GetParamStrRef(attr::CHAT_ID_paramName);
			const auto& chain = chain_.GetChain(chat_id);
			auto peer_to = GetParamStrFromMsgContent(msg, msg::toKeyName);
			if (chain.empty() && !peer_to.empty())
			{
				if (!chain_.UpdateChainFromDB(chat_id))
				{
					CreateP2PChatByFirstMessage(std::move(msg));
					return;
				}
			}
		}
		auto res = ProcessingMsg(msg, true);
		if(res.first)
		{
			this->Send(std::move(msg), std::move(res.second));
		}
	}));
}
template<typename LayersTraits>
void MainStorageLayer<LayersTraits>::OnChatMessageArrived(
	msg::ChatMessagePtr&&msg,
	CallIDRef sender)
{
	if (ProcessingReqResp(msg))
		return;
	if(ProcessingMsg(msg, false).first)
		ForwardAboveMessage(std::move(msg), sender);
}
template<typename LayersTraits>
void MainStorageLayer<LayersTraits>::ShutDown()
{
	LayerHelper<LayersTraits>::ShutDown();
}
template<typename LayersTraits>
void MainStorageLayer<LayersTraits>::FetchAllUserPersonalContexts(const cb::FetchAllUserPersonalContextsCallBack& cb)
{
	auto id = pending_fetch_all_pers_ctxs_.withLock([&](auto &cnt)
	{
		uint32_t id(0);
		do
		{
			id = Generate32();
		} while (!cnt.emplace(id, cb).second);
		return id;
	});
	msg::FecthAllUserPersonalCtxsReqMessage m(id, current_account_->GetBS(), current_account_->GetExactCallID());
	this->Send(m.AcquireMessage(), {});
}
template<typename LayersTraits>
void MainStorageLayer<LayersTraits>::RequestGlobalContext(
	ChatIDRef id, CallIDRef from, const cb::GetGlobalContextCallBack &cb)
{
	auto reqId = pending_get_glob_ctx_.withLock([&](get_global_ctxs_cbs &cnt)
	{
		uint32_t reqId(0);
		auto check_req_id = [&cnt, &cb](uint32_t reqId)
		{
			return cb
				? cnt.emplace(reqId, cb).second
				: cnt.count(reqId) == 0;
		};
		do
		{
			reqId = Generate32();
		} while (!check_req_id(reqId));
		return reqId;
	});
	msg::GetGlobalContextReqMessage m(reqId, id,
		from, current_account_->GetExactCallID());
	this->Send(m.AcquireMessage(), {});
}
template<typename LayersTraits>
bool MainStorageLayer<LayersTraits>::ProcessingReqResp(
	const msg::ChatMessagePtr& msg)
{
	if (msg::FecthAllUserPersonalCtxsReqMessage::IsMyMessage(msg))
	{
		auto req = msg::FecthAllUserPersonalCtxsReqMessage::GetReqData(msg);
		if (req.res && current_account_->IsMyCallId(req.to))
		{
			auto call_id = CallID(GetCallIDByEpName(req.from));
			auto ctx = storage_backend_->GetAllUserPersonalContexts(call_id);
			msg::FetchAllUserPersonalCtxsRespMessage resp;
			if (resp.MakeResponse(req, ctx))
				this->Send(resp.AcquireMessage(), {});
		}
		return true;
	}
	else if (msg::FetchAllUserPersonalCtxsRespMessage::IsMyMessage(msg))
	{
		auto resp = msg::FetchAllUserPersonalCtxsRespMessage::GetResponseData(msg);
		auto cb = pending_fetch_all_pers_ctxs_.withLock([&](auto& cnt) -> cb::FetchAllUserPersonalContextsCallBack
		{
			auto cb_it = cnt.find(resp.reqId);
			if (cb_it == cnt.end())
				return {};
			auto cb = cb_it->second;
			cnt.erase(cb_it);
			return cb;
		});
		for (const auto& ctx : resp.allCtxs)
		{
			storage_backend_->SavePersonalContext(ctx);
		}
		if (cb)
			cb();
		return true;
	}
	else if (msg::GetGlobalContextReqMessage::IsMyMessage(msg))
	{
		auto res = msg::GetGlobalContextReqMessage::GetReqData(msg);
		if (res.res)
		{
			// FIXME: check that sender has rights to get chat info
			auto ctx = storage_backend_->GetGlobalContext(res.chatId);
			if (!ctx.chatId.empty())
			{
				msg::GetGlobalContextRespMessage resp;
				if (resp.MakeResponse(res.reqId, res.from, ctx))
					this->Send(resp.AcquireMessage(), {});
			}
		}
		return true;
	}
	else if (msg::GetGlobalContextRespMessage::IsMyMessage(msg))
	{
		auto respInfo = msg::GetGlobalContextRespMessage::GetResponseData(msg);
		if (respInfo.res)
		{
			storage_backend_->SaveGlobalContext(respInfo.ctxInfo);
			auto cb = pending_get_glob_ctx_.withLock([&](get_global_ctxs_cbs &cnt)
			{
				auto cb_it = cnt.find(respInfo.reqId);
				auto cb = cb_it == cnt.end()
					? get_global_ctxs_cbs::value_type::second_type()
					: cb_it->second;
				if (cb_it != cnt.end())
					cnt.erase(cb_it);
				return cb;
			});
			if (cb)
				cb(std::move(respInfo.ctxInfo));
		}
		return true;
	}
	return false;
}
template<typename LayersTraits>
std::pair<bool, std::vector<ParticipantDescr>>
MainStorageLayer<LayersTraits>::ProcessingMsg(const msg::ChatMessagePtr &msg,
	bool is_outgoing)
{
	if (!IsMessageStorable(msg))
		return { true, {} };
	// FIXME: DB returns appropriate error code if message already exists.
	if (!is_outgoing
		&& storage_backend_->IsMessageExist(msg->GetParamStr(attr::MESSAGE_ID_paramName)))
	{
		return { false, {} };
	}
	if (!bucket_.PutMessage(msg, is_outgoing))
	{
		if (events_notifier_)
		{
			events_notifier_->OnErrMessageMalformed(
				msg,
				main_storage_layer_id_);
		}
		return { false, {} };
	}
	// add prev_id param for outgoing messages
	auto put_res = chain_.PutMessage(msg, is_outgoing);
	if (!is_outgoing && msg::AddPartMessage::IsMyMessage(msg))
	{
		auto call_id = current_account_->GetCallID();
		auto partId = GetParamStrFromMsgContent(msg, msg::nameKeyName);
		if (call_id == partId)
		{
			auto ctx = msg::AddPartMessage::GetGlobalContext(msg);
			if (!ctx.chatId.empty())
				storage_backend_->SaveGlobalContext(ctx);
		}
	}

	auto save_msg_res = SaveMessage(msg, !is_outgoing);
	if (save_msg_res.error != detail::SaveMessageError::success)
		return { false, {} };
	auto chat_id = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	if (put_res.err == detail::PutToChainErrorCode::out_of_range)
		chain_.UpdateChainFromDB(chat_id);
	assert(save_msg_res.chainUpdateEv.filled);
	auto parts = std::move(save_msg_res.participants);
	Notify(std::move(save_msg_res), msg, is_outgoing);
	return { true, std::move(parts) };
}
template<typename LayersTraits>
void MainStorageLayer<LayersTraits>::Notify(
	detail::SaveChatMessageResult&& saveMsgRes,
	const msg::ChatMessagePtr& msg,
	bool isOutgoing)
{
	auto notifier = GetCfg()->GetEventsNotifier();
	MsgInChain(msg, std::move(saveMsgRes.chainUpdateEv.msgIdAndOrder));
	auto msgId = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	if (saveMsgRes.personalCtxUpdateEv.filled)
	{
		for (const auto& id : saveMsgRes.personalCtxUpdateEv.chats)
		{
			auto ctx = storage_backend_->GetPersonalContext(
				id, saveMsgRes.personalCtxUpdateEv.owner);
			if (!ctx.chatId.empty())
				notifier->OnPersonalContextUpdated(ctx);
		}
	}
	auto global_ctx_update_ev_notifier = [this, notifier, msgId](const msg::ChatMessagePtr& mes) {
		if (msg::CreateChatMessage::IsMyMessage(mes)
			|| msg::CreateP2PChatMessage::IsMyMessage(mes))
		{
			auto chatInfo = msg::CreateChatMessage::IsMyMessage(mes)
				? msg::CreateChatMessage::GetGlobalContext(mes)
				: msg::CreateP2PChatMessage::GetGlobalContext(mes);
			if (chatInfo.chatId.empty())
				return;
			notifier->OnChatCreated(chatInfo);
			notifier->OnGlobalContextUpdated(storage_backend_->GetGlobalContext(chatInfo.chatId));
		}
		else if (msg::AddPartMessage::IsMyMessage(mes))
		{
			auto chatId = mes->GetParamStrRef(attr::CHAT_ID_paramName);
			auto author = mes->GetParamStrRef(attr::FROM_paramName);
			auto partId = GetParamStrFromMsgContent(mes, msg::nameKeyName);
			if (chatId.empty()
				|| author.empty()
				|| partId.empty())
				return;
			notifier->OnPartAdded(chatId, partId, msgId, author);
			notifier->OnGlobalContextUpdated(storage_backend_->GetGlobalContext(chat::ChatID{ chatId }));
		}
		else if (msg::RemovePartMessage::IsMyMessage(mes))
		{
			auto chatId = mes->GetParamStrRef(attr::CHAT_ID_paramName);
			auto partId = GetParamStrFromMsgContent(mes, msg::nameKeyName);
			if (chatId.empty() || partId.empty())
				return;
			notifier->OnPartRemoved(chatId, partId, msgId);
			notifier->OnGlobalContextUpdated(storage_backend_->GetGlobalContext(chat::ChatID{ chatId }));
		}
	};
	if (saveMsgRes.globalCtxUpdateEv.filled)
		for (const auto& mes : saveMsgRes.globalCtxUpdateEv.msgs)
			global_ctx_update_ev_notifier(mes);
	else
		global_ctx_update_ev_notifier(msg);
}
template<typename LayersTraits>
detail::SaveChatMessageResult
MainStorageLayer<LayersTraits>::SaveMessage(const msg::ChatMessagePtr &m, bool is_incoming)
{
	CHAT_TRACE(log_stream << "MainStorageLayer::SaveMessage(" << current_account_->GetExactCallID() <<")"
		<< "; chat_id = "
		<< m->GetParamStrRef(attr::CHAT_ID_paramName) << "; msg_id = " << m->GetParamStrRef(attr::MESSAGE_ID_paramName) <<"; from = "
		<< GetParamStrFromMsgContent(m, msg::fromInstanceKeyName));
	auto res = storage_backend_->SaveChatMessage(m, current_account_->GetCallID(), is_incoming);
	m->OnMsgIsStored(res.error == detail::SaveMessageError::success
		? cb::ProcessingResult::ok
		: cb::ProcessingResult::failed);
	return res;
}
template<typename LayersTraits>
void MainStorageLayer<LayersTraits>::MsgInChain(const msg::ChatMessagePtr &msg,
					cb::MsgIdAndOrderInChain&& order_in_chain) const
{
	msg->OnChainUpdateByMsg(cb::ProcessingResult::ok, std::move(order_in_chain));
	events_notifier_->OnChainUpdated(msg);
}
}

#undef DEBUG_CURRENT_MODULE

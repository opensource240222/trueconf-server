#pragma once
#include "chatlib/chain/BucketsOfMessages.h"
#include "chatlib/chain/ChainOfMessages.h"
#include "chatlib/layers/ChatLayerAbstract.h"

namespace chat
{
namespace detail
{
struct SaveChatMessageResult;
}
/*
	FIXME: Rename class
	Responsobility:
		- store message;
		- notify about events generated by message insert
		- provide FetchAllUserPersonalContexts() and RequestGlobalContext() methods
*/
template<typename LayersTraits>
class MainStorageLayer
	: public LayerInterface
	, public LayerHelper<LayersTraits>
{
	const static std::string main_storage_layer_id_;
	void ForwardBelowMessage(msg::ChatMessagePtr&& msg,
		std::vector<ParticipantDescr>&& dstParts) override;
	void OnChatMessageArrived(
		msg::ChatMessagePtr&&msg,
		CallIDRef sender) override;
	void ShutDown() override;
	detail::SaveChatMessageResult
	SaveMessage(const msg::ChatMessagePtr &m, bool is_incoming);
	void MsgInChain(const msg::ChatMessagePtr &m,
		cb::MsgIdAndOrderInChain&& order_in_chain) const;
	std::pair<bool, std::vector<ParticipantDescr>>
		ProcessingMsg(const msg::ChatMessagePtr &m,
			bool is_outgoing);
	// if true skip message else make further processing
	bool ProcessingReqResp(
		const msg::ChatMessagePtr &m);
	void Notify(
		detail::SaveChatMessageResult&& saveMsgRes,
		const msg::ChatMessagePtr& msg,
		bool isOutgoing);
	void CreateP2PChatByFirstMessage(msg::ChatMessagePtr&& msg);
	void GetP2PChat(CallIDRef peer1, CallIDRef peer2,
		const cb::GetGlobalContextCallBack& cb);
	GlobalConfigPtr GetCfg() const;

	std::weak_ptr<GlobalConfigInterface> cfg_;
	AccountInfoPtr current_account_;
	BucketsOfMessages bucket_;
	ChatEventsNotifierPtr events_notifier_;
	ChatStoragePtr storage_backend_;
	ChainOfMessages	chain_;
	// FIXME: invalidate requests by timeout;
	using FetchAllPersCtxsCbs = vs::map<uint32_t, cb::FetchAllUserPersonalContextsCallBack>;
	vs::Synchronized<FetchAllPersCtxsCbs> pending_fetch_all_pers_ctxs_;

	using get_global_ctxs_cbs = vs::map<uint32_t, cb::GetGlobalContextCallBack>;
	vs::Synchronized<get_global_ctxs_cbs> pending_get_glob_ctx_;
	std::set<ChatID> ctxs_updated_by_server_;
public:
	template<typename ...Args>
	MainStorageLayer(const ChatStoragePtr& impl, Args&&...);
	// FIXME: Try delete Init() method
	void Init(const GlobalConfigPtr &ptr);
	// request from BS all personal contexts for store localy
	void FetchAllUserPersonalContexts(const cb::FetchAllUserPersonalContextsCallBack& cb);
	void RequestGlobalContext(ChatIDRef id, CallIDRef from, const cb::GetGlobalContextCallBack &cb);
};
}
#pragma once
#include "chatlib/chatinfo/info_types.h"
#include "chatlib/msg/ChatMessage.h"
#include "chatlib/msg/parse_message.h"
#include "chatlib/msg/parse_msg_content.h"
#include "chatlib/msg_content_types.h"

#include "std-generic/compat/map.h"
#include "std-generic/compat/set.h"
#include "std-generic/cpplib/macro_utils.h"
#include "std-generic/cpplib/StrCompare.h"

namespace chat
{
namespace msg
{
// JSON key names

// Common keys
extern const std::string typeKeyName;
extern const std::string versionKeyName;
extern const std::string titleKeyName;
extern const std::string chatTitleKeyName;
extern const std::string participantsKeyName;
extern const std::string nameKeyName;
extern const std::string toKeyName;
extern const std::string contentKeyName;
extern const std::string codeKeyName;
extern const std::string msgIdKeyName;
extern const std::string chatIdKeyName;
extern const std::string ownerKeyName;
extern const std::string whoAddedKeyName;
extern const std::string p2pPartKeyName;
extern const std::string replyIdKeyName;
extern const std::string forwardTitleKeyName;
extern const std::string permissionsKeyName;
extern const std::string fromInstanceKeyName;
// Global and personal context keys
extern const std::string creatorKeyName;
extern const std::string banListKeyName;
extern const std::string chatCreateTimestampKeyName;
extern const std::string ctxCreateTimestampKeyName;
extern const std::string chatCreatedTimeKeyName;
extern const std::string ctxCreatedTimeKeyName;
extern const std::string deletedKeyName;
extern const std::string getNotificationKeyName;
extern const std::string unreadMsgsKeyName;
extern const std::string draftKeyName;
extern const std::string lastMessageType;
extern const std::string lastMessageSender;
extern const std::string lastMessageSenderType;
extern const std::string lastMessageDBTimestamp;
extern const std::string lastMessageContent;

// FIXME: Move json Encode/Decode in separate file (?)
std::string PartListToJSON(const vs::set<ParticipantDescr, vs::less<>>& parts);
template<typename Res>
Res PartListFromJSON(const std::string& json);
// Chat control messages
class CreateChatMessage : public ChatMessageKeeper
{
public:
	explicit CreateChatMessage(std::unique_ptr<ChatMessage> &&);
	CreateChatMessage(
		ChatIDRef,
		VersionRef,
		ChatType,
		string_view titile,
		CallIDRef creator,
		CallIDRef fromInstance,
		vs::CallIDType senderType,
		const cb::OnChainUpdateByMsgCb &);
	static bool IsMyMessage(const ChatMessagePtr &);
	static chat::GlobalContext
		GetGlobalContext(const ChatMessagePtr& msg);
	static detail::parse_msg_res<create_chat> Parse(const ChatMessagePtr& msg);
	static detail::parse_msg_content_res<create_chat_content>
		ParseMsgContent(const ChatMessagePtr& msg);
};
class CreateP2PChatMessage : public ChatMessageKeeper
{
public:
	CreateP2PChatMessage(
		ChatIDRef chatId,
		VersionRef ver,
		const vs::set<ParticipantDescr, vs::less<>>& partList,
		CallIDRef from,
		CallIDRef fromInstance,
		vs::CallIDType senderType,
		const cb::OnChainUpdateByMsgCb& cb);
	static bool IsMyMessage(const ChatMessagePtr& msg);
	static chat::GlobalContext
		GetGlobalContext(const ChatMessagePtr& msg);
	static detail::parse_msg_res<create_p2p_chat> Parse(const ChatMessagePtr& msg);
	static detail::parse_msg_content_res<create_p2p_chat_content>
		ParseMsgContent(const ChatMessagePtr& msg);
};
class InviteMessage : public ChatMessageKeeper
{
public:
	InviteMessage(
		ChatIDRef chatId,
		ChatType type,
		string_view title,
		VersionRef ver,
		CallIDRef to,
		CallIDRef from,
		CallIDRef fromInstance,
		vs::CallIDType senderType,
		const cb::OnChainUpdateByMsgCb &);
	static bool IsMyMessage(const ChatMessagePtr &);
	static detail::parse_msg_res<invite> Parse(const ChatMessagePtr& msg);
	static detail::parse_msg_content_res<invite_content>
		ParseMsgContent(const ChatMessagePtr& msg);
	bool SetChatMessage(ChatMessagePtr&& m);
};
class InviteResponseMessage : public ChatMessageKeeper
{
public:
	InviteResponseMessage(
		ChatIDRef id,
		CallIDRef to,
		CallIDRef from,
		CallIDRef fromInstance,
		vs::CallIDType senderType,
		ChatMessageIDRef inviteMsgId,
		InviteResponseCode code,
		const cb::OnChainUpdateByMsgCb&);
	static bool IsMyMessage(const ChatMessagePtr &);
	static detail::parse_msg_res<invite_response> Parse(const ChatMessagePtr& msg);
	static detail::parse_msg_content_res<invite_response_content>
		ParseMsgContent(const ChatMessagePtr& msg);
};

class AddPartMessage : public ChatMessageKeeper
{
public:
	AddPartMessage(
		const chat::GlobalContext& ctx,
		CallIDRef partId,
		ParticipantType type,
		CallIDRef from,
		CallIDRef fromInstance,
		vs::CallIDType senderType,
		const cb::OnChainUpdateByMsgCb &);
	static bool IsMyMessage(const ChatMessagePtr &);
	static detail::parse_msg_res<add_part> Parse(const ChatMessagePtr& msg);
	static detail::parse_msg_content_res<add_part_content>
		ParseMsgContent(const ChatMessagePtr& msg);
	static chat::GlobalContext
		GetGlobalContext(const ChatMessagePtr& msg);
};
class RemovePartMessage : public ChatMessageKeeper
{
public:
	RemovePartMessage(
		ChatIDRef chatId,
		CallIDRef partId,
		ParticipantType type,
		CallIDRef from,
		CallIDRef fromInstance,
		vs::CallIDType senderType,
		const cb::OnChainUpdateByMsgCb& cb);
	static bool IsMyMessage(const ChatMessagePtr& msg);
	static detail::parse_msg_res<remove_part> Parse(const ChatMessagePtr& msg);
	static detail::parse_msg_content_res<remove_part_content>
		ParseMsgContent(const ChatMessagePtr& msg);
};
// User messages
class ContentMessage : public ChatMessageKeeper
{
	chat::MessageType m_type = MessageType::undefined;
public:
	ContentMessage&& Reply(ChatMessageIDRef reply_msg_id);
	ContentMessage&& Edit(ChatMessageIDRef original_msg_id);
	ContentMessage&& Text(const std::string& text);
	ContentMessage&& Forward(string_view title);
	ChatMessagePtr Seal(
		ChatIDRef id,
		CallIDRef from,
		CallIDRef fromInstance,
		vs::CallIDType senderType,
		ChatMessageTimestamp timestamp,
		const cb::OnChainUpdateByMsgCb& cb) &&;
};
class TextChatMessage : public ChatMessageKeeper
{
public:
	static bool IsMyMessage(const ChatMessagePtr&);
	static detail::parse_msg_res<text_message> Parse(const ChatMessagePtr& msg);
	static detail::parse_msg_content_res<text_message_content>
		ParseMsgContent(const ChatMessagePtr& msg);
};
// System chat messages
class PartAddedToChatMessage : public ChatMessageKeeper
{
public:
	/**
		ChatIDRef id,
		CallIDRef from,
		CallIDRef fromInstance,
		CallIDRef to,
		ChatMessageTimestamp timestamp,
		const cb::OnChainUpdateByMsgCb& cb

		chat_id - where added
		topic - topic
		msg_id - marker
		p2p_part - for p2p chat
	*/
	PartAddedToChatMessage(
		ChatIDRef chatId,
		CallIDRef from,
		CallIDRef fromInstance,
		vs::CallIDType senderType,
		CallIDRef to,
		ChatIDRef whereWasAdded, // id of chat to which user was added
		string_view title,
		ChatMessageIDRef addPartMsgId, // AddPart message id
		CallIDRef addedPart, // participant who was added
		CallIDRef whoAdded, // who add participant
		CallIDRef p2pPart, // Peer for p2p chat
		ChatMessageTimestamp timestamp,
		const cb::OnChainUpdateByMsgCb& cb);
	static bool IsMyMessage(const ChatMessagePtr& msg);
	static detail::parse_msg_res<part_added_to_chat> Parse(const ChatMessagePtr& msg);
	static detail::parse_msg_content_res<part_added_to_chat_content>
		ParseMsgContent(const ChatMessagePtr& msg);
};
class PartRemovedFromChatMessage : public ChatMessageKeeper
{
public:
	PartRemovedFromChatMessage(
		ChatIDRef chatId,
		CallIDRef from,
		CallIDRef fromInstance,
		vs::CallIDType senderType,
		CallIDRef to,
		ChatIDRef whereRemoved,
		CallID removedPart, // Participant that has been removed
		ChatMessageIDRef removePartMsgId, // RemovePart message id
		std::string leavePermissions, // Set of flags in json {show history, ban, ...}. Not implement yet
		ChatMessageTimestamp timestamp,
		const cb::OnChainUpdateByMsgCb& cb);
	static bool IsMyMessage(const ChatMessagePtr& msg);
	static detail::parse_msg_res<part_removed_from_chat> Parse(const ChatMessagePtr& msg);
	static detail::parse_msg_content_res<part_removed_from_chat_content>
		ParseMsgContent(const ChatMessagePtr& msg);
};

struct MsgIdsInBucket
{
	explicit MsgIdsInBucket(uint64_t bucketId)
		: id(bucketId) {}
	VS_FORWARDING_CTOR2(MsgIdsInBucket, id, msgIds) {}
	uint64_t id;
	std::vector<ChatMessageID> msgIds;
};
using MsgIdsByBuckets = std::vector<MsgIdsInBucket>;

// Internal control messages
class BucketSyncMessage : public ChatMessageKeeper
{
	void PreFill(ChatIDRef chatId, CallIDRef from, string_view method);
	string_view m_sync_id;
public:
	enum class SyncRes
	{
		undef = -1,
		ok = 0,
		failed
	};
	struct CheckSyncInfoSt
	{
		VS_FORWARDING_CTOR5(CheckSyncInfoSt,
			chatId, from, msgsInTail, hash, masterWeight) {}
		CheckSyncInfoSt(){}
		ChatID chatId;
		CallID from;
		uint32_t msgsInTail = 0;
		std::string hash;
		uint32_t masterWeight = 0;
	};
	struct CheckSyncResponseSt
	{
		ChatID chatId;
		CallID from;
		SyncRes result = SyncRes::failed;
	};
	struct BucketIdHash
	{
		VS_FORWARDING_CTOR2(BucketIdHash, id, hash) {}
		bool operator <(const BucketIdHash&rhs) const
		{
			return std::tie(id, hash) < std::tie(rhs.id, rhs.hash);
		}
		uint64_t    id;
		std::string hash;
	};
	static bool IsMyMessage(const ChatMessagePtr&);
	explicit BucketSyncMessage(string_view syncId);
	//make methods
	void MakeBucketListReq(
		ChatIDRef, CallIDRef,
		uint32_t,string_view tailHash, bool syncStart);
	void MakeBucketListResp(
		ChatIDRef, CallIDRef,
		const std::vector<BucketIdHash> &bucketsList);

	void MakeDiffReq(
		ChatIDRef chatId, CallIDRef from,
		const MsgIdsByBuckets& msgsByBuckets);
	void MakeDiffResp(
		ChatIDRef chatId,
		CallIDRef from,
		const std::vector<ChatMessageID>& insertedMsgs);

	void MakeCheckSyncReq(
		ChatIDRef chatId, CallIDRef from,
		uint32_t tailLen, string_view tailHash,
		uint32_t masterWeight);
	void MakeCheckSyncResp(
		ChatIDRef chatId,
		CallIDRef from,
		SyncRes result);

	void MakeSyncResetReq(ChatIDRef chatId, CallIDRef from);


	// get methods
	bool SetChatMessage(ChatMessagePtr&& m);
	static std::vector<BucketIdHash> GetBucketList(const ChatMessagePtr& msg);
	static MsgIdsByBuckets GetDiffReqInfo(const ChatMessagePtr& msg);
	static vs::set<ChatMessageID> GetDiffMsgIds(const ChatMessagePtr& msg);
	static CheckSyncInfoSt GetCheckSyncInfo(const ChatMessagePtr& msg);
	static CheckSyncResponseSt GetCheckSyncResult(const ChatMessagePtr& msg);

};
class DeliveryConfirm : public ChatMessageKeeper
{
	struct DeliveryInfo
	{
		VS_FORWARDING_CTOR5(DeliveryInfo, result, msgId, from, fromInstance, to)
		{}
		DeliveryInfo()
			: result(false)
		{}
		bool result;
		ChatMessageID msgId;
		CallID from;
		CallID fromInstance;
		CallID to;
	};
public:
	static bool IsMyMessage(const ChatMessagePtr& msg);
	DeliveryConfirm(ChatMessageIDRef msgId, CallIDRef from,
		CallIDRef fromInstance, CallIDRef to);
	static DeliveryInfo GetDeliveryInfo(const ChatMessagePtr& msg);
};
class FecthAllUserPersonalCtxsReqMessage : public ChatMessageKeeper
{
public:
	struct GetReqResult
	{
		VS_FORWARDING_CTOR4(GetReqResult, res, reqId, from, to) {}
		GetReqResult()
			: res(false)
			, reqId(-1)
		{}
		bool res;
		uint32_t reqId;
		CallID from;
		CallID to;
	};
	FecthAllUserPersonalCtxsReqMessage(uint32_t id, CallIDRef to, CallIDRef from);
	static bool IsMyMessage(const ChatMessagePtr& msg);
	static GetReqResult GetReqData(const ChatMessagePtr &m);
};
class FetchAllUserPersonalCtxsRespMessage : public ChatMessageKeeper
{
	struct GetResponseResult
	{
		VS_FORWARDING_CTOR3(GetResponseResult, res, reqId, allCtxs)
		{}
		GetResponseResult(uint32_t id)
			: res(false)
			, reqId(id)
		{}
		bool res;
		uint32_t reqId;
		chat::PersonalContextList allCtxs;
	};
public:
	static bool IsMyMessage(const ChatMessagePtr& msg);
	bool MakeResponse(const FecthAllUserPersonalCtxsReqMessage::GetReqResult& req,
		const chat::PersonalContextList& ctxs);
	static GetResponseResult GetResponseData(const ChatMessagePtr& msg);
};
class GetGlobalContextReqMessage : public ChatMessageKeeper
{
	struct GetReqResult
	{
		VS_FORWARDING_CTOR4(GetReqResult,
			res, reqId, chatId, from) {}
		bool res;
		uint32_t reqId;
		ChatID chatId;
		CallID from;
	};
public:
	static bool IsMyMessage(const ChatMessagePtr &m);
	GetGlobalContextReqMessage(uint32_t reqId, ChatIDRef chatId,
		CallIDRef to, CallIDRef from);
	static GetReqResult GetReqData(const ChatMessagePtr &m);
};
class GetGlobalContextRespMessage : public ChatMessageKeeper
{
	struct GetResponseResult
	{
		VS_FORWARDING_CTOR3(GetResponseResult, res, reqId, ctxInfo){}
		GetResponseResult()
			: res(false)
			, reqId(0)
		{}

		bool res;
		uint32_t reqId;
		chat::GlobalContext ctxInfo;
	};
public:
	static bool IsMyMessage(const ChatMessagePtr &m);
	bool MakeResponse(uint32_t reqId, CallIDRef from,
		const chat::GlobalContext &info);
	static GetResponseResult GetResponseData(const ChatMessagePtr&m);
};
class GetTailHashReqMessage : public ChatMessageKeeper
{
	struct GetReqResult
	{
		VS_FORWARDING_CTOR4(GetReqResult, success, chatId, tailLen, from){}
		GetReqResult()
			: success(false)
			, tailLen(0)
		{}
		bool success;
		ChatID chatId;
		uint32_t tailLen;
		CallID from;
	};
public:
	static bool IsMyMessage(const ChatMessagePtr &msg);
	GetTailHashReqMessage(ChatIDRef chatId, uint32_t tailLen,
			CallIDRef from, CallIDRef to);
	static GetReqResult GetReqData(const ChatMessagePtr &msg);
};
class GetTailHashRespMessage : public ChatMessageKeeper
{
public:
	enum class ReqResult
	{
		success,
		chat_not_exist,
		failed
	};
	struct GetResponseResult
	{
		VS_FORWARDING_CTOR6(GetResponseResult,
			success, chatId, reqResult, tailLen, hash, from){}
		GetResponseResult()
			: success(false)
			, reqResult(ReqResult::failed)
			, tailLen(0)
		{}
		bool success;
		ChatID chatId;
		ReqResult reqResult;
		uint32_t tailLen;
		std::string hash;
		CallID from;
	};
	static bool IsMyMessage(const ChatMessagePtr &msg);
	GetTailHashRespMessage(ChatIDRef chatId, ReqResult reqResult, uint32_t tailLen,
			string_view hash, CallIDRef from, CallIDRef to);
	static GetResponseResult GetResponseData(const ChatMessagePtr &msg);
};
class MessageRemovalMessage : public ChatMessageKeeper
{
public:
	MessageRemovalMessage(
		ChatIDRef chatId,
		ChatMessageIDRef msgId,
		bool forAll,
		CallIDRef from,
		CallIDRef fromInstance,
		vs::CallIDType senderType,
		const cb::OnChainUpdateByMsgCb &);
	static bool IsMyMessage(const ChatMessagePtr &);
	static detail::parse_msg_res<remove_message> Parse(const ChatMessagePtr& msg);
	static detail::parse_msg_content_res<remove_message_content>
		ParseMsgContent(const ChatMessagePtr& msg);
};
class ClearHistoryMessage : public ChatMessageKeeper
{
public:
	ClearHistoryMessage(
		chat::ChatIDRef chatId,
		bool forAll,
		CallIDRef from,
		CallIDRef fromInstance,
		vs::CallIDType senderType,
		const cb::OnChainUpdateByMsgCb &);
	static bool IsMyMessage(const ChatMessagePtr &);
	static detail::parse_msg_res<clear_history> Parse(const ChatMessagePtr& msg);
	static detail::parse_msg_content_res<clear_history_content>
		ParseMsgContent(const ChatMessagePtr& msg);
};
}
}
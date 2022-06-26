#pragma once

#include "chatlib/chat_defs.h"
#include "chatlib/messages_types.h"

#include "std-generic/cpplib/macro_utils.h"
#include "std-generic/cpplib/StrCompare.h"

#include <map>
namespace chat
{
namespace cb
{
struct ChatDescription
{
	VS_FORWARDING_CTOR4(ChatDescription, chatID,
		type, title, version) {};
	ChatDescription(ChatDescription&&) = default;
	ChatID chatID;
	ChatType type;
	std::string title;
	Version version;
};
struct UndeliveredMsg
{
	VS_FORWARDING_CTOR2(UndeliveredMsg, msg, parts)
	{}
	msg::ChatMessagePtr msg;
	std::vector<ParticipantDescr> parts;
};
enum class CreateChatResult
{
	ok,
	failed
};
enum class ProcessingResult
{
	undef,
	ok,
	failed
};
enum class ErrorCode
{
	success,
	pending,
	expired,
	failed
};
using MsgIdAndOrderInChain = std::vector<std::pair<ChatMessageID, OrderInChain>>;
using GetGlobalContextCallBack = std::function<void(GlobalContext&&)>;
// FIXME: Check necessity
using ChatMsgIDChainCallBack = std::function<void(ErrorCode, std::vector<ChatMessageID>&&)>;
using FetchAllUserPersonalContextsCallBack = std::function<void()>;
using UndeliveredMsgCallBack = std::function<void(std::vector<UndeliveredMsg>&&)>;
using CreateChatCallBack = std::function<void(CreateChatResult, ChatIDRef)>;
using InviteResponseCallBack = std::function<void(const InviteResponseCode &)>;
using OnInviteArrivedCallBack =
std::function<void(ChatDescription&&, const InviteResponseCallBack &)>;
using OnInviteResponseArrivedCallBack =
std::function<void(ChatIDRef, CallIDRef, InviteResponseCode)>;
using OnMsgRecv = nod::signal<void(
	const cb::MsgIdAndOrderInChain&,
	const msg::ChatMessagePtr & /*msg*/)>;
using OnChainUpdateByMsgCb = std::function<void(
	ProcessingResult,
	ChatMessageIDRef, // msg id
	const MsgIdAndOrderInChain& // order in chain
	)>;
using OnMessageIsStoredCallBack =
std::function<void(
	ProcessingResult,
	ChatIDRef,
	ChatMessageIDRef)>;
}
}
#pragma once
#include "chatlib/callbacks.h"
#include "chatlib/chatinfo/info_types.h"
#include "chatlib/msg/ChatMessage.h"

#include "std-generic/cpplib/macro_utils.h"

#include <map>
#include <vector>
namespace chat
{
namespace detail
{
struct ChainUpdateEvent
{
	VS_FORWARDING_CTOR2(ChainUpdateEvent,
		filled,	msgIdAndOrder)
	{}
	ChainUpdateEvent()
		: filled(false)
	{}
	bool filled;
	cb::MsgIdAndOrderInChain msgIdAndOrder;
};
struct GlobalContextUpdateEvent
{
	VS_FORWARDING_CTOR2(GlobalContextUpdateEvent,
		filled, msgs)
	{}
	GlobalContextUpdateEvent()
		: filled(false)
	{}
	bool filled;
	std::vector<msg::ChatMessagePtr> msgs;
};
struct PersonalContextUpdateEvent
{
	VS_FORWARDING_CTOR3(PersonalContextUpdateEvent,
		filled, owner, chats)
	{}
	PersonalContextUpdateEvent()
		: filled(false)
	{}
	bool filled;
	CallID owner;
	std::set<ChatID> chats;
};
enum class SaveMessageError
{
	undef = -1,
	success = 0,
	parseContentError = 1,
	invalidMsgTypeForChatError = 2,
	alreadyExistError = 3
};
struct SaveChatMessageResult
{
	VS_FORWARDING_CTOR5(SaveChatMessageResult,
		error, participants,
		chainUpdateEv, globalCtxUpdateEv,
		personalCtxUpdateEv) {}
	explicit SaveChatMessageResult(SaveMessageError err)
		: error(err)
	{}
	SaveChatMessageResult()
		: error(SaveMessageError::undef)
	{}
	// error
	SaveMessageError error;
	// parts for transmit
	std::vector<ParticipantDescr> participants;
	// events with data
	ChainUpdateEvent chainUpdateEv;
	GlobalContextUpdateEvent globalCtxUpdateEv;
	PersonalContextUpdateEvent personalCtxUpdateEv;
};
enum class ExecType
{
	execute,
	query
};
struct ScriptDescr
{
	VS_FORWARDING_CTOR3(ScriptDescr, query, how, usePlaceholder) {}
	ScriptDescr(std::string query_p)
		: query(std::move(query_p))
		, how(ExecType::execute)
		, usePlaceholder(false)
	{}
	ScriptDescr(std::string query_p, ExecType how_p)
		: query(std::move(query_p))
		, how(how_p)
		, usePlaceholder(false)
	{}
	std::string query;
	ExecType how;
	bool usePlaceholder;
};
}
}
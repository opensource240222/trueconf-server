#pragma once
#include "chatlib/chat_defs.h"
#include "chatlib/msg/attr.h"

#include "std-generic/compat/functional.h"
#include "std-generic/compat/set.h"
#include "std-generic/cpplib/macro_utils.h"

namespace chat
{
struct ParticipantDescr
{
	VS_FORWARDING_CTOR2(ParticipantDescr, partId, tp) {}
	CallID partId;
	ParticipantType tp;
	bool operator==(const ParticipantDescr &rhs) const
	{
		return std::tie(partId, tp) == std::tie(rhs.partId, rhs.tp);
	}
	bool operator<(const ParticipantDescr &rhs) const
	{
		return std::tie(partId, tp)
			< std::tie(rhs.partId, rhs.tp);
	}
};
struct ParticipantDescrRef
{
	VS_FORWARDING_CTOR2(ParticipantDescrRef, partId, tp) {}
	CallIDRef partId;
	ParticipantType tp;
	bool operator<(const ParticipantDescrRef &rhs) const
	{
		return std::tie(partId, tp)
			< std::tie(rhs.partId, rhs.tp);
	}
	explicit operator ParticipantDescr() const
	{
		return {partId, tp };
	}
};
inline bool operator<(const ParticipantDescr &lhs, const ParticipantDescrRef &rhs)
{
	return std::tie(lhs.partId, lhs.tp) < std::tie(rhs.partId, rhs.tp);
}
inline bool operator<(const ParticipantDescrRef &lhs, const ParticipantDescr &rhs)
{
	return std::tie(lhs.partId, lhs.tp) < std::tie(rhs.partId, rhs.tp);
}
template<class It>
It FindPartDescrById(It first, const It last, CallIDRef partId)
{
	return std::find_if(first, last,
		[&partId](const ParticipantDescr &i)
			{ return i.partId == partId; });
}
struct LastVisibleMessage
{
	VS_FORWARDING_CTOR5(LastVisibleMessage,
		msgType, sender, senderType, DBTimestamp, content)
	{}
	LastVisibleMessage() = default;
	MessageType msgType = MessageType::undefined;
	CallID sender;
	ParticipantType senderType = ParticipantType::undef;
	ChatMessageTimestamp DBTimestamp;
	std::string content;
};
struct PersonalContext
{
	VS_FORWARDING_CTOR14(PersonalContext,
		owner, chatId, title, chatType, version, creator,
		chatCreatedTime, ctxCreatedTime, msgId, isDeleted,
		getNotification, unreadMsgs, draft, lastVisibleMessage)
	{}
	PersonalContext()
		: chatType(ChatType::undef)
		, isDeleted(false)
		, getNotification(false)
	{}
	CallID owner;
	ChatID chatId;
	std::string title;
	ChatType chatType;
	Version version;
	CallID creator;
	ChatMessageTimestamp chatCreatedTime;
	ChatMessageTimestamp ctxCreatedTime;
	ChatMessageID msgId;
	bool isDeleted;
	bool getNotification;
	std::vector<ChatMessageID> unreadMsgs;
	std::string draft;
	LastVisibleMessage lastVisibleMessage;
};
using PersonalContextList = std::vector<PersonalContext>;
struct GlobalContext
{
	VS_FORWARDING_CTOR10(GlobalContext,
		chatId, title, type, version, creator,
		createTimestamp, ctxCreateTimestamp, msgId,
		participants, banList)
		{}
	GlobalContext()
		: type(ChatType::undef)
	{}
	ChatID chatId;
	std::string title;
	ChatType type;
	Version version;
	CallID creator;
	ChatMessageTimestamp createTimestamp;
	ChatMessageTimestamp ctxCreateTimestamp;
	ChatMessageID msgId;
	vs::set<ParticipantDescr, vs::less<>> participants;
	vs::set<ParticipantDescr, vs::less<>> banList;
};
}
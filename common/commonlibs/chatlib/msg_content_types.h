#pragma once
#include "chatlib/chat_defs.h"
#include "chatlib/chatinfo/info_types.h"

#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/macro_utils.h"
#include "std-generic/compat/map.h"

namespace chat
{
namespace msg
{
struct create_chat_content
{
	VS_FORWARDING_CTOR4(create_chat_content, ver, type, title, from_instance) {}
	create_chat_content()
		: type{ChatType::undef}
	{}
	bool operator==(const create_chat_content& other) const
	{
		return ver == other.ver
			&& type == other.type
			&& title == other.title
			&& from_instance == other.from_instance;
	}
	Version ver;
	ChatType type;
	std::string title;
	CallID from_instance;
};
struct create_p2p_chat_content
{
	VS_FORWARDING_CTOR3(create_p2p_chat_content, ver, part_list, from_instance) {}
	create_p2p_chat_content() = default;
	bool operator==(const create_p2p_chat_content& other) const
	{
		return ver == other.ver
			&& part_list == other.part_list
			&& from_instance == other.from_instance;
	}
	Version ver;
	vs::set<ParticipantDescr, vs::less<>> part_list;
	CallID from_instance;
};
struct invite_content
{
	VS_FORWARDING_CTOR5(invite_content, ver, type, title, invitee, from_instance) {}
	invite_content()
		: type{ChatType::undef}
	{}
	bool operator==(const invite_content& other) const
	{
		return ver == other.ver
			&& type == other.type
			&& title == other.title
			&& invitee == other.invitee
			&& from_instance == other.from_instance;
	}
	Version ver;
	ChatType type;
	std::string title;
	CallID invitee;
	CallID from_instance;
};
struct invite_response_content
{
	VS_FORWARDING_CTOR4(invite_response_content, code, invite_msg_id,
		response_to, from_instance) {}
	invite_response_content()
		: code{InviteResponseCode::undef}
	{}
	bool operator==(const invite_response_content& other) const
	{
		return code == other.code
			&& invite_msg_id == other.invite_msg_id
			&& response_to == other.response_to
			&& from_instance == other.from_instance;
	}
	InviteResponseCode code;
	ChatMessageID invite_msg_id;
	CallID response_to;
	CallID from_instance;
};
struct add_part_content
{
	VS_FORWARDING_CTOR3(add_part_content, call_id, type, from_instance) {}
	add_part_content()
		: type(ParticipantType::undef)
	{}
	bool operator==(const add_part_content& other) const
	{
		return call_id == other.call_id
			&& type == other.type
			&& from_instance == other.from_instance;
	}
	CallID call_id;
	ParticipantType type;
	CallID from_instance;
};
struct remove_part_content
{
	VS_FORWARDING_CTOR3(remove_part_content, part, type, from_instance) {}
	remove_part_content()
		: type(ParticipantType::undef)
	{}
	bool operator==(const remove_part_content& other) const
	{
		return part == other.part
			&& type == other.type
			&& from_instance == other.from_instance;
	}
	CallID part;
	ParticipantType type;
	CallID from_instance;
};
struct text_message_content
{
	VS_FORWARDING_CTOR5(text_message_content, text, to, reply_id,
		forward_title, from_instance) {}
	text_message_content() = default;
	bool operator==(const text_message_content& other) const
	{
		return text == other.text
			&& to == other.to
			&& reply_id == other.reply_id
			&& forward_title == other.forward_title
			&& from_instance == other.from_instance;
	}
	std::string text;
	CallID to; // empty in group chat
	ChatMessageID reply_id;
	ChatMessageID forward_title;
	CallID from_instance;
};
struct part_added_to_chat_content
{
	VS_FORWARDING_CTOR8(part_added_to_chat_content, where_chat_id, title,
		add_part_msg_id, who_was_added, who_added, p2p_part, to, from_instance) {}
	part_added_to_chat_content() = default;
	bool operator==(const part_added_to_chat_content& other) const
	{
		return where_chat_id == other.where_chat_id
			&& title == other.title
			&& add_part_msg_id == other.add_part_msg_id
			&& who_was_added == other.who_was_added
			&& who_added == other.who_added
			&& p2p_part == other.p2p_part
			&& to == other.to
			&& from_instance == other.from_instance;
	}
	ChatID where_chat_id;
	std::string title;
	ChatMessageID add_part_msg_id;
	CallID who_was_added;
	CallID who_added;
	CallID p2p_part;
	CallID to; // empty in group chat
	CallID from_instance;
};
struct part_removed_from_chat_content
{
	VS_FORWARDING_CTOR6(part_removed_from_chat_content, where_chat_id, removed_part,
		remove_part_msg_id, leave_permissions, to, from_instance) {}
	part_removed_from_chat_content() = default;
	bool operator==(const part_removed_from_chat_content& other) const
	{
		return where_chat_id == other.where_chat_id
			&& removed_part == other.removed_part
			&& remove_part_msg_id == other.remove_part_msg_id
			&& leave_permissions == other.leave_permissions
			&& to == other.to
			&& from_instance == other.from_instance;
	}
	ChatID where_chat_id;
	CallID removed_part;
	ChatMessageID remove_part_msg_id;
	std::string leave_permissions;
	CallID to; // empty in group chat
	CallID from_instance;
};
struct remove_message_content
{
	VS_FORWARDING_CTOR2(remove_message_content, removed_msg_id, from_instance) {}
	remove_message_content() = default;
	bool operator==(const remove_message_content& other) const
	{
		return removed_msg_id == other.removed_msg_id
			&& from_instance == other.from_instance;
	}
	ChatMessageID removed_msg_id;
	CallID from_instance;
};
struct clear_history_content
{
	VS_FORWARDING_CTOR1(clear_history_content, from_instance) {}
	clear_history_content() = default;
	bool operator==(const clear_history_content& other) const
	{
		return from_instance == other.from_instance;
	}
	CallID from_instance;
};
struct unrecognized_message_content
{
	unrecognized_message_content() = default;
	bool operator==(const unrecognized_message_content& other) const
	{
		return true;
	}
};
} // namespace msg
} // namespace chat

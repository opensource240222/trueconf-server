#pragma once
#include "chatlib/chat_defs.h"
#include "chatlib/chatinfo/info_types.h"
#include "chatlib/msg_content_types.h"

#include "std-generic/cpplib/macro_utils.h"

namespace chat
{
namespace msg
{
struct create_chat
{
	VS_FORWARDING_CTOR5(create_chat, chat_id, msg_id, timestamp, creator, content) {}
	create_chat() = default;
	bool operator==(const create_chat& op) const
	{
		return chat_id == op.chat_id
			&& msg_id == op.msg_id
			&& timestamp == op.timestamp
			&& creator == op.creator
			&& content == op.content;
	}
	ChatID chat_id;
	ChatMessageID msg_id;
	ChatMessageTimestamp timestamp;
	CallID creator;
	create_chat_content content;
};
struct create_p2p_chat
{
	VS_FORWARDING_CTOR4(create_p2p_chat, chat_id, msg_id, timestamp, content) {}
	create_p2p_chat() = default;
	bool operator==(const create_p2p_chat& op) const
	{
		return chat_id == op.chat_id
			&& msg_id == op.msg_id
			&& timestamp == op.timestamp
			&& content == op.content;
	}
	ChatID chat_id;
	ChatMessageID msg_id;
	ChatMessageTimestamp timestamp;
	create_p2p_chat_content content;
};
struct invite
{
	VS_FORWARDING_CTOR5(invite, chat_id, msg_id, timestamp, inviter, content) {}
	invite() = default;
	bool operator==(const invite& op) const
	{
		return chat_id == op.chat_id
			&& msg_id == op.msg_id
			&& timestamp == op.timestamp
			&& inviter == op.inviter
			&& content == op.content;
	}
	ChatID chat_id;
	ChatMessageID msg_id;
	ChatMessageTimestamp timestamp;
	CallID inviter;
	invite_content content;
};
struct invite_response
{
	VS_FORWARDING_CTOR5(invite_response, chat_id, msg_id, timestamp,
		response_from, content) {}
	invite_response() = default;
	bool operator==(const invite_response& op) const
	{
		return chat_id == op.chat_id
			&& msg_id == op.msg_id
			&& timestamp == op.timestamp
			&& response_from == op.response_from
			&& content == op.content;
	}
	ChatID chat_id;
	ChatMessageID msg_id;
	ChatMessageTimestamp timestamp;
	CallID response_from;
	invite_response_content content;
};
struct add_part
{
	VS_FORWARDING_CTOR5(add_part,
		chat_id, msg_id, timestamp, from, content) {}
	add_part() = default;
	bool operator==(const add_part& op) const
	{
		return chat_id == op.chat_id
			&& msg_id == op.msg_id
			&& timestamp == op.timestamp
			&& from == op.from
			&& content == op.content;
	}
	ChatID chat_id;
	ChatMessageID msg_id;
	ChatMessageTimestamp timestamp;
	CallID from;
	add_part_content content;
};
struct remove_part
{
	VS_FORWARDING_CTOR5(remove_part,
		chat_id, msg_id, timestamp, from, content) {}
	remove_part() = default;
	bool operator==(const remove_part& op) const
	{
		return chat_id == op.chat_id
			&& msg_id == op.msg_id
			&& timestamp == op.timestamp
			&& from == op.from
			&& content == op.content;
	}
	ChatID chat_id;
	ChatMessageID msg_id;
	ChatMessageTimestamp timestamp;
	CallID from;
	remove_part_content content;
};
struct text_message
{
	VS_FORWARDING_CTOR6(text_message, chat_id, msg_id, original_id, author, timestamp, content) {}
	text_message() = default;
	bool operator==(const text_message& op) const
	{
		return chat_id == op.chat_id
			&& msg_id == op.msg_id
			&& original_id == op.original_id
			&& author == op.author
			&& timestamp == op.timestamp
			&& content == op.content;
	}
	ChatID chat_id;
	ChatMessageID msg_id;
	ChatMessageID original_id;
	CallID author; 
	ChatMessageTimestamp timestamp;
	text_message_content content;
};
struct part_added_to_chat
{
	VS_FORWARDING_CTOR3(part_added_to_chat, chat_id, msg_id, content) {}
	part_added_to_chat() = default;
	bool operator==(const part_added_to_chat& op) const
	{
		return chat_id == op.chat_id
			&& msg_id == op.msg_id
			&& content == op.content;
	}
	ChatID chat_id;
	ChatMessageID msg_id;
	part_added_to_chat_content content;
};
struct part_removed_from_chat
{
	VS_FORWARDING_CTOR3(part_removed_from_chat, chat_id, msg_id, content) {}
	part_removed_from_chat() = default;
	bool operator==(const part_removed_from_chat& op) const
	{
		return chat_id == op.chat_id
			&& msg_id == op.msg_id
			&& content == op.content;
	}
	ChatID chat_id;
	ChatMessageID msg_id;
	part_removed_from_chat_content content;
};
struct remove_message
{
	VS_FORWARDING_CTOR6(remove_message, chat_id, msg_id, timestamp,
		from, for_all, content) {}
	remove_message() = default;
	bool operator==(const remove_message& op) const
	{
		return chat_id == op.chat_id
			&& msg_id == op.msg_id
			&& timestamp == op.timestamp
			&& from == op.from
			&& for_all == op.for_all
			&& content == op.content;
	}
	ChatID chat_id;
	ChatMessageID msg_id;
	ChatMessageTimestamp timestamp;
	CallID from;
	bool for_all;
	remove_message_content content;
};
struct clear_history
{
	VS_FORWARDING_CTOR6(clear_history, chat_id, msg_id, timestamp,
		from, for_all, content) {}
	clear_history() = default;
	bool operator==(const clear_history& op) const
	{
		return chat_id == op.chat_id
			&& msg_id == op.msg_id
			&& timestamp == op.timestamp
			&& from == op.from
			&& for_all == op.for_all
			&& content == op.content;
	}
	ChatID chat_id;
	ChatMessageID msg_id;
	ChatMessageTimestamp timestamp;
	CallID from;
	bool for_all;
	clear_history_content content;
};
struct unrecognized_msg {
	bool operator==(const unrecognized_msg&) const
	{
		return true;
	}
};
}
}
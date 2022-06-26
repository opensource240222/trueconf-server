#pragma once
#include "chatlib/messages_types.h"
#include "chatlib/msg_content_types.h"

namespace chat
{
namespace msg
{
namespace detail
{
template<typename T>
struct parse_msg_content_res
{
	VS_FORWARDING_CTOR2(parse_msg_content_res<T>, success, content) {}
	parse_msg_content_res<T>()
		: success{false}
	{}
	bool success;
	T content;
};
parse_msg_content_res<create_chat_content>
parse_create_content(const ChatMessagePtr& msg);
parse_msg_content_res<create_p2p_chat_content>
parse_create_p2p_chat_content(const ChatMessagePtr& msg);
parse_msg_content_res<invite_content>
parse_invite_content(const ChatMessagePtr& msg);
parse_msg_content_res<invite_response_content>
parse_invite_response_content(const ChatMessagePtr& msg);
parse_msg_content_res<add_part_content>
parse_add_part_content(const ChatMessagePtr& msg);
parse_msg_content_res<remove_part_content>
parse_remove_part_content(const ChatMessagePtr& msg);
parse_msg_content_res<text_message_content>
parse_text_message_content(const ChatMessagePtr& msg);
parse_msg_content_res<part_added_to_chat_content>
parse_part_added_to_chat_content(const ChatMessagePtr& msg);
parse_msg_content_res<part_removed_from_chat_content>
parse_part_removed_from_chat_content(const ChatMessagePtr& msg);
parse_msg_content_res<remove_message_content>
parse_remove_message_content(const ChatMessagePtr& msg);
parse_msg_content_res<clear_history_content>
parse_clear_history_content(const ChatMessagePtr& msg);
} // namespace detail
template<typename T>
void parse_message_content(const ChatMessagePtr& msg, T&& cb)
{
	{
		auto parse_content_result = detail::parse_create_content(msg);
		if (parse_content_result.success)
		{
			cb(std::move(parse_content_result.content));
		}
	}
	{
		auto parse_content_result = detail::parse_create_p2p_chat_content(msg);
		if (parse_content_result.success)
		{
			cb(std::move(parse_content_result.content));
		}
	}
	{
		auto parse_content_result = detail::parse_invite_content(msg);
		if (parse_content_result.success)
		{
			cb(std::move(parse_content_result.content));
		}
	}
	{
		auto parse_content_result = detail::parse_invite_response_content(msg);
		if (parse_content_result.success)
		{
			cb(std::move(parse_content_result.content));
		}
	}
	{
		auto parse_content_result = detail::parse_add_part_content(msg);
		if (parse_content_result.success)
		{
			cb(std::move(parse_content_result.content));
		}
	}
	{
		auto parse_content_result = detail::parse_remove_part_content(msg);
		if (parse_content_result.success)
		{
			cb(std::move(parse_content_result.content));
		}
	}
	{
		auto parse_content_result = detail::parse_text_message_content(msg);
		if (parse_content_result.success)
		{
			cb(std::move(parse_content_result.content));
		}
	}
	{
		auto parse_content_result = detail::parse_part_added_to_chat_content(msg);
		if (parse_content_result.success)
		{
			cb(std::move(parse_content_result.content));
		}
	}
	{
		auto parse_content_result = detail::parse_part_removed_from_chat_content(msg);
		if (parse_content_result.success)
		{
			cb(std::move(parse_content_result.content));
		}
	}
	{
		auto parse_content_result = detail::parse_remove_message_content(msg);
		if (parse_content_result.success)
		{
			cb(std::move(parse_content_result.content));
		}
	}
	{
		auto parse_content_result = detail::parse_clear_history_content(msg);
		if (parse_content_result.success)
		{
			cb(std::move(parse_content_result.content));
		}
	}
}
} // namespace msg
} // namespace chat

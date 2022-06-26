#pragma once
#include "chatlib/messages_types.h"
namespace chat
{
namespace msg
{
namespace detail
{
template<typename T>
struct parse_msg_res
{
	VS_FORWARDING_CTOR2(parse_msg_res<T>, success, msg) {}
	parse_msg_res<T>()
		: success(false)
	{}
	bool success;
	T msg;
};

parse_msg_res<create_chat>
parse_create(const ChatMessagePtr& msg);
parse_msg_res<create_p2p_chat>
parse_create_p2p(const ChatMessagePtr& msg);
parse_msg_res<invite>
parse_invite(const ChatMessagePtr& msg);
parse_msg_res<invite_response>
parse_invite_response(const ChatMessagePtr& msg);
parse_msg_res<add_part>
parse_add_part(const ChatMessagePtr& msg);
parse_msg_res<remove_part>
parse_remove_part(const ChatMessagePtr& msg);
parse_msg_res<text_message>
parse_text_msg(const ChatMessagePtr& msg);
parse_msg_res<part_added_to_chat>
parse_part_added_to_chat(const ChatMessagePtr& msg);
parse_msg_res<part_removed_from_chat>
parse_part_removed_from_chat(const ChatMessagePtr& msg);
parse_msg_res<remove_message>
parse_remove_message_from_chat(const ChatMessagePtr& msg);
parse_msg_res<clear_history>
parse_clear_history(const ChatMessagePtr& msg);
}
template<typename T>
void parse_message(const ChatMessagePtr& msg, T&& cb)
{
	{
		auto parse_result = detail::parse_create(msg);
		if (parse_result.success)
		{
			cb(std::move(parse_result.msg));
			return;
		}
	}
	{
		auto parse_result = detail::parse_create_p2p(msg);
		if (parse_result.success)
		{
			cb(std::move(parse_result.msg));
			return;
		}
	}
	{
		auto parse_result = detail::parse_invite(msg);
		if (parse_result.success)
		{
			cb(std::move(parse_result.msg));
			return;
		}
	}
	{
		auto parse_result = detail::parse_invite_response(msg);
		if (parse_result.success)
		{
			cb(std::move(parse_result.msg));
			return;
		}
	}
	{
		auto parse_result = detail::parse_add_part(msg);
		if (parse_result.success)
		{
			cb(std::move(parse_result.msg));
			return;
		}
	}
	{
		auto parse_result = detail::parse_remove_part(msg);
		if (parse_result.success)
		{
			cb(std::move(parse_result.msg));
			return;
		}
	}
	{
		auto parse_result = detail::parse_text_msg(msg);
		if(parse_result.success)
		{
			cb(std::move(parse_result.msg));
			return;
		}
	}
	{
		auto parse_result = detail::parse_part_added_to_chat(msg);
		if (parse_result.success)
		{
			cb(std::move(parse_result.msg));
			return;
		}
	}
	{
		auto parse_result = detail::parse_part_removed_from_chat(msg);
		if (parse_result.success)
		{
			cb(std::move(parse_result.msg));
			return;
		}
	}
	{
		auto parse_result = detail::parse_remove_message_from_chat(msg);
		if (parse_result.success)
		{
			cb(std::move(parse_result.msg));
			return;
		}
	}
	{
		auto parse_result = detail::parse_clear_history(msg);
		if (parse_result.success)
		{
			cb(std::move(parse_result.msg));
			return;
		}
	}
	cb(unrecognized_msg());
}
}
}
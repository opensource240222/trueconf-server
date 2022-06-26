#include "parse_msg_content.h"
#include "chatlib/msg/chat_messages_construct.h"

namespace chat
{
namespace msg
{
namespace detail
{
parse_msg_content_res<create_chat_content>
parse_create_content(const ChatMessagePtr& msg)
{
	return CreateChatMessage::ParseMsgContent(msg);
}
parse_msg_content_res<create_p2p_chat_content>
parse_create_p2p_chat_content(const ChatMessagePtr& msg)
{
	return CreateP2PChatMessage::ParseMsgContent(msg);
}
parse_msg_content_res<invite_content>
parse_invite_content(const ChatMessagePtr& msg)
{
	return InviteMessage::ParseMsgContent(msg);
}
parse_msg_content_res<invite_response_content>
parse_invite_response_content(const ChatMessagePtr& msg)
{
	return InviteResponseMessage::ParseMsgContent(msg);
}
parse_msg_content_res<add_part_content>
parse_add_part_content(const ChatMessagePtr& msg)
{
	return AddPartMessage::ParseMsgContent(msg);
}
parse_msg_content_res<remove_part_content>
parse_remove_part_content(const ChatMessagePtr& msg)
{
	return RemovePartMessage::ParseMsgContent(msg);
}
parse_msg_content_res<text_message_content>
parse_text_message_content(const ChatMessagePtr& msg)
{
	return TextChatMessage::ParseMsgContent(msg);
}
parse_msg_content_res<part_added_to_chat_content>
parse_part_added_to_chat_content(const ChatMessagePtr& msg)
{
	return PartAddedToChatMessage::ParseMsgContent(msg);
}
parse_msg_content_res<part_removed_from_chat_content>
parse_part_removed_from_chat_content(const ChatMessagePtr& msg)
{
	return PartRemovedFromChatMessage::ParseMsgContent(msg);
}
parse_msg_content_res<remove_message_content>
parse_remove_message_content(const ChatMessagePtr& msg)
{
	return MessageRemovalMessage::ParseMsgContent(msg);
}
parse_msg_content_res<clear_history_content>
parse_clear_history_content(const ChatMessagePtr& msg)
{
	return ClearHistoryMessage::ParseMsgContent(msg);
}
} // namespace detail
} // namespace msg
} // namespace chat

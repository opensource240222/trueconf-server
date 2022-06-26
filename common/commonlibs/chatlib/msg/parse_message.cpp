#include "parse_message.h"
#include "chatlib/msg/chat_messages_construct.h"
namespace chat
{
namespace msg
{
namespace detail
{
parse_msg_res<create_chat>
parse_create(const ChatMessagePtr &msg)
{
	return CreateChatMessage::Parse(msg);
}
parse_msg_res<create_p2p_chat>
parse_create_p2p(const ChatMessagePtr& msg)
{
	return CreateP2PChatMessage::Parse(msg);
}
parse_msg_res<invite>
parse_invite(const ChatMessagePtr &msg)
{
	return InviteMessage::Parse(msg);
}
parse_msg_res<invite_response>
parse_invite_response(const ChatMessagePtr &msg)
{
	return InviteResponseMessage::Parse(msg);
}
parse_msg_res<add_part>
parse_add_part(const ChatMessagePtr &msg)
{
	return AddPartMessage::Parse(msg);
}
parse_msg_res<remove_part>
parse_remove_part(const ChatMessagePtr &msg)
{
	return RemovePartMessage::Parse(msg);
}
parse_msg_res<text_message>
parse_text_msg(const ChatMessagePtr &msg)
{
	return TextChatMessage::Parse(msg);
}
parse_msg_res<part_added_to_chat>
parse_part_added_to_chat(const ChatMessagePtr& msg)
{
	return PartAddedToChatMessage::Parse(msg);
}
parse_msg_res<part_removed_from_chat>
parse_part_removed_from_chat(const ChatMessagePtr& msg)
{
	return PartRemovedFromChatMessage::Parse(msg);
}
parse_msg_res<remove_message>
parse_remove_message_from_chat(const ChatMessagePtr& msg)
{
	return MessageRemovalMessage::Parse(msg);
}
parse_msg_res<clear_history>
parse_clear_history(const ChatMessagePtr& msg)
{
	return ClearHistoryMessage::Parse(msg);
}
}
}
}
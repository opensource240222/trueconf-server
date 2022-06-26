#include "chatlib/msg/attr.h"
#include "chatlib/msg/chat_messages_construct.h"
#include "chatlib/msg/parse_message.h"
#include "chatlib/utils/chat_utils.h"
#include "chatlib/utils/msg_utils.h"

#include "tests/UnitTestChat/TestHelpers.h"

#include <gtest/gtest.h>

namespace
{
static const std::string s_title = "Chat Title";
static const chat::CallID s_call_id_from = "user@trueconf.com";
static const chat::BSInfo s_bs = "bs.trueconf.com#bs";
static const chat::CallID s_call_id_to = "user2@trueconf.com";
static const std::string s_msg_text = "Hello, chat!";

chat::ChatID GetChatID()
{
	static const auto s_chat_id = chat::GenerateChatID();
	return s_chat_id;
}
}
namespace chat_test
{

TEST(ParseMessages, CreateChatMessage)
{
	// Create chat
	auto chat_id = GetChatID();
	auto msg = chat::msg::CreateChatMessage(
		chat_id,
		chat::current_chat_version,
		chat::ChatType::symmetric,
		s_title,
		s_call_id_from,
		s_call_id_from,
		vs::CallIDType::client,
		{}).AcquireMessage();
	parse_msg_handler h;
	chat::msg::parse_message(msg, h);
	auto create = boost::get<chat::msg::create_chat>(&h.parsed_msg);
	ASSERT_NE(create, nullptr);
	EXPECT_EQ(create->chat_id, chat_id);
	EXPECT_EQ(create->creator, s_call_id_from);
	EXPECT_EQ(create->msg_id, msg->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(create->content.title, s_title);
	EXPECT_EQ(create->content.type, chat::ChatType::symmetric);
	EXPECT_EQ(create->content.ver, chat::current_chat_version);
	EXPECT_EQ(create->content.from_instance, s_call_id_from);
	chat::ChatMessageTimestamp timestamp;
	EXPECT_TRUE(msg->GetParam(chat::attr::TIMESTAMP_paramName, timestamp));
	EXPECT_EQ(create->timestamp, timestamp);
}
TEST(ParseMessages, CreateP2PChatMessage)
{
	// Create p2p chat
	auto chat_id = GetChatID();
	vs::set<chat::ParticipantDescr, vs::less<>> parts = {
		{s_call_id_from, chat::ParticipantType::client},
		{s_call_id_to, chat::ParticipantType::client},
		{s_bs, chat::ParticipantType::server}
	};
	auto msg = chat::msg::CreateP2PChatMessage(
		chat_id,
		chat::current_chat_version,
		parts,
		s_call_id_from,
		s_call_id_from,
		vs::CallIDType::client,
		{}).AcquireMessage();
	parse_msg_handler h;
	chat::msg::parse_message(msg, h);
	auto create_p2p = boost::get<chat::msg::create_p2p_chat>(&h.parsed_msg);
	ASSERT_NE(create_p2p, nullptr);
	EXPECT_EQ(create_p2p->chat_id, chat_id);
	EXPECT_EQ(create_p2p->msg_id, msg->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(create_p2p->content.part_list, parts);
	EXPECT_EQ(create_p2p->content.ver, chat::current_chat_version);
	EXPECT_EQ(create_p2p->content.from_instance, s_call_id_from);
	chat::ChatMessageTimestamp timestamp;
	EXPECT_TRUE(msg->GetParam(chat::attr::TIMESTAMP_paramName, timestamp));
	EXPECT_EQ(create_p2p->timestamp, timestamp);
}
TEST(ParseMessage, InviteMessage)
{
	auto chat_id = GetChatID();
	auto msg = chat::msg::InviteMessage(
		chat_id,
		chat::ChatType::symmetric,
		s_title,
		chat::current_chat_version,
		s_call_id_to,
		s_call_id_from,
		s_call_id_from,
		vs::CallIDType::client,
		{}).AcquireMessage();
	parse_msg_handler h;
	chat::msg::parse_message(msg, h);
	auto invite = boost::get<chat::msg::invite>(&h.parsed_msg);
	ASSERT_NE(invite, nullptr);
	EXPECT_EQ(invite->chat_id, chat_id);
	EXPECT_EQ(invite->content.invitee, s_call_id_to);
	EXPECT_EQ(invite->inviter, s_call_id_from);
	EXPECT_EQ(invite->msg_id, msg->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(invite->content.type, chat::ChatType::symmetric);
	EXPECT_EQ(invite->content.ver, chat::current_chat_version);
	EXPECT_EQ(invite->content.title, s_title);
	EXPECT_EQ(invite->content.from_instance, s_call_id_from);
	chat::ChatMessageTimestamp timestamp;
	EXPECT_TRUE(msg->GetParam(chat::attr::TIMESTAMP_paramName, timestamp));
	EXPECT_EQ(invite->timestamp, timestamp);
}
TEST(ParseMessage, InviteResponseMessage)
{
	static const std::string invite_msg_id = "1234";
	auto chat_id = GetChatID();
	auto msg = chat::msg::InviteResponseMessage(
		chat_id,
		s_call_id_to,
		s_call_id_from,
		s_call_id_from,
		vs::CallIDType::client,
		invite_msg_id,
		chat::InviteResponseCode::accept,
		{}).AcquireMessage();
	parse_msg_handler h;
	chat::msg::parse_message(msg, h);
	auto invite_resp = boost::get<chat::msg::invite_response>(&h.parsed_msg);
	ASSERT_NE(invite_resp, nullptr);
	EXPECT_EQ(invite_resp->chat_id, chat_id);
	EXPECT_EQ(invite_resp->content.code, chat::InviteResponseCode::accept);
	EXPECT_EQ(invite_resp->content.invite_msg_id, invite_msg_id);
	EXPECT_EQ(invite_resp->msg_id, msg->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(invite_resp->response_from, s_call_id_from);
	EXPECT_EQ(invite_resp->content.response_to, s_call_id_to);
	EXPECT_EQ(invite_resp->content.from_instance, s_call_id_from);
	chat::ChatMessageTimestamp timestamp;
	EXPECT_TRUE(msg->GetParam(chat::attr::TIMESTAMP_paramName, timestamp));
	EXPECT_EQ(invite_resp->timestamp, timestamp);
}
TEST(ParseMessage, AddPart)
{
	auto chat_id = GetChatID();
	auto chatInfo = chat::GlobalContext(
		chat_id, s_title, chat::ChatType::symmetric,
		chat::current_chat_version, s_call_id_from,
		chat::ChatMessageTimestamp(), chat::ChatMessageTimestamp(),
		chat::ChatMessageID(),
		vs::set<chat::ParticipantDescr, vs::less<>>(),
		vs::set<chat::ParticipantDescr, vs::less<>>()
	);
	auto msg = chat::msg::AddPartMessage(
		chatInfo,
		s_call_id_to,
		chat::ParticipantType::client,
		s_call_id_from,
		s_call_id_from,
		vs::CallIDType::client,
		{}).AcquireMessage();
	parse_msg_handler h;
	chat::msg::parse_message(msg, h);
	auto add_part = boost::get<chat::msg::add_part>(&h.parsed_msg);
	ASSERT_NE(add_part, nullptr);
	EXPECT_EQ(add_part->chat_id, chat_id);
	EXPECT_EQ(add_part->from, s_call_id_from);
	EXPECT_EQ(add_part->msg_id, msg->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(add_part->content.call_id, s_call_id_to);
	EXPECT_EQ(add_part->content.type, chat::ParticipantType::client);
	EXPECT_EQ(add_part->content.from_instance, s_call_id_from);
	chat::ChatMessageTimestamp timestamp;
	EXPECT_TRUE(msg->GetParam(chat::attr::TIMESTAMP_paramName, timestamp));
	EXPECT_EQ(add_part->timestamp, timestamp);
}
TEST(ParseMessage, PlainTextMessageGroup)
{
	auto chat_id = GetChatID();
	auto msg = chat::msg::ContentMessage{}.Text(s_msg_text).Seal(
		chat_id,
		s_call_id_from,
		s_call_id_from,
		vs::CallIDType::client,
		{}, {});
	parse_msg_handler h;
	chat::msg::parse_message(msg, h);
	auto text_msg = boost::get<chat::msg::text_message>(&h.parsed_msg);
	ASSERT_NE(text_msg, nullptr);
	EXPECT_EQ(text_msg->author, s_call_id_from);
	EXPECT_EQ(text_msg->chat_id, chat_id);
	EXPECT_EQ(text_msg->msg_id, msg->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_TRUE(text_msg->original_id.empty());
	chat::ChatMessageTimestamp timestamp;
	EXPECT_TRUE(msg->GetParam(chat::attr::TIMESTAMP_paramName, timestamp));
	EXPECT_EQ(text_msg->timestamp, timestamp);
	EXPECT_EQ(text_msg->content.text, s_msg_text);
	EXPECT_TRUE(text_msg->content.to.empty());
	EXPECT_TRUE(text_msg->content.reply_id.empty());
	EXPECT_TRUE(text_msg->content.forward_title.empty());
	EXPECT_EQ(text_msg->content.from_instance, s_call_id_from);
}
TEST(ParseMessage, PlainTextMessageP2P)
{
	auto chat_id = GetChatID();
	auto msg = chat::msg::ContentMessage{}
		.Text(s_msg_text)
		.Seal(
			chat_id,
			s_call_id_from,
			s_call_id_from,
			vs::CallIDType::client,
			{}, {});
	chat::msg::InsertParamStrIntoMsgContent(msg, chat::msg::toKeyName, s_call_id_to);
	parse_msg_handler h;
	chat::msg::parse_message(msg, h);
	auto text_msg = boost::get<chat::msg::text_message>(&h.parsed_msg);
	ASSERT_NE(text_msg, nullptr);
	EXPECT_EQ(text_msg->author, s_call_id_from);
	EXPECT_EQ(text_msg->chat_id, chat_id);
	EXPECT_EQ(text_msg->msg_id, msg->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_TRUE(text_msg->original_id.empty());
	chat::ChatMessageTimestamp timestamp;
	EXPECT_TRUE(msg->GetParam(chat::attr::TIMESTAMP_paramName, timestamp));
	EXPECT_EQ(text_msg->timestamp, timestamp);
	EXPECT_EQ(text_msg->content.text, s_msg_text);
	EXPECT_EQ(text_msg->content.to, s_call_id_to);
	EXPECT_TRUE(text_msg->content.reply_id.empty());
	EXPECT_TRUE(text_msg->content.forward_title.empty());
	EXPECT_EQ(text_msg->content.from_instance, s_call_id_from);
}
TEST(ParseMessage, AddToChatSysMsg)
{
	auto chat_id = GetChatID();
	auto sys_chat_id = chat::GetP2PChatID(s_call_id_from, s_bs);
	{
		// group conf
		auto add_part_msg_id = chat::GenerateUUID();
		auto msg = chat::msg::PartAddedToChatMessage(
			sys_chat_id,
			s_call_id_from,
			s_call_id_from,
			vs::CallIDType::client,
			s_bs,
			chat_id,
			s_title,
			add_part_msg_id,
			s_call_id_from,
			s_call_id_from,
			{}, {}, {}).AcquireMessage();
		parse_msg_handler h;
		chat::msg::parse_message(msg, h);
		auto part_added = boost::get<chat::msg::part_added_to_chat>(&h.parsed_msg);
		ASSERT_NE(part_added, nullptr);
		EXPECT_EQ(part_added->chat_id, sys_chat_id);
		EXPECT_EQ(part_added->msg_id, msg->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
		EXPECT_EQ(part_added->content.add_part_msg_id, add_part_msg_id);
		EXPECT_EQ(part_added->content.title, s_title);
		EXPECT_EQ(part_added->content.where_chat_id, chat_id);
		EXPECT_EQ(part_added->content.who_was_added, s_call_id_from);
		EXPECT_EQ(part_added->content.who_added, s_call_id_from);
		EXPECT_TRUE(part_added->content.p2p_part.empty());
		EXPECT_EQ(part_added->content.to, s_bs);
		EXPECT_EQ(part_added->content.from_instance, s_call_id_from);
	}
	{
		// p2p
		auto add_part_msg_id = chat::GenerateUUID();
		auto p2p_chat_id = chat::GetP2PChatID(s_call_id_from, s_call_id_to);
		auto msg = chat::msg::PartAddedToChatMessage(
			sys_chat_id,
			s_call_id_from,
			s_call_id_from,
			vs::CallIDType::client,
			s_bs,
			p2p_chat_id,
			s_title,
			add_part_msg_id,
			s_call_id_to,
			s_call_id_from,
			s_call_id_to,
			{}, {}).AcquireMessage();
		parse_msg_handler h;
		chat::msg::parse_message(msg, h);
		auto part_added = boost::get<chat::msg::part_added_to_chat>(&h.parsed_msg);
		ASSERT_NE(part_added, nullptr);
		EXPECT_EQ(part_added->chat_id, sys_chat_id);
		EXPECT_EQ(part_added->msg_id, msg->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
		EXPECT_EQ(part_added->content.add_part_msg_id, add_part_msg_id);
		EXPECT_EQ(part_added->content.title, s_title);
		EXPECT_EQ(part_added->content.where_chat_id, p2p_chat_id);
		EXPECT_EQ(part_added->content.who_was_added, s_call_id_to);
		EXPECT_EQ(part_added->content.who_added, s_call_id_from);
		EXPECT_EQ(part_added->content.p2p_part, s_call_id_to);
		EXPECT_EQ(part_added->content.to, s_bs);
		EXPECT_EQ(part_added->content.from_instance, s_call_id_from);
	}
}
TEST(ParseMessage, RemoveFromChatSysMsg)
{
	auto chat_id = GetChatID();
	auto sys_chat_id = chat::GetP2PChatID(s_call_id_from, s_bs);
	std::string leave_permissions = "{}";
	auto remove_part_msg_id = chat::GenerateUUID();
	auto msg = chat::msg::PartRemovedFromChatMessage(
		sys_chat_id,
		s_call_id_from,
		s_call_id_from,
		vs::CallIDType::client,
		s_bs,
		chat_id,
		s_call_id_from,
		remove_part_msg_id,
		leave_permissions,
		{}, {}).AcquireMessage();
	parse_msg_handler h;
	chat::msg::parse_message(msg, h);
	auto part_removed = boost::get<chat::msg::part_removed_from_chat>(&h.parsed_msg);
	ASSERT_NE(part_removed, nullptr);
	EXPECT_EQ(part_removed->chat_id, sys_chat_id);
	EXPECT_EQ(part_removed->msg_id, msg->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(part_removed->content.leave_permissions, leave_permissions);
	EXPECT_EQ(part_removed->content.remove_part_msg_id, remove_part_msg_id);
	EXPECT_EQ(part_removed->content.where_chat_id, chat_id);
	EXPECT_EQ(part_removed->content.removed_part, s_call_id_from);
	EXPECT_EQ(part_removed->content.to, s_bs);
	EXPECT_EQ(part_removed->content.from_instance, s_call_id_from);
}
}
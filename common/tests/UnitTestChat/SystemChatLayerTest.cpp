#include "tests/common/TestHelpers.h"
#include "tests/UnitTestChat/globvars.h"
#include "tests/UnitTestChat/SystemChatLayerFixture.h"

#include "chatlib/helpers/AccountInfo.h"
#include "chatlib/msg/attr.h"
#include "chatlib/msg/parse_message.h"
#include "chatlib/utils/chat_utils.h"
#include "chatlib/utils/msg_utils.h"
#include "chatutils/GlobalConfigStub.h"

#include "std-generic/cpplib/scope_exit.h"

#include <atomic>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Mock;
using ::testing::NotNull;

namespace chat_test
{
TEST_F(SystemChatLayerFixture, PartAddPartRemovedClient)
{
	/**
		user1 and user2
		user1 Add to chat - > user1->SystemChatLayer->AddPartToChat, user2 dosn't
		user2 Remove from chat -> user2->SystemChatLayer -> RemoveFromChat, user1 dosn't
		create p2p chat for user1 and user2 -> SystemChatLayer->AddPartToChat for user1 and user2
	*/

	auto user1 = std::make_shared<chat::AccountInfo>(
		m_user1Name, m_user1NameInstance,
		vs::CallIDType::client, m_bs1Name,
		[](chat::CallIDRef) {return std::vector<chat::CallID>{}; });

	auto user2 = std::make_shared<chat::AccountInfo>(
		m_user2Name, m_user2NameInstance,
		vs::CallIDType::client, m_bs2Name,
		[](chat::CallIDRef) {return std::vector<chat::CallID>{}; });

	auto sysChat1 = GetSysChatLayer(user1);
	auto sysChat2 = GetSysChatLayer(user2);
	auto notifier = GetEventsNotifier();
	AddCtxStamp(m_symChatCtx);
	auto ev = std::make_shared<vs::event>(false);
	// Add user1 to chat
	EXPECT_CALL(*sysChat1.mockedLayer, ForwardBelowMessage(NotNull(), _))
		.WillOnce(Invoke([&, ev](auto msg, auto)
	{
		VS_SCOPE_EXIT{ ev->set(); };
		auto parseResult = chat::msg::detail::parse_part_added_to_chat(*msg);
		EXPECT_TRUE(parseResult.success);
		EXPECT_EQ(parseResult.msg.content.add_part_msg_id, m_addUser1MsgId);
		EXPECT_EQ(parseResult.msg.content.where_chat_id, m_addUser1ChatID);
		EXPECT_EQ(parseResult.msg.content.title, m_symChatCtx.title);
		EXPECT_EQ(chat::msg::GetParamStrFromMsgContent(*msg, chat::msg::toKeyName), m_bs1Name);
	}));
	EXPECT_CALL(*sysChat2.mockedLayer, ForwardBelowMessage(_, _))
		.Times(0);
	notifier->OnPartAdded(
		m_addUser1ChatID,
		user1->GetCallID(),
		m_addUser1MsgId,
		m_user2Name);
	EXPECT_TRUE(test::WaitFor("Waiting PartAddedToChat message", *ev, g_max_wait_timeout_ms));
	// clean
	Mock::VerifyAndClearExpectations(sysChat1.mockedLayer.get());
	Mock::VerifyAndClearExpectations(sysChat2.mockedLayer.get());

	// Remove user2 from chat
	EXPECT_CALL(*sysChat2.mockedLayer, ForwardBelowMessage(NotNull(), _))
		.WillOnce(Invoke([&, ev](auto msg, auto)
	{
		VS_SCOPE_EXIT{ ev->set(); };
		auto parseResult = chat::msg::detail::parse_part_removed_from_chat(*msg);
		EXPECT_TRUE(parseResult.success);
		EXPECT_EQ(parseResult.msg.content.remove_part_msg_id, m_removeUser2MsgId);
		EXPECT_EQ(parseResult.msg.content.where_chat_id, m_removeUser2ChatID);
		EXPECT_EQ(chat::msg::GetParamStrFromMsgContent(*msg, chat::msg::toKeyName), m_bs2Name);
	}));
	EXPECT_CALL(*sysChat1.mockedLayer, ForwardBelowMessage(_, _))
		.Times(0);
	notifier->OnPartRemoved(
		m_removeUser2ChatID,
		user2->GetCallID(),
		m_removeUser2MsgId);
	EXPECT_TRUE(test::WaitFor("Waiting PartAddedToChat message", *ev, g_max_wait_timeout_ms));
	// clean
	Mock::VerifyAndClearExpectations(sysChat1.mockedLayer.get());
	Mock::VerifyAndClearExpectations(sysChat2.mockedLayer.get());

	// p2p user1 and user1
	std::atomic<bool> mock1Handled(false);
	EXPECT_CALL(*sysChat1.mockedLayer, ForwardBelowMessage(NotNull(), _))
		.WillOnce(Invoke([&, ev](auto msg, auto)
	{
		VS_SCOPE_EXIT{ mock1Handled = true; };
		auto parseResult = chat::msg::detail::parse_part_added_to_chat(*msg);
		EXPECT_TRUE(parseResult.success);
		EXPECT_EQ(parseResult.msg.content.add_part_msg_id, m_firstMsgInP2PId);
		EXPECT_EQ(parseResult.msg.content.where_chat_id, m_firstMsgInP2PChatID);
		EXPECT_EQ(parseResult.msg.content.who_was_added, m_user1Name);
		EXPECT_EQ(parseResult.msg.content.p2p_part, m_user2Name);
		EXPECT_EQ(chat::msg::GetParamStrFromMsgContent(*msg, chat::msg::toKeyName), m_bs1Name);
	}));
	std::atomic<bool> mock2Handled(false);
	EXPECT_CALL(*sysChat2.mockedLayer, ForwardBelowMessage(NotNull(), _))
		.WillOnce(Invoke([&, ev](auto msg, auto)
	{
		VS_SCOPE_EXIT{ mock2Handled = true; };
		auto parseResult = chat::msg::detail::parse_part_added_to_chat(*msg);
		EXPECT_TRUE(parseResult.success);
		EXPECT_EQ(parseResult.msg.content.add_part_msg_id, m_firstMsgInP2PId);
		EXPECT_EQ(parseResult.msg.content.where_chat_id, m_firstMsgInP2PChatID);
		EXPECT_EQ(parseResult.msg.content.who_was_added, m_user2Name);
		EXPECT_EQ(parseResult.msg.content.p2p_part, m_user1Name);
		EXPECT_EQ(chat::msg::GetParamStrFromMsgContent(*msg, chat::msg::toKeyName), m_bs2Name);
	}));
	notifier->OnChatCreated(m_p2pChatCtx);
	EXPECT_TRUE(test::WaitFor("Waiting PartAddedToChat message",
		[&]() { return mock1Handled && mock2Handled; },
		10, g_max_wait_timeout_ms));
	Mock::VerifyAndClearExpectations(sysChat1.mockedLayer.get());
	Mock::VerifyAndClearExpectations(sysChat2.mockedLayer.get());
}
TEST_F(SystemChatLayerFixture, PartAddPartRemovedServer)
{
	/**
		user1 user2 -> bs1
		user3 user4 -> bs2

		AddPart(user1) -> bs1->SystemChat->SendTo(user1)->AddPartToChat, other don't
		AddPart(user4) -> bs2->SystemChat->SendTo(user4)->AddPartToChat, other don't
		RemovePart(user2)-> bs1->SystemChat->SendTo(user2)->RemovePartFromChat, other don't
		RemovePart(user3)-> bs2->SystemChat->SendTo(user3)->RemovePartFromChat, other don't
		create p2p chat for user1 and user2 -> SystemChatLayer->AddPartToChat for bs1 and bs2
	*/
	auto bs1 = std::make_shared<chat::AccountInfo>(
		m_bs1Name, m_bs1Name,
		vs::CallIDType::server, m_bs1Name,
		[](chat::CallIDRef) {return std::vector<chat::CallID>{}; });

	auto bs2 = std::make_shared<chat::AccountInfo>(
		m_bs2Name, m_bs2Name,
		vs::CallIDType::server, m_bs2Name,
		[](chat::CallIDRef) {return std::vector<chat::CallID>{}; });

	AddResolveRecord(m_user1Name, vs::CallIDType::client, m_bs1Name, { m_user1Name });
	AddResolveRecord(m_user2Name, vs::CallIDType::client, m_bs2Name, { m_user2Name });

	auto sysChatBS1 = GetSysChatLayer(bs1);
	auto sysChatBS2 = GetSysChatLayer(bs2);
	AddCtxStamp(m_symChatCtx);
	auto notifier = GetEventsNotifier();

	auto ev = std::make_shared<vs::event>(false);

	// Add user1 to chat
	EXPECT_CALL(*sysChatBS1.mockedLayer, ForwardBelowMessage(NotNull(), _))
		.WillOnce(Invoke([&, ev](auto msg, auto)
	{
		VS_SCOPE_EXIT{ ev->set(); };
		auto parseResult = chat::msg::detail::parse_part_added_to_chat(*msg);
		EXPECT_TRUE(parseResult.success);
		EXPECT_EQ(parseResult.msg.content.add_part_msg_id, m_addUser1MsgId);
		EXPECT_EQ(parseResult.msg.content.where_chat_id, m_addUser1ChatID);
		EXPECT_EQ(chat::msg::GetParamStrFromMsgContent(*msg, chat::msg::toKeyName), m_user1Name);
	}));
	EXPECT_CALL(*sysChatBS2.mockedLayer, ForwardBelowMessage(_, _))
		.Times(0);
	notifier->OnPartAdded(
		m_addUser1ChatID,
		m_user1Name,
		m_addUser1MsgId, m_user2Name);
	EXPECT_TRUE(test::WaitFor("Waiting PartAddedToChat message", *ev, g_max_wait_timeout_ms));
	// clean
	Mock::VerifyAndClearExpectations(sysChatBS1.mockedLayer.get());
	Mock::VerifyAndClearExpectations(sysChatBS2.mockedLayer.get());

	// Remove user2 from chat
	EXPECT_CALL(*sysChatBS2.mockedLayer, ForwardBelowMessage(NotNull(), _))
		.WillOnce(Invoke([&, ev](auto msg, auto)
	{
		VS_SCOPE_EXIT{ ev->set(); };
		auto parseResult = chat::msg::detail::parse_part_removed_from_chat(*msg);
		EXPECT_TRUE(parseResult.success);
		EXPECT_EQ(parseResult.msg.content.remove_part_msg_id, m_removeUser2MsgId);
		EXPECT_EQ(parseResult.msg.content.where_chat_id, m_removeUser2ChatID);
		EXPECT_EQ(chat::msg::GetParamStrFromMsgContent(*msg, chat::msg::toKeyName), m_user2Name);
	}));
	EXPECT_CALL(*sysChatBS1.mockedLayer, ForwardBelowMessage(_, _))
		.Times(0);
	notifier->OnPartRemoved(
		m_removeUser2ChatID,
		m_user2Name,
		m_removeUser2MsgId);
	EXPECT_TRUE(test::WaitFor("Waiting PartAddedToChat message", *ev, g_max_wait_timeout_ms));
	// clean
	Mock::VerifyAndClearExpectations(sysChatBS1.mockedLayer.get());
	Mock::VerifyAndClearExpectations(sysChatBS2.mockedLayer.get());

	// p2p user1 and user1
	std::atomic<bool> mock1Handled(false);
	EXPECT_CALL(*sysChatBS1.mockedLayer, ForwardBelowMessage(NotNull(), _))
		.WillOnce(Invoke([&, ev](auto msg, auto)
	{
		VS_SCOPE_EXIT{ mock1Handled = true; };
		auto parseResult = chat::msg::detail::parse_part_added_to_chat(*msg);
		EXPECT_TRUE(parseResult.success);
		EXPECT_EQ(parseResult.msg.content.add_part_msg_id, m_firstMsgInP2PId);
		EXPECT_EQ(parseResult.msg.content.where_chat_id, m_firstMsgInP2PChatID);
		EXPECT_EQ(parseResult.msg.content.p2p_part, m_user2Name);
		EXPECT_EQ(chat::msg::GetParamStrFromMsgContent(*msg, chat::msg::toKeyName), m_user1Name);
	}));
	std::atomic<bool> mock2Handled(false);
	EXPECT_CALL(*sysChatBS2.mockedLayer, ForwardBelowMessage(NotNull(), _))
		.WillOnce(Invoke([&, ev](auto msg, auto)
	{
		VS_SCOPE_EXIT{ mock2Handled = true; };
		auto parseResult = chat::msg::detail::parse_part_added_to_chat(*msg);
		EXPECT_TRUE(parseResult.success);
		EXPECT_EQ(parseResult.msg.content.add_part_msg_id, m_firstMsgInP2PId);
		EXPECT_EQ(parseResult.msg.content.where_chat_id, m_firstMsgInP2PChatID);
		EXPECT_EQ(parseResult.msg.content.p2p_part, m_user1Name);
		EXPECT_EQ(chat::msg::GetParamStrFromMsgContent(*msg, chat::msg::toKeyName), m_user2Name);
	}));
	notifier->OnChatCreated(m_p2pChatCtx);
	EXPECT_TRUE(test::WaitFor("Waiting PartAddedToChat message",
		[&]() { return mock1Handled && mock2Handled; },
		10, g_max_wait_timeout_ms));
	Mock::VerifyAndClearExpectations(sysChatBS1.mockedLayer.get());
	Mock::VerifyAndClearExpectations(sysChatBS2.mockedLayer.get());
}
}
#pragma once
#include "tests/UnitTestChat/mock/ChatLayerAbstractMock.h"

#include "chatlib/msg/attr.h"
#include "chatlib/factory_asio/make_layers_asio.h"
#include "chatlib/notify/GlobalChatEvents.h"
#include "chatlib/utils/chat_utils.h"
#include "chatutils/ResolverStub.h"

#include "std/cpplib/ASIOThreadPool.h"
#include "std-generic/cpplib/macro_utils.h"

#include "std-generic/sqlite/CppSQLite3.h"

#include <gtest/gtest.h>

#include <map>
class GlobalConfigStub;
namespace chat_test
{
class ChatContextStorageStub;

class SystemChatLayerFixture : public testing::Test
{
protected:
	const std::string m_user1Name = "user1@trueconf.com";
	const std::string m_user1NameInstance = "user1@trueconf.com/1";
	const std::string m_user2Name = "user2@trueconf.com";
	const std::string m_user2NameInstance = "user2@trueconf.com/1";
	const std::string m_bs1Name = "bs1.trueconf.com#bs";
	const std::string m_bs2Name = "bs2.trueconf.com#bs";
	const chat::ChatID m_addUser1ChatID = chat::GenerateChatID();
	const chat::ChatMessageID m_addUser1MsgId = chat::GenerateUUID();;
	const chat::ChatID m_removeUser2ChatID = chat::GenerateChatID();
	const chat::ChatMessageID m_removeUser2MsgId = chat::GenerateUUID();
	const chat::ChatID m_firstMsgInP2PChatID = chat::GetP2PChatID(m_user1Name, m_user2Name);
	const chat::ChatMessageID m_firstMsgInP2PId = chat::GenerateUUID();

	const chat::GlobalContext m_symChatCtx = chat::GlobalContext(
		m_addUser1ChatID, "Chat Title", chat::ChatType::symmetric,
		chat::current_chat_version, chat::CallID(),
		chat::ChatMessageTimestamp(), chat::ChatMessageTimestamp(),
		m_addUser1MsgId, vs::set<chat::ParticipantDescr, vs::less<>>(),
		vs::set<chat::ParticipantDescr, vs::less<>>()
	);
	chat::GlobalContext m_p2pChatCtx = chat::GlobalContext(
		m_firstMsgInP2PChatID, std::string(), chat::ChatType::p2p,
		chat::current_chat_version, m_user1Name,
		chat::ChatMessageTimestamp(), chat::ChatMessageTimestamp(),
		m_firstMsgInP2PId,
		vs::set<chat::ParticipantDescr, vs::less<>>
	{
		{ m_user1Name, chat::ParticipantType::client },
		{ m_user2Name, chat::ParticipantType::client },
		{ m_bs1Name, chat::ParticipantType::server },
		{ m_bs2Name, chat::ParticipantType::server }
	},
		vs::set<chat::ParticipantDescr, vs::less<>>());

	struct SysChatLayerWithMock
	{
		VS_FORWARDING_CTOR2(SysChatLayerWithMock, sysChatLayer, mockedLayer) {}
		std::shared_ptr<chat::asio_sync::SystemChatLayer> sysChatLayer;
		std::shared_ptr<LayerInterface_Mock> mockedLayer;
	};
public:
	SystemChatLayerFixture();
	~SystemChatLayerFixture();
	SysChatLayerWithMock GetSysChatLayer(const chat::AccountInfoPtr& account);
	chat::ChatEventsNotifierPtr GetEventsNotifier() const;
	void AddResolveRecord(chat::CallIDRef callId, vs::CallIDType type,
		string_view bs, std::vector<chat::CallID>&& instances);
	void AddCtxStamp(const chat::GlobalContext& ctx);
private:
	vs::ASIOThreadPool m_atp;
	std::shared_ptr<chat::notify::ChatEventsFuncs> m_notifier;
	std::shared_ptr<ResolverStub> m_resolver;
	std::vector<std::pair<std::shared_ptr<GlobalConfigStub>, CppSQLite3DB>> m_cfgs;
	std::vector<std::shared_ptr<chat::asio_sync::MainStorageLayer>> m_storages;
};
}
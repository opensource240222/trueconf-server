#include "ChatContinuationFixture.h"
#include "tests/common/TestHelpers.h"
#include "tests/UnitTestChat/globvars.h"
#include "tests/UnitTestChat/sql/TestDBDump.h"

#include "chatlib/storage/make_chat_storage.h"
#include "chatlib/utils/chat_utils.h"
#include "chatlib/utils/msg_utils.h"

#include "chatutils/GlobalConfigStub.h"
#include "chatutils/ResolverStub.h"
#include "chatutils/TransportChannelRouter.h"

namespace chat
{
template <class charT, class traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& s,
	const std::vector<chat::chain_item>& x)
{
	for (const auto& i : x)
	{
		s << std::setw(36) <<
			i.msg_id << " " << std::setw(36) <<
			i.prev_id << " " <<
			chat::timestamp_to_uint(i.timestamp) << '\n';
	}
	return s;
}
namespace chain
{
namespace detail
{
template <class charT, class traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& s,
	const std::vector<chat::chain::detail::Data>& x)
{
	for (const auto& i : x)
	{
		s << std::setw(36) <<
			i.id << " " << std::setw(36) <<
			i.parent << " " <<
			chat::timestamp_to_uint(i.timestamp) << '\n';
	}
	return s;
}
}
}
}
namespace chat_test
{
using ::testing::Pointwise;
ChatContinuationFixture::ChatContinuationFixture()
	: atp_(1)
{
	atp_.Start();
	env_.emplace(
		std::make_shared<TransportChannelRouter>(atp_.get_io_service()),
		std::make_shared<ResolverStub>(),
		std::make_shared<steady_clock_wrapper>(),
		atp_.get_io_service());
	CleanChatsFromPostgreSQL();
}
ChatContinuationFixture::~ChatContinuationFixture()
{
	for (const auto& part : all_parts_)
	{
		for(const auto& ep : part.second)
		{
			ep->app_layer_->ShutDown();
		}
	}
	atp_.Stop();
	if (HasFailure())
	{
		std::map<std::string, std::string> uri;
		for (const auto &part : all_parts_)
		{
			std::transform(
				part.second.begin(),
				part.second.end(),
				std::inserter(uri, uri.end()),
				[](const decltype(part.second)::value_type &val) -> decltype(uri)::value_type
					{ return {val->account_->GetExactCallID(), val->db_conn_param.dbUri}; });
		}
		chat_test::SaveDBToDisk(uri);
	}
	else
		CleanChatsFromPostgreSQL();

	all_parts_.clear();
	only_bs_.clear();
	only_users_.clear();
}

::testing::AssertionResult
ChatContinuationFixture::FillChatWithPGBackEnd(string_view storage_config)
{
	return ResurrectDB(g_dump_bs, g_dump_bs, { GetPostgreSQLConfiguration(), "" }, false, { storage_config, "" });
}

::testing::AssertionResult
ChatContinuationFixture::ResurrectDB(
	chat::CallIDRef call_id, chat::CallID ep_name, SQLiteDBParam destination, bool make_empty,
	SQLiteDBParam source
)
{
	static std::map<std::string, CppSQLite3DB> cached_dbs = {};

	if (!make_empty) {
		auto db_it = cached_dbs.find(destination.connectionString);
		if (db_it != cached_dbs.end()) {
			db_it->second.copyTo(destination.dbUri.c_str());
			return ::testing::AssertionSuccess();
		}
	}

	vs::ASIOThreadPool atp(1);

	atp.Start();
	auto resolver = std::make_shared<ResolverStub>();
	resolver->AddAlias(g_dump_bs, g_dump_bs);
	resolver->Add(g_dump_bs, vs::CallIDType::server, g_dump_bs, {g_dump_bs});
	for (const auto& user : g_dump_users)
	{
		resolver->AddAlias(user, user);
		resolver->Add(user, vs::CallIDType::client,
			g_dump_bs, user == call_id ? std::vector<chat::CallID>{ ep_name } : std::vector<chat::CallID>());
	}
	ChatUtilsEnvironment env(
		std::make_shared<TransportChannelRouter>(atp.get_io_service()),
		resolver,
		std::make_shared<steady_clock_wrapper>(),
		atp.get_io_service());
	std::vector<EpCreateInfo> epcis;
	epcis.emplace_back(ep_name, "", SQLiteDBParam(destination.connectionString, ""), boost::optional<CppSQLite3DB>{});
	auto eps = MakeChatUser(call_id, std::move(epcis), g_dump_bs,
		s_chain_len, s_bucket_capacity,
		s_tail_length, false, env);
	chat::ChatStoragePtr sqlite_backend;
	CppSQLite3DB db_holder;
	if (source.connectionString.empty()) {
		auto src_db = LoadDBDump(make_empty, "tmp");
		sqlite_backend = chat::make_chat_storage(src_db.dbConnParam.connectionString);
		db_holder = std::move(src_db.db);
	}
	else {
		sqlite_backend = chat::make_chat_storage(source.connectionString);
	}
	auto test_channel = env.router->GetChannel("testep@trueconf.com");
	std::map<chat::ChatID, uint32_t> msg_count_by_chat;
	for (const auto& chat_id : g_dump_chat_id)
	{
		auto items = sqlite_backend->GetLastMessages(chat_id,
			std::numeric_limits<int32_t>::max());
		for (const auto& item : items)
		{
			auto msg = sqlite_backend->GetMessage(item.msg_id, std::string{ call_id });
			if (!msg)
			{
				return ::testing::AssertionFailure()
					<< "ResurrectDB(): GetMessage() error";
			}
			auto senderType = vs::CallIDType::undef;
			if (!msg->GetParamI32(chat::attr::SENDER_TYPE_paramName, senderType))
				msg->SetParamI32(chat::attr::SENDER_TYPE_paramName, vs::CallIDType::client);
			auto chat_id = msg->GetParamStrRef(chat::attr::CHAT_ID_paramName);
			auto iter = msg_count_by_chat.emplace(chat_id, 0).first;
			test_channel->Send(std::move(msg), { ep_name });
			++iter->second;
		}
	}
	/*
		wait until all regular messages and all system chat messages
		will be saved in db

	*/
	auto new_backend = chat::make_chat_storage(destination.connectionString);
	std::vector<chat::ChatID> sys_chats = {};
	if (!make_empty) {
		if (g_dump_bs == call_id)
			sys_chats = GetAllSystemChatsForDump();
		else
			sys_chats.emplace_back(chat::GetP2PChatID(call_id, g_dump_bs));
	}
	auto res = test::WaitFor("ResurrectDB",
		[&]()
		{
			// wait for regular messages
			for (const auto& id : msg_count_by_chat)
			{
				auto count = new_backend->CountMessagesInChat(id.first);
				if (count != id.second)
					return false;
			}
			// wait for system chat messgae
			for (const auto& id : sys_chats)
			{
				auto part_added_msg_count = 0u;
				for (const auto& item : new_backend->GetLastMessages(id, std::numeric_limits<int32_t>::max()))
				{
					auto msg = new_backend->GetMessage(item.msg_id, std::string{ call_id });
					auto type = chat::MessageType::undefined;
					msg->GetParamI32(chat::attr::MESSAGE_TYPE_paramName, type);
					// count only MessageType::add_part_notification
					if (type == chat::MessageType::add_part_notification)
						++part_added_msg_count;
				}
				if (part_added_msg_count != g_dump_chat_id.size())
					return false;
			}
			return true;
		}, 10, g_max_wait_timeout_ms);
	eps.front()->app_layer_->ShutDown();
	atp.Stop();
	for (const auto& um : new_backend->GetAllUndeliveredMessages()) {
		for (const auto& part : um.parts)
		{
			new_backend->RemoveUndeliveredMessage(
				um.msg->GetParamStr(chat::attr::MESSAGE_ID_paramName), part.partId);
		}
	}
	if (!destination.dbUri.empty() && !make_empty) {
		auto& db_to = cached_dbs[destination.connectionString];
		db_to.open(":memory:");
		db_to.copyFrom(destination.dbUri.c_str());
	}
	return res;
}

::testing::AssertionResult ChatContinuationFixture::InitBS(bool empty_db, bool try_pg_db_init)
{
	EpCreateInfo ep;
	if (HasPostgreSQLCfg() && instance_with_pg_backend_.empty() && try_pg_db_init)
	{
		ep = EpCreateInfo(g_dump_bs, "", SQLiteDBParam(GetPostgreSQLConfiguration(), ""), boost::optional<CppSQLite3DB>{});
		instance_with_pg_backend_ = g_dump_bs;
	}
	else
	{
		auto res = LoadDBDump(true, g_dump_bs);
		if (!res.result)
			return ::testing::AssertionFailure() << "InitBS(): CreateDBFromSql() failed";
		ep = EpCreateInfo(g_dump_bs, "", res.dbConnParam, std::move(res.db));
	}
	if (!ResurrectDB(g_dump_bs, g_dump_bs, ep.db_conn_param, empty_db)) {
		return ::testing::AssertionFailure() << "InitBS(): ResurrectDB() failed";
	}
	std::vector<EpCreateInfo> bs_ep_create;
	bs_ep_create.emplace_back(std::move(ep));
	//bs_ep_create.emplace_back(g_dump_bs, res.uri, std::move(res.db));
	auto bs_ep = MakeChatUser(g_dump_bs, std::move(bs_ep_create), g_dump_bs,
		s_chain_len, s_bucket_capacity,
		s_tail_length, empty_db, env_.value());
	all_parts_.emplace(g_dump_bs, bs_ep);
	only_bs_.emplace(g_dump_bs, bs_ep);
	return ::testing::AssertionSuccess();
}
::testing::AssertionResult ChatContinuationFixture::InitPart(string_view user, bool empty_db)
{
	// create 3 ep for each;
	std::vector<EpCreateInfo> eps_info;
	for (auto j = 0; j < 3; ++j)
	{
		auto name = std::string(user) + "/" + std::to_string(j + 1);
		auto res = LoadDBDump(true, name);
		if (!res.result)
			return ::testing::AssertionFailure() << "InitPart(): LoadDBDump() failed";
		if (!ResurrectDB(user, name, res.dbConnParam, empty_db)) {
			return ::testing::AssertionFailure() << "InitPart(): ResurrectDB() failed";
		}
		eps_info.emplace_back(name, "", res.dbConnParam, std::move(res.db));
	}
	auto eps = MakeChatUser(user, std::move(eps_info),
		g_dump_bs, s_chain_len, s_bucket_capacity,
		s_tail_length, empty_db, env_.value());
	all_parts_.emplace(user, eps);
	only_users_.emplace(user, eps);
	return ::testing::AssertionSuccess();
}
::testing::AssertionResult ChatContinuationFixture::InitPartsDBFilled(bool try_pg_db_init)
{
	if (!InitBS(false, try_pg_db_init))
		return ::testing::AssertionFailure()
		<< "InitPartsDBFilled(): InitBS(false) failed";
	for (const auto&user : g_dump_users)
	{
		if (!InitPart(user, false))
			return ::testing::AssertionFailure()
			<< "InitPartsDBFilled(): InitPart(" << user << ", false) failed";
	}
	return ::testing::AssertionSuccess();
}
::testing::AssertionResult
ChatContinuationFixture::InitPartsFillDBExcept(std::vector<chat::CallID> &&call_ids, bool try_pg_db_init)
{
	if (!InitBS(std::count(call_ids.begin(), call_ids.end(), g_dump_bs) > 0, try_pg_db_init))
		return ::testing::AssertionFailure()
		<< "InitPartsFillDBExcept(): InitBS() failed";
	for (const auto&user : g_dump_users)
	{
		auto empty_db = std::count(call_ids.begin(), call_ids.end(), user) > 0;
		if (!InitPart(user, empty_db))
			return ::testing::AssertionFailure()
			<< "InitPartsFillDBExcept(): InitPart(" << user << ", " << empty_db << ") failed";
	}
	return ::testing::AssertionSuccess();
}
std::vector<chat::chain_item>
ChatContinuationFixture::GetChainFromStorage(
	chat::ChatIDRef chat_id,
	const chat::ChatStoragePtr &storage)
{
	auto id = chat::ChatID(chat_id);
	auto msg_count = storage->CountMessagesInChat(id);
	return storage->GetLastMessages(id, static_cast<size_t>(msg_count));
}
ChatContinuationFixture::LostMsgsTestParams
ChatContinuationFixture::PrepareLostMessagesTestParams(
	chat::ChatIDRef chat_id,
	const std::function<void(chat::msg::ChatMessagePtr&&, chat::CallIDRef)> &msg_catch_func)
{
	chat::ChatStoragePtr storage;
	const std::string test_db_name = "ProcessingLostMessages";
	auto res = LoadDBDump(false, test_db_name);
	if (!res.result)
		return LostMsgsTestParams(std::move(res.result));
	switch(GetParam())
	{
	case DBBackEnd::sqlite:
		storage = chat::make_chat_storage(res.dbConnParam.connectionString);
		break;
	case DBBackEnd::postgresql:

		auto fill_db_res = FillChatWithPGBackEnd(res.dbConnParam.connectionString);
		if (!fill_db_res)
		{
			return LostMsgsTestParams(std::move(fill_db_res));
		}
		storage = chat::make_chat_storage(GetPostgreSQLConfiguration());
		break;
	}
	auto main_storage = chat::asio_sync::MakeStorageLayer(storage,
		atp_.get_io_service());
	auto cfg = std::make_shared<GlobalConfigStub>(nullptr);
	cfg->SetClockWrapper(std::make_shared<steady_clock_wrapper>());
	auto user_info = std::make_shared<chat::AccountInfo>(
		g_dump_bs,
		g_dump_bs,
		vs::CallIDType::server,
		g_dump_bs,
		nullptr);
	cfg->SetAccountInfo(user_info);
	cfg->SetChatStorage(storage);
	cfg->SetMaxChainLen(s_chain_len);
	cfg->SetBucketCapacity(s_bucket_capacity);
	main_storage->Init(cfg);
	auto items = GetChainFromStorage(chat_id, storage);
	std::vector<chat::msg::ChatMessagePtr> msgs_for_test;
	// msg follows by 20-th by parent
	msgs_for_test.emplace_back(CreateMessageThatFollowsID(chat_id,
		(items.begin() + 19)->msg_id,
		(items.begin() + 19)->timestamp + std::chrono::milliseconds(1),
		"1 "));
	auto msg_item_1 = chat::GetMsgChainItem(msgs_for_test.back());
	// msg follows by 2-th by timestamp, parent is msg_item_1
	msgs_for_test.emplace_back(CreateMessageThatFollowsID(chat_id,
		msg_item_1.msg_id,
		(items.begin() + 1)->timestamp + std::chrono::milliseconds(1),
		"2 "));
	// msg follows by last by timestamp, parent is msg_item_2
	auto msg_item_2 = chat::GetMsgChainItem(msgs_for_test.back());
	msgs_for_test.emplace_back(CreateMessageThatFollowsID(chat_id,
		msg_item_2.msg_id, items.back().timestamp + std::chrono::milliseconds(1),
		"3 "));
	auto msg_item_3 = chat::GetMsgChainItem(msgs_for_test.back());
	// msg follows by 15 by timestamp, parent os msg_item_3
	msgs_for_test.emplace_back(CreateMessageThatFollowsID(chat_id,
		msg_item_3.msg_id,
		(items.begin() + 14)->timestamp + std::chrono::milliseconds(1),
		"4 "));
	// msg follows by 24, parent is msg_item_1
	msgs_for_test.emplace_back(CreateMessageThatFollowsID(chat_id,
		msg_item_1.msg_id,
		(items.begin() + 24)->timestamp + std::chrono::milliseconds(1),
		"5 "));
	// msg follows by first
	msgs_for_test.emplace_back(CreateMessageThatFollowsID(chat_id,
		msg_item_1.msg_id,
		items.front().timestamp - std::chrono::milliseconds(1),
		"6 "));
	std::reverse(msgs_for_test.begin(), msgs_for_test.end());
	auto proxy_above = std::make_shared<ProxyAboveLayer>();
	main_storage->SetNextLayer(proxy_above);
	auto msg_catch = std::make_shared<ArrivedMsgCatchLayer>(msg_catch_func);
	msg_catch->SetNextLayer(main_storage);
	return { ::testing::AssertionSuccess, storage,
		std::move(msgs_for_test), proxy_above, msg_catch, cfg };
}

::testing::AssertionResult
ChatContinuationFixture::OrderConsistencyCheck(
	chat::ChatIDRef chat_id,
	const chat::ChatStoragePtr &storage)
{
	auto items = GetChainFromStorage(chat_id, storage);
	auto chain = PrepareChain(items);
	std::vector<chat::chain::detail::Data> chain_data;
	std::copy(chain.begin(), chain.end(), std::back_inserter(chain_data));

	if (std::equal(chain_data.begin(), chain_data.end(), items.begin(),
		[](const auto& t1, const auto& t2) { return t1.id == t2.msg_id; }))
	{
		return ::testing::AssertionSuccess();
	}
	else
	{
		return ::testing::AssertionFailure() <<
			"Order consistency violation. Message order in ChainContainer differs from database.\n" <<
			"Expected order is:\n" << chain_data <<
			"Actual order is:\n" << items;
	}
}
size_t ChatContinuationFixture::RecvMsgsCount(
	chat::MessageType msgType,
	const std::vector<chat::CallIDRef>& Ids) const
{
	size_t res(0);
	if (Ids.empty()) // count all
	{
		for (const auto &part : all_parts_)
		{
			for (const auto &ep : part.second)
			{
				if (msgType == chat::MessageType::undefined)
				{
					res += ep->recv_msgs_->size();
				}
				else
				{
					res += ep->recv_msgs_.withLock(
						[&](const vs::map<chat::ChatMessageID, chat::MessageType, vs::str_less> &cnt)
					{
						return std::count_if(cnt.begin(), cnt.end(),
							[&](const std::pair<chat::ChatMessageID, chat::MessageType> &item)
						{
							return item.second == msgType;
						});
					});
				}
			}
		}
	}
	else
	{
		for (const auto Id : Ids)
		{
			auto part_i = all_parts_.find(Id);
			if (part_i != all_parts_.end())
			{
				for (const auto &ep : part_i->second)
				{
					if (msgType == chat::MessageType::undefined)
					{
						res += ep->recv_msgs_->size();
					}
					else
					{
						res += ep->recv_msgs_.withLock(
							[&](const vs::map<chat::ChatMessageID, chat::MessageType, vs::str_less> &cnt)
						{
							return std::count_if(cnt.begin(), cnt.end(),
								[&](const std::pair<chat::ChatMessageID, chat::MessageType> &item)
							{
								return item.second == msgType;
							});
						});
					}
				}
			}
		}
	}
	return res;
}
size_t ChatContinuationFixture::EpCount() const
{
	size_t res(0);
	for (const auto & part : all_parts_)
	{
		res += part.second.size();
	}
	return res;
}

void ChatContinuationFixture::ClearHistory(bool forAll)
{
	/*
	1. Init base;
	2. Choose chat id;
	3. Choose participant;
	4. Send ClearHistory(All/Self) message;
	5. Wait until all participants will receive message;
	6. Check that history is cleared;
	7. Choose another participant, who will send test text message;
	8. Send test text message;
	9. Wait until all participants will receive test message;
	10. Check that test message is received.
	*/

	// 1.
	ASSERT_TRUE(InitPartsDBFilled(GetParam() == DBBackEnd::postgresql));
	// 2.
	const auto& test_chat_id = g_dump_chat_id.back();
	ASSERT_FALSE(test_chat_id.empty());
	// 3.
	ASSERT_GT(only_users_.size(), 0u);
	const auto& part_ep = only_users_.begin()->second.back();
	ASSERT_TRUE(part_ep);
	const auto& part_id = only_users_.begin()->first;
	ASSERT_FALSE(part_id.empty());
	// 4.
	part_ep->app_layer_->ClearHistory(test_chat_id, forAll);
	// 5.
	EXPECT_TRUE(test::WaitFor("HistoryClearing",
		[
			this,
			msg_count = EpCount() - 1 // sender does not receive
		]()
	{ return msg_count == RecvMsgsCount(); }, 10, g_max_wait_timeout_ms));
	// 6.
	for (const auto& part : all_parts_)
	{
		for (const auto& ep : part.second)
		{
			auto all_msgs = ep->app_layer_->GetMessages(test_chat_id, {}, std::numeric_limits<int32_t>::max());
			if (forAll == true)
			{
				ASSERT_EQ(all_msgs.size(), 1u);
				auto msg_type = chat::MessageType::undefined;
				all_msgs.back().first->GetParamI32(chat::attr::MESSAGE_TYPE_paramName, msg_type);
				ASSERT_EQ(msg_type, chat::MessageType::clear_history_for_all);
			}
			else
			{
				if (part.first == part_id)
					ASSERT_EQ(all_msgs.size(), 1u);
				else
					ASSERT_NE(all_msgs.size(), 1u);
				auto msg_type = chat::MessageType::undefined;
				all_msgs.back().first->GetParamI32(chat::attr::MESSAGE_TYPE_paramName, msg_type);
				ASSERT_EQ(msg_type,
					chat::MessageType::clear_history_for_part);
			}
		}
	}
	// 7.
	const auto& another_part_ep = std::prev(only_users_.end())->second.front();
	ASSERT_TRUE(another_part_ep);
	// 8.
	another_part_ep->app_layer_->SendGroup(test_chat_id, chat::msg::ContentMessage{}.Text("Test message"), {});
	// 9.
	EXPECT_TRUE(test::WaitFor("MessageSending",
		[
			this,
			msg_count = 2 * (EpCount() - 1) // sender does not receive
		]()
	{ return msg_count == RecvMsgsCount(); }, 10, g_max_wait_timeout_ms));
	// 10.
	for (const auto& part : all_parts_)
	{
		for (const auto& ep : part.second)
		{
			auto all_msgs = ep->app_layer_->GetMessages(test_chat_id, {}, std::numeric_limits<int32_t>::max());
			if (forAll == true)
			{
				ASSERT_EQ(all_msgs.size(), 2u);
				ASSERT_EQ(GetParamStrFromMsgContent(all_msgs.back().first, chat::msg::contentKeyName), "Test message");

			}
			else
			{
				if (part.first == part_id)
					ASSERT_EQ(all_msgs.size(), 2u);
				else
					ASSERT_NE(all_msgs.size(), 2u);
				ASSERT_EQ(GetParamStrFromMsgContent(all_msgs.back().first, chat::msg::contentKeyName), "Test message");
			}
		}
	}
}

void ChatContinuationFixture::RemoveMessage(bool forAll)
{
	/*
	1. Init base;
	2. Choose chat id;
	3. Choose participant;
	4. Choose message for removal;
	5. Choose remover from participants;
	6. Send RemoveMessage;
	7. Wait until all participants will receive message;
	8. Search for removed message in every instance.
	*/

	// 1.
	ASSERT_TRUE(InitPartsDBFilled(GetParam() == DBBackEnd::postgresql));
	// 2.
	const auto& test_chat_id = g_dump_chat_id.back();
	ASSERT_FALSE(test_chat_id.empty());
	// 3.
	ASSERT_GT(only_users_.size(), 0u);
	const auto& part_ep = only_users_.begin()->second.back();
	ASSERT_TRUE(part_ep);
	// 4.
	const auto& length = 20u;
	auto msgs = part_ep->app_layer_->GetMessages(test_chat_id, {}, length);
	ASSERT_FALSE(msgs.empty());
	EXPECT_EQ(msgs.size(), length);
	chat::ChatMessageIDRef msg_id;
	for (const auto& m : msgs)
	{
		if (chat::msg::TextChatMessage::IsMyMessage(m.first))
		{
			msg_id = m.first->GetParamStrRef(chat::attr::MESSAGE_ID_paramName);
			break;
		}
	}
	ASSERT_FALSE(msg_id.empty());
	// 5.
	const auto& remover_part_ep = std::prev(only_users_.end())->second.front();
	ASSERT_TRUE(remover_part_ep);
	const auto& remover_part_id = std::prev(only_users_.end())->first;
	ASSERT_FALSE(remover_part_id.empty());
	// 6.
	remover_part_ep->app_layer_->RemoveMessage(test_chat_id, msg_id, forAll);
	// 7.
	EXPECT_TRUE(test::WaitFor("MessageRemoving",
		[
			this,
			msg_count = EpCount() - 1 // sender does not receive
		]()
	{ return msg_count == RecvMsgsCount(); }, 10, g_max_wait_timeout_ms));
	// 8.
	for (const auto& part : all_parts_)
	{
		for (const auto& ep : part.second)
		{
			auto all_msgs = ep->app_layer_->GetMessages(test_chat_id, {}, std::numeric_limits<int32_t>::max());
			if (forAll == true)
			{
				ASSERT_TRUE(std::none_of(all_msgs.begin(), all_msgs.end(),
					[&](const auto& item) { return item.first->GetParamStrRef(chat::attr::MESSAGE_ID_paramName) == msg_id; })
				);
			}
			else
			{
				if (part.first == remover_part_id)
					ASSERT_TRUE(std::none_of(all_msgs.begin(), all_msgs.end(),
						[&](const auto& item) { return item.first->GetParamStrRef(chat::attr::MESSAGE_ID_paramName) == msg_id; })
					);
				else
					ASSERT_TRUE(std::any_of(all_msgs.begin(), all_msgs.end(),
						[&](const auto& item) { return item.first->GetParamStrRef(chat::attr::MESSAGE_ID_paramName) == msg_id; })
					);
			}
		}
	}
}
}

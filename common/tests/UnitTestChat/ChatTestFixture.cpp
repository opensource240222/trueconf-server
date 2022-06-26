#include "ChatTestFixture.h"
#include "globvars.h"
#include "tests/common/GMockOverride.h"
#include "tests/common/TestHelpers.h"
#include "tests/UnitTestChat/router/ChatMessageRouter.h"
#include "tests/UnitTestChat/SetLayerHelperSimple.h"
#include "tests/UnitTestChat/TestHelpers.h"

#include "chatlib/helpers/AccountInfo.h"
#include "chatlib/msg/attr.h"
#include "chatlib/msg/ChatMessage.h"
#include "chatlib/notify/ChatEventsFuncs.h"
#include "chatlib/storage/make_chat_storage.h"
#include "chatlib/utils/chat_utils.h"
#include "chatlib/utils/msg_utils.h"
#include "chatutils/ExternalComponentsStub.h"
#include "chatutils/GlobalConfigStub.h"

#include "std-generic/compat/map.h"
#include "std-generic/compat/memory.h"

#include <bitset>
using ::testing::InSequence;
using ::testing::Truly;
using ::testing::Eq;
using ::testing::_;
using ::testing::InvokeWithoutArgs;
namespace {
class OnTextMessageRecvVirtual
{
public:
	virtual void OnTextMessage(
		const chat::ChatID&,
		const chat::CallID&,
		const chat::CallID&,
		const chat::ChatMessageID&,
		std::string*) = 0;
};

class OnTextMessageRecv_Mock : public OnTextMessageRecvVirtual
{
public:
	MOCK_METHOD5_OVERRIDE(
		OnTextMessage,
		void(
			const chat::ChatID&,
			const chat::CallID&,
			const chat::CallID&,
			const chat::ChatMessageID&,
			std::string*));
};

// wrapper for rvalue param
class OnTextMessageRecv_MockWrapper
{
	OnTextMessageRecv_Mock& delegate_;
public:
	explicit OnTextMessageRecv_MockWrapper(OnTextMessageRecv_Mock& delegate)
		: delegate_(delegate)
	{}
	void OnTextMessage(
		const chat::ChatID&id,
		const chat::CallID& author,
		const chat::CallID&from,
		const chat::ChatMessageID& msg_id,
		std::string&& content)
	{
		delegate_.OnTextMessage(id, author, from, msg_id, &content);
	}
};
}

const std::string ChatTestFixture::CREATOR_CALL_ID = "creator@trueconf.com";
const auto c_get_related_callids_func = [](chat::CallIDRef id)
{
	std::vector<chat::CallID> result;
	for (auto i = 1; i < 2; ++i)
		result.push_back(
			static_cast<chat::CallID>(id)
			+ std::string("/")
			+ std::to_string(i));
	return result;
};
const std::vector<string_view>
ChatTestFixture::BASE_SERVERS={
	"bs1.trueconf.com#bs",
	"bs2.trueconf.com#bs" };
const std::vector<chat::AccountInfoPtr>
ChatTestFixture::PARTICIPANTS_ACCOUNTS = {
	std::make_shared<chat::AccountInfo>(
		"user1@trueconf.com",
		"user1@trueconf.com/1",
		vs::CallIDType::client,
		static_cast<vs::CallID>(ChatTestFixture::BASE_SERVERS[0]),
		c_get_related_callids_func),
	std::make_shared<chat::AccountInfo>(
		"user2@trueconf.com",
		"user2@trueconf.com/1",
		vs::CallIDType::client,
		static_cast<vs::CallID>(ChatTestFixture::BASE_SERVERS[1]),
		c_get_related_callids_func)
};
ChatTestFixture::ParticipantContext::ParticipantContext(
	const chat::AccountInfoPtr &info,
	const  chat::GlobalConfigPtr & cfg)
	: accaunt_info(info)
	, global_cfg(cfg)
	, events_notifier(cfg->GetEventsNotifier())
{}
ChatTestFixture::~ChatTestFixture()
{
	if (HasFailure())
	{
		chat_test::SaveDBToDisk(db_uri_);
	}
	else
	{
		CleanPostgreSQLChats();
	}
}
void ChatTestFixture::CleanPostgreSQLChats()
{
	if(chat_test::HasPostgreSQLCfg())
	{
		auto bknd = chat::make_chat_storage(chat_test::GetPostgreSQLConfiguration());
		std::vector<chat::CallID> call_idx;
		call_idx.emplace_back(CREATOR_CALL_ID);
		for (const auto& server : BASE_SERVERS)
			call_idx.emplace_back(server);
		std::transform(PARTICIPANTS_ACCOUNTS.begin(), PARTICIPANTS_ACCOUNTS.end(),
			std::back_inserter(call_idx), [](const auto& item) { return item->GetCallID(); });
		std::set<chat::ChatID> chats_for_del;
		// Get all undelivered messages, get chat id from each
		auto undelivered_msgs = bknd->GetAllUndeliveredMessages();
		std::transform(undelivered_msgs.begin(), undelivered_msgs.end(),
			std::inserter(chats_for_del, chats_for_del.begin()),
			[&](const auto& item){
				return item.msg->GetParamStr(chat::attr::CHAT_ID_paramName);});

		// Get user personal context for each user
		for(const auto& call_id : call_idx)
		{
			auto ctxs = bknd->GetAllUserPersonalContexts(call_id);
			std::transform(ctxs.begin(), ctxs.end(),
				std::inserter(chats_for_del, chats_for_del.begin()),
				[](const auto& ctx) { return ctx.chatId; });
		}
		// get all system chats
		for (const auto& user : call_idx)
		{
			for(const auto& bs : BASE_SERVERS)
			{
				chats_for_del.emplace(chat::GetP2PChatID(user, bs));
			}
		}
		for (const auto& id : chats_for_del)
		{
			chat_test::PostgreSQLCleanChat(id);
		}
	}
}
void ChatTestFixture::SendAddPartMess(
	const chat::ParticipantType& tp,
	const chat::CallID &id_from,
	const chat::CallID&part_id,
	chat::msg::ChatMessagePtr *msg)
{
	using namespace chat;
	auto & msg_ref = *msg;
	auto part_ctx = participants_ctx_.find(id_from);
	ASSERT_TRUE(part_ctx != participants_ctx_.end());
	/**
	CHAT_MESSAGE_TYPE_paramName
	attr::CHAT_ID_paramName
	attr::CALL_ID_paramName
	attr::FROM_paramName
	attr::FROM_INSTANCE_paramName
	attr::TIMESTAMP_paramName
	attr::MESSAGE_ID_paramName
	*/

	ASSERT_GE(msg_ref->count(), 8u);
	std::string chat_id, call_id, from, from_instance, mess_id;
	auto msgType = MessageType::undefined;
	ASSERT_TRUE(msg_ref->GetParamI32(attr::MESSAGE_TYPE_paramName, msgType));
	ASSERT_EQ(msgType, MessageType::add_part);

	ASSERT_TRUE(msg_ref->GetParam(attr::CHAT_ID_paramName, chat_id));
	ASSERT_EQ(chat_id, current_chat_id_);

	ASSERT_TRUE(GetParamFromMsgContent(msg_ref, chat::msg::nameKeyName, call_id));
	ASSERT_EQ(call_id, part_id);

	ASSERT_TRUE(msg_ref->GetParam(attr::FROM_paramName, from));
	ASSERT_EQ(from, part_ctx->second->accaunt_info->GetCallID());

	ASSERT_TRUE(GetParamFromMsgContent(msg_ref, chat::msg::fromInstanceKeyName, from_instance));
	ASSERT_EQ(from_instance, part_ctx->second->accaunt_info->GetExactCallID());

	auto partType = ParticipantType::undef;
	ASSERT_TRUE(GetParamFromMsgContent(msg_ref, chat::msg::typeKeyName, partType));
	ASSERT_EQ(tp, partType);

	chat::ChatMessageTimestamp timestamp;
	ASSERT_TRUE(msg_ref->GetParam(attr::TIMESTAMP_paramName, timestamp));

	ASSERT_TRUE(msg_ref->GetParam(attr::MESSAGE_ID_paramName, mess_id));
	ASSERT_EQ(mess_id.length(), 36u); // uuid

	part_ctx->second->events_notifier->OnPartAdded(
		chat_id,
		call_id,
		mess_id,
		from);
	msg_ref->OnMsgIsStored(cb::ProcessingResult::ok);
	msg_ref->OnChainUpdateByMsg(cb::ProcessingResult::ok, {});
}

void  ChatTestFixture::InitParticipantsWithMockLayer()
{
	/**
		update ChatContexts by events on owner side,
		because IntegrityLayer is absent and chat command is not sync (user1 and user2 don't get CreateChat);
	*/
	auto events = std::make_shared<chat::notify::ChatEventsFuncs>();
	auto part_info = PARTICIPANTS_ACCOUNTS;
	part_info.push_back(
		std::make_shared<chat::AccountInfo>(
			CREATOR_CALL_ID,
			CREATOR_CALL_ID + "/1",
			vs::CallIDType::client,
			BASE_SERVERS[0],
			c_get_related_callids_func));
	for (const auto &bs : BASE_SERVERS)
		part_info.push_back(std::make_shared<chat::AccountInfo>(
			static_cast<chat::CallID>(bs),
			static_cast<chat::CallID>(bs),
			vs::CallIDType::server,
			static_cast<chat::CallID>(bs),
			[bs = static_cast<chat::CallID>(bs)](chat::CallIDRef)
				{ return std::vector<chat::CallID>{bs}; }));
	for (const auto &info : part_info)
	{
		resolver_->Add(info->GetCallID(), info);
		resolver_->AddAlias(info->GetCallID(), info->GetCallID());
		resolver_->AddAlias(info->GetCallID(), info->GetExactCallID());
		auto global_cfg = std::make_shared<GlobalConfigStub>(resolver_, events);
		global_cfg->SetAccountInfo(info);
		global_cfg->SetClockWrapper(clock_wrapper_);
		global_cfg->SetMaxChainLen(c_default_chain_len);
		global_cfg->SetBucketCapacity(c_default_bucket_capacity);
		global_cfg->SetTailLength(c_default_tail_len);

		auto db_descr = chat_test::CreateSharedDBInMemory(info->GetCallID());
		auto bknd = chat::make_chat_storage(db_descr.dbConnParam.connectionString);
		auto storage = chat::asio_sync::MakeStorageLayer(bknd, atp_.get_io_service());

		auto last_layer = std::make_shared<LayerInterface_Mock>();
		global_cfg->SetChatStorage(bknd);
		storage->Init(global_cfg);

		auto part_ctx = std::make_shared<ParticipantContext>(
			global_cfg,
			storage,
			std::move(db_descr.db),
			atp_.get_io_service(),
			last_layer);
		part_ctx->mocked_layer = last_layer;
		participants_ctx_.emplace(info->GetCallID(), part_ctx);
		if(info->GetCallID() == CREATOR_CALL_ID)
			fake_router_->SetChatStorage(part_ctx->chat_storage);
	}
}
void ChatTestFixture::InitParticipantsWithMainStorage(
	const std::vector<chat::AccountInfoPtr> &parts)
{
	auto part_info = parts.empty() ? PARTICIPANTS_ACCOUNTS:parts;
	auto creator = std::make_shared<chat::AccountInfo>(
		CREATOR_CALL_ID, CREATOR_CALL_ID + "/1",
		vs::CallIDType::client,
		static_cast<chat::CallID>(BASE_SERVERS[0]),
		c_get_related_callids_func);
	part_info.push_back(creator);
	for (const auto &bs : BASE_SERVERS)
		part_info.push_back(std::make_shared<chat::AccountInfo>(
			static_cast<chat::CallID>(bs),
			static_cast<chat::CallID>(bs),
			vs::CallIDType::server,
			static_cast<chat::CallID>(bs),
			[bs = static_cast<chat::CallID>(bs)](chat::CallIDRef)
				{ return std::vector<chat::CallID>{bs}; }));
	for (const auto &info : part_info)
	{
		resolver_->Add(info->GetCallID(), info);
		resolver_->AddAlias(info->GetCallID(), info->GetCallID());
		resolver_->AddAlias(info->GetCallID(), info->GetExactCallID());
		auto global_cfg = std::make_shared<GlobalConfigStub>(resolver_);
		global_cfg->SetAccountInfo(info);
		global_cfg->SetClockWrapper(clock_wrapper_);
		global_cfg->SetMaxChainLen(c_default_chain_len);
		global_cfg->SetBucketCapacity(c_default_bucket_capacity);
		global_cfg->SetTailLength(c_default_tail_len);
		auto db_descr = chat_test::CreateSharedDBInMemory(info->GetCallID());
		boost::optional<CppSQLite3DB> db_holder{};
		chat::ChatStoragePtr bknd;
		if (info == creator)
		{
			if (chat_test::HasPostgreSQLCfg())
			{
				bknd = chat::make_chat_storage(chat_test::GetPostgreSQLConfiguration());
			}
			else
			{
				auto connParam = chat_test::GetSQLITEConnectionString(info->GetCallID(), true);
				bknd = chat::make_chat_storage(connParam.connectionString);
				db_uri_.emplace(info->GetCallID(), std::move(connParam.dbUri));
			}
		}
		else {
			bknd = chat::make_chat_storage(db_descr.dbConnParam.connectionString);
			db_holder = std::move(db_descr.db);
			db_uri_.emplace(info->GetExactCallID(), std::move(db_descr.dbConnParam.dbUri));
		}
		auto storage = chat::asio_sync::MakeStorageLayer(bknd, atp_.get_io_service());
		global_cfg->SetChatStorage(bknd);
		storage->Init(global_cfg);
		auto integrity = chat::asio_sync::MakeIntegrityLayer(global_cfg, atp_.get_io_service());
		global_cfg->SetSyncChat(integrity);
		auto delivery = chat::asio_sync::MakeDeliveryLayer(global_cfg, atp_.get_io_service());
		auto stub = std::make_shared<LayerInterfaceAgregator_Stub>();
		auto part_ctx = std::make_shared<ParticipantContext>(
			global_cfg,
			storage,
			std::move(db_holder),
			atp_.get_io_service(),
			integrity,
			delivery,
			stub);
		participants_ctx_.emplace(info->GetCallID(), part_ctx);
	}
}
void ChatTestFixture::CreateChat(
	const chat::CallID &creator,
	const chat::ChatType&tp,
	std::string title,
	const chat::cb::CreateChatCallBack &res_cb)
{
	auto part = participants_ctx_.find(creator);
	if (part == participants_ctx_.end())
	{
		res_cb(chat::cb::CreateChatResult::failed, nullptr);
		return;
	}
	part->second->appLayer->CreateChat(
		tp,
		std::move(title),
		[res_cb,this](chat::cb::CreateChatResult res, chat::ChatIDRef id)
		{
			if (res == chat::cb::CreateChatResult::ok)
				current_chat_id_ = chat::ChatID(id);
			res_cb(res, id);
		});
}
void ChatTestFixture::RegisterAllInstancesInFakeRotuer()
{
	for (const auto &i : participants_ctx_)
		fake_router_->RegisterNewPart(
			i.second->accaunt_info->GetExactCallID(),
			std::static_pointer_cast<chat_test::SetLayerHelperSimple>(
				i.second->last_layer));
}
void ChatTestFixture::SendMessageToChatCase()
{
	/**
	1. Owner creates chat;
	2. Owner adds user1 and user2 to chat
	3. owner sends message1 - user1 and user2 get message1;
	4. user1 sends message2 after receiving message from owner - user2 and owner get message2
	5. user2 sends message3 after receiving messages from owner and user1 - user1 and user2 get message3
	*/

	//message payload
	const std::string owner_message = "Hello! I'm owner!";
	const std::string user1_message = "Hello! I'm user1!";
	const std::string user2_message = "Hello! I'm user2!";

	std::bitset<5> part_added_flag;
	auto all_part_added_ev = std::make_shared<vs::event>(false);

	auto i = participants_ctx_.find(CREATOR_CALL_ID);
	ASSERT_TRUE(i != participants_ctx_.end());
	auto owner = i->second;
	auto account_iter = PARTICIPANTS_ACCOUNTS.begin();
	auto user1_account = *account_iter;
	++account_iter;
	ASSERT_TRUE(account_iter != PARTICIPANTS_ACCOUNTS.end());
	auto user2_account = *account_iter;

	i = participants_ctx_.find(user1_account->GetCallID());
	ASSERT_TRUE(i != participants_ctx_.end());
	auto user1 = i->second;

	i = participants_ctx_.find(user2_account->GetCallID());
	ASSERT_TRUE(i != participants_ctx_.end());
	auto user2 = i->second;
	// owner creates chat
	chat::ChatID chat_id;

	auto bs1 = participants_ctx_.find(BASE_SERVERS[0])->second;
	auto bs2 = participants_ctx_.find(BASE_SERVERS[1])->second;
	// functors for send messages
	// owner sends mess
	auto func_owner_sends_message =
		[&owner, &chat_id, owner_message = owner_message] () mutable
	{
		owner->appLayer->SendGroup(
			chat_id,
			chat::msg::ContentMessage{}.Text(owner_message),
			[](
				chat::cb::ProcessingResult r,
				chat::ChatMessageIDRef,
				const chat::cb::MsgIdAndOrderInChain&)
			{
				ASSERT_EQ(r, chat::cb::ProcessingResult::ok);
			});
	};
	//user1 sends message
	auto func_user1_sends_message =
		[&user1, &chat_id, user1_message = user1_message] () mutable
	{
		user1->appLayer->SendGroup(
			chat_id,
			chat::msg::ContentMessage{}.Text(user1_message),
			[](
				chat::cb::ProcessingResult r,
				chat::ChatMessageIDRef,
				const chat::cb::MsgIdAndOrderInChain&)
			{
				ASSERT_EQ(r, chat::cb::ProcessingResult::ok);
			});
	};
	//user2 sends message
	auto func_user2_sends_message =
		[&user2, &chat_id, user2_message = user2_message]() mutable
	{
		user2->appLayer->SendGroup(
			chat_id,
			chat::msg::ContentMessage{}.Text(user2_message),
			[](
				chat::cb::ProcessingResult r,
				chat::ChatMessageIDRef,
				const chat::cb::MsgIdAndOrderInChain&)
			{
				ASSERT_EQ(r, chat::cb::ProcessingResult::ok);
			});
	};
	/**
	on main message recv handlers
	*/
	OnTextMessageRecv_Mock on_msg_recv_owner;
	OnTextMessageRecv_Mock on_msg_recv_user1;
	OnTextMessageRecv_Mock on_msg_recv_user2;

	/**Mock processing*/

	auto user1_call_id = user1->accaunt_info->GetCallID();
	auto user2_call_id = user2->accaunt_info->GetCallID();
	auto owner_call_id = owner->accaunt_info->GetCallID();
	owner->global_cfg->GetEventsSubscription()->SubscribeToPartAdded(
		[
			all_part_added_ev,
			owner_call_id,
			user1_call_id,
			user2_call_id,
			&part_added_flag,
			&bs1, &bs2
		](
			chat::ChatIDRef,
			chat::CallIDRef part,
			chat::ChatMessageIDRef,
			chat::CallIDRef)
	{
		if (part == owner_call_id)
			part_added_flag.set(0);
		else if (part == user1_call_id)
			part_added_flag.set(1);
		else if (part == user2_call_id)
			part_added_flag.set(2);
		else if(part == bs1->accaunt_info->GetCallID())
			part_added_flag.set(3);
		else if (part == bs2->accaunt_info->GetCallID())
			part_added_flag.set(4);
		if(part_added_flag.all())
			all_part_added_ev->set();
	}
	);

	std::bitset<12> msgs_is_recved_flags;
	auto all_msgs_arrived_ev = std::make_shared<vs::event>(false);
	{
		//owner waits messages from user1 and user2. Order is not important
		EXPECT_CALL(
			on_msg_recv_owner,
			OnTextMessage(
				Truly([&chat_id]( const chat::ChatID&id)
					{return id == chat_id; }),
				Eq(user1_call_id),
				_,
				_,
				Truly([&user1_message](std::string* rcv)->bool
					{
						return user1_message == *rcv;
					})))
			.WillOnce(InvokeWithoutArgs([all_msgs_arrived_ev, &msgs_is_recved_flags]()
					{
						msgs_is_recved_flags.set(0);
						if (msgs_is_recved_flags.all())
							all_msgs_arrived_ev->set();
					}));

		EXPECT_CALL(
			on_msg_recv_owner,
			OnTextMessage(
				Truly(
					[&chat_id](const chat::ChatID&id)
					{return id == chat_id; }),
				Eq(user2_call_id),
				_,
				_,
				Truly(
					[&user2_message](std::string* rcv)->bool
					{
						return user2_message == *rcv;
					})))
			.WillOnce(InvokeWithoutArgs([all_msgs_arrived_ev, &msgs_is_recved_flags]()
					{
						msgs_is_recved_flags.set(1);
						if (msgs_is_recved_flags.all())
							all_msgs_arrived_ev->set();
					}));
	}
	{
		//user1 waits messages from owner and user2. Order is not important
		EXPECT_CALL(
			on_msg_recv_user1,
			OnTextMessage(
				Truly([&chat_id](const chat::ChatID&id)
					{return id == chat_id; }),
				Eq(owner_call_id),
				_,
				_,
				Truly([&owner_message](std::string* rcv)->bool
					{
						return owner_message == *rcv;
					})))
			.WillOnce(InvokeWithoutArgs([&msgs_is_recved_flags, &func_user1_sends_message, all_msgs_arrived_ev]()
					{
						func_user1_sends_message();
						msgs_is_recved_flags.set(2);
						if (msgs_is_recved_flags.all())
							all_msgs_arrived_ev->set();
					}));
		EXPECT_CALL(
			on_msg_recv_user1,
			OnTextMessage(
				Truly(
					[&chat_id](const chat::ChatID&id)
					{return id == chat_id; }),
				Eq(user2_call_id),
				_,
				_,
				Truly([&user2_message](std::string* rcv)->bool
					{
						return user2_message == *rcv;
					})))
			.WillOnce(InvokeWithoutArgs([all_msgs_arrived_ev, &msgs_is_recved_flags]()
					{
						msgs_is_recved_flags.set(3);
						if (msgs_is_recved_flags.all())
							all_msgs_arrived_ev->set();
					}));
	}

	{
		//user2 waits messages from owner and user1. Order is not important.
		EXPECT_CALL(
			on_msg_recv_user2,
			OnTextMessage(
				Truly([&chat_id](const chat::ChatID&id)
					{return id == chat_id; }),
				Eq(owner_call_id),
				_,
				_,
				Truly([&owner_message](std::string* rcv)->bool
					{
						return owner_message == *rcv;
					})))
			.WillOnce(InvokeWithoutArgs([all_msgs_arrived_ev, &msgs_is_recved_flags]()
					{
						msgs_is_recved_flags.set(4);
						if (msgs_is_recved_flags.all())
							all_msgs_arrived_ev->set();
					}));
		EXPECT_CALL(
			on_msg_recv_user2,
			OnTextMessage(
				Truly([&chat_id](const chat::ChatID&id)
					{return id == chat_id; }),
				Eq(user1_call_id),
				_,
				_,
				Truly([&user1_message](std::string* rcv)->bool
				{
					return user1_message == *rcv;
				})))
			.WillOnce(InvokeWithoutArgs([all_msgs_arrived_ev, &func_user2_sends_message, &msgs_is_recved_flags]()
				{
					func_user2_sends_message();
					msgs_is_recved_flags.set(5);
					if (msgs_is_recved_flags.all())
						all_msgs_arrived_ev->set();

				})); // user2 sends message after receiving messages from owner and user1
	}
	vs::map<chat::CallID, OnTextMessageRecv_MockWrapper> handlers;
	handlers.emplace(
			owner->accaunt_info->GetCallID(),
			OnTextMessageRecv_MockWrapper(on_msg_recv_owner));
	handlers.emplace(
			user1->accaunt_info->GetCallID(),
			OnTextMessageRecv_MockWrapper(on_msg_recv_user1));
	handlers.emplace(
			user2->accaunt_info->GetCallID(),
			OnTextMessageRecv_MockWrapper(on_msg_recv_user2));

	auto on_rcv_message = [&handlers](
		chat::CallIDRef to, chat::ChatIDRef id,
		chat::CallIDRef author, chat::CallIDRef from,
		chat::ChatMessageIDRef msg_id,
		std::string&& content)
	{
		const auto &i = handlers.find(chat::CallID(to));
		ASSERT_TRUE(i != handlers.end());
		i->second.OnTextMessage(
			chat::ChatID(id),
			chat::CallID(author),
			chat::CallID(from),
			chat::ChatMessageID(msg_id),
			std::move(content));
	};

	auto on_rcv_msg_owner = [&on_rcv_message, &owner](
		chat::ChatIDRef id, chat::CallIDRef author,
		chat::CallIDRef from, chat::ChatMessageIDRef msg_id,
		std::string&& content)
	{
		on_rcv_message(
			owner->accaunt_info->GetCallID(),
			id,
			author,
			from,
			msg_id,
			std::move(content));
	};

	auto on_rcv_msg_user1 = [&on_rcv_message, &user1](
		chat::ChatIDRef id, chat::CallIDRef author,
		chat::CallIDRef from, chat::ChatMessageIDRef msg_id,
		std::string&& content)
	{
		on_rcv_message(
			user1->accaunt_info->GetCallID(),
			id,
			author,
			from,
			msg_id,
			std::move(content));
	};

	auto on_rcv_msg_user2 = [&on_rcv_message, &user2](
		chat::ChatIDRef id, chat::CallIDRef author,
		chat::CallIDRef from, chat::ChatMessageIDRef msg_id,
		std::string&& content)
	{
		on_rcv_message(
			user2->accaunt_info->GetCallID(),
			id,
			author,
			from,
			msg_id,
			std::move(content));
	};
	owner->appLayer->SetOnMsgRecvSlot(
		[&](const chat::cb::MsgIdAndOrderInChain&, const chat::msg::ChatMessagePtr& msg)
	{
		// process only text messages
		auto parsed_msg = chat::msg::detail::parse_text_msg(msg);
		if (!parsed_msg.success)
			return;
		on_rcv_msg_owner(
			parsed_msg.msg.chat_id,
			parsed_msg.msg.author,
			parsed_msg.msg.author,
			parsed_msg.msg.msg_id,
			std::move(parsed_msg.msg.content.text));
	});
	user1->appLayer->SetOnMsgRecvSlot(
		[&](const chat::cb::MsgIdAndOrderInChain&, const chat::msg::ChatMessagePtr& msg)
	{
		// process only text messages
		auto parsed_msg = chat::msg::detail::parse_text_msg(msg);
		if (!parsed_msg.success)
			return;
		on_rcv_msg_user1(
			parsed_msg.msg.chat_id,
			parsed_msg.msg.author,
			parsed_msg.msg.author,
			parsed_msg.msg.msg_id,
			std::move(parsed_msg.msg.content.text));
	});
	user2->appLayer->SetOnMsgRecvSlot(
		[&](const chat::cb::MsgIdAndOrderInChain&, const chat::msg::ChatMessagePtr& msg)
	{
		// process only text messages
		auto parsed_msg = chat::msg::detail::parse_text_msg(msg);
		if (!parsed_msg.success)
			return;
		on_rcv_msg_user2(
			parsed_msg.msg.chat_id,
			parsed_msg.msg.author,
			parsed_msg.msg.author,
			parsed_msg.msg.msg_id,
			std::move(parsed_msg.msg.content.text));
	});
	bs1->appLayer->SetOnMsgRecvSlot(
		[
			all_msgs_arrived_ev,
			&msgs_is_recved_flags,
			owner_call_id,
			user1_call_id,
			user2_call_id
		](const chat::cb::MsgIdAndOrderInChain&, const chat::msg::ChatMessagePtr& msg)
	{
		// process only text messages
		auto parsed_msg = chat::msg::detail::parse_text_msg(msg);
		if (!parsed_msg.success)
			return;
		string_view author = parsed_msg.msg.author;
		if (author == owner_call_id)
			msgs_is_recved_flags.set(6);
		else if (author == user1_call_id)
			msgs_is_recved_flags.set(7);
		else if (author == user2_call_id)
			msgs_is_recved_flags.set(8);
		if (msgs_is_recved_flags.all())
			all_msgs_arrived_ev->set();
	});
	bs2->appLayer->SetOnMsgRecvSlot(
		[
			all_msgs_arrived_ev,
			&msgs_is_recved_flags,
			owner_call_id,
			user1_call_id,
			user2_call_id
		](const chat::cb::MsgIdAndOrderInChain&, const chat::msg::ChatMessagePtr& msg)
	{
		// process only text messages
		auto parsed_msg = chat::msg::detail::parse_text_msg(msg);
		if (!parsed_msg.success)
			return;
		string_view author = parsed_msg.msg.author;
		if (author == owner_call_id)
			msgs_is_recved_flags.set(9);
		else if (author == user1_call_id)
			msgs_is_recved_flags.set(10);
		else if (author == user2_call_id)
			msgs_is_recved_flags.set(11);
		if (msgs_is_recved_flags.all())
			all_msgs_arrived_ev->set();
	});
	owner->appLayer->CreateChat(chat::ChatType::symmetric, "Test Chat Title",
		[&](
			chat::cb::CreateChatResult r,
			chat::ChatIDRef id)
	{
		ASSERT_EQ(r, chat::cb::CreateChatResult::ok);
		ASSERT_TRUE(id.length() > 0);
		current_chat_id_ = chat_id = chat::ChatID(id);
		owner->appLayer->AddParticipant(chat::ChatID(id), user1->accaunt_info->GetCallID(),
			[](chat::cb::ProcessingResult r, chat::ChatMessageIDRef,
				const chat::cb::MsgIdAndOrderInChain&)
		{
			ASSERT_EQ(r, chat::cb::ProcessingResult::ok);
		});
		owner->appLayer->AddParticipant(chat::ChatID(id), user2->accaunt_info->GetCallID(),
			[](chat::cb::ProcessingResult r, chat::ChatMessageIDRef,
				const chat::cb::MsgIdAndOrderInChain&)
		{
			ASSERT_EQ(r, chat::cb::ProcessingResult::ok);
		});
	});
	EXPECT_TRUE(test::WaitFor("AddParticipants", *all_part_added_ev,g_max_wait_timeout_ms));
	func_owner_sends_message();
	EXPECT_TRUE(test::WaitFor("Messages Arrived", *all_msgs_arrived_ev, g_max_wait_timeout_ms)) <<
		msgs_is_recved_flags.to_string();
}

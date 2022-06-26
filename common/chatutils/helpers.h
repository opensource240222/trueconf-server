#pragma once
#include "chatlib/callbacks.h"
#include "chatlib/chat_defs.h"
#include "chatlib/factory_asio/make_layers_asio.h"
#include "chatlib/helpers/AccountInfo.h"
#include "chatlib/interface/GlobalConfigInterface.h"
#include "chatlib/msg/attr.h"
#include "chatlib/notify/GlobalChatEvents.h"

#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/synchronized.h"
#include "std-generic/sqlite/CppSQLite3.h"
#include "std-generic/asio_fwd.h"

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <iostream>
#include <memory>

class ResolverStub;
class TransportChannelRouter;

struct ChatUtilsEnvironment
{
	VS_FORWARDING_CTOR4(ChatUtilsEnvironment,
		router, resolver, clock_wrapper, ios) {}
	std::shared_ptr<TransportChannelRouter> router;
	std::shared_ptr<ResolverStub>           resolver;
	chat::ClockWrapperPtr                   clock_wrapper;
	boost::asio::io_service                 &ios;
};
struct SQLiteDBParam
{
	VS_FORWARDING_CTOR2(SQLiteDBParam, connectionString, dbUri)
	{}
	SQLiteDBParam() : connectionString(), dbUri() {}
	std::string connectionString;
	std::string dbUri;
};
struct PartInstance
{
	void SetCfg(const chat::GlobalConfigPtr &cfg)
	{
		account_ = cfg->GetCurrentAccount();
		cfg_ = cfg;
		auto notify = cfg->GetEventsSubscription();
		notify->SubscribeToMsgRead(
			[this](
				chat::ChatIDRef,
				chat::ChatMessageIDRef msg_id,
				chat::CallIDRef call_id)
		{
			read_msgs_by_call_id_.withLock([&](ReadMsgsByCallID& cnt)
			{
				auto iter = cnt.emplace(call_id, std::vector<chat::ChatMessageID>());
				iter.first->second.emplace_back(msg_id);
			});
		}
		);
		notify->SubscribeToPartAdded(
			[this](auto chat_id, auto call_id, auto, auto)
		{
			if (account_->GetCallID() == call_id)
				my_chats_->emplace(chat_id);
		});
	}
	void SetAppLayer(const chat::asio_sync::AppLayerPtr &app, bool invite_ignore)
	{
		app_layer_ = app;
		app_layer_->SetOnInviteCallBack(
			[this, invite_ignore](
				chat::cb::ChatDescription &&chat_descr,
				const chat::cb::InviteResponseCallBack &cb)
		{
			std::cout << "Invite received.\n\t"
				<< "ChatID: " << chat_descr.chatID << ";\n\t"
				<< "User: " << account_->GetCallID() << ";\n\t"
				<< "Endpoint: " << account_->GetExactCallID() << ";\n\t"
				<< "Accept\n";
			if (!invite_ignore)
				cb(chat::InviteResponseCode::accept);
		});
		app_layer_->SetOnInviteResponseCallBack(
			[this](
				chat::ChatIDRef chat_id,
				chat::CallIDRef call_id,
				chat::InviteResponseCode r)
		{
			std::cout << "InviteResponse received.\n\t"
				<< "ChatID: " << chat_id << ";\n\t"
				<< "User: " << account_->GetCallID() << "\n\t"
				<< "Endpoint: " << account_->GetExactCallID() << ";\n\t"
				<< "From: " << call_id << ";\n\t"
				<< "Response code: " << static_cast<int>(r) << "\n";
		});
		app_layer_->SetOnMsgRecvSlot([this](
			const chat::cb::MsgIdAndOrderInChain&,
			const chat::msg::ChatMessagePtr& msg)
		{
				auto type = chat::MessageType::undefined;
				msg->GetParamI32(chat::attr::MESSAGE_TYPE_paramName, type);
			recv_msgs_->emplace(
				msg->GetParamStrRef(chat::attr::MESSAGE_ID_paramName), type);
		});
	}
	void SetStorageLayer(const chat::asio_sync::MainStorageLayerPtr& storage_layer)
	{
		storage_layer_ = storage_layer;
	}
	chat::AccountInfoPtr account_;
	chat::asio_sync::AppLayerPtr app_layer_;
	chat::asio_sync::MainStorageLayerPtr storage_layer_;
	chat::GlobalConfigPtr cfg_;
	SQLiteDBParam db_conn_param;
	boost::optional<CppSQLite3DB> db_holder;
	vs::Synchronized<
		vs::map<chat::ChatMessageID, chat::MessageType, vs::str_less>> recv_msgs_;
	using ReadMsgsByCallID = vs::map<
		chat::CallID,
		std::vector<chat::ChatMessageID>,
		vs::less<>>;
	vs::Synchronized<ReadMsgsByCallID> read_msgs_by_call_id_;
	vs::Synchronized<std::set<chat::ChatID, vs::str_less>> my_chats_;
};

using endpoints_collection = std::vector<std::shared_ptr<PartInstance>>;
using part_collection = vs::map<chat::CallID, endpoints_collection, vs::str_less>;
struct EpCreateInfo
{
	VS_FORWARDING_CTOR4(EpCreateInfo, name, db_filename, db_conn_param, db_holder) {}
	EpCreateInfo()
	{}
	EpCreateInfo(EpCreateInfo&&) = default;
	EpCreateInfo& operator=(EpCreateInfo&&) = default;
	chat::CallID name;
	std::string db_filename;
	SQLiteDBParam db_conn_param;
	boost::optional<CppSQLite3DB> db_holder;
};
endpoints_collection MakeChatUser(
	chat::CallIDRef user_name,
	std::vector<EpCreateInfo> &&eps,
	string_view bs,
	uint32_t max_chain_len,
	uint32_t bucket_capacity,
	uint32_t tail_length,
	bool delete_old_db,
	const ChatUtilsEnvironment &env);

part_collection  GenerateChatParts(
	size_t user_count,
	size_t ep_for_each_user,
	size_t bs_count,
	uint32_t max_chain_len,
	uint32_t bucket_capacity,
	uint32_t tail_length,
	bool delete_old_db,
	const ChatUtilsEnvironment &env);

chat::ChatID CreateChatAddParts(
	const std::vector<std::shared_ptr<PartInstance>> &parts);

void GenerateChat(
	uint32_t msg_count_for_each,
	const part_collection &parts);
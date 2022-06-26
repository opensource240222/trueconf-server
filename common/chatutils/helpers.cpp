#include "helpers.h"

#include "ExternalComponentsStub.h"
#include "GlobalConfigStub.h"
#include "ResolverStub.h"
#include "TransportChannelRouter.h"

#include "chatlib/storage/make_chat_storage.h"

#include "std/cpplib/event.h"
#include "std-generic/compat/memory.h"
#include "std-generic/cpplib/scope_exit.h"
#include <random>

std::random_device rd;

endpoints_collection MakeChatUser(
	chat::CallIDRef user_name,
	std::vector<EpCreateInfo> &&eps,
	string_view bs,
	uint32_t max_chain_len,
	uint32_t bucket_capacity,
	uint32_t tail_length,
	bool delete_old_db,
	const ChatUtilsEnvironment &env)
{
	endpoints_collection res;
	auto router = env.router;
	auto resolver = env.resolver;
	auto clock_wrapper = env.clock_wrapper;
	auto callIdType = bs == user_name ? vs::CallIDType::server : vs::CallIDType::client;
	std::vector<chat::CallID> ep_names;
	std::transform(eps.begin(), eps.end(), std::back_inserter(ep_names),
		[](const EpCreateInfo &ep) { return ep.name; });
	for (auto && ep : eps)
	{
		auto part = std::make_shared<PartInstance>();
		auto cfg = std::make_shared<GlobalConfigStub>(resolver);
		cfg->SetClockWrapper(std::make_shared<steady_clock_wrapper>());
		auto user_info = std::make_shared<chat::AccountInfo>(
			vs::CallID(user_name),
			ep.name, callIdType,
			vs::BSInfo(bs),
			[ep_names](chat::CallIDRef) { return ep_names; });
		cfg->SetAccountInfo(user_info);
		resolver->AddAlias(std::string(user_name), ep.name);
		chat::ChatStoragePtr storage;
		string_view config_string = ep.db_conn_param.connectionString;
		if (config_string.find("postgresql:") == 0)
		{
			storage = chat::make_chat_storage(config_string);
		}
		else if (config_string.find("sqlite3:") == 0)
		{
			if (delete_old_db)
				std::remove(ep.db_filename.c_str());
			storage = chat::make_chat_storage(config_string);
			part->db_holder = std::move(ep.db_holder);
		}
		else
		{
			assert(false);
		}
		part->db_conn_param = ep.db_conn_param;
		auto storage_layer = chat::asio_sync::MakeStorageLayer(storage, env.ios);
		cfg->SetChatStorage(storage);
		cfg->SetMaxChainLen(max_chain_len);
		cfg->SetBucketCapacity(bucket_capacity);
		cfg->SetTailLength(tail_length);
		storage_layer->Init(cfg);

		auto app_layer = chat::asio_sync::MakeAppLayer(cfg, storage_layer, env.ios);
		auto integrity = chat::asio_sync::MakeIntegrityLayer(cfg, env.ios);
		cfg->SetSyncChat(integrity);
		auto delivery = chat::asio_sync::MakeDeliveryLayer(cfg, env.ios);
		auto transport = chat::asio_sync::MakeTransportLayer(
			cfg,
			router->GetChannel(ep.name),
			env.ios);
		auto system_chat = chat::asio_sync::MakeSystemChatLayer(cfg, env.ios);
		app_layer->SetNextLayer(system_chat);
		system_chat->SetNextLayer(storage_layer);
		storage_layer->SetNextLayer(integrity);
		integrity->SetNextLayer(delivery);
		delivery->SetNextLayer(transport);
		part->SetCfg(cfg);
		part->SetAppLayer(app_layer, eps.front().name == ep.name ? false : true);
		part->SetStorageLayer(storage_layer);
		res.push_back(part);
	}
	resolver->AddAlias(vs::CallID(user_name), vs::CallID(user_name));
	resolver->Add(vs::CallID(user_name), callIdType, bs, std::move(ep_names));
	return res;
}
part_collection GenerateChatParts(
	size_t user_count,
	size_t ep_for_each_user,
	size_t bs_count,
	uint32_t max_chain_len,
	uint32_t bucket_capacity,
	uint32_t tail_length,
	bool delete_old_db,
	const ChatUtilsEnvironment &env)
{
	std::mt19937 gen(rd());
	const std::string user_prefix = "user";
	const std::string user_postfix = "@trueconf.com";
	const std::string bs_prefix = "bs";
	const std::string bs_postfix = ".trueconf.com#bs";

	auto clock_wrapper = env.clock_wrapper;
	auto router = env.router;
	auto resolver = env.resolver;

	std::vector<std::string> bs_list;
	std::vector<chat::CallID> user_names;

	part_collection chat_parts;
	/**
	create bs
	*/
	constexpr const char* db_conn_prefix = "sqlite3:busy_timeout=10000;db=";
	for (auto i = 0u; i < bs_count; ++i)
	{
		bs_list.emplace_back(
			bs_prefix
			+ std::to_string(i + 1)
			+ bs_postfix);
		const auto &bs_name = bs_list.back();
		std::string db_file_name = bs_name;
		std::replace(db_file_name.begin(), db_file_name.end(), '/', '_');
		db_file_name += ".sqlite";
		auto ep_create = EpCreateInfo(bs_name, db_file_name,
			SQLiteDBParam(db_conn_prefix + db_file_name, db_file_name), boost::optional<CppSQLite3DB>{});
		std::vector<EpCreateInfo> ep_info;
		ep_info.emplace_back(std::move(ep_create));
		chat_parts.emplace(
			bs_name,
			MakeChatUser(
				bs_name,
				std::move(ep_info),
				bs_name,
				max_chain_len,
				bucket_capacity,
				tail_length,
				delete_old_db,
				env
			));
	}
	for (auto i = 0u; i < user_count; ++i)
	{
		user_names.emplace_back(
			user_prefix
			+ std::to_string(i + 1)
			+ user_postfix);
		const auto &current_user_nm = user_names.back();
		std::vector<EpCreateInfo> ep_list;
		for (auto i = 0u; i < ep_for_each_user; ++i)
		{
			auto name = current_user_nm
				+ chat::CALL_ID_SEPARATOR
				+ std::to_string(i);
			auto db_file_name = name;
			std::replace(db_file_name.begin(), db_file_name.end(), '/', '_');
			db_file_name += ".sqlite";
			ep_list.emplace_back(std::move(name), db_file_name,
				SQLiteDBParam(db_conn_prefix + db_file_name, db_file_name), boost::optional<CppSQLite3DB>{});
		}
		std::uniform_int_distribution<size_t> dist(0, bs_list.size() - 1);
		const auto &current_bs = bs_list[dist(gen)];
		chat_parts.emplace(
			current_user_nm,
			MakeChatUser(
				current_user_nm,
				std::move(ep_list),
				current_bs,
				max_chain_len,
				bucket_capacity,
				tail_length,
				delete_old_db,
				env));
	}
	return chat_parts;
}
chat::ChatID CreateChatAddParts(
	const std::vector<std::shared_ptr<PartInstance>> &parts)
{
	chat::ChatID chat_id;
	std::mt19937 g(rd());
	std::uniform_int_distribution<size_t> dist(0, parts.size() - 1);
	auto creator = *std::next(parts.begin(), dist(g));
	vs::event ev{false};
	auto title = "Title for chat";
	vs::set<chat::CallID> parts_call_ids;
	std::transform(
		parts.begin(),parts.end(),
		std::inserter(parts_call_ids,parts_call_ids.end()),
		[&](const std::shared_ptr<PartInstance>&part)
		{
			parts_call_ids.emplace(part->account_->GetBS());
			return part->account_->GetCallID();
		});
	creator->cfg_->GetEventsSubscription()->SubscribeToPartAdded(
		[&](
			chat::ChatIDRef,
			chat::CallIDRef part,
			chat::ChatMessageIDRef,
			chat::CallIDRef)
	{
		parts_call_ids.erase(chat::CallID(part));
		if (parts_call_ids.empty())
			ev.set();
	});
	creator->app_layer_->CreateChat(
		chat::ChatType::symmetric,
		title,
		[&](chat::cb::CreateChatResult, chat::ChatIDRef id)
	{
		chat_id = chat::ChatID(id);
		/**
			invite all
		*/
		vs::set<chat::ChatIDRef, vs::less<>> in_chat;
		in_chat.emplace(creator->account_->GetCallID());
		for (const auto& part : parts)
		{
			if (in_chat.count(part->account_->GetCallID()) == 0)
			{
				creator->app_layer_->AddParticipant(
					chat_id,
					part->account_->GetCallID());
				in_chat.emplace(part->account_->GetCallID());
			}
		}
	});
	ev.wait();
	return chat_id;
}
void wait_until_ep_added_in_chat(
	const chat::ChatID& chat_id,
	const std::vector<std::shared_ptr<PartInstance>> &ep)
{
	for (const auto &i : ep)
	{
		bool in_chat(false);
		do
		{
			std::this_thread::yield();
			in_chat = i->my_chats_->count(chat_id) > 0;
		} while (!in_chat);
	}
}
void wait_until_ep_recv_msg(
	chat::ChatMessageIDRef msg_id,
	const std::shared_ptr<PartInstance> &ep)
{
	while (ep->recv_msgs_->count(msg_id) == 0)
	{
		std::this_thread::yield();
	}
}
void GenerateChat(
	uint32_t msg_count_for_each,
	const part_collection &parts)
{
	part_collection only_users;
	part_collection only_bs;
	vs::map<chat::CallID, uint32_t> sent_msgs;
	std::vector<std::shared_ptr<PartInstance>> all_endpoints;
	for (const auto &i : parts)
	{
		if (i.first != i.second.front()->account_->GetBS())
		{
			only_users.emplace(i);
			sent_msgs.emplace(i.first, 0);
			std::copy(
				i.second.begin(),
				i.second.end(),
				std::back_inserter(all_endpoints));
		}
		else
			only_bs.emplace(i);
	}
	auto chat_id = CreateChatAddParts(all_endpoints);
	wait_until_ep_added_in_chat(chat_id, all_endpoints);
	std::mt19937 g(rd());
	chat::ChatMessageID last_sent_msg_id;
	vs::event msg_sent_ev(false);
	auto copy_all_endpoints = all_endpoints;
	while (!all_endpoints.empty())
	{
		/**
		shake what ep send
		*/
		std::shuffle(all_endpoints.begin(), all_endpoints.end(), g);
		auto send_from_ep = *all_endpoints.begin();
		auto msg_num = ++sent_msgs[send_from_ep->account_->GetCallID()];
		std::string msg_data = "Message #"
			+ std::to_string(msg_num)
			+ " from CallID: "
			+ send_from_ep->account_->GetCallID()
			+ " from ep: "
			+ send_from_ep->account_->GetExactCallID();
		send_from_ep->app_layer_->SendGroup(
			chat_id,
			chat::msg::ContentMessage{}.Text(msg_data),
			[&](chat::cb::ProcessingResult,
				chat::ChatMessageIDRef msg_id,
				const chat::cb::MsgIdAndOrderInChain&)
		{
			last_sent_msg_id = chat::ChatMessageID(msg_id);
			msg_sent_ev.set();
		}
		);
		msg_sent_ev.wait();
		send_from_ep->recv_msgs_->emplace(
			last_sent_msg_id, chat::MessageType::text);
		if (msg_num >= msg_count_for_each)
			all_endpoints.erase(
				std::remove_if(
					all_endpoints.begin(),
					all_endpoints.end(),
					[send_from_ep](const std::shared_ptr<PartInstance>&part) {
			return part->account_->GetCallID()
				== send_from_ep->account_->GetCallID(); }),
				all_endpoints.end());
	}
	for (const auto &i : copy_all_endpoints)
	{
		wait_until_ep_recv_msg(last_sent_msg_id, i);
	}
}
#pragma once
#include "mock/ChatLayerAbstractMock.h"
#include "router/ChatMessageRouter.h"

#include "chatlib/factory_asio/make_layers_asio.h"
#include "chatlib/interface/GlobalConfigInterface.h"
#include "chatlib/notify/GlobalChatEvents.h"
#include "chatutils/ResolverStub.h"

#include "commonlibs/std-generic/compat/functional.h"
#include "std/cpplib/ASIOThreadPool.h"
#include "std/cpplib/MakeShared.h"

#include "std-generic/sqlite/CppSQLite3.h"

#include <gtest/gtest.h>

#include <vector>

#include <boost/optional.hpp>

class ChatMessageFakeRouter;
class ChatTestFixture : public testing::Test {
public:
	const static std::vector<chat::AccountInfoPtr> PARTICIPANTS_ACCOUNTS;
	const static std::vector<string_view> BASE_SERVERS;
	void SendAddPartMess(
		const chat::ParticipantType& tp,
		const chat::CallID &id_from,
		const chat::CallID&part_id,
		chat::msg::ChatMessagePtr *msg);
	ChatTestFixture()
		: atp_(1)
	{
		atp_.Start();
		CleanPostgreSQLChats();
	}
	~ChatTestFixture();

protected:
	struct ParticipantContext
	{
		template<typename... Args>
		ParticipantContext(const chat::GlobalConfigPtr & cfg,
			const chat::asio_sync::MainStorageLayerPtr& storageLayer,
			boost::optional<CppSQLite3DB> &&db,
			boost::asio::io_service&ios, Args&... args)
			: accaunt_info(cfg->GetCurrentAccount())
			, global_cfg(cfg)
			, events_notifier(cfg->GetEventsNotifier())
			, db_holder(std::move(db))
		{
			chat_storage = cfg->GetChatStorage();
			appLayer = chat::asio_sync::MakeAppLayer(global_cfg,
				storageLayer,
				ios);
			InitLayers(appLayer, storageLayer, args...);
		}
		ParticipantContext(
			const chat::AccountInfoPtr &info,
			const  chat::GlobalConfigPtr & cfg);

		chat::AccountInfoPtr accaunt_info;
		chat::GlobalConfigPtr global_cfg;
		std::shared_ptr<chat::asio_sync::AppLayer> appLayer;
		std::shared_ptr<LayerInterface_Mock> mocked_layer;
		std::shared_ptr<LayerInterfaceAgregator_Mock> last_layer;
		chat::ChatStoragePtr chat_storage;
		std::shared_ptr<chat::notify::ChatEventsNotifier> events_notifier;
		boost::optional<CppSQLite3DB> db_holder;
	private:
		template<typename T, typename P, typename... Args>
		void InitLayers(T& first_layer, P& next_layer, Args&... args)
		{
			first_layer->SetNextLayer(next_layer);
			InitLayers(next_layer, args...);
		}
		template <typename T, typename P>
		void InitLayers(T& first_layer, P& next_layer)
		{
			first_layer->SetNextLayer(next_layer);
			last_layer = next_layer;
		}
	};
	const static std::string CREATOR_CALL_ID;
	void SetUp() override
	{
		clock_wrapper_ = std::make_shared<chat::ClockWrapper>();
		fake_router_->SetResolver(resolver_);
	}
	void TearDown() override
	{
		for(const auto& part: participants_ctx_)
		{
			part.second->appLayer->ShutDown();
		}
		atp_.Stop();
	}
	void CleanPostgreSQLChats();
	void InitParticipantsWithMockLayer();
	void InitParticipantsWithMainStorage(
		const std::vector<chat::AccountInfoPtr> &parts = {});
	void CreateChat(
		const chat::CallID &creator,
		const chat::ChatType&tp,
		std::string title,
		const chat::cb::CreateChatCallBack &res_cb);
	void RegisterAllInstancesInFakeRotuer();
	///test case
	void SendMessageToChatCase();

	vs::map<
		chat::CallID,
		std::shared_ptr<struct ParticipantContext>,
		vs::less<>> participants_ctx_;
	std::shared_ptr<ResolverStub> resolver_ = std::make_shared<ResolverStub>();
	chat::ChatID	current_chat_id_;
	std::shared_ptr<ChatMessageFakeRouter> fake_router_
		= vs::MakeShared<ChatMessageFakeRouter>();
	chat::ClockWrapperPtr	clock_wrapper_;
	vs::ASIOThreadPool atp_;
	// part name -> uri
	std::map<std::string, std::string> db_uri_;
};

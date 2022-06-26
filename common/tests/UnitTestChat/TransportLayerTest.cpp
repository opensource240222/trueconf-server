#include "tests/common/TestHelpers.h"
#include "tests/common/GMockOverride.h"
#include "tests/UnitTestChat/ChatTestFixture.h"
#include "tests/UnitTestChat/globvars.h"

#include "chatlib/factory_asio/make_layers_asio.h"
#include "chatlib/helpers/AccountInfo.h"
#include "chatlib/interface/TransportChannel.h"
#include "chatlib/msg/attr.h"
#include "chatlib/msg/ChatMessage.h"

#include "chatutils/ExternalComponentsStub.h"
#include "chatutils/GlobalConfigStub.h"

#include "std/cpplib/ASIOThreadPool.h"
#include "std/cpplib/event.h"
#include "std-generic/compat/memory.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

/**
	1.	Send message to server_id;
	2.
*/
using ::testing::_;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::InvokeWithoutArgs;
using ::testing::UnorderedElementsAreArray;

class TransportChannel_Mock
{
public:
	MOCK_METHOD2(Send, void(chat::msg::ChatMessagePtr& m, std::vector<chat::CallID>& endpoints));
};
class TransportChannel_Proxy : public chat::TransportChannel
{
	std::shared_ptr<TransportChannel_Mock> channel_;

private:
	void Send(chat::msg::ChatMessagePtr &&m, std::vector < chat::CallID > &&endpoints) override
	{
		channel_->Send(m, endpoints);
	}
public:
	explicit TransportChannel_Proxy(const std::shared_ptr<TransportChannel_Mock>& ch) : channel_(ch)
	{
	}

};

class TransportLayerTest : public testing::Test
{
public:
	TransportLayerTest()
		: atp_(1)
	{
		atp_.Start();
		global_cfg_->SetAccountInfo(std::make_shared<chat::AccountInfo>(
			"user@trueconf.com",
			"user@trueconf.com/1",
			vs::CallIDType::client,
			"bs.trueconf.com#bs",
			nullptr));
	}
	void SetUp() override
	{
		transport_layer_ =
			chat::asio_sync::MakeTransportLayer(
				global_cfg_,
				vs::make_unique<TransportChannel_Proxy>(transport_channel_),
				atp_.get_io_service());
	}
	void TearDown() override
	{
		atp_.Stop();
	}
protected:
	std::shared_ptr<ResolverStub> resolver_
		= std::make_shared<ResolverStub>();
	std::shared_ptr<GlobalConfigStub> global_cfg_
		= std::make_shared<GlobalConfigStub>(resolver_);
	std::shared_ptr<TransportChannel_Mock> transport_channel_ =
		std::make_shared<TransportChannel_Mock>();
	std::shared_ptr<chat::LayerInterface> transport_layer_;
	vs::ASIOThreadPool atp_;;

	std::unique_ptr<chat::msg::ChatMessage> MakeMsg() const
	{
		return vs::make_unique<chat::msg::ChatMessage>();
	}
};
TEST_F(TransportLayerTest,SendMsgToServer)
{
	/**
		server call id: bs.trueconf.com#bs, tcs.trueconf.name#vcs;
	*/
	const std::string bs = "bs.trueconf.com#bs";
	const std::string tcs = "tcs.trueconf.name#vcs";
	{
		auto ev = std::make_shared<vs::event>(false);
		auto msg = MakeMsg();
		msg->SetParam(chat::attr::DST_CALLID_paramName, bs);
		EXPECT_CALL(*transport_channel_, Send(_, ElementsAre(bs)))
			.Times(1)
			.WillOnce(InvokeWithoutArgs([ev]()
			{
				ev->set();
			}));
		transport_layer_->ForwardBelowMessage(std::move(msg), {});
		EXPECT_TRUE(test::WaitFor("Send message", *ev, g_max_wait_timeout_ms));
	}
	{
		auto ev = std::make_shared<vs::event>(false);
		auto msg = MakeMsg();
		msg->SetParam(chat::attr::DST_CALLID_paramName, tcs);
		EXPECT_CALL(*transport_channel_, Send(_, ElementsAre(tcs)))
			.Times(1)
			.WillOnce(InvokeWithoutArgs([ev]()
			{
				ev->set();
			}));
		transport_layer_->ForwardBelowMessage(std::move(msg), {});
		EXPECT_TRUE(test::WaitFor("Send message", *ev, g_max_wait_timeout_ms));
	}
}
TEST_F(TransportLayerTest, SendMsgToCallID)
{
	/**
		general id
			user1@server
		endpoints
			user@server/1
			user@server/2
			user@server/3
	*/
	chat::CallID user_id = "user@servername";
	std::vector<chat::CallID> endpoints{ "user@servername/1","user@servername/2","user@servername/3" };
	auto get_related_callid = [&endpoints, &user_id](chat::CallIDRef id) -> std::vector<chat::CallID>
	{
		if (id == user_id)
			return endpoints;
		return {};
	};
	auto info = std::make_shared<vs::AccountInfo>(user_id,
		endpoints.front(), vs::CallIDType::client,
		vs::BSInfo(ChatTestFixture::BASE_SERVERS.front()), get_related_callid);
	global_cfg_->SetAccountInfo(info);
	resolver_->Add(user_id, info);
	resolver_->AddAlias(user_id, user_id);
	for(const auto &i:endpoints)
		resolver_->AddAlias(user_id, i);
	{
		auto ev = std::make_shared<vs::event>(false);
		auto msg = MakeMsg();
		msg->SetParam(chat::attr::DST_CALLID_paramName, user_id);
		EXPECT_CALL(*transport_channel_,
			Send(_, UnorderedElementsAreArray(endpoints.begin() + 1,endpoints.end())))
			.Times(1)
			.WillOnce(InvokeWithoutArgs([ev]()
			{
				ev->set();
			}));
		transport_layer_->ForwardBelowMessage(std::move(msg), {});
		EXPECT_TRUE(test::WaitFor("Send message", *ev, g_max_wait_timeout_ms));
	}
	{
		auto ev = std::make_shared<vs::event>(false);
		auto msg = MakeMsg();
		msg->SetParam(chat::attr::DST_ENDPOINT_paramName, endpoints[1]);
		EXPECT_CALL(*transport_channel_, Send(_, ElementsAre(endpoints[1])))
			.Times(1)
			.WillOnce(InvokeWithoutArgs([ev]()
			{
				ev->set();
			}));
		transport_layer_->ForwardBelowMessage(std::move(msg), {});
		EXPECT_TRUE(test::WaitFor("Send message", *ev, g_max_wait_timeout_ms));
	}
}
#pragma once

#include "tests/common/ASIOEnvironment.h"
#include "acs_v2/Service.h"
#include "std/cpplib/event.h"
#include "std/cpplib/MakeShared.h"
#include "std-generic/cpplib/scope_exit.h"
#include "statuslib/status_types.h"
#include "ldap_core/common/Common.h"

#include "http/Router.h"
#include "newtransport/Router/Router.h"
#include "http/handlers/OnlineUsers.h"
#include "http/handlers/TorrentAnnounce.h"
#include "http/handlers/ServerConfigurator.h"
#include "http/handlers/RouterMonitor.h"
#include "http_v2/Handler_v2.h"
#include "raw_http.h"

#include "tests/mocks/transport/ServiceMock.h"
#include "tests/mocks/transport/EndpointMock.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/make_shared.hpp>

#include <atomic>

static const boost::asio::ip::address_v4 ADDR_FOR_TEST = net::address_v4::loopback();
static const unsigned short PORT_FOR_TEST = 46782;

class FakeUsersStatuses : public UsersStatusesInterface
{
public:
	bool UsersStatuses(UsersList& users) override
	{
		start_processing.wait();
		++processed_requests;
		for (auto &u : users)
			u.second = VS_UserPresence_Status::USER_STATUS_UNDEF;
		return true;
	}

	void ListOfOnlineUsers(UsersList& users) override
	{
		users.emplace_back("vasya", VS_UserPresence_Status::USER_AVAIL);
	}

	vs::event start_processing { true, true }; // Manual reset event, this is important. Initially signaled because waiting if only needed in DDoS tests.
	std::atomic<unsigned> processed_requests { 0 };
};

struct GetTorrentService : public vs::enable_shared_from_this<GetTorrentService>,
	public http::handlers::Interface,
	public http::handlers::GetTorrentServiceInterface
{
	virtual boost::optional<std::string> HandleRequest(string_view in, const net::address& /*from_ip*/, const net::port /*from_port*/) override
	{
		//ASSERT_STREQ(((std::string)in).c_str(), std::string(raw_torrent_announce).c_str()) << "";
		return { std::string(in) };
	}
	virtual std::shared_ptr<http::handlers::Interface> AsHandler() override
	{
		return shared_from_this();
	}

protected:
	GetTorrentService() {}
};

struct WebConfigSlot
{
	MOCK_METHOD2(Slot_SendConfiguratorCommand, bool(const char* /*buf*/, uint32_t /*buf_len*/));
};

struct HttpHandlerTest : public testing::Test
{
	std::shared_ptr<acs::Service> acs_srv;
	std::shared_ptr<GetTorrentService> get_torrent_service = vs::MakeShared<GetTorrentService>();
	boost::shared_ptr<FakeUsersStatuses> fake_users_statuses = boost::make_shared<FakeUsersStatuses>();
	std::shared_ptr<http::handlers::OnlineUsers> online_users_handler = std::make_shared<http::handlers::OnlineUsers>(fake_users_statuses, std::string());
	WebConfigSlot web_config_slot;
	VS_PushDataSignalSlot slot = boost::bind(&WebConfigSlot::Slot_SendConfiguratorCommand, &web_config_slot, _1, _2);
	std::shared_ptr<http::handlers::ServerConfigurator> server_configurator = std::make_shared<http::handlers::ServerConfigurator>(slot, std::string());
	std::shared_ptr<transport::Router> router_transport= vs::MakeShared<transport::Router>(g_asio_environment->IOService(), "ub1lx.trueconf.name#vcs");
	std::shared_ptr<stream::RouterV2> stream_router = std::make_shared<stream::RouterV2>(g_asio_environment->IOService(), "ub1lx.trueconf.name#vcs");
	std::shared_ptr<http::handlers::RouterMonitor> transport_router_handler = std::make_shared< http::handlers::RouterMonitor>(router_transport, stream_router, acs_srv);
	std::shared_ptr<http::Router> router = std::make_shared<http::Router>();
	std::shared_ptr<http_v2::ACSHandler> handler = std::make_shared<http_v2::ACSHandler>(g_asio_environment->IOService());

	void SetUp() override
	{
		acs_srv = acs::Service::Create(g_asio_environment->IOService());
		auto start_f = acs_srv->Start();
		EXPECT_EQ(std::future_status::ready, start_f.wait_for(std::chrono::seconds(1))) << "acs::Service::Start took too long";
		if (start_f.valid())
			EXPECT_TRUE(start_f.get()) << "acs::Service::Start failed";

		router->AddHandler("GET /rm/tr", transport_router_handler);
		router->AddHandler("GET /rm/acs", transport_router_handler);
		router->AddHandler("GET /rm/sr", transport_router_handler);
		router->AddHandler("GET /s4/", online_users_handler);
		router->AddHandler("GET /s2/", online_users_handler);
		router->AddHandler("GET /announce", std::make_shared<http::handlers::TorrentAnnounce>(get_torrent_service));
		router->AddHandler("POST /", server_configurator);
		handler->SetHttpRouter(router);

		acs_srv->AddHandler("HttpHandlerV2", handler);
		EXPECT_TRUE(!acs_srv->AddListener(ADDR_FOR_TEST, PORT_FOR_TEST, net::protocol::TCP));
	}
	void TearDown() override
	{
		VS_SCOPE_EXIT
		{
			if (acs_srv)
			EXPECT_EQ(std::future_status::ready, acs_srv->Stop().wait_for(std::chrono::seconds(1))) << "acs::Service::Stop took too long";
		};

	}
	std::string connect_send_read(string_view in);
	std::string get_str_to_hash(string_view sv, const int N);
	void prepare_ddos_request(std::string& out);
};


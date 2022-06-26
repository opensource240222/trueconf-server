#include "std/cpplib/MakeShared.h"
#ifdef _WIN32
#include <gtest/gtest.h>
#include "std-generic/cpplib/VS_Container.h"
#include "std-generic/cpplib/VS_Container_io.h"
#include "../ServerServices/VS_SubscriptionHub.h"
#include "../AppServer/Services/VS_PresenceService.h"
#include "ServerServices/VS_ConfRestrictInterface.h"
#include "transport/Router/VS_TransportRouterServiceTypes.h"
#include "transport/Router/VS_RouterMessage_io.h"

#include "TrueGateway/VS_SignalConnectionsMgr.h"
#include "TrueGateway/sip/VS_SIPParser.h"
#include "TrueGateway/CallConfig/VS_IndentifierSIP.h"
#include "TrueGateway/CallConfig/VS_CallConfigStorage.h"
#include "std/cpplib/VS_Policy.h"
#include "std/cpplib/ThreadUtils.h"
#include "tests/UnitTestCommon/TransceiversPoolFake.h"

#include <boost/asio/io_service.hpp>
#include <boost/make_shared.hpp>

#include <atomic>
#include <thread>

class VS_TransportRouterService;
struct VS_TransportRouter_CallService;

namespace
{
	bool g_loop_detected = false;		// in case of fail, there will be an infinite loop with stack overflow; this flag is used break the loop and then fail the test

	class ConfRestrictInterface_fake : public VS_ConfRestrictInterface
	{
	public:
		virtual long CheckInsertMultiConf(long tarif_opt, VS_ConferenceDescription &cd, const char *host, bool FromBS = false) { return 0; }
		virtual void VCS_SetUserStatus(const VS_SimpleStr& call_id, int status, const VS_ExtendedStatusStorage &extStatus, bool set_server = false, const VS_SimpleStr& server = VS_SimpleStr()){}
		bool FindUser(const vs_user_id&, VS_StorageUserData&) const override { return false; }
		virtual int FindMultiConference(const char* name, VS_ConferenceDescription& conf, VS_Container& cnt, const vs_user_id& from_user, bool FromBS = false) { return 0; }
		virtual bool UpdateMultiConference(VS_ConferenceDescription &cd, bool curr) { return false; }
		virtual bool UpdateConference_RTSPAnnounce(VS_ConferenceDescription& cd, string_view announce_id) { return false; }
		virtual unsigned UpdateConfDuration(VS_ConferenceDescription& conf) { return 0; }
		virtual bool GetSSL(VS_SimpleStr& key) { return false; }
		virtual VS_SimpleStr GetAnyBSbyDomain(string_view call_id) { return ""; }
		virtual bool Tarif_CreateConf(VS_Container& cnt, VS_ConferenceDescription& cd, VS_UserData& ud, VS_TransportRouterServiceReplyHelper* caller = 0) { return false; }
		virtual bool OnJoinConf(VS_Container& cnt, VS_ConferenceDescription& cd, vs_user_id user_id, bool FromBS = false, VS_TransportRouterServiceReplyHelper* caller = 0) { return false; }
		virtual void OnRemovePartEvent(const VS_ParticipantDescription& pd, VS_ConferenceDescription& cd, VS_TransportRouterServiceReplyHelper* caller) { }
		virtual bool CheckInviteMulti_Roaming(vs_user_id user_id) { return false; }
		virtual void GetFirstBS(const char* dst_user, const char* our_endpoint, VS_SimpleStr &server) {}
		virtual void SetOfflineChatMessage(VS_Container &cnt) {}
		virtual void DeleteOfflineChatMessage(VS_Container& cnt) {}
		virtual void GetRoamingOfflineMessages(const char* our_sid, VS_ChatMsgs& v) {}
		virtual bool DoWriteConference(const VS_ConferenceDescription& cd) { return false; }
		virtual bool DoBroadcastConference(const VS_ConferenceDescription& cd) { return false; }
		virtual bool LogSlideShow(VS_Container &cnt) { return false; }
		virtual bool LogGroupChat(VS_Container &cnt) { return false; }
		virtual bool IsVCS() const { return false; }
		virtual VS_SimpleStr GetLocalMultiConfID(const char* conf_id) const { return ""; }
		virtual boost::signals2::connection Connect_AliasesChanged(const AliasesChangedSlot &slot) { return boost::signals2::connection(); }
		virtual void UpdateAliasList() {}
		virtual void SetRoamingSettings(const eRoamingMode_t mode, const std::string& params) {}
		virtual vs_conf_id NewConfID(VS_SimpleStr) { return vs_conf_id(); };
	};

	class VS_PresenceService_fake : public VS_PresenceService
	{
	public:
		VS_PresenceService_fake(boost::asio::io_service& ios, VS_TransportRouterService_Implementation* post_mes)
			: VS_PresenceService(ios)
		{
			VS_TransportRouterServiceBase::imp.reset(post_mes);
		}
	};

	std::map<std::string, boost::shared_ptr<VS_PresenceService>> m_servers;

	class VS_TransportRouterService_Implementation_fake : public VS_TransportRouterService_Implementation
	{
		std::function<bool(VS_RouterMessage*)>	m_fail_checker;
	public:
		VS_TransportRouterService_Implementation_fake(const char *endpointName, const char *serviceName, const std::function<bool(VS_RouterMessage*)> fail_checker) :
			VS_TransportRouterService_Implementation(/*VS_TransportRouterService *trs*/nullptr,
			/*endpointName*/nullptr, /*serviceName*/nullptr,
			/*VS_TransportRouter_CallService *tr*/nullptr),
			m_fail_checker(fail_checker)
		{
			strcpy(VS_TransportRouterServiceBase_Implementation::endpointName, endpointName);
			strcpy(VS_TransportRouterServiceBase_Implementation::serviceName, serviceName);
		}
		virtual inline bool PostMes(VS_RouterMessage *mes)
		{
			if (mes)
			{
				std::cout << *mes << "\n";

				if (!m_fail_checker(mes))
				{
					g_loop_detected = true;
					return false;
				}

				if (mes->DstServer() && *mes->DstServer())
				{
					auto it = m_servers.find(mes->DstServer());
					if (it != m_servers.end())
						m_servers[mes->DstServer()]->Processing(std::unique_ptr<VS_RouterMessage>(mes));
					else
						std::cout << "dst server " << mes->DstServer() << " not found\n";
				}
			}
			return true;
		}
	};

	class SubHubTest : public ::testing::Test {
	public:
		virtual void SetUp() {
		}
		virtual void TearDown() {
		}

		boost::asio::io_service m_ios;
	};

	TEST_F(SubHubTest, LoveTriangle)
	{
		auto check_hops_in_cnt_for_servers = [](VS_RouterMessage* msg) -> bool {
			if (strcasecmp(msg->AddString(), "CallFunction") == 0)
				return true;
			VS_Container cnt;
			if (!cnt.Deserialize(msg->Body(), msg->BodySize()))
				return false;
			string_view src_user = msg->SrcUser();
			string_view dst_user = msg->DstUser();
			auto method = cnt.GetStrValueRef();
			if (!!method && src_user.empty() && dst_user.empty() && cnt.GetLongValueRef(HOPS_PARAM) == nullptr)		// only to/from user is allowed to send without hops
				return false;
			return true;
		};
		auto check_loop_back_to_server1 = [](VS_RouterMessage* msg) -> bool
		{
			string_view dst = msg->DstServer();
			return (dst == "server1") ? false : true;
		};

		VS_TransportRouterService_Implementation_fake* post_mes_imp1 = new VS_TransportRouterService_Implementation_fake("server1", PRESENCE_SRV, check_hops_in_cnt_for_servers);
		VS_TransportRouterService_Implementation_fake* post_mes_imp2 = new VS_TransportRouterService_Implementation_fake("server2", PRESENCE_SRV, check_hops_in_cnt_for_servers);
		VS_TransportRouterService_Implementation_fake* post_mes_imp3 = new VS_TransportRouterService_Implementation_fake("server3", PRESENCE_SRV, check_loop_back_to_server1);

		m_servers["server1"] = boost::make_shared<VS_PresenceService_fake>(m_ios, post_mes_imp1);
		m_servers["server2"] = boost::make_shared<VS_PresenceService_fake>(m_ios, post_mes_imp2);
		m_servers["server3"] = boost::make_shared<VS_PresenceService_fake>(m_ios, post_mes_imp3);

		auto confResctrict = boost::make_shared<ConfRestrictInterface_fake>();
		for (auto& srv : m_servers)
		{
			srv.second->SetConfRestrict(confResctrict);
			srv.second->Init(srv.first.c_str(), PRESENCE_SRV);
		}

		VS_SimpleStr vasya = "vasya";
		VS_FullID sub1("server1", nullptr);
		VS_FullID sub2("server2", nullptr);
		VS_FullID sub3("server3", nullptr);

		// user vasya become avail
		{
			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, UPDATESTATUS_METHOD);
			cnt.AddValueI32(USERPRESSTATUS_PARAM, USER_AVAIL);
			cnt.AddValueI32(MULTI_LOGIN_CAPABILITY_PARAM, VS_MultiLoginCapability::SINGLE_USER);
			void* body;
			size_t bodySize;
			ASSERT_TRUE(cnt.SerializeAlloc(body, bodySize));

			auto r_msg = std::make_unique<VS_RouterMessage>(
				m_servers["server1"]->OurService(), nullptr, m_servers["server1"]->OurService(),	// service
				nullptr, vasya,						// user
				m_servers["server1"]->OurEndpoint(), m_servers["server1"]->OurEndpoint(),			// server
				30000, (const char*)body, bodySize);

			m_servers["server1"]->Processing(std::move(r_msg));		// infinite loop of love triangle should be here
			ASSERT_FALSE(g_loop_detected);
		}

		// love triangle
		m_servers["server1"]->Subscribe(vasya, sub2);		ASSERT_FALSE(g_loop_detected);
		m_servers["server2"]->Subscribe(vasya, sub3);		ASSERT_FALSE(g_loop_detected);
		m_servers["server3"]->Subscribe(vasya, sub1);		ASSERT_FALSE(g_loop_detected);

		// test PushStatusDirectly()
		{
			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, PUSHSTATUSDIRECTLY_METHOD);
			cnt.AddValue(CALLID_PARAM, vasya);
			cnt.AddValueI32(USERPRESSTATUS_PARAM, USER_AVAIL);
			cnt.AddValue(SERVER_PARAM, m_servers["server1"]->OurEndpoint());
			cnt.AddValueI32(MULTI_LOGIN_CAPABILITY_PARAM, VS_MultiLoginCapability::SINGLE_USER);
			void* body;
			size_t bodySize;
			ASSERT_TRUE(cnt.SerializeAlloc(body, bodySize));

			auto r_msg = std::make_unique<VS_RouterMessage>(
				m_servers["server1"]->OurService(), nullptr, m_servers["server1"]->OurService(),	// service
				nullptr, nullptr,						// user
				m_servers["server1"]->OurEndpoint(), m_servers["server1"]->OurEndpoint(),			// server
				30000, (const char*)body, bodySize);

			m_servers["server1"]->Processing(std::move(r_msg));		// infinite loop of love triangle should be here
			ASSERT_FALSE(g_loop_detected);
		}

		// test ServerOffline
		// todo(kt): not working, because @GetServer does not return the server
		//{
		//	VS_Container cnt;
		//	cnt.AddValue(METHOD_PARAM, POINTDISCONNECTED_METHOD);
		//	cnt.AddValue(NAME_PARAM, "server3");
		//	unsigned long bodySize;	void *body;
		//	ASSERT_TRUE(cnt.SerializeAlloc(body, bodySize));

		//	VS_RouterMessage* r_msg = new VS_RouterMessage(
		//		m_servers["server1"]->OurService(), nullptr, m_servers["server1"]->OurService(),	// service
		//		nullptr, nullptr,						// user
		//		m_servers["server1"]->OurEndpoint(), m_servers["server1"]->OurEndpoint(),			// server
		//		30000, (const char*)body, bodySize);

		//	m_servers["server1"]->Processing(r_msg);		// infinite loop of love triangle should be here
		//	ASSERT_FALSE(g_loop_detected);
		//}

		VS_UserData ud;
		ud.m_name = vasya;

		// test OnUserLoginEnd_Event()
		m_servers["server1"]->OnUserLoginEnd_Event(ud, std::string());
		ASSERT_FALSE(g_loop_detected);

		// test OnUserLogoff_Event()
		m_servers["server1"]->OnUserLogoff_Event(ud, std::string());
		ASSERT_FALSE(g_loop_detected);

	}

	TEST_F(SubHubTest, SipMsgWaitDeadlock)
	{
		const char* server_name = "server1";
		VS_TransportRouterService_Implementation_fake* post_mes_imp1 = new VS_TransportRouterService_Implementation_fake(server_name, PRESENCE_SRV, [](VS_RouterMessage* msg) -> bool { return true; });
		auto presence = boost::make_shared<VS_PresenceService_fake>(m_ios, post_mes_imp1);

		presence->SetConfRestrict(boost::make_shared<ConfRestrictInterface_fake>());
		presence->Init(server_name, PRESENCE_SRV);

		boost::asio::io_service ios;
		std::atomic<bool> should_run(true);
		std::thread t([&]() {
			vs::SetThreadName("T:SipConnMgr");
			while (should_run.load(std::memory_order_acquire))
				if (ios.poll_one() == 0)
					std::this_thread::yield();
		});

		auto storage = vs::MakeShared<VS_CallConfigStorage>();

		auto ident = boost::make_shared<VS_IndentifierSIP>(m_ios, "serverVendor");
		storage->RegisterProtocol(ident);

		std::function<bool(const std::string&, const std::string&)> check_digest;
		std::shared_ptr<test::TransceiversPoolFake> fakePool = std::make_shared<test::TransceiversPoolFake>();
		boost::asio::io_service::strand strand(ios);
		VS_SignalConnectionsMgr::InitInfo info(strand);
		info.checkDigest = check_digest;
		info.getUserStatus = [](string_view /*call_id*/, bool /*use_cache*/, bool /*do_ext_resolve*/) -> UserStatusInfo { return UserStatusInfo(); }, // get_user_status;
		info.ourEndpoint = "";
		info.ourService = "";
		info.postMes = [](VS_RouterMessage*) { return true; };
		info.peerConfig = storage;
		info.transcPool = fakePool;
		auto sipProtocol = vs::MakeShared<VS_SignalConnectionsMgr>(std::move(info));

		VS_ResolveServerFinder	*resolve_srv = VS_ResolveServerFinder::Instance();
		ASSERT_TRUE(resolve_srv != nullptr);
		resolve_srv->RegisterExternalPresence(sipProtocol);

		VS_SimpleStr vasya = "vasya";
		presence->Subscribe("#sip:qwe", VS_FullID(server_name, vasya));

		// heart of test: emulate that thread is blocked by stopping it
		should_run.store(false, std::memory_order_release);
		t.join();

		VS_UserData ud;
		ud.m_name = vasya;
		presence->OnUserLogoff_Event(ud, "cid1");

		// If we have reached this point then there is no deadlock and we need to stop everything:
		// Remove external pointers to sipProtocol
		resolve_srv->UnRegisterExternalPresence(sipProtocol);
		// Execute tasks posted to io_service until VS_SignalConnectionsMgr is destroyed,
		// or until 2 seconds have passed (in this case the process will likely crash, but it is better than a deadlock).
		std::weak_ptr<VS_SignalConnectionsMgr> w = sipProtocol;
		sipProtocol = nullptr;
		const auto wait_start_time = std::chrono::steady_clock::now();
		while (!w.expired() && std::chrono::steady_clock::now() - wait_start_time < std::chrono::seconds(2))
			if (ios.poll_one() == 0)
				std::this_thread::yield();
		ASSERT_TRUE(w.expired()) << "VS_SignalConnectionsMgr wasn't deleted in time, process is expected to crash later.";
	}
}

#endif
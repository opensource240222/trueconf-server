#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/make_shared.hpp>

#include "AppServer/Services/VS_AppServerData.h"
#include "AppServer/Services/VS_MultiConfService.h"
#include "dummies/ConfRestrictEmpty.h"
#include "std-generic/compat/memory.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/VS_ClientCaps.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "streams/Router/Buffer.h"
#include "tests/common/ASIOEnvironment.h"
#include "tests/common/GMockOverride.h"
#include "tests/UnitTestCommon/TransceiversPoolFake.h"
#include "tests/UnitTestStreams/RouterMock.h"
#include "transport/Router/VS_TransportRouterServiceBase.h"

namespace tc3_test
{
const std::string FROM_USER = "from_user";

using streams_test::RouterMock;

struct ConfRestrictMock final : public ConfRestrictEmpty {
	ConfRestrictMock(bool isVCS)
		: ConfRestrictEmpty(isVCS)
	{}
	MOCK_METHOD4_OVERRIDE(CheckInsertMultiConf, long(long tarif_opt, VS_ConferenceDescription &cd, const char *host, bool FromBS));
	MOCK_METHOD4_OVERRIDE(Tarif_CreateConf, bool(VS_Container& cnt, VS_ConferenceDescription& cd, VS_UserData& ud, VS_TransportRouterServiceReplyHelper* caller));
	MOCK_CONST_METHOD0_OVERRIDE(IsVCS, bool());
	MOCK_METHOD1_OVERRIDE(UpdateConfDuration, unsigned(VS_ConferenceDescription& conf));
	MOCK_METHOD5_OVERRIDE(FindMultiConference, int(const char* name, VS_ConferenceDescription& conf, VS_Container& cnt, const vs_user_id& from_user, bool FromBS));
	MOCK_METHOD5_OVERRIDE(OnJoinConf, bool(VS_Container& cnt, VS_ConferenceDescription& cd, vs_user_id user_id, bool FromBS, VS_TransportRouterServiceReplyHelper* caller));
};

class PostMesToContainer : public VS_TransportRouterServiceBase::Impl
{
public:
	PostMesToContainer() = default;
	std::function<bool(const VS_Container&)>	out_cnt_handler;

	virtual inline bool PostMes(VS_RouterMessage *mes)
	{
		VS_SCOPE_EXIT { delete mes; };
		// todo(kt): mes to cnt
		VS_Container cnt;
		cnt.Deserialize(mes->Body(), mes->BodySize());
		if (out_cnt_handler)
			out_cnt_handler(cnt);
		return true;
	}

	// VS_TransportRouterServiceBase::Impl
	virtual bool SendMes(VS_RouterMessage* mes) { return false; }
	virtual bool RequestResponse(VS_RouterMessage* mes, ResponseCallBackT&& cb, RequestLifeTimeT&& life_time) { return false; }
	virtual ResponseFutureT RequestResponse(VS_RouterMessage* mes, RequestLifeTimeT&& life_time) { return ResponseFutureT(); }
	virtual const char* OurEndpoint() const { return "PostMesToContainer"; }
	virtual const char* OurService() const { return "PostMesToContainer"; }
	virtual bool GetStatistics(struct VS_TransportRouterStatistics* stat) { return false; }
	virtual bool IsThereEndpoint(const char* endpoint) { return false; }
	virtual void DisconnectEndpoint(const char* endpoint) {}
	virtual void FullDisconnectEndpoint(const char* endpoint) {}
	virtual void FullDisconnectAllEndpoints() {}
	virtual bool AuthorizeClient(const char* uid, const char* new_uid) { return false; }
	virtual bool UnauthorizeClient(const char* uid) { return false; }
	virtual bool IsAuthorized(const char* uid) { return false; }
	virtual std::string GetCIDByUID(const char* uid) { return {}; }
	virtual bool GetIPByCID(const char* cid, std::string& ip) { return false; }
	virtual bool PutTask(VS_PoolThreadsTask* task, const char* nameExtension, unsigned lifetimeSec, unsigned priority)
	{
		delete task;
		return false;
	}

	// VS_TransportRouterService::Impl
	virtual bool SetThread(VS_TransportRouterServiceBase::Impl* instance) { return false; }
	virtual void ResetThread() {}
	virtual int ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, bool allAdopted) { return 0; }
	virtual int ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, std::chrono::steady_clock::duration& wait_time, bool allAdopted) { return 0; }
};

class VS_MultiConfServiceFake: public VS_MultiConfService
{
	std::shared_ptr<test::TransceiversPoolFake> fake_pool;
public:
	VS_MultiConfServiceFake(): fake_pool(std::make_shared<test::TransceiversPoolFake>()){
		m_transceiversPool = fake_pool;
		SetIOservice(g_asio_environment->IOService());
	}

};

VS_UserData CreateUser(const string_view name) {
	VS_UserData ud;
	ud.m_name = std::string(name).c_str();
	ud.m_homeServer = g_tr_endpoint_name.c_str();
	ud.m_rights |= VS_UserData::UR_COMM_CREATEMULTI | VS_UserData::UR_COMM_CALL;
	return ud;
}

class MultiConfServiceTest : public ::testing::Test {
public:
	MultiConfServiceTest() {
		VS_RegistryKey	c_root(false, "Conferences", false, true);
		c_root.SetString("0", "Last Conference Name");
	}
	VS_MultiConfServiceFake mconf_srv;
	std::vector<VS_Container> out_cnts;
	boost::shared_ptr<VS_ConfRestrictInterface> conf_restrict_mock = boost::make_shared<ConfRestrictMock>(false);
	boost::shared_ptr<RouterMock> streams_router_mock = boost::make_shared<RouterMock>();

	virtual void SetUp() {
		g_storage = new VS_Storage(g_tr_endpoint_name.c_str());
		g_storage->SetConfRestrict(boost::make_shared<ConfRestrictMock>(false));
		g_storage->Init();
		AddTestUserToStorage();
		g_appServer = new VS_AppServerData;
		auto srv = vs::make_unique<PostMesToContainer>();
		srv->out_cnt_handler = [this](const VS_Container& cnt)->bool {
			out_cnts.emplace_back(std::move(cnt));
			return true;
		};
		mconf_srv.VS_TransportRouterServiceBase::imp = std::move(srv);
		mconf_srv.SetStreamRouter(streams_router_mock.get());
		mconf_srv.SetConfRestrict(conf_restrict_mock);
	}
	virtual void TearDown() {
		((VS_TransportRouterServiceBase*)&mconf_srv)->imp = nullptr;
		out_cnts.clear();

		delete g_appServer;
		g_appServer = nullptr;

		delete g_storage;
		g_storage = nullptr;
	}
	void Processing(const VS_Container& cnt)
	{
		size_t body_sz = 0;
		cnt.Serialize(nullptr, body_sz);
		ASSERT_NE(body_sz, 0);
		auto body = vs::make_unique<uint8_t[]>(body_sz);
		ASSERT_TRUE(cnt.Serialize(body.get(), body_sz));
		auto recvMess = vs::make_unique<VS_RouterMessage>(MULTICONF_SRV, nullptr, MULTICONF_SRV, "to_user", FROM_USER.c_str(), g_tr_endpoint_name.c_str(), g_tr_endpoint_name.c_str(), ~0, body.get(), body_sz);
		mconf_srv.Processing(std::move(recvMess));
	}

	void Set_MNow() {
		using namespace std::chrono;
		mconf_srv.m_now = steady_clock::now() + milliseconds(30001);
	}

	void AddTestUserToStorage() {
		g_storage->UpdateUser(FROM_USER, CreateUser(FROM_USER));
	}
};

void CapsToContainer(VS_Container &cnt, const VS_ClientCaps &caps)
{
	void* body(nullptr);
	size_t bodySize(0);
	((VS_ClientCaps *)&caps)->Get(body, bodySize);
	cnt.AddValue(CLIENTCAPS_PARAM, body, bodySize);
	VS_SCOPE_EXIT{ free(body); };
}

VS_Container CreateConference(const string_view name, const string_view topic, VS_Conference_Type confType, int32_t subType, int32_t maxPart, bool is_public) {
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, CREATECONFERENCE_METHOD);
	rCnt.AddValueI32(MAXPARTISIPANTS_PARAM, maxPart);
	rCnt.AddValueI32(DURATION_PARAM, 0);
	rCnt.AddValueI32(TYPE_PARAM, confType);
	rCnt.AddValueI32(SUBTYPE_PARAM, subType);
	rCnt.AddValueI32(SCOPE_PARAM, is_public ? GS_PUBLIC : GS_PERSONAL);
	rCnt.AddValue(NAME_PARAM, name);
	rCnt.AddValue(TOPIC_PARAM, topic);

	VS_ClientCaps dummy;
	CapsToContainer(rCnt,dummy);
	return rCnt;
}

VS_Container JoinToConference(const string_view name, const string_view topic, bool is_public) {
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, JOIN_METHOD);
	rCnt.AddValueI32(TYPE_PARAM, CT_MULTISTREAM);
	rCnt.AddValue(PASSWORD_PARAM, "");
	rCnt.AddValue(TOPIC_PARAM, topic);
	rCnt.AddValueI32(SCOPE_PARAM, is_public ? GS_PUBLIC : GS_PERSONAL);
	rCnt.AddValue(NAME_PARAM, name);
	VS_ClientCaps dummy;
	CapsToContainer(rCnt, dummy);
	return rCnt;
}

VS_Container CreateMultyConference(const string_view name, const string_view topic, int32_t subType, int32_t maxPart, bool is_public) {
	return CreateConference(name, topic, CT_MULTISTREAM, subType, maxPart, is_public);
}

VS_Container CreateP2PConference(const string_view name, const string_view topic) {
	return CreateConference(name, topic, CT_PRIVATE, GCST_FULL, 2, false);
}

VS_Container CreatePublicConference(const string_view name, const string_view topic) {
	return CreateConference(name, topic, CT_PUBLIC, GCST_FULL, 2, false);
}

TEST_F(MultiConfServiceTest, CreateMultyConference) {
	using ::testing::_;
	using ::testing::Return;

	ON_CALL(*boost::static_pointer_cast<ConfRestrictMock>(conf_restrict_mock), Tarif_CreateConf(_,_,_,_)).WillByDefault(Return(true));
	ON_CALL(*boost::static_pointer_cast<ConfRestrictMock>(conf_restrict_mock), IsVCS()).WillByDefault(Return(true));
	ON_CALL(*streams_router_mock, CreateConference(_, _, _, _, _, _)).WillByDefault(Return(true));

	Processing(CreateMultyConference("test_name", "topic", 0, 10, false));
	ASSERT_GT(out_cnts.size(), static_cast<size_t>(0));
	int32_t result; out_cnts.back().GetValueI32(RESULT_PARAM, result);
	EXPECT_EQ(result, CONFERENCE_CREATED_OK);
}

TEST_F(MultiConfServiceTest, CanNotCreateMultyConferenceByTarifRestrictions) {
	using ::testing::_;
	using ::testing::Return;

	ON_CALL(*boost::static_pointer_cast<ConfRestrictMock>(conf_restrict_mock), Tarif_CreateConf(_, _, _, _)).WillByDefault(Return(false));

	Processing(CreateMultyConference("test_name", "topic", 0, 10, false));
	ASSERT_GT(out_cnts.size(), static_cast<size_t>(0));
	int32_t result; out_cnts[0].GetValueI32(RESULT_PARAM, result);
	EXPECT_EQ(result, CREATE_ACCESS_DENIED);
}

TEST_F(MultiConfServiceTest, CreateP2pConference) {
	Processing(CreateP2PConference("test_name", "topic"));
	ASSERT_GT(out_cnts.size(), static_cast<size_t>(0));
	int32_t result; out_cnts[0].GetValueI32(RESULT_PARAM, result);
	EXPECT_EQ(result, CONFERENCE_CREATED_OK);
}

TEST_F(MultiConfServiceTest, HaveNoRightsToCreateConference) {
	auto ud = CreateUser(FROM_USER);
	ud.m_rights &= ~VS_UserData::UR_COMM_CALL;
	g_storage->UpdateUser(FROM_USER, std::move(ud));

	Processing(CreateP2PConference("test_name", "topic"));
	ASSERT_GT(out_cnts.size(), static_cast<size_t>(0));
	int32_t result; out_cnts[0].GetValueI32(RESULT_PARAM, result);
	EXPECT_EQ(result, CREATE_ACCESS_DENIED);
}

TEST_F(MultiConfServiceTest, CreatePublicConference) {
	using ::testing::_;
	using ::testing::Invoke;
	using ::testing::Return;
	using ::testing::WithArg;

	ON_CALL(*streams_router_mock, CreateConference(_, _, _, _, _, _)).WillByDefault(Return(true));
	ON_CALL(*streams_router_mock, AddParticipant(_, _, _, _, _)).WillByDefault(WithArg<2>(Invoke([](stream::Buffer* buffer) {
		delete buffer;
		return true;
	})));
	EXPECT_CALL(*streams_router_mock, SetParticipantCaps(_, _, _, _));
	EXPECT_CALL(*boost::static_pointer_cast<ConfRestrictMock>(conf_restrict_mock), UpdateConfDuration(_));

	Processing(CreatePublicConference("test_name", "topic"));
	ASSERT_GT(out_cnts.size(), static_cast<size_t>(0));
	int32_t result; out_cnts[0].GetValueI32(RESULT_PARAM, result);
	EXPECT_EQ(result, CONFERENCE_CREATED_OK);
}

TEST_F(MultiConfServiceTest, HaveNoResoursesToCreateConference) {
	using ::testing::_;
	using ::testing::Return;

	ON_CALL(*streams_router_mock, CreateConference(_, _, _, _, _, _)).WillByDefault(Return(false));
	Processing(CreatePublicConference("test_name", "topic"));
	ASSERT_GT(out_cnts.size(), static_cast<size_t>(0));
	int32_t result; out_cnts[0].GetValueI32(RESULT_PARAM, result);
	EXPECT_EQ(result, NO_ENOUGH_RESOURCES_FOR_CONFERENCE);
}

TEST_F(MultiConfServiceTest, HaveNoResoursesToAddParticipant) {
	using ::testing::_;
	using ::testing::Return;

	ON_CALL(*streams_router_mock, CreateConference(_, _, _, _, _, _)).WillByDefault(Return(true));
	ON_CALL(*streams_router_mock, AddParticipant(_, _, _, _, _)).WillByDefault(Return(false));
	Processing(CreatePublicConference("test_name", "topic"));
	ASSERT_GT(out_cnts.size(), static_cast<size_t>(0));
	int32_t result; out_cnts[0].GetValueI32(RESULT_PARAM, result);
	EXPECT_EQ(result, NO_ENOUGH_RESOURCES_FOR_CONFERENCE);
}

void MakeSpecialConfName(vs_conf_id &OUT_name) {
	OUT_name = SPECIAL_CONF_PREFIX;
	OUT_name += std::to_string(CT_MULTISTREAM).c_str();
	OUT_name += std::to_string(GCST_ROLE).c_str();
}

TEST_F(MultiConfServiceTest, JoinToConference) {
	using ::testing::_;
	using ::testing::Return;

	enum join_type{to_stream_conf, to_CID, to_special_conf};
	std::vector<join_type> jt{ to_stream_conf , to_CID , to_special_conf };

	ON_CALL(*boost::static_pointer_cast<ConfRestrictMock>(conf_restrict_mock), OnJoinConf(_, _, _, _, _)).WillByDefault(Return(true));


	for (const auto& join : jt)
	{
		TearDown();
		SetUp();
		VS_ConferenceDescription cd;
		cd.m_MaxParticipants = 100;
		switch (join)
		{
		case to_stream_conf:break;
		case to_CID: cd.m_name = GROUPCONF_PREFIX; cd.m_name += "123456"; break;
		case to_special_conf: MakeSpecialConfName(cd.m_name); break;
		default:
			assert(false);
			break;
		}

		g_storage->InsertConference(cd);
		Processing(JoinToConference(cd.m_name.m_str, "topic", false));
		auto join_cnt = std::find_if(out_cnts.begin(), out_cnts.end(), [](const VS_Container& cnt) {
			const auto method = cnt.GetStrValueRef(METHOD_PARAM);
			return method && strcmp(method, JOIN_METHOD) == 0;
		});
		ASSERT_NE(join_cnt, out_cnts.end());
		int32_t result; join_cnt->GetValueI32(RESULT_PARAM, result);
		EXPECT_EQ(result, JOIN_OK);
	}
}

TEST_F(MultiConfServiceTest, FECC_DenyByTimeout)
{
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, FECC_METHOD);
	cnt.AddValueI32(TYPE_PARAM, eFeccRequestType::REQUEST_ACCESS);
	cnt.AddValue(FROM_PARAM, "vasya");
	cnt.AddValue(TO_PARAM, "petya");
	Processing(cnt);

	{ // propagate reques to user
		ASSERT_GT(out_cnts.size(), static_cast<size_t>(0));
		string_view from = out_cnts.back().GetStrValueRef(FROM_PARAM);
		string_view to = out_cnts.back().GetStrValueRef(TO_PARAM);
		ASSERT_EQ(from, "vasya");
		ASSERT_EQ(to, "petya");
		int32_t type;
		out_cnts.back().GetValue(TYPE_PARAM, type);
		ASSERT_EQ(type, (int32_t)eFeccRequestType::REQUEST_ACCESS);
		out_cnts.back().Clear();
	}
	mconf_srv.Timer(0);
	ASSERT_TRUE(out_cnts.back().IsEmpty());
	Set_MNow();
	mconf_srv.Timer(30001);
	ASSERT_FALSE(out_cnts.back().IsEmpty());
	{
		string_view from = out_cnts.back().GetStrValueRef(FROM_PARAM);
		string_view to = out_cnts.back().GetStrValueRef(TO_PARAM);
		ASSERT_EQ(from, "petya");		// reverse order
		ASSERT_EQ(to, "vasya");
		int32_t type;
		out_cnts.back().GetValue(TYPE_PARAM, type);
		ASSERT_EQ(type, (int32_t)eFeccRequestType::DENY_BY_TIMEOUT_ACCESS);
	}
}

namespace
{
	inline void gen_random_str(char *s, const unsigned len)
	{
		static const char alphanum[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";

		for (unsigned i = 0; i < len; ++i) {
			s[i] = alphanum[::rand() % (sizeof(alphanum) - 1)];
		}
		s[len] = 0;
	}
}


TEST(AutoInviteService, ConcurrencyAccess_Bug50177)
{
	::srand(time(0));

	VS_MultiConfService multi_service;

	VS_AutoInviteService &auto_service = multi_service;

	std::vector<std::thread> threads;

	for (unsigned i = 0; i < std::thread::hardware_concurrency(); i++)
	{
		threads.emplace_back([&auto_service]()
		{
			vs::SetThreadName("AutoInviteTest1");

			char call_id[16];
			char server_id[16];
			const std::size_t count = 1000;

			for (std::size_t i = 0; i < count; i++)
			{
				gen_random_str(call_id, sizeof(call_id) - 1);
				gen_random_str(server_id, sizeof(server_id) - 1);

				auto_service.OnServerChange(call_id, server_id);

			}
		});
	}


	for (unsigned i = 0; i < std::thread::hardware_concurrency(); i++)
	{
		threads.emplace_back([&auto_service]()
		{
			vs::SetThreadName("AutoInviteTest2");

			char conf_name[16];
			char user_name[16];
			const std::size_t count = 1000;
			for (std::size_t i = 0; i < count; i++)
			{
				gen_random_str(conf_name, sizeof(conf_name) - 1);
				gen_random_str(user_name, sizeof(user_name) - 1);

				auto_service.Subscribe(conf_name, user_name);

				auto_service.SetSupport(conf_name, user_name, true);
			}
		});
	}

	for (auto &thread : threads)
	{
		thread.join();
	}
}
}

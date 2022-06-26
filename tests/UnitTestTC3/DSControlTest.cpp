#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/make_shared.hpp>

#include "AppServer/Services/VS_DSControlService.h"
#include "AppServer/Services/VS_Storage.h"
#include "dummies/ConfRestrictEmpty.h"
#include "std-generic/compat/memory.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/VS_ClientCaps.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "streams/Router/Buffer.h"
#include "tests/common/ASIOEnvironment.h"
#include "tests/common/GMockOverride.h"
#include "transport/Router/VS_TransportRouterServiceBase.h"

namespace tc3_test
{
	class FakeTRImpl : public VS_TransportRouterServiceBase::Impl
	{
	public:
		FakeTRImpl() = default;

		bool PostMes(VS_RouterMessage *mes) { delete mes; return true; }
		bool SendMes(VS_RouterMessage* mes) { return false; }
		bool RequestResponse(VS_RouterMessage* mes, ResponseCallBackT&& cb, RequestLifeTimeT&& life_time) { return false; }
		ResponseFutureT RequestResponse(VS_RouterMessage* mes, RequestLifeTimeT&& life_time) { return ResponseFutureT(); }
		const char* OurEndpoint() const { return g_tr_endpoint_name.c_str(); }
		const char* OurService() const { return DS_CONTROL_SRV; }
		bool GetStatistics(struct VS_TransportRouterStatistics* stat) { return false; }
		bool IsThereEndpoint(const char* endpoint) { return false; }
		void DisconnectEndpoint(const char* endpoint) {}
		void FullDisconnectEndpoint(const char* endpoint) {}
		void FullDisconnectAllEndpoints() {}
		bool AuthorizeClient(const char* uid, const char* new_uid) { return false; }
		bool UnauthorizeClient(const char* uid) { return false; }
		bool IsAuthorized(const char* uid) { return false; }
		std::string GetCIDByUID(const char* uid) { return {}; }
		bool GetIPByCID(const char* cid, std::string& ip) { return false; }
		bool PutTask(VS_PoolThreadsTask* task, const char* nameExtension, unsigned lifetimeSec, unsigned priority) {return true;}

		// VS_TransportRouterService::Impl
		virtual bool SetThread(VS_TransportRouterServiceBase::Impl* instance) { return false; }
		virtual void ResetThread() {}
		virtual int ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, bool allAdopted) { return 0; }
		virtual int ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, std::chrono::steady_clock::duration& wait_time, bool allAdopted) { return 0; }
	};

	const std::string FROM_USER = "from_user";
	const std::string TO_USER = "to_user";
	std::string CONF;


	VS_UserData CreateUserDS(const string_view name) {
		VS_UserData ud;
		ud.m_name = std::string(name).c_str();
		ud.m_homeServer = g_tr_endpoint_name.c_str();
		ud.m_rights |= VS_UserData::UR_COMM_CREATEMULTI | VS_UserData::UR_COMM_CALL | VS_UserData::UR_COMM_DSHARING | VS_UserData::UR_COMM_SHARE_CONTROL;
		return ud;
	}

	VS_ParticipantDescription CreatePart(const string_view name) {
		VS_ParticipantDescription pd;
		pd.m_user_id = std::string(name).c_str();
		pd.m_server_id = g_tr_endpoint_name.c_str();
		pd.m_conf_id = CONF.c_str();
		return pd;
	}


	class VS_DSControlServiceTest : public ::testing::Test {
	public:
		VS_DSControlServiceTest() {
		}
		VS_DSControlService dsc_srv;

		virtual void SetUp() {
			g_tr_endpoint_name = "unittest.com#vcs";
			CONF = "DS_test_conf@" + g_tr_endpoint_name;
			g_storage = new VS_Storage(g_tr_endpoint_name.c_str());
			g_storage->SetConfRestrict(boost::make_shared<ConfRestrictEmpty>(false));
			g_storage->Init();

			g_storage->UpdateUser(FROM_USER.c_str(), CreateUserDS(FROM_USER));
			g_storage->UpdateUser(TO_USER.c_str(), CreateUserDS(TO_USER));

			VS_ConferenceDescription cd;
			cd.SetTimeExp(1000);
			cd.m_state = cd.CONFERENCE_CREATED;
			cd.m_MaxParticipants = 2;
			cd.m_type = CT_PRIVATE;
			cd.m_owner = TO_USER.c_str();
			cd.m_name = CONF.c_str();
			int error_code = g_storage->InsertConference(cd);
			ASSERT_EQ(error_code, 0);

			VS_ParticipantDescription pd = CreatePart(TO_USER);
			error_code = g_storage->AddParticipant(pd);
			ASSERT_EQ(error_code, 0);

			pd = CreatePart(FROM_USER);
			error_code = g_storage->AddParticipant(pd);
			ASSERT_EQ(error_code, 0);

			auto srv = vs::make_unique<FakeTRImpl>();
			dsc_srv.VS_TransportRouterServiceBase::imp = std::move(srv);

		}
		virtual void TearDown() {
			delete g_storage;
			g_storage = nullptr;
		}
		void Processing(const VS_Container& cnt, const char* user)
		{
			size_t body_sz = 0;
			cnt.Serialize(nullptr, body_sz);
			ASSERT_NE(body_sz, 0);
			auto body = vs::make_unique<uint8_t[]>(body_sz);
			ASSERT_TRUE(cnt.Serialize(body.get(), body_sz));
			auto recvMess = vs::make_unique<VS_RouterMessage>(DS_CONTROL_SRV, "51", DS_CONTROL_SRV, nullptr, user, g_tr_endpoint_name.c_str(), g_tr_endpoint_name.c_str(), ~0, body.get(), body_sz);
			dsc_srv.Processing(std::move(recvMess));
		}

		void VideoSource(const char* u, bool use) {
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, VIDEOSOURCETYPE_METHOD);
			rCnt.AddValue(CONFERENCE_PARAM, CONF);
			rCnt.AddValue(USERNAME_PARAM, u);
			rCnt.AddValue(TYPE_PARAM, use ? VST_DESKTOP : VST_UNKNOWN);

			Processing(rCnt, u);
		};

		void ReqContr(const char* f, const char* t, bool use) {
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, DSCONTROL_REQUEST_METHOD);
			rCnt.AddValue(CONFERENCE_PARAM, CONF);
			rCnt.AddValue(TO_PARAM, t);
			rCnt.AddValue(FROM_PARAM, f);

			Processing(rCnt, f);
		};

		void RespContr(const char* f, const char* t, bool use) {
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, DSCONTROL_RESPONSE_METHOD);
			rCnt.AddValue(CONFERENCE_PARAM, CONF);
			rCnt.AddValue(TO_PARAM, t);
			rCnt.AddValue(FROM_PARAM, f);
			rCnt.AddValue(RESULT_PARAM, use ? DSCR_ALLOW : DSCR_ACCESS_DENIED);

			Processing(rCnt, t);
		};

		void FinContr(const char* f, const char* t, bool from) {
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, DSCONTROL_FINISH_METHOD);
			rCnt.AddValue(CONFERENCE_PARAM, CONF);
			rCnt.AddValue(TO_PARAM, t);
			rCnt.AddValue(FROM_PARAM, f);

			Processing(rCnt, from ? f : t);
		};

		void DelPart(const char* u) {
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, DELETEPARTICIPANT_METHOD);
			rCnt.AddValue(CONFERENCE_PARAM, CONF);
			rCnt.AddValue(USERNAME_PARAM, u);

			Processing(rCnt, nullptr);
		};

		void Command(const char* f, const char* t) {
			unsigned char cmd[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, DSCOMMAND_METHOD);
			rCnt.AddValue(TO_PARAM, t);
			rCnt.AddValue(DATA_PARAM, cmd, sizeof(cmd));

			Processing(rCnt, f);
		};

		bool IsC(const char* f, const char* t) {
			return dsc_srv.PairControlled(t, f) != nullptr;
		}
	};

	TEST_F(VS_DSControlServiceTest, CheckMethodsInDifferentOrder) {
		using ::testing::_;
		using ::testing::Return;

		const char *t = TO_USER.c_str();
		const char *f = FROM_USER.c_str();

		ASSERT_FALSE(IsC(f, t));

		VideoSource(t, true);
		ReqContr(f, t, true);
		RespContr(f, t, true);
		Command(f, t);
		ASSERT_TRUE(IsC(f, t));

		Command(t, f);
		ASSERT_FALSE(IsC(t, f));

		FinContr(f, t, true);
		ASSERT_FALSE(IsC(f, t));

		VideoSource(t, false);
		ReqContr(f, t, true);
		ASSERT_FALSE(IsC(f, t));

		VideoSource(t, true);
		ReqContr(f, t, true);
		RespContr(f, t, true);
		VideoSource(t, false);
		ASSERT_FALSE(IsC(f, t));

		VideoSource(t, true);
		ReqContr(f, t, true);
		RespContr(f, t, false);
		ASSERT_FALSE(IsC(f, t));

		VideoSource(t, true);
		ReqContr(f, t, true);
		RespContr(f, t, true);
		FinContr(f, t, false);
		ASSERT_FALSE(IsC(f, t));

		DelPart(f);
		DelPart(t);
	}
}

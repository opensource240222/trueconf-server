#ifdef _WIN32
#include "BaseServer/Services/storage/VS_DBStorage_TrueConf.h"
#include "std-generic/cpplib/macro_utils.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "tests/UnitTestTC3/status_to_bs/StatusTestFixture.h"
#include "tools/Server/VS_ApplicationInfo.h"

#include <limits>
#include <tuple>

namespace tc3_test
{

TEST(FakeRouter, simple_dispatch)
{
	uint16_t counter(0);
	std::vector<std::tuple<std::string, FakeRouter::ProcessingFuncT>> endpoits = {
		std::make_tuple("user1@server", [&counter](VS_RouterMessage *msg)
		{
			std::string dst = msg->DstUser();
			ASSERT_EQ(dst, "user1@server");
			counter++;
		}),

			std::make_tuple("user2@server", [&counter](VS_RouterMessage *msg)
		{
			std::string dst = msg->DstUser();
			ASSERT_EQ(dst, "user2@server");
			counter++;
		}),

			std::make_tuple("server", [&counter](VS_RouterMessage *msg)
		{
			std::string dst_srv = msg->DstServer();
			ASSERT_EQ(dst_srv, "server");
			counter++;
		})
	};

	FakeRouter router;
	for (auto& i : endpoits)
		router.RegisterEndpoint(std::get<0>(i), std::get<1>(i),[](const VS_PointParams *p) {});

	auto msg1 = std::make_unique<VS_RouterMessage>(nullptr, nullptr, nullptr, "user1@server", "from_user", "ToServer", "FromServer", 0, "1", 1);
	auto msg2 = std::make_unique<VS_RouterMessage>(nullptr, nullptr, nullptr, "user2@server", "from_user", "ToServer", "FromServer", 0, "2", 1);
	auto msg3 = std::make_unique<VS_RouterMessage>(nullptr, nullptr, nullptr, nullptr, "from_user", "server", "FromServer", 0, "3", 1);

	router.SendMsg(msg1.get());
	router.SendMsg(msg2.get());
	router.SendMsg(msg3.get());

	ASSERT_EQ(counter, 3);
}

TEST(InStatusSyncTest, simple)
{
	VS_InputSync input;
	VS_FullID id("server","user");

	/**
		1. first pack is init;
		2. correct next seq;
		3. correct next seq again;
		4. seq_id is jumped;
		5. next seq_id;
		6. Reset;
		7. correct seq_id;
	*/
	VS_Container cnt;
	long seq_id = 1;

	//1
	cnt.AddValueI32(SEQUENCE_ID_PARAM, seq_id);
	cnt.AddValueI32(CAUSE_PARAM, 1);

	ASSERT_TRUE(input.ConsistentCheck(id, cnt));

	//2
	++seq_id;
	cnt.Clear();
	cnt.AddValueI32(SEQUENCE_ID_PARAM, seq_id);

	ASSERT_TRUE(input.ConsistentCheck(id, cnt));

	//3
	++seq_id;
	cnt.Clear();
	cnt.AddValueI32(SEQUENCE_ID_PARAM, seq_id);

	ASSERT_TRUE(input.ConsistentCheck(id, cnt));

	//4
	seq_id += 100;
	cnt.Clear();
	cnt.AddValueI32(SEQUENCE_ID_PARAM, seq_id);
	ASSERT_FALSE(input.ConsistentCheck(id, cnt));

	//5
	++seq_id;
	cnt.Clear();
	cnt.AddValueI32(SEQUENCE_ID_PARAM, seq_id);
	//whait for timeout
	ASSERT_TRUE(input.ConsistentCheck(id, cnt));

	//6
	++seq_id;
	cnt.Clear();

	cnt.AddValueI32(SEQUENCE_ID_PARAM, seq_id);
	cnt.AddValueI32(CAUSE_PARAM, 1);

	ASSERT_TRUE(input.ConsistentCheck(id, cnt));

	//7

	++seq_id;
	cnt.Clear();
	cnt.AddValueI32(SEQUENCE_ID_PARAM, seq_id);

	ASSERT_TRUE(input.ConsistentCheck(id, cnt));

}
TEST(ExtendedStatusStorage, InsertToCntOnlyChanged)
{
	static const char *status_descr_1= "Test Description1";
	static const char *status_descr_2 = "Test Description2";
	VS_ExtendedStatusStorage storage;
	storage.UpdateStatus(EXTSTATUS_NAME_EXT_STATUS, 3);
	VS_Container cnt;
	VS_Container ext_status_cnt;
	storage.ToContainer(cnt,false);
	ASSERT_EQ(1u, cnt.Count());
	int32_t val(0);
	ASSERT_TRUE(cnt.GetValue(EXTSTATUS_PARAM, ext_status_cnt));
	ASSERT_EQ(1u, ext_status_cnt.Count());
	ASSERT_TRUE(ext_status_cnt.GetValue(EXTSTATUS_NAME_EXT_STATUS, val));
	ASSERT_EQ(3, val);
	storage.UpdateStatus(EXTSTATUS_NAME_DESCRIPTION, std::string(status_descr_1));
	cnt.Clear();
	ext_status_cnt.Clear();
	storage.ToContainer(cnt,false);
	ASSERT_TRUE(cnt.GetValue(EXTSTATUS_PARAM, ext_status_cnt));
	ASSERT_EQ(1u, ext_status_cnt.Count());
	const char * p = ext_status_cnt.GetStrValueRef(EXTSTATUS_NAME_DESCRIPTION);
	ASSERT_NE(nullptr, p);
	string_view descr = p;
	ASSERT_EQ(descr, status_descr_1);
	storage.UpdateStatus(EXTSTATUS_NAME_DESCRIPTION, std::string(status_descr_2));
	int64_t change_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	storage.UpdateStatus(EXTSTATUS_NAME_LAST_ONLINE_TIME,change_time);
	cnt.Clear();
	ext_status_cnt.Clear();
	storage.ToContainer(cnt,false);
	ASSERT_TRUE(cnt.GetValue(EXTSTATUS_PARAM, ext_status_cnt));
	ASSERT_EQ(2u, ext_status_cnt.Count());
	p = ext_status_cnt.GetStrValueRef(EXTSTATUS_NAME_DESCRIPTION);
	ASSERT_NE(nullptr, p);
	descr = p;
	ASSERT_EQ(descr, status_descr_2);
	int64_t int64_val(0);
	ASSERT_TRUE(ext_status_cnt.GetValue(EXTSTATUS_NAME_LAST_ONLINE_TIME, int64_val));
	ASSERT_EQ(int64_val, change_time);
	cnt.Clear();
	ext_status_cnt.Clear();
	storage.ToContainer(cnt,false);
	ASSERT_FALSE(cnt.GetValue(EXTSTATUS_PARAM, ext_status_cnt));
	ASSERT_EQ(3u,storage.GetAllStatuses().size());
}
TEST(ExtendedStatusStorage, CAUSE_PARAM)
{
	/**
		insert all statuses;
		set CAUSE_PARAM to 1
	*/
	VS_ExtendedStatusStorage storage;
	storage.UpdateStatus(EXTSTATUS_NAME_EXT_STATUS,3);
	storage.UpdateStatus(EXTSTATUS_NAME_DESCRIPTION,std::string("Test Description"));
	int64_t change_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	storage.UpdateStatus(EXTSTATUS_NAME_LAST_ONLINE_TIME,change_time);
	storage.UpdateStatus(EXTSTATUS_NAME_CAMERA,2);
	storage.UpdateStatus(EXTSTATUS_NAME_IN_CLOUD,1);
	storage.UpdateStatus(EXTSTATUS_NAME_DEVICE_TYPE, 0);
	storage.UpdateStatus(EXTSTATUS_NAME_FWD_TYPE,2);
	storage.UpdateStatus(EXTSTATUS_NAME_FWD_CALL_ID,std::string("matv@t.trueconf.com"));
	storage.UpdateStatus(EXTSTATUS_NAME_FWD_TIMEOUT,20);
	storage.UpdateStatus(EXTSTATUS_NAME_FWD_TIMEOUT_CALL_ID,std::string("+79104731655"));
	VS_Container cnt;
	VS_Container ext_st_cnt;
	ASSERT_EQ(storage.GetAllStatuses().size(), 10u);
	storage.ToContainer(cnt,false);
	ASSERT_TRUE(cnt.GetValue(EXTSTATUS_PARAM, ext_st_cnt));
	ASSERT_EQ(10u, ext_st_cnt.Count());
	cnt.Clear();
	ext_st_cnt.Clear();
	storage.ToContainer(cnt,false);
	ASSERT_FALSE(cnt.GetValue(EXTSTATUS_PARAM, ext_st_cnt));
	ASSERT_EQ(0u, ext_st_cnt.Count());
	cnt.Clear();
	ext_st_cnt.Clear();
	storage.ToContainer(cnt, true);
	ASSERT_TRUE(cnt.GetValue(EXTSTATUS_PARAM, ext_st_cnt));
	ASSERT_EQ(11u, ext_st_cnt.Count());
	int32_t val(0);
	ASSERT_TRUE(ext_st_cnt.GetValue(CAUSE_PARAM, val));
	ASSERT_EQ(val, 1);
}
TEST(ExtendedStatusStorage, DeleteStatus)
{
	/**
		insert list of deleted statuses by DELETE_EXT_STATUS_PARAM
	*/
	VS_ExtendedStatusStorage storage;
	storage.UpdateStatus(EXTSTATUS_NAME_EXT_STATUS, 3);
	storage.UpdateStatus(EXTSTATUS_NAME_DESCRIPTION, std::string("Test Description"));
	int64_t change_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	storage.UpdateStatus(EXTSTATUS_NAME_LAST_ONLINE_TIME, change_time);
	storage.UpdateStatus(EXTSTATUS_NAME_CAMERA, 2);
	storage.UpdateStatus(EXTSTATUS_NAME_IN_CLOUD, 1);
	storage.UpdateStatus(EXTSTATUS_NAME_DEVICE_TYPE, 0);
	storage.UpdateStatus(EXTSTATUS_NAME_FWD_TYPE, 2);
	storage.UpdateStatus(EXTSTATUS_NAME_FWD_CALL_ID, std::string("matv@t.trueconf.com"));
	storage.UpdateStatus(EXTSTATUS_NAME_FWD_TIMEOUT, 20);
	storage.UpdateStatus(EXTSTATUS_NAME_FWD_TIMEOUT_CALL_ID, std::string("+79104731655"));
	VS_Container cnt;
	VS_Container ext_st_cnt;
	ASSERT_EQ(10u, storage.GetAllStatuses().size());
	storage.ToContainer(cnt,false);
	ASSERT_TRUE(cnt.GetValue(EXTSTATUS_PARAM, ext_st_cnt));
	ASSERT_EQ(10u, ext_st_cnt.Count());
	cnt.Clear();
	ext_st_cnt.Clear();
	storage.DeleteStatus(EXTSTATUS_NAME_LAST_ONLINE_TIME);
	storage.DeleteStatus(EXTSTATUS_NAME_CAMERA);
	storage.ToContainer(cnt,false);
	ASSERT_TRUE(cnt.GetValue(EXTSTATUS_PARAM, ext_st_cnt));
	ASSERT_EQ(2u, ext_st_cnt.Count());
	ext_st_cnt.Reset();
	while(ext_st_cnt.Next())
	{
		ASSERT_EQ(string_view(ext_st_cnt.GetName()), string_view(DELETE_EXT_STATUS_PARAM));
		string_view p = ext_st_cnt.GetStrValueRef();
		ASSERT_TRUE(p == EXTSTATUS_NAME_LAST_ONLINE_TIME || p == EXTSTATUS_NAME_CAMERA);
	}
	cnt.Clear();
	ext_st_cnt.Clear();
	storage.ToContainer(cnt,true);
	ASSERT_TRUE(cnt.GetValue(EXTSTATUS_PARAM,ext_st_cnt));
	ASSERT_EQ(9u, ext_st_cnt.Count());
	int64_t int64_val(0);
	ASSERT_FALSE(ext_st_cnt.GetValue(EXTSTATUS_NAME_LAST_ONLINE_TIME, int64_val));
	ASSERT_FALSE(ext_st_cnt.GetValue(EXTSTATUS_NAME_CAMERA, int64_val));
}

TEST_F(StatusTestFixture, register_unregister)
{
	std::string as1_name = "as1.trueconf.com#as";
	std::vector<Endpoint::MsgTraceType> as1_trace;
	size_t as1_trace_sz(0);
	std::string as2_name = "as2.trueconf.com#as";
	std::vector<Endpoint::MsgTraceType> as2_trace;
	size_t as2_trace_sz(0);
	std::string as3_name = "as3.trueconf.com#as";
	std::vector<Endpoint::MsgTraceType> as3_trace;
	size_t as3_trace_sz(0);
	std::string bs1_name = "bs1.trueconf.com#bs";
	auto user1 = std::make_tuple(std::string("user1@trueconf.com"), std::string("cid1"), std::vector<Endpoint::MsgTraceType>());
	AddAS(as1_name);
	AddAS(as2_name);
	AddAS(as3_name);
	AddBS(bs1_name);
	auto user1_processnig = [&user1](VS_RouterMessage*m){
		ASSERT_TRUE(false);
	};

	AddUser(std::get<0>(user1), { "user1@mail.ru", "user1@ya.ru" }, "User1 Displ Name", std::get<1>(user1), as1_name, bs1_name, user1_processnig);

	//1. Register on as1
	std::string seq = "s1";
	ASSERT_TRUE(RegisterStatus(std::get<0>(user1), as1_name, seq, std::get<1>(user1)));
	GetMsgTrace(as1_name, as1_trace);
	as1_trace_sz = as1_trace.size();
	bool is_found = false;
	for (auto &i : as1_trace)
	{
		auto &dst_service = std::get<0>(i);
		auto &method = std::get<1>(i);
		auto& cnt = std::get<2>(i);
		auto& src_server = std::get<3>(i);
		auto &src_usr = std::get<4>(i);
		auto &src_service = std::get<5>(i);
		if (dst_service == AUTH_SRV && method == USERLOGGEDIN_METHOD)
		{
			ASSERT_EQ(src_server, bs1_name);
			ASSERT_TRUE(src_usr.empty());
			ASSERT_EQ(src_service, PRESENCE_SRV);
			int32_t res(VS_UserLoggedin_Result::ACCESS_DENIED);
			cnt.GetValue(RESULT_PARAM, res);
			ASSERT_EQ(res, VS_UserLoggedin_Result::USER_LOGGEDIN_OK);
			string_view rcv_tmp_id = cnt.GetStrValueRef(ENDPOINT_PARAM);
			string_view  rcv_seq = cnt.GetStrValueRef(SEQUENCE_PARAM);
			ASSERT_EQ(rcv_tmp_id, std::get<1>(user1));
			ASSERT_EQ(rcv_seq, seq);
			is_found = true;
			break;
		}
	}
	ASSERT_TRUE(is_found);
	//2. register on as2 (as1 gets logout from bs);
	seq = "s2";
	ASSERT_TRUE(RegisterStatus(std::get<0>(user1), as2_name, seq, std::get<1>(user1)));
	GetMsgTrace(as1_name, as1_trace);
	ASSERT_GE(as1_trace.size(), as1_trace_sz);
	is_found = false;
	for (auto i = as1_trace_sz; i < as1_trace.size();++i)
	{
		auto item = as1_trace[i];
		auto &dst_service = std::get<0>(item);
		auto &method = std::get<1>(item);
		auto& cnt = std::get<2>(item);
		auto& src_server = std::get<3>(item);
		auto &src_usr = std::get<4>(item);
		auto &src_service = std::get<5>(item);
		if (dst_service == AUTH_SRV && method == LOGOUTUSER_METHOD)
		{
			ASSERT_EQ(src_server, bs1_name);
			ASSERT_TRUE(src_usr.empty());
			ASSERT_EQ(src_service, PRESENCE_SRV);
			string_view user_name = cnt.GetStrValueRef(USERNAME_PARAM);
			ASSERT_EQ(user_name, std::get<0>(user1));
			is_found = true;
			break;
		}
	}
	ASSERT_TRUE(is_found);
	as1_trace_sz = as1_trace.size();
	GetMsgTrace(as2_name, as2_trace);
	as2_trace_sz = as2_trace.size();
	for (auto&i : as2_trace)
	{
		auto &dst_service = std::get<0>(i);
		auto &method = std::get<1>(i);
		auto& cnt = std::get<2>(i);
		auto& src_server = std::get<3>(i);
		auto &src_usr = std::get<4>(i);
		auto &src_service = std::get<5>(i);
		ASSERT_FALSE(dst_service == AUTH_SRV && method == USERLOGGEDIN_METHOD);
	}

	//3. Unregister from as1;
	ASSERT_TRUE(UnregisterStatus(std::get<0>(user1), as1_name));
	GetMsgTrace(as2_name, as2_trace);
	ASSERT_GE(as2_trace.size(), as2_trace_sz);
	is_found = false;
	for (auto i = as2_trace_sz; i < as2_trace.size(); i++)
	{
		auto item = as2_trace[i];
		auto &dst_service = std::get<0>(item);
		auto &method = std::get<1>(item);
		auto& cnt = std::get<2>(item);
		auto& src_server = std::get<3>(item);
		auto &src_usr = std::get<4>(item);
		auto &src_service = std::get<5>(item);
		if (dst_service == AUTH_SRV && method == USERLOGGEDIN_METHOD)
		{
			ASSERT_EQ(src_server, bs1_name);
			ASSERT_TRUE(src_usr.empty());
			ASSERT_EQ(src_service, PRESENCE_SRV);
			int32_t res(VS_UserLoggedin_Result::ACCESS_DENIED);
			cnt.GetValue(RESULT_PARAM, res);
			ASSERT_EQ(res, VS_UserLoggedin_Result::USER_LOGGEDIN_OK);
			string_view rcv_tmp_id = cnt.GetStrValueRef(ENDPOINT_PARAM);
			string_view  rcv_seq = cnt.GetStrValueRef(SEQUENCE_PARAM);
			ASSERT_EQ(rcv_tmp_id, std::get<1>(user1));
			ASSERT_EQ(rcv_seq, seq);
			is_found = true;
			break;
		}
	}
	ASSERT_TRUE(is_found);
	as2_trace_sz = as2_trace.size();
	//4. check that user is registered on as2
	{
		auto res = GetStatusFromBSCache(bs1_name, std::get<0>(user1));
		ASSERT_TRUE(res.first);
		ASSERT_EQ(as2_name, res.second.m_serverID);
	}
	//1. now user is registered on as2
	//2. register on as1
	std::string seq_as1 = "s3";
	std::string temp_id_as1 = "cid_as1";
	ASSERT_TRUE(RegisterStatus(std::get<0>(user1), as1_name, seq_as1, temp_id_as1));
	//3. register on as3 => SilentReject for as1;
	std::string seq_as3 = "s4";
	std::string temp_id_as3 = "cid_as3";
	ASSERT_TRUE(RegisterStatus(std::get<0>(user1), as3_name, seq_as3, temp_id_as3));
	GetMsgTrace(as1_name, as1_trace);
	ASSERT_GE(as1_trace.size(), as1_trace_sz);
	is_found = false;
	for (auto i = as1_trace_sz; i < as1_trace.size(); i++)
	{
		auto item = as1_trace[i];
		auto &dst_service = std::get<0>(item);
		auto &method = std::get<1>(item);
		auto& cnt = std::get<2>(item);
		auto& src_server = std::get<3>(item);
		auto &src_usr = std::get<4>(item);
		auto &src_service = std::get<5>(item);
		if (dst_service == AUTH_SRV && method == USERLOGGEDIN_METHOD)
		{
			ASSERT_EQ(src_server, bs1_name);
			ASSERT_TRUE(src_usr.empty());
			ASSERT_EQ(src_service, PRESENCE_SRV);
			int32_t res(VS_UserLoggedin_Result::ACCESS_DENIED);
			cnt.GetValue(RESULT_PARAM, res);
			ASSERT_EQ(res, VS_UserLoggedin_Result::RETRY_LOGIN);
			string_view rcv_tmp_id = cnt.GetStrValueRef(ENDPOINT_PARAM);
			string_view  rcv_seq = cnt.GetStrValueRef(SEQUENCE_PARAM);
			ASSERT_EQ(rcv_tmp_id, temp_id_as1);
			ASSERT_EQ(rcv_seq, seq_as1);
			is_found = true;
			break;
		}
	}
	ASSERT_TRUE(is_found);
	as1_trace_sz = as1_trace.size();

	//4. Register again (relogin)
	//5. user is registered on as3;
	ASSERT_TRUE(RegisterStatus(std::get<0>(user1), as3_name, seq_as3, temp_id_as3));
	{
		auto res = GetStatusFromBSCache(bs1_name, std::get<0>(user1));
		ASSERT_TRUE(res.first);
		ASSERT_EQ(as3_name, res.second.m_serverID);
	}
}

TEST_F(StatusTestFixture, generic_test)
{
	struct user_descr_st
	{
		std::string user_id;
		std::string cid;
		uint32_t counter;
		VS_ExtendedStatusStorage my_ext_st;
		std::map < std::string, VS_ExtendedStatusStorage> peers_ext_st;
	};
	std::string as_name = "as.trueconf.com#as";
	std::string bs1_name = "bs1.trueconf.com#bs";
	std::string bs2_name = "bs2.trueconf.com#bs";
	std::string rs_name = "rs.trueconf.com#rs";

	auto user1 = user_descr_st{ "user1@trueconf.com", "cid1", 0};
	auto user2 = user_descr_st{ "user2@trueconf.com", "cid2",0};

	SetResolveServerForDomain("trueconf.com", bs1_name);

	//user1
	auto user1_processing = [&user1, &user2](VS_RouterMessage*m)
	{
		const string_view dst_user = m->DstUser();
		const string_view dst_service = m->DstService();

		ASSERT_EQ(dst_user, user1.user_id);
		ASSERT_EQ(dst_service, PRESENCE_SRV);

		auto & counter = user1.counter;

		VS_Container cnt;
		ASSERT_TRUE(cnt.Deserialize(m->Body(), m->BodySize()));
		const string_view method = cnt.GetStrValueRef(METHOD_PARAM);
		switch (counter)
		{
		case 0:
		{
			//user1 subscribes to user2(user2 is offline);
			ASSERT_EQ(method, UPDATESTATUS_METHOD);
			const string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
			ASSERT_EQ(call_id, user2.user_id);
			int32_t status(0);
			ASSERT_TRUE(cnt.GetValue(USERPRESSTATUS_PARAM, status));
			ASSERT_EQ(status, VS_UserPresence_Status::USER_LOGOFF);
			auto & ext_st = user1.peers_ext_st[static_cast<std::string>(call_id)];
			VS_Container extStatusCnt;
			if (cnt.GetValue(EXTSTATUS_PARAM, extStatusCnt))
				ext_st.UpdateStatus(extStatusCnt);
			ASSERT_EQ(ext_st,user2.my_ext_st);
			break;
		}
		case 1:
		{
			//login user2;
			ASSERT_EQ(method, UPDATESTATUS_METHOD);
			const string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
			ASSERT_EQ(call_id, user2.user_id);
			int32_t status(0);
			ASSERT_TRUE(cnt.GetValue(USERPRESSTATUS_PARAM, status));
			ASSERT_EQ(status, VS_UserPresence_Status::USER_AVAIL);
			auto & ext_st = user1.peers_ext_st[static_cast<std::string>(call_id)];
			VS_Container extStatusCnt;
			if (cnt.GetValue(EXTSTATUS_PARAM, extStatusCnt))
				ext_st.UpdateStatus(extStatusCnt);
			ASSERT_EQ(ext_st,user2.my_ext_st);
			break;
		}
		default:
			ASSERT_TRUE(false);
		}
		++counter;

	};
	auto user2_processing = [&user1, &user2](VS_RouterMessage*m)
	{
		const string_view dst_user = m->DstUser();
		const string_view dst_service = m->DstService();
		ASSERT_EQ(dst_user, user2.user_id);
		ASSERT_EQ(dst_service, PRESENCE_SRV);

		auto & counter = user2.counter;

		VS_Container cnt;
		ASSERT_TRUE(cnt.Deserialize(m->Body(), m->BodySize()));
		const string_view method = cnt.GetStrValueRef(METHOD_PARAM);
		switch (counter)
		{
		case 0:
		{
			//user2 subscribes to user 1 (user1 is online);
			ASSERT_EQ(method, UPDATESTATUS_METHOD);
			const string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
			ASSERT_EQ(call_id, user1.user_id);
			int32_t status(0);
			ASSERT_TRUE(cnt.GetValue(USERPRESSTATUS_PARAM, status));
			ASSERT_EQ(status, VS_UserPresence_Status::USER_AVAIL);
			auto &ext_st = user2.peers_ext_st[static_cast<std::string>(call_id)];
			VS_Container extStatusCnt;
			if (cnt.GetValue(EXTSTATUS_PARAM, extStatusCnt))
				ext_st.UpdateStatus(extStatusCnt);
			ASSERT_EQ(ext_st,user1.my_ext_st);
		}
		break;
		case 1:
		{
			// user1 pushStatus
			ASSERT_EQ(method, UPDATESTATUS_METHOD);
			const string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
			ASSERT_EQ(call_id, user1.user_id);
			int32_t status(0);
			ASSERT_TRUE(cnt.GetValue(USERPRESSTATUS_PARAM, status));
			ASSERT_EQ(status, VS_UserPresence_Status::USER_BUSY);
			auto &ext_st = user2.peers_ext_st[static_cast<std::string>(call_id)];
			VS_Container extStatusCnt;
			if (cnt.GetValue(EXTSTATUS_PARAM, extStatusCnt))
				ext_st.UpdateStatus(extStatusCnt);
			ASSERT_EQ(ext_st,user1.my_ext_st);
		}
		break;
		case 2:
		{
			// 2 - push status by logoff user2
			ASSERT_EQ(method, UPDATESTATUS_METHOD);
			const string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
			ASSERT_EQ(call_id, user1.user_id);
			int32_t status(0);
			ASSERT_TRUE(cnt.GetValue(USERPRESSTATUS_PARAM, status));
			ASSERT_EQ(status, VS_UserPresence_Status::USER_LOGOFF);
			auto &ext_st = user2.peers_ext_st[static_cast<std::string>(call_id)];
			VS_Container extStatusCnt;
			if (cnt.GetValue(EXTSTATUS_PARAM, extStatusCnt))
				ext_st.UpdateStatus(extStatusCnt);
			ASSERT_EQ(ext_st, user1.my_ext_st);
		}
		break;
		default:
		{
			ASSERT_TRUE(false);
		}
		}

		++counter;
	};

	AddAS(as_name);
	AddBS(bs1_name);
	AddBS(bs2_name);
	AddRS(rs_name);

	//1. user1 registers on router;
	AddUser(user1.user_id, {"user1@mail.ru","user1@ya.ru"},"User1 Displ Name", user1.cid, as_name, bs1_name, user1_processing);
	//2. login user1 (presense->OnUserLoggedIn)
	LoginUser(user1.user_id);
	//3. user1 subscribes to user2(user2 is offline);
	Subscribe(user1.user_id, user2.user_id);

	//4. user2 registers on router;
	AddUser(user2.user_id, { "user2@mail.ru", "user2@ya.ru" }, "User2 Display Name", user2.cid, as_name, bs2_name, user2_processing);
	//5. login user2;
	LoginUser(user2.user_id);
	//6. user2 subscribes to user 1 (user1 is online);
	Subscribe(user2.user_id, user1.user_id);
	// 7. user1 pushStatus
	auto &user1_ext_st = user1.my_ext_st;
	user1_ext_st.UpdateStatus(EXTSTATUS_NAME_EXT_STATUS,2);
	PushStatus(user1.user_id, VS_UserPresence_Status::USER_BUSY,user1_ext_st);
	LogOutUser(user1.user_id);
	LogOutUser(user2.user_id);

	ASSERT_EQ(user1.counter, 2);
	ASSERT_EQ(user2.counter, 3);

	std::vector<Endpoint::MsgTraceType> rs_msg_trace;
	std::vector<Endpoint::MsgTraceType> as_msg_trace;
	std::vector<Endpoint::MsgTraceType> bs1_msg_trace;
	std::vector<Endpoint::MsgTraceType> bs2_msg_trace;

	GetMsgTrace(as_name, as_msg_trace);
	GetMsgTrace(rs_name, rs_msg_trace);
	GetMsgTrace(bs1_name, bs1_msg_trace);
	GetMsgTrace(bs2_name, bs2_msg_trace);
}

TEST_F(StatusTestFixture, SubscribeUnsubscribe)
{
	/**
		domain truaeconf.com -> {bs1, bs2}
		default bs = bs1;

		user1->bs1
		user2->bs1
		user3->bs2
		user4->bs1

		user1->as1
		user2->as2
		user3->as3
		user4->as1

		user1 -> subscribe (user2)
		user1 -> subscribe(user3)
		user4->subscribe(user2)

		user2 -> subscribe (user1)
		user2 -> subscribe(user3)


		user3 -> subscribe (user1)
		user3 -> subscribe(user2)

		user3->pushStatus(busy);
		user3->pushStatus(avail);
		user2->PushStatus(busy);

		user1->unsibscribe(user2,user3)
		user2->pushStatus;
			user1 didnt get status;
			user3 got status;
			user4 got status;

		user2->unsibscribe(user1,user3)
		user3->unsibscribe(user1,user2)

		Logout;
	*/
	std::string domain = "test-trueconf.com";
	std::string bs1_name = "bs1.test-trueconf.com#bs";
	Endpoint::MsgTraceType bs1_msg;
	size_t bs1_trace_current_index(-1);
	std::string bs2_name = "bs2.test-trueconf.com#bs";
	Endpoint::MsgTraceType bs2_msg;
	size_t bs2_trace_current_index(-1);

	std::string as1_name = "as1.test-trueconf.com#as";
	Endpoint::MsgTraceType as1_msg;
	size_t as1_trace_current_index(-1);
	std::string as2_name = "as2.test-trueconf.com#as";
	Endpoint::MsgTraceType as2_msg;
	size_t as2_trace_current_index(-1);
	std::string as3_name = "as3.test-trueconf.com#as";
	Endpoint::MsgTraceType as3_msg;
	size_t as3_trace_current_index(-1);

	auto user1 = std::make_tuple(
		std::string("user1@test-trueconf.com"),
		std::list<std::string>({ "user1@mail.ru", "user1@ya.ru" }),
		std::string("User1 Displ"),
		std::string("u1_cid"),
		as1_name,
		bs1_name,
		VS_ExtendedStatusStorage()
	);
	auto user2 = std::make_tuple(
		std::string("user2@test-trueconf.com"),
		std::list<std::string>({ "user2@mail.ru", "user2@ya.ru" }),
		std::string("User2 Displ"),
		std::string("u2_cid"),
		as2_name,
		bs1_name,
		VS_ExtendedStatusStorage()
		);

	auto user3 = std::make_tuple(
		std::string("user3@test-trueconf.com"),
		std::list<std::string>({ "user3@mail.ru", "user3@ya.ru" }),
		std::string("User3 Displ"),
		std::string("u3_cid"),
		as3_name,
		bs2_name,
		VS_ExtendedStatusStorage()
	);
	auto user4 = std::make_tuple(
		std::string("user4@test-trueconf.com"),
		std::list<std::string>({ "user4@mail.ru", "user4@ya.ru" }),
		std::string("User4 Displ"),
		std::string("u4_cid"),
		as1_name,
		bs2_name,
		VS_ExtendedStatusStorage()
	);

	AddBS(bs1_name);
	AddBS(bs2_name);

	SetResolveServerForDomain(domain, bs1_name);

	AddAS(as1_name);
	AddAS(as2_name);
	AddAS(as3_name);

	AddUser(std::get<0>(user1), std::get<1>(user1), std::get<2>(user1), std::get<3>(user1), std::get<4>(user1), std::get<5>(user1));
	AddUser(std::get<0>(user2), std::get<1>(user2), std::get<2>(user2), std::get<3>(user2), std::get<4>(user2), std::get<5>(user2));
	AddUser(std::get<0>(user3), std::get<1>(user3), std::get<2>(user3), std::get<3>(user3), std::get<4>(user3), std::get<5>(user3));
	AddUser(std::get<0>(user4), std::get<1>(user4), std::get<2>(user4), std::get<3>(user4), std::get<4>(user4), std::get<5>(user4));

	LoginUser(std::get<0>(user1)); // as1 => registerStatus=>bs1
	bs1_trace_current_index = GetMsgByFilter(bs1_name, PRESENCE_SRV, REGISTERSTATUS_METHOD, bs1_trace_current_index + 1, bs1_msg, [&bs1_name, &user1, &as1_name](const Endpoint::MsgTraceType &msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(REALID_PARAM);
		if (src_server != as1_name || !src_usr.empty() || call_id != std::get<0>(user1))
			return false;
		return true;
	}
	);
	ASSERT_NE(bs1_trace_current_index, std::numeric_limits<size_t>::max());

	LoginUser(std::get<0>(user2)); // registerStatus to bs1
	bs1_trace_current_index = GetMsgByFilter(bs1_name, PRESENCE_SRV, REGISTERSTATUS_METHOD, bs1_trace_current_index + 1, bs1_msg, [&bs1_name, &user2, &as2_name](const Endpoint::MsgTraceType &msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(REALID_PARAM);
		if (src_server != as2_name || !src_usr.empty() || call_id != std::get<0>(user2))
			return false;
		return true;
	}
	);
	ASSERT_NE(bs1_trace_current_index, std::numeric_limits<size_t>::max());

	LoginUser(std::get<0>(user3)); // registerStatus to bs2
	bs2_trace_current_index = GetMsgByFilter(bs2_name, PRESENCE_SRV, REGISTERSTATUS_METHOD, bs2_trace_current_index + 1, bs2_msg, [&bs2_name, &user3,as3_name](const Endpoint::MsgTraceType &msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(REALID_PARAM);
		if (src_server != as3_name || !src_usr.empty() || call_id != std::get<0>(user3))
			return false;
		return true;
	}
	);
	ASSERT_NE(bs2_trace_current_index, std::numeric_limits<size_t>::max());

	LoginUser(std::get<0>(user4)); // just login

	Subscribe(std::get<0>(user1), std::get<0>(user2)); // subscription as1->bs1; us1 gets user_avail for us2
	bs1_trace_current_index = GetMsgByFilter(bs1_name, PRESENCE_SRV, SUBSCRIBE_METHOD, bs1_trace_current_index + 1, bs1_msg, [&as1_name,&user2](const Endpoint::MsgTraceType&msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
		if (as1_name != src_server || !src_usr.empty() || std::get<0>(user2) != call_id)
			return false;
		return true;
	});
	ASSERT_NE(bs1_trace_current_index, std::numeric_limits<size_t>::max());
	{
		auto st = GetStatus(std::get<0>(user1), std::get<0>(user2));
		ASSERT_EQ(st.m_status, VS_UserPresence_Status::USER_AVAIL);
	}
	Subscribe(std::get<0>(user1), std::get<0>(user3)); // subscription as1->bs2; us1 gets user_avail for u3
	bs2_trace_current_index = GetMsgByFilter(bs2_name, PRESENCE_SRV, SUBSCRIBE_METHOD, bs2_trace_current_index + 1, bs2_msg, [&as1_name, &user3](const Endpoint::MsgTraceType&msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
		if (as1_name != src_server || !src_usr.empty() || std::get<0>(user3) != call_id)
			return false;
		return true;
	});
	ASSERT_NE(bs2_trace_current_index, std::numeric_limits<size_t>::max());
	{
		auto st = GetStatus(std::get<0>(user1), std::get<0>(user3));
		ASSERT_EQ(st.m_status, VS_UserPresence_Status::USER_AVAIL);
	}

	Subscribe(std::get<0>(user2), std::get<0>(user1)); // subscription as2->bs1; us2 gets user_avail for u1
	bs1_trace_current_index = GetMsgByFilter(bs1_name, PRESENCE_SRV, SUBSCRIBE_METHOD, bs1_trace_current_index + 1, bs1_msg, [&as2_name, &user1](const Endpoint::MsgTraceType&msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
		if (as2_name != src_server || !src_usr.empty() || std::get<0>(user1) != call_id)
			return false;
		return true;
	});
	ASSERT_NE(bs1_trace_current_index, std::numeric_limits<size_t>::max());
	{
		auto st = GetStatus(std::get<0>(user2), std::get<0>(user1));
		ASSERT_EQ(st.m_status, VS_UserPresence_Status::USER_AVAIL);
	}

	Subscribe(std::get<0>(user2), std::get<0>(user3)); // subscription as2->bs2; us2 gets user_avail for us3
	bs2_trace_current_index = GetMsgByFilter(bs2_name, PRESENCE_SRV, SUBSCRIBE_METHOD, bs2_trace_current_index + 1, bs2_msg, [&as2_name, &user3](const Endpoint::MsgTraceType&msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
		if (as2_name != src_server || !src_usr.empty() || std::get<0>(user3) != call_id)
			return false;
		return true;
	});
	ASSERT_NE(bs2_trace_current_index, std::numeric_limits<size_t>::max());
	{
		auto st = GetStatus(std::get<0>(user2), std::get<0>(user3));
		ASSERT_EQ(st.m_status, VS_UserPresence_Status::USER_AVAIL);
	}

	Subscribe(std::get<0>(user3), std::get<0>(user1)); // subscription as3->bs1; us3 gets user_avail for u1
	bs1_trace_current_index = GetMsgByFilter(bs1_name, PRESENCE_SRV, SUBSCRIBE_METHOD, bs1_trace_current_index + 1, bs1_msg, [&as3_name, &user1](const Endpoint::MsgTraceType&msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
		if (as3_name != src_server || !src_usr.empty() || std::get<0>(user1) != call_id)
			return false;
		return true;
	});
	ASSERT_NE(bs1_trace_current_index, std::numeric_limits<size_t>::max());
	{
		auto st = GetStatus(std::get<0>(user3), std::get<0>(user1));
		ASSERT_EQ(st.m_status, VS_UserPresence_Status::USER_AVAIL);
	}

	Subscribe(std::get<0>(user3), std::get<0>(user2)); // subscription as3->bs1; u3 gets user_avail for u2;

	bs1_trace_current_index = GetMsgByFilter(bs1_name, PRESENCE_SRV, SUBSCRIBE_METHOD, bs1_trace_current_index + 1, bs1_msg, [&as3_name, &user2](const Endpoint::MsgTraceType&msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
		if (as3_name != src_server || !src_usr.empty() || std::get<0>(user2) != call_id)
			return false;
		return true;
	});
	ASSERT_NE(bs1_trace_current_index, std::numeric_limits<size_t>::max());
	ASSERT_NE(bs1_trace_current_index, std::numeric_limits<size_t>::max());
	{
		auto st = GetStatus(std::get<0>(user3), std::get<0>(user2));
		ASSERT_EQ(st.m_status, VS_UserPresence_Status::USER_AVAIL);
	}
	Subscribe(std::get<0>(user4), std::get<0>(user2)); // subscription as1->bs2; us4 gets user_avail for us2
	{
		auto st = GetStatus(std::get<0>(user4), std::get<0>(user2));
		ASSERT_EQ(st.m_status, VS_UserPresence_Status::USER_AVAIL);
	}

	////	user3->pushStatus(busy);
	////	user3->pushStatus(avail);
	////	user2->PushStatus(busy);
	auto ext_st_u3 = std::get<6>(user3);
	ext_st_u3.UpdateStatus(EXTSTATUS_NAME_DESCRIPTION, std::string("user3 description"));
	PushStatus(std::get<0>(user3), VS_UserPresence_Status::USER_BUSY,ext_st_u3); // us1 and us2 got status us3
	{
		auto u1_st = GetStatus(std::get<0>(user1), std::get<0>(user3));
		auto u2_st = GetStatus(std::get<0>(user2), std::get<0>(user3));
		auto status_check = VS_UserPresence_Status::USER_BUSY;
		ASSERT_EQ(u1_st.m_status, status_check);
		ASSERT_EQ(u1_st.m_extStatusStorage, ext_st_u3);
		ASSERT_EQ(u2_st.m_status, status_check);
		ASSERT_EQ(u2_st.m_extStatusStorage, ext_st_u3);
	}
	ext_st_u3.UpdateStatus(EXTSTATUS_NAME_CAMERA, 1);
	PushStatus(std::get<0>(user3), VS_UserPresence_Status::USER_AVAIL,ext_st_u3); // us1 and us2 got status
	{
		auto u1_st = GetStatus(std::get<0>(user1), std::get<0>(user3));
		auto u2_st = GetStatus(std::get<0>(user2), std::get<0>(user3));
		auto status_check = VS_UserPresence_Status::USER_AVAIL;
		ASSERT_EQ(u1_st.m_status, status_check);
		ASSERT_EQ(u1_st.m_extStatusStorage, ext_st_u3);
		ASSERT_EQ(u2_st.m_status, status_check);
		ASSERT_EQ(u2_st.m_extStatusStorage, ext_st_u3);
	}
	auto ext_st_u2 = std::get<6>(user2);
	ext_st_u2.UpdateStatus(EXTSTATUS_NAME_EXT_STATUS, 1);
	PushStatus(std::get<0>(user2), VS_UserPresence_Status::USER_BUSY, ext_st_u2); //us1 and us3 got status
	{
		auto u1_st = GetStatus(std::get<0>(user1), std::get<0>(user2));
		auto u3_st = GetStatus(std::get<0>(user3), std::get<0>(user2));
		auto status_check = VS_UserPresence_Status::USER_BUSY;
		ASSERT_EQ(u1_st.m_status, status_check);
		ASSERT_EQ(u1_st.m_extStatusStorage, ext_st_u2);
		ASSERT_EQ(u3_st.m_status, status_check);
		ASSERT_EQ(u3_st.m_extStatusStorage, ext_st_u2);
	}

	UnSubscribe(std::get<0>(user1), std::get<0>(user2)); // as1 dosnt unsubscribe from bs1, becouse of user4 is subscribed for user2 (as1 must be subscribed too)
	auto old_index = bs1_trace_current_index;
	bs1_trace_current_index = GetMsgByFilter(bs1_name, PRESENCE_SRV, UNSUBSCRIBE_METHOD, bs1_trace_current_index + 1, bs1_msg, [&as1_name, &user2](const Endpoint::MsgTraceType&msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
		if (as1_name != src_server || !src_usr.empty() || std::get<0>(user2) != call_id)
			return false;
		return true;
	});
	ASSERT_EQ(bs1_trace_current_index, std::numeric_limits<size_t>::max()); //
	bs1_trace_current_index = old_index;

	ext_st_u2.UpdateStatus(EXTSTATUS_NAME_DESCRIPTION, std::string("2user2 description"));
	PushStatus(std::get<0>(user2), VS_UserPresence_Status::USER_AVAIL, ext_st_u2); // us3 got status, us4 got status, us1 didnt
	{
		auto u1_st = GetStatus(std::get<0>(user1),std::get<0>(user2));
		auto u3_st = GetStatus(std::get<0>(user3), std::get<0>(user2));
		auto u4_st = GetStatus(std::get<0>(user4), std::get<0>(user2));
		auto status_check = VS_UserPresence_Status::USER_AVAIL;
		ASSERT_EQ(u1_st.m_status, VS_UserPresence_Status::USER_BUSY); // <-- old status
		ASSERT_EQ(u3_st.m_status, status_check); // <-- new status
		ASSERT_EQ(u3_st.m_extStatusStorage, ext_st_u2); // <-- new status
		ASSERT_EQ(u4_st.m_status, status_check); // <-- new status
		ASSERT_EQ(u4_st.m_extStatusStorage,ext_st_u2); // <-- new status
	}
	UnSubscribe(std::get<0>(user1), std::get<0>(user3)); // as1 unsubscribe from bs2

	bs2_trace_current_index = GetMsgByFilter(bs2_name, PRESENCE_SRV, UNSUBSCRIBE_METHOD, bs2_trace_current_index + 1, bs2_msg, [&as1_name, &user3](const Endpoint::MsgTraceType&msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
		if (as1_name != src_server || !src_usr.empty() || std::get<0>(user3) != call_id)
			return false;
		return true;
	});
	ASSERT_NE(bs2_trace_current_index, std::numeric_limits<size_t>::max());

	UnSubscribe(std::get<0>(user2), std::get<0>(user1)); // as2 unsubscribe from bs1
	bs1_trace_current_index = GetMsgByFilter(bs1_name, PRESENCE_SRV, UNSUBSCRIBE_METHOD, bs1_trace_current_index + 1, bs1_msg, [&as2_name, &user1](const Endpoint::MsgTraceType&msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
		if (as2_name != src_server || !src_usr.empty() || std::get<0>(user1) != call_id)
			return false;
		return true;
	});
	ASSERT_NE(bs1_trace_current_index, std::numeric_limits<size_t>::max());

	UnSubscribe(std::get<0>(user2), std::get<0>(user3)); // as2 unsubscribe from bs2
	bs2_trace_current_index = GetMsgByFilter(bs2_name, PRESENCE_SRV, UNSUBSCRIBE_METHOD, bs2_trace_current_index + 1, bs2_msg, [&as2_name, &user3](const Endpoint::MsgTraceType&msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
		if (as2_name != src_server || !src_usr.empty() || std::get<0>(user3) != call_id)
			return false;
		return true;
	});
	ASSERT_NE(bs2_trace_current_index, std::numeric_limits<size_t>::max());

	UnSubscribe(std::get<0>(user3), std::get<0>(user1)); // as3 unsubscribe from bs1
	bs1_trace_current_index = GetMsgByFilter(bs1_name, PRESENCE_SRV, UNSUBSCRIBE_METHOD, bs1_trace_current_index + 1, bs1_msg, [&as3_name, &user1](const Endpoint::MsgTraceType&msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
		if (as3_name != src_server || !src_usr.empty() || std::get<0>(user1) != call_id)
			return false;
		return true;
	});
	ASSERT_NE(bs1_trace_current_index, std::numeric_limits<size_t>::max());

	UnSubscribe(std::get<0>(user3), std::get<0>(user2)); // as3 unsubscribe from bs1
	bs1_trace_current_index = GetMsgByFilter(bs1_name, PRESENCE_SRV, UNSUBSCRIBE_METHOD, bs1_trace_current_index + 1, bs1_msg, [&as3_name, &user2](const Endpoint::MsgTraceType&msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
		if (as3_name != src_server || !src_usr.empty() || std::get<0>(user2) != call_id)
			return false;
		return true;
	});
	ASSERT_NE(bs1_trace_current_index, std::numeric_limits<size_t>::max());


	LogOutUser(std::get<0>(user1)); // unregisterStatus to bs1

	bs1_trace_current_index = GetMsgByFilter(bs1_name, PRESENCE_SRV, UNREGISTERSTATUS_METHOD, bs1_trace_current_index + 1, bs1_msg, [&bs1_name, &user1, &as1_name](const Endpoint::MsgTraceType &msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
		if (src_server != as1_name || !src_usr.empty() || call_id != std::get<0>(user1))
			return false;
		return true;
	}
	);
	ASSERT_NE(bs1_trace_current_index, std::numeric_limits<size_t>::max());

	LogOutUser(std::get<0>(user2)); // unregisterStatus to bs1

	bs1_trace_current_index = GetMsgByFilter(bs1_name, PRESENCE_SRV, UNREGISTERSTATUS_METHOD, bs1_trace_current_index + 1, bs1_msg, [&bs1_name, &user2, &as2_name](const Endpoint::MsgTraceType &msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
		if (src_server != as2_name || !src_usr.empty() || call_id != std::get<0>(user2))
			return false;
		return true;
	}
	);
	ASSERT_NE(bs1_trace_current_index, std::numeric_limits<size_t>::max());

	LogOutUser(std::get<0>(user3)); // unregisterStatus to bs2
	bs2_trace_current_index = GetMsgByFilter(bs2_name, PRESENCE_SRV, UNREGISTERSTATUS_METHOD, bs2_trace_current_index + 1, bs2_msg, [&bs2_name, &user3, as3_name](const Endpoint::MsgTraceType &msg) -> bool
	{
		auto& cnt = std::get<2>(msg);
		auto& src_server = std::get<3>(msg);
		auto &src_usr = std::get<4>(msg);
		auto &src_service = std::get<5>(msg);
		string_view call_id = cnt.GetStrValueRef(CALLID_PARAM);
		if (src_server != as3_name || !src_usr.empty() || call_id != std::get<0>(user3))
			return false;
		return true;
	}
	);
	ASSERT_NE(bs2_trace_current_index, std::numeric_limits<size_t>::max());
}


TEST_F(StatusTestFixture, SyncLost_test)
{
	/**
		as1
		as2
		bs1

		u1->as1
		u2->as1

		u3->as2
		u4->as2

		1. Subscribe u3->u1 is lost;
			u4->subscribe for u2 => subscription is recovered;
		2. PushStatus(u1) is lost
			PushStatus(u2) recovers status of u1 (u3 and u4 have got status pf u2);
	*/


	std::string domain = "test-trueconf.com";
	std::string bs1_name = "bs1.test-trueconf.com#bs";

	std::string as1_name = "as1.test-trueconf.com#as";
	std::string as2_name = "as2.test-trueconf.com#as";
	auto user1 = std::make_tuple(
		std::string("user1@test-trueconf.com"),
		std::list<std::string>({ "user1@mail.ru", "user1@ya.ru" }),
		std::string("User1 Displ"),
		std::string("u1_cid"),
		as1_name,
		bs1_name
		);
	auto & user1_name = std::get<0>(user1);
	auto user2 = std::make_tuple(
		std::string("user2@test-trueconf.com"),
		std::list<std::string>({ "user2@mail.ru", "user2@ya.ru" }),
		std::string("User2 Displ"),
		std::string("u2_cid"),
		as1_name,
		bs1_name);
	auto & user2_name = std::get<0>(user2);
	auto user3 = std::make_tuple(
		std::string("user3@test-trueconf.com"),
		std::list<std::string>({ "user3@mail.ru", "user3@ya.ru" }),
		std::string("User3 Displ"),
		std::string("u3_cid"),
		as2_name,
		bs1_name);
	auto & user3_name = std::get<0>(user3);
	auto user4 = std::make_tuple(
		std::string("user4@test-trueconf.com"),
		std::list<std::string>({ "user4@mail.ru", "user4@ya.ru" }),
		std::string("User4 Displ"),
		std::string("u4_cid"),
		as2_name,
		bs1_name);
	auto & user4_name = std::get<0>(user4);

	AddAS(as1_name);
	AddAS(as2_name);
	AddBS(bs1_name);
	SetResolveServerForDomain(domain, bs1_name);

	AddUser(std::get<0>(user1), std::get<1>(user1), std::get<2>(user1), std::get<3>(user1), std::get<4>(user1), std::get<5>(user1));
	AddUser(std::get<0>(user2), std::get<1>(user2), std::get<2>(user2), std::get<3>(user2), std::get<4>(user2), std::get<5>(user2));
	AddUser(std::get<0>(user3), std::get<1>(user3), std::get<2>(user3), std::get<3>(user3), std::get<4>(user3), std::get<5>(user3));
	AddUser(std::get<0>(user4), std::get<1>(user4), std::get<2>(user4), std::get<3>(user4), std::get<4>(user4), std::get<5>(user4));

	LoginUser(user1_name);
	LoginUser(user2_name);
	LoginUser(user3_name);
	LoginUser(user4_name);
	Subscribe(user1_name, user2_name);
	Subscribe(user2_name, user1_name);

	/**
		insert hook
		 - skipp Subscribe from as2->bs1
	*/
	boost::signals2::connection conn;
	conn = router_->AddTesterForSkip([&as2_name, &bs1_name, &conn](const VS_RouterMessage *m)->bool
	{
		if (!m->IsFromServer())
			return false;
		if (as2_name != m->SrcServer() || bs1_name != m->DstServer())
			return false;
		VS_Container cnt;
		if (!cnt.Deserialize(m->Body(), m->BodySize()))
			return false;
		string_view method = cnt.GetStrValueRef(METHOD_PARAM);
		if (method != SUBSCRIBE_METHOD)
			return false;
		conn.disconnect();
		return true;
	});
	Subscribe(user3_name, user1_name);
	//Check that status is offline
	{
		auto st = GetStatus(user3_name, user1_name);
		ASSERT_NE(st.m_status, VS_UserPresence_Status::USER_AVAIL);
	}
	PushStatus(user1_name, VS_UserPresence_Status::USER_BUSY);
	// u2 got status, u3 didnt;
	{
		auto st3 = GetStatus(user3_name, user1_name);
		auto st2 = GetStatus(user2_name, user1_name);
		ASSERT_NE(st3.m_status, VS_UserPresence_Status::USER_BUSY);
		ASSERT_EQ(st2.m_status, VS_UserPresence_Status::USER_BUSY);
	}
	Subscribe(user4_name, user2_name);
	/**
		user3 got user1 status;
	*/
	{
		auto st = GetStatus(user3_name, user1_name);
		ASSERT_EQ(st.m_status, VS_UserPresence_Status::USER_BUSY);
		// check correct as
		auto st_from_as = GetStatusFromAS(as2_name, user1_name);
		ASSERT_TRUE(st_from_as.first);
		EXPECT_EQ(st_from_as.second.m_serverID, as1_name);
	}
	/**
		insert hook
		1 skip UPDATE_STATUS from as1 to bs1
		2. skip UPDATE_STATUS from bs1 to as2

	*/
	conn = router_->AddTesterForSkip([&conn,as1_name,bs1_name](const VS_RouterMessage*m) -> bool
	{
		if (!m->IsFromServer())
			return false;
		if (as1_name != m->SrcServer() || bs1_name != m->DstServer())
			return false;
		VS_Container cnt;
		if (!cnt.Deserialize(m->Body(), m->BodySize()))
			return false;
		string_view method = cnt.GetStrValueRef(METHOD_PARAM);
		if (method != UPDATESTATUS_METHOD)
			return false;
		conn.disconnect();
		return true;
	}
		);
	PushStatus(user1_name, VS_UserPresence_Status::USER_MULTIHOST);
	/*
		user2 got status
		user3 didnt;
		**/
	{
		auto st2 = GetStatus(user2_name, user1_name);
		auto  st3 = GetStatus(user3_name, user1_name);
		ASSERT_EQ(st2.m_status, VS_UserPresence_Status::USER_MULTIHOST);
		ASSERT_EQ(st3.m_status, VS_UserPresence_Status::USER_BUSY); //old status;

		auto st_from_as = GetStatusFromAS(as2_name, user1_name);
		ASSERT_TRUE(st_from_as.first);
		EXPECT_EQ(st_from_as.second.m_serverID, as1_name);
	}
	PushStatus(user2_name, VS_UserPresence_Status::USER_BUSY);
	/**
		user3 got user1 status;
		user4 got user2 status;
		user1 got user2 status;
	*/
	{
		auto st3 = GetStatus(user3_name, user1_name);
		ASSERT_EQ(st3.m_status, VS_UserPresence_Status::USER_MULTIHOST);
		auto st4 = GetStatus(user4_name, user2_name);
		ASSERT_EQ(st4.m_status, VS_UserPresence_Status::USER_BUSY);
		auto st1 = GetStatus(user1_name, user2_name);
		ASSERT_EQ(st1.m_status, VS_UserPresence_Status::USER_BUSY);

		auto st_from_as = GetStatusFromAS(as2_name, user1_name);
		ASSERT_TRUE(st_from_as.first);
		EXPECT_EQ(st_from_as.second.m_serverID, as1_name);

		st_from_as = GetStatusFromAS(as2_name, user2_name);
		ASSERT_TRUE(st_from_as.first);
		EXPECT_EQ(st_from_as.second.m_serverID, as1_name);
	}

	/**
	insert hook
	 skip UPDATE_STATUS from bs1 to as2

	*/
	conn = router_->AddTesterForSkip([&conn, as2_name, bs1_name](const VS_RouterMessage*m) -> bool
	{
		if (!m->IsFromServer())
			return false;
		if (bs1_name != m->SrcServer() || as2_name != m->DstServer())
			return false;
		VS_Container cnt;
		if (!cnt.Deserialize(m->Body(), m->BodySize()))
			return false;
		string_view method = cnt.GetStrValueRef(METHOD_PARAM);
		if (method != UPDATESTATUS_METHOD)
			return false;
		conn.disconnect();
		return true;
	}
	);
	PushStatus(user1_name, VS_UserPresence_Status::USER_AVAIL);
	/*
		user2 got status of us1
		user3 didnt;
	*/
	{
		auto st2 = GetStatus(user2_name, user1_name);
		auto st3 = GetStatus(user3_name, user1_name);
		ASSERT_EQ(st2.m_status, VS_UserPresence_Status::USER_AVAIL);
		ASSERT_EQ(st3.m_status, VS_UserPresence_Status::USER_MULTIHOST);

		auto st_from_as = GetStatusFromAS(as2_name, user1_name);
		ASSERT_TRUE(st_from_as.first);
		EXPECT_EQ(st_from_as.second.m_serverID, as1_name);
	}
	PushStatus(user2_name, VS_UserPresence_Status::USER_AVAIL);
	/**
	user3 got user1 status;
	user4 got user2 status;
	user1 got user2 status;
	*/
	{
		auto st3 = GetStatus(user3_name, user1_name);
		ASSERT_EQ(st3.m_status, VS_UserPresence_Status::USER_AVAIL);
		auto st4 = GetStatus(user4_name, user2_name);
		ASSERT_EQ(st4.m_status, VS_UserPresence_Status::USER_AVAIL);
		auto st1 = GetStatus(user1_name, user2_name);
		ASSERT_EQ(st1.m_status, VS_UserPresence_Status::USER_AVAIL);

		auto st_from_as = GetStatusFromAS(as2_name, user1_name);
		ASSERT_TRUE(st_from_as.first);
		EXPECT_EQ(st_from_as.second.m_serverID, as1_name);

		st_from_as = GetStatusFromAS(as2_name, user2_name);
		ASSERT_TRUE(st_from_as.first);
		EXPECT_EQ(st_from_as.second.m_serverID, as1_name);
	}
}
struct UserDescr
{
	VS_FORWARDING_CTOR6(UserDescr, callID, displayName, cid, aliases, locationServer, homeBS)
	{}
	std::string callID;
	std::string displayName;
	std::string cid;
	std::list<std::string> aliases;
	std::string locationServer;
	std::string homeBS;
};
TEST_F(StatusTestFixture, TCSSyncLost_test)
{
	/**
		u1 -> tcs1;
		u2 -> tcs2;
		u3 -> tcs3;

		u1 -> sub(u3) lost
		Check that status u3 is empty


		u1 -> sub(u2)
		u1 get status
		tcs1->GetStatus(u2).m_serverID == tcs2;
		Check that u3 status restored on user1 and tcs1

		PushStatus(u2) -> lost

		u1 has old status
		tcs1->GetStatus(u2).m_serverID == tcs2;

		PushStatus(u3)

		u1 has new status
		tcs1->GetStatus(u2).m_serverID == tcs2;
	*/
	std::string tcs1 = "tcs1.trueconf.loc#vcs";
	std::string tcs2 = "tcs2.trueconf.loc#vcs";

	auto user1_call_id = "user1@tcs1.trueconf.loc";
	auto user2_call_id = "user2@tcs2.trueconf.loc";
	auto user3_call_id = "user3@tcs2.trueconf.loc";
	std::list<std::string> aliases = {"user1@mail.ru", "user1@ya.ru"};
	auto user1 = UserDescr(user1_call_id, "User1 Name", "cid1", std::move(aliases), tcs1, tcs1);
	aliases = {"user2@mail.ru", "user2@ya.ru"};
	auto user2 = UserDescr(user2_call_id, "User2 Name", "cid2", std::move(aliases), tcs2, tcs2);
	auto user3 = UserDescr(user3_call_id, "User3 Name", "cid3", std::move(aliases), tcs2, tcs2);
	AddTCS(tcs1);
	AddTCS(tcs2);
	AddUser(user1.callID, user1.aliases,
			user1.displayName, user1.cid,
			user1.locationServer,user1.homeBS);
	AddUser(user2.callID, user2.aliases,
			user2.displayName, user2.cid,
			user2.locationServer,user2.homeBS);
	AddUser(user3.callID, user3.aliases,
			user3.displayName, user3.cid,
			user3.locationServer,user3.homeBS);
	LoginUser(user1.callID);
	LoginUser(user2.callID);
	LoginUser(user3.callID);
	/**
		u1 -> sub(u3) -> tcs1 -> tc2.sub(u3) lost
		skipp Subscribe tcs1->tcs2
	 */
	boost::signals2::connection conn;
	conn = router_->AddTesterForSkip([&](const VS_RouterMessage *msg)
	{
		if(!msg->IsFromServer())
			return false;
		string_view src_server = msg->SrcServer();
		string_view dst_server = msg->DstServer();
		if(src_server != tcs1 || dst_server != tcs2)
			return false;
		VS_Container cnt;
		if(!cnt.Deserialize(msg->Body(), msg->BodySize()))
			return false;
		string_view method = cnt.GetStrValueRef(METHOD_PARAM);
		if(method != SUBSCRIBE_METHOD)
			return false;
		conn.disconnect();
		return true;
	});
	Subscribe(user1.callID, user3.callID);
	{
		auto st = GetStatus(user1.callID, user3.callID);
		auto st_from_tcs1 = GetStatusFromAS(tcs1, user3.callID);
		// tcs1 has subsciption, so status is exist
		EXPECT_TRUE(st_from_tcs1.first);
		EXPECT_EQ(st.m_status, VS_UserPresence_Status::USER_LOGOFF);
		EXPECT_EQ(st_from_tcs1.second.m_status, VS_UserPresence_Status::USER_LOGOFF);
		EXPECT_TRUE(st_from_tcs1.second.m_serverID.empty());
	}
	// user1 subscribscribe(user2)
	Subscribe(user1.callID, user2.callID);
	{
		// check user2 and user3 status on user1 and tcs1
		auto st_u2 = GetStatus(user1.callID, user2.callID);
		auto st_u2_from_tcs1 = GetStatusFromAS(tcs1, user2.callID);
		EXPECT_TRUE(st_u2_from_tcs1.first);
		EXPECT_EQ(st_u2.m_status, VS_UserPresence_Status::USER_AVAIL);
		EXPECT_EQ(st_u2_from_tcs1.second.m_status, VS_UserPresence_Status::USER_AVAIL);
		EXPECT_EQ(st_u2_from_tcs1.second.m_serverID, tcs2);

		auto st_u3 = GetStatus(user1.callID, user3.callID);
		auto st_u3_from_tcs1 = GetStatusFromAS(tcs1, user3.callID);
		EXPECT_TRUE(st_u3_from_tcs1.first);
		EXPECT_EQ(st_u3.m_status, VS_UserPresence_Status::USER_AVAIL);
		EXPECT_EQ(st_u3_from_tcs1.second.m_status, VS_UserPresence_Status::USER_AVAIL);
		EXPECT_EQ(st_u3_from_tcs1.second.m_serverID, tcs2);
	}
	// PushStatus(u2) -> emulate lost
	conn = router_->AddTesterForSkip([&](const VS_RouterMessage *msg)
	{
		if(!msg->IsFromServer())
			return false;
		string_view src_server = msg->SrcServer();
		string_view dst_server = msg->DstServer();
		if(src_server != tcs2 || dst_server != tcs1)
			return false;
		VS_Container cnt;
		if(!cnt.Deserialize(msg->Body(), msg->BodySize()))
			return false;
		string_view method = cnt.GetStrValueRef(METHOD_PARAM);
		if(method != UPDATESTATUS_METHOD)
			return false;
		conn.disconnect();
		return true;
	});
	PushStatus(user2.callID, VS_UserPresence_Status::USER_BUSY);
	{
		// user1 and tcs1 lost user1 status
		auto st_u2 = GetStatus(user1.callID, user2.callID);
		auto st_u2_from_tcs1 = GetStatusFromAS(tcs1, user2.callID);
		EXPECT_TRUE(st_u2_from_tcs1.first);
		// current status is USER_BUSY
		EXPECT_EQ(st_u2.m_status, VS_UserPresence_Status::USER_AVAIL);
		EXPECT_EQ(st_u2_from_tcs1.second.m_status, VS_UserPresence_Status::USER_AVAIL);
		EXPECT_EQ(st_u2_from_tcs1.second.m_serverID, tcs2);
	}
	// PushStatus(u3) and restore u2 status
	PushStatus(user3.callID, VS_UserPresence_Status::USER_BUSY);
	{
		// check user2 and user3 status on user1 and tcs1
		auto st_u2 = GetStatus(user1.callID, user2.callID);
		auto st_u2_from_tcs1 = GetStatusFromAS(tcs1, user2.callID);
		EXPECT_TRUE(st_u2_from_tcs1.first);
		EXPECT_EQ(st_u2.m_status, VS_UserPresence_Status::USER_BUSY);
		EXPECT_EQ(st_u2_from_tcs1.second.m_status, VS_UserPresence_Status::USER_BUSY);
		EXPECT_EQ(st_u2_from_tcs1.second.m_serverID, tcs2);

		auto st_u3 = GetStatus(user1.callID, user3.callID);
		auto st_u3_from_tcs1 = GetStatusFromAS(tcs1, user3.callID);
		EXPECT_TRUE(st_u3_from_tcs1.first);
		EXPECT_EQ(st_u3.m_status, VS_UserPresence_Status::USER_BUSY);
		EXPECT_EQ(st_u3_from_tcs1.second.m_status, VS_UserPresence_Status::USER_BUSY);
		EXPECT_EQ(st_u3_from_tcs1.second.m_serverID, tcs2);
	}
	PushStatus(user2.callID, VS_UserPresence_Status::USER_AVAIL);
	{
		// check user2 and user3 status on user1 and tcs1
		auto st_u2 = GetStatus(user1.callID, user2.callID);
		auto st_u2_from_tcs1 = GetStatusFromAS(tcs1, user2.callID);
		EXPECT_TRUE(st_u2_from_tcs1.first);
		EXPECT_EQ(st_u2.m_status, VS_UserPresence_Status::USER_AVAIL);
		EXPECT_EQ(st_u2_from_tcs1.second.m_status, VS_UserPresence_Status::USER_AVAIL);
		EXPECT_EQ(st_u2_from_tcs1.second.m_serverID, tcs2);
	}
}
TEST_F(StatusTestFixture, PointConnectDisconnect)
{
	/**
		u1 -> as1
		u2 -> as2

		u1->subs u2
		u2->subs u1

		bs->PointDisconnect(as2)

		u2 is logoff for u1;

		u1 update status busy;

		on u2 only old status (avail)

		bs->on_point_connect(as2)
		u1 got status u2
		u2 got ststus u1;
	*/
	/**
		register as1 u1;
		register as2 u1

		onpointDisconnect as1

		onpointDisconnect as2
	*/
	std::string domain = "test-trueconf.com";
	std::string bs1_name = "bs1.test-trueconf.com#bs";

	std::string as1_name = "as1.test-trueconf.com#as";
	std::string as2_name = "as2.test-trueconf.com#as";
	auto user1 = std::make_tuple(
		std::string("user1@test-trueconf.com"),
		std::list<std::string>({ "user1@mail.ru", "user1@ya.ru" }),
		std::string("User1 Displ"),
		std::string("u1_cid"),
		as1_name,
		bs1_name
		);
	auto & user1_name = std::get<0>(user1);
	auto user2 = std::make_tuple(
		std::string("user2@test-trueconf.com"),
		std::list<std::string>({ "user2@mail.ru", "user2@ya.ru" }),
		std::string("User2 Displ"),
		std::string("u2_cid"),
		as2_name,
		bs1_name);
	auto & user2_name = std::get<0>(user2);

	AddAS(as1_name);
	AddAS(as2_name);
	AddBS(bs1_name);
	SetResolveServerForDomain(domain, bs1_name);
	AddUser(std::get<0>(user1), std::get<1>(user1), std::get<2>(user1), std::get<3>(user1), std::get<4>(user1), std::get<5>(user1));
	AddUser(std::get<0>(user2), std::get<1>(user2), std::get<2>(user2), std::get<3>(user2), std::get<4>(user2), std::get<5>(user2));

	LoginUser(user1_name);
	LoginUser(user2_name);

	Subscribe(user1_name, user2_name);
	Subscribe(user2_name, user1_name);

	{
		auto st_u1 = GetStatus(user1_name, user2_name);
		ASSERT_EQ(st_u1.m_status, VS_UserPresence_Status::USER_AVAIL);
		auto st_u2 = GetStatus(user2_name, user1_name);
		ASSERT_EQ(st_u2.m_status, VS_UserPresence_Status::USER_AVAIL);
	}
	OnPointDisconnected(bs1_name, as2_name);
	{
		auto st_u1 = GetStatus(user1_name, user2_name);
		ASSERT_EQ(st_u1.m_status, VS_UserPresence_Status::USER_LOGOFF);
		auto st_u2 = GetStatus(user2_name, user1_name);
		ASSERT_EQ(st_u2.m_status, VS_UserPresence_Status::USER_LOGOFF);
	}
	PushStatus(user1_name, VS_UserPresence_Status::USER_BUSY);
	{
		auto st_u2 = GetStatus(user2_name, user1_name);
		ASSERT_EQ(st_u2.m_status, VS_UserPresence_Status::USER_LOGOFF);
	}
	OnPointConnected(bs1_name, as2_name);
	{
		auto st_u1 = GetStatus(user1_name, user2_name);
		ASSERT_EQ(st_u1.m_status, VS_UserPresence_Status::USER_AVAIL);
		auto st_u2 = GetStatus(user2_name, user1_name);
		ASSERT_EQ(st_u2.m_status, VS_UserPresence_Status::USER_BUSY);
	}
	/**
		1. as1 register user3 on bs
		2. as2 register user3 on bs
		3. bs1 onpointDisconnect as1;
	*/
	{
		std::string user3_name = "user3@test-trueconf.com";
		AddUser(user3_name, { "" }, "display name", "cid1", as1_name, bs1_name);

		RegisterStatus(user3_name, as1_name, "seq1", "cid1");
		RegisterStatus(user3_name, as2_name, "seq2", "cid2");

		//check location as user3 by resolve method;

		OnPointDisconnected(as1_name, bs1_name);
		OnPointConnected(as1_name, bs1_name);

		std::string user4_name = "user4@test-trueconf.com";
		AddUser(user4_name, { "" }, "display name4", "cid3", as2_name, bs1_name);
		RegisterStatus(user4_name, as1_name, "seq3", "cid3");
		RegisterStatus(user4_name, as2_name, "seq4", "cid4");
		OnPointDisconnected(as2_name, bs1_name);
	}
}

TEST_F(StatusTestFixture, ResolveTest)
{
	/**
		u1 -> as1 -> bs1
		u2 -> as2 -> bs2
		u3 -> offline -> bs2

		Login(u1);
		Login(u2)

		as1->Resolve(u1);
		as1->Resolve(u2)
		as1->Resolve(u3)
	*/
	std::string domain = "test-trueconf.com";
	std::string bs1_name = "bs1.test-trueconf.com#bs";
	std::string bs2_name = "bs2.trueconf.com#bs";

	std::string as1_name = "as1.test-trueconf.com#as";
	std::string as2_name = "as2.test-trueconf.com#as";
	auto user1 = std::make_tuple(
		std::string("user1@test-trueconf.com"),
		std::list<std::string>({ "user1@mail.ru", "user1@ya.ru" }),
		std::string("User1 Displ"),
		std::string("u1_cid"),
		as1_name,
		bs1_name
		);
	auto & user1_name = std::get<0>(user1);
	auto user2 = std::make_tuple(
		std::string("user2@test-trueconf.com"),
		std::list<std::string>({ "user2@mail.ru", "user2@ya.ru" }),
		std::string("User2 Displ"),
		std::string("u2_cid"),
		as2_name,
		bs2_name);
	auto & user2_name = std::get<0>(user2);
	auto user3 = std::make_tuple(
		std::string("user3@test-trueconf.com"),
		std::list<std::string>({ "user3@mail.ru", "user3@ya.ru" }),
		std::string("User3 Displ"),
		std::string("u3_cid"),
		"none",
		bs2_name);
	auto & user3_name = std::get<0>(user3);

	//user4 is unknown user;
	auto user4 = std::make_tuple(
		std::string("user4@test-trueconf.com"),
		std::list<std::string>({ "user4@mail.ru", "user4@ya.ru" }),
		std::string("User4 Displ"),
		std::string("u4_cid"),
		"none",
		"none");
	auto & user4_name = std::get<0>(user4);


	AddAS(as1_name);
	AddAS(as2_name);
	AddBS(bs1_name);
	AddBS(bs2_name);
	SetResolveServerForDomain(domain, bs1_name);

	AddUser(std::get<0>(user1), std::get<1>(user1), std::get<2>(user1), std::get<3>(user1), std::get<4>(user1), std::get<5>(user1));
	AddUser(std::get<0>(user2), std::get<1>(user2), std::get<2>(user2), std::get<3>(user2), std::get<4>(user2), std::get<5>(user2));
	AddUser(std::get<0>(user3), std::get<1>(user3), std::get<2>(user3), std::get<3>(user3), std::get<4>(user3), std::get<5>(user3));

	LoginUser(user1_name);
	LoginUser(user2_name);

	{
		//use cache

		auto u1_from_as1 = Resolve(as1_name, user1_name, true);
		//Local user
		ASSERT_EQ(std::get<0>(u1_from_as1), user1_name);
		ASSERT_EQ(VS_UserPresence_Status::USER_AVAIL, std::get<1>(u1_from_as1).m_status);
		// user from other as
		auto u1_from_as2 = Resolve(as2_name, user1_name, true);
		ASSERT_EQ(VS_UserPresence_Status::USER_LOGOFF, std::get<1>(u1_from_as2).m_status);
		// ext
		auto u2_from_as1 = Resolve(as1_name, user2_name, true);
		ASSERT_EQ(VS_UserPresence_Status::USER_LOGOFF, std::get<1>(u2_from_as1).m_status);
		// local
		auto u2_from_as2 = Resolve(as2_name, user2_name, true);
		ASSERT_EQ(VS_UserPresence_Status::USER_AVAIL, std::get<1>(u2_from_as2).m_status);
	}

	{
		//don't use cache

		auto u1_from_as1 = Resolve(as1_name, user1_name, false);
		//Local user
		ASSERT_EQ(std::get<0>(u1_from_as1), user1_name);
		ASSERT_EQ(VS_UserPresence_Status::USER_AVAIL, std::get<1>(u1_from_as1).m_status);
		ASSERT_EQ(as1_name, std::get<1>(u1_from_as1).m_serverID);
		ASSERT_EQ(bs1_name, std::get<1>(u1_from_as1).m_homeServer);
		// user from other as
		auto u1_from_as2 = Resolve(as2_name, user1_name, false);
		ASSERT_EQ(VS_UserPresence_Status::USER_AVAIL, std::get<1>(u1_from_as2).m_status);
		ASSERT_EQ(as1_name, std::get<1>(u1_from_as2).m_serverID);
		ASSERT_EQ(bs1_name, std::get<1>(u1_from_as2).m_homeServer);
		// ext
		auto u2_from_as1 = Resolve(as1_name, user2_name, false);
		ASSERT_EQ(VS_UserPresence_Status::USER_AVAIL, std::get<1>(u2_from_as1).m_status);
		ASSERT_EQ(as2_name,std::get<1>(u2_from_as1).m_serverID);
		ASSERT_EQ(bs2_name, std::get<1>(u2_from_as1).m_homeServer);
		// local
		auto u2_from_as2 = Resolve(as2_name, user2_name, false);
		ASSERT_EQ(VS_UserPresence_Status::USER_AVAIL, std::get<1>(u2_from_as2).m_status);
		ASSERT_EQ(as2_name, std::get<1>(u2_from_as2).m_serverID);
		ASSERT_EQ(bs2_name, std::get<1>(u2_from_as2).m_homeServer);

	}
	{
		//don't use cache. user3 is offline
		auto u3_from_as1 = Resolve(as1_name, user3_name,false);
		ASSERT_EQ(VS_UserPresence_Status::USER_LOGOFF, std::get<1>(u3_from_as1).m_status);
		ASSERT_EQ(std::get<1>(u3_from_as1).m_serverID.length(),0);
		ASSERT_EQ(bs2_name, std::get<1>(u3_from_as1).m_homeServer);
	}

	{
		//don't use cache. user4 is unknown
		auto u4_from_as1 = Resolve(as1_name, user4_name,false);
		ASSERT_EQ(VS_UserPresence_Status::USER_INVALID, std::get<1>(u4_from_as1).m_status);
	}
}

TEST_F(StatusTestFixture, ResolveAllSync_test)
{
	/**
		u1 -> bs1,
		u2 -> bs1, as2

		u3 -> bs2, as1
		u4 -> bs2, as2

		u1->loggof
		u2->Avail
		u3->busy
		u4->multihost


		bs3->ResolveAllSyn(u1,u2,u3,u4);

	*/
	std::string domain = "test-trueconf.com";
	std::string bs1_name = "bs1.test-trueconf.com#bs";
	std::string bs2_name = "bs2.trueconf.com#bs";

	std::string bs3_name = "bs3.trueconf.com#bs";

	std::string as1_name = "as1.test-trueconf.com#as";
	std::string as2_name = "as2.test-trueconf.com#as";

	auto user1 = std::make_tuple(
		std::string("user1@test-trueconf.com"),
		std::list<std::string>({ "user1@mail.ru", "user1@ya.ru" }),
		std::string("User1 Displ"),
		std::string("u1_cid"),
		as1_name,
		bs1_name
		);
	auto & user1_name = std::get<0>(user1);

	auto user2 = std::make_tuple(
		std::string("user2@test-trueconf.com"),
		std::list<std::string>({ "user2@mail.ru", "user2@ya.ru" }),
		std::string("User2 Displ"),
		std::string("u2_cid"),
		as2_name,
		bs1_name);
	auto & user2_name = std::get<0>(user2);

	auto user3 = std::make_tuple(
		std::string("user3@test-trueconf.com"),
		std::list<std::string>({ "user3@mail.ru", "user3@ya.ru" }),
		std::string("User3 Displ"),
		std::string("u3_cid"),
		as1_name,
		bs2_name);
	auto & user3_name = std::get<0>(user3);


	auto user4 = std::make_tuple(
		std::string("user4@test-trueconf.com"),
		std::list<std::string>({ "user4@mail.ru", "user4@ya.ru" }),
		std::string("User4 Displ"),
		std::string("u4_cid"),
		as2_name,
		bs2_name);
	auto & user4_name = std::get<0>(user4);

	auto users = {user1,user2,user3,user4};

	AddAS(as1_name);
	AddAS(as2_name);
	AddBS(bs1_name);
	AddBS(bs2_name);
	AddBS(bs3_name);
	SetResolveServerForDomain(domain, bs1_name);

	for (auto &i : users)
		AddUser(std::get<0>(i), std::get<1>(i), std::get<2>(i), std::get<3>(i), std::get<4>(i), std::get<5>(i));

	LoginUser(user2_name);
	LoginUser(user3_name);
	LoginUser(user4_name);
	PushStatus(user3_name, VS_UserPresence_Status::USER_BUSY);
	PushStatus(user4_name, VS_UserPresence_Status::USER_MULTIHOST);

	auto statuses = ResolveAllSync(bs3_name, { user1_name, user2_name, user3_name, user4_name });
	ASSERT_EQ(statuses.size(), 4);
	for (auto &i : statuses)
	{
		if (i.m_realID == user1_name)
		{
			ASSERT_EQ(i.m_status, VS_UserPresence_Status::USER_LOGOFF);
			ASSERT_STREQ(i.m_homeServer.c_str(), bs1_name.c_str());
		}
		else if (i.m_realID == user2_name)
		{
			ASSERT_EQ(i.m_status, VS_UserPresence_Status::USER_AVAIL);
			ASSERT_STREQ(i.m_homeServer.c_str(), bs1_name.c_str());
			ASSERT_STREQ(i.m_serverID.c_str(), as2_name.c_str());
		}
		else if (i.m_realID == user3_name)
		{
			ASSERT_EQ(i.m_status, VS_UserPresence_Status::USER_BUSY);
			ASSERT_STREQ(i.m_homeServer.c_str(), bs2_name.c_str());
			ASSERT_STREQ(i.m_serverID.c_str(), as1_name.c_str());
		}
		else if (i.m_realID == user4_name)
		{
			ASSERT_EQ(i.m_status, VS_UserPresence_Status::USER_MULTIHOST);
			ASSERT_STREQ(i.m_homeServer.c_str(), bs2_name.c_str());
			ASSERT_STREQ(i.m_serverID.c_str(), as2_name.c_str());
		}
		else
		{
			ASSERT_TRUE(false);
		}
	}
}
TEST(ExtendedStorage, General)
{
	VS_ExtendedStatusStorage ext_status_for_send;
	/*
		ext_status
		status_description
		status_change_time
		camera
		mic
		CallFwd
			fwd_type
			fwd_call_id
			fwd_timeout
			timeout_call_id
	*/

	std::map<std::string, VS_ExtendedStatusStorage::StatusValueType> ext_st_vals;

	//int32_t ext_status_val = 3;
	ext_st_vals[EXTSTATUS_NAME_EXT_STATUS] = 3;
	//string_view descr_val = "Test Description";
	ext_st_vals[EXTSTATUS_NAME_DESCRIPTION] = std::string("Test Description");
	int64_t change_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	ext_st_vals[EXTSTATUS_NAME_LAST_ONLINE_TIME] = change_time;
	//int32_t camera = 2;
	ext_st_vals[EXTSTATUS_NAME_CAMERA] = 2;
	//int32_t in_cloud = 1;
	ext_st_vals[EXTSTATUS_NAME_IN_CLOUD] = 1;
	//int32_t device type
	ext_st_vals[EXTSTATUS_NAME_DEVICE_TYPE] = 0;

	ext_st_vals[EXTSTATUS_NAME_FWD_TYPE] = 2;
	ext_st_vals[EXTSTATUS_NAME_FWD_CALL_ID] = std::string("matv@t.trueconf.com");
	ext_st_vals[EXTSTATUS_NAME_FWD_TIMEOUT] = 20;
	ext_st_vals[EXTSTATUS_NAME_FWD_TIMEOUT_CALL_ID] = std::string("+79104731655");
	for (auto& i : ext_st_vals)
		ext_status_for_send.UpdateStatus(i.first.c_str(),std::move(i.second));
	VS_Container ext_stats_cnt;

	ext_status_for_send.ToContainer(ext_stats_cnt,false);
	static const auto c_max_serialized_bytes = 1328u;
	size_t sz(0);
	ext_stats_cnt.Serialize(nullptr, sz);
	std::cout << "\n Serialized statuses takes " << sz << " bytes. Benefit is "<<c_max_serialized_bytes - sz <<" bytes for each person\n";

	VS_ExtendedStatusStorage ext_status_recv;
	VS_Container extracted;
	ext_stats_cnt.GetValue(EXTSTATUS_PARAM, extracted);
	ASSERT_TRUE(ext_status_recv.UpdateStatus(extracted));

	auto set_statuses = ext_status_for_send.GetAllStatuses();
	auto got_statuses = ext_status_recv.GetAllStatuses();
	ASSERT_EQ(set_statuses.size(), got_statuses.size());
	ASSERT_TRUE(set_statuses == got_statuses);
}
TEST(DISABLED_VSDBStorageDebug, GetSetStickyExtendedStatus)
{
	/**
		BS: bs.t.trueconf.com#bs
		Connection string must be in registry
	*/
	const std::string bs_name = "bs.t.trueconf.com#bs";
	const std::string call_id = "mtv@t.trueconf.com";
	const std::string old_reg_root(VS_RegistryKey::GetDefaultRoot());
	VS_RegistryKey::SetDefaultRoot(VS_BASE_SERVER_WS_ROOT_KEY_NAME);
	VS_SCOPE_EXIT{ VS_RegistryKey::SetDefaultRoot(old_reg_root); };
	g_dbStorage = std::make_shared<VS_DBStorage_TrueConf>();
	auto db = g_dbStorage;
	db->Init(bs_name.c_str());
	/**
		SetSticky
	*/
	VS_ExtendedStatusStorage set_sticky;
	/**
		ext_status
		status_description
		status_change_time
		camera
		mic
		CallFwd
			fwd_type
			fwd_call_id
			fwd_timeout
			timeout_call_id
	*/

	std::map<std::string, VS_ExtendedStatusStorage::StatusValueType> ext_st_vals;
	//int32_t ext_status_val = 3;
	ext_st_vals[EXTSTATUS_NAME_EXT_STATUS] = 3;
	//string_view descr_val = "Test Description";
	ext_st_vals[EXTSTATUS_NAME_DESCRIPTION] = std::string("BugaBuGaGa");
	auto tomorow = std::chrono::system_clock::now() + std::chrono::hours(24);
	int64_t change_time = std::chrono::duration_cast<std::chrono::seconds>(tomorow.time_since_epoch()).count();
	//int64_t change_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	ext_st_vals[EXTSTATUS_NAME_LAST_ONLINE_TIME] = change_time;
	//int32_t camera = 2;
	ext_st_vals[EXTSTATUS_NAME_CAMERA] = 2;
	//int32_t mic = 1;
	//ext_st_vals[MIC_ST_NAME] = 1; // status is not allowed and skipped;
	ext_st_vals[EXTSTATUS_NAME_FWD_TYPE] = 2;
	ext_st_vals[EXTSTATUS_NAME_FWD_CALL_ID] = std::string("matv@t.trueconf.com");
	ext_st_vals[EXTSTATUS_NAME_FWD_TIMEOUT] = 20;
	ext_st_vals[EXTSTATUS_NAME_FWD_TIMEOUT_CALL_ID] = std::string("+79104731655");
	for (auto& i : ext_st_vals)
		set_sticky.UpdateStatus(i.first.c_str(), std::move(i.second));
	db->SetUserStatus(call_id.c_str(), VS_UserPresence_Status::USER_AVAIL, set_sticky, true, "matvey.pca.ru#as");
	VS_ExtendedStatusStorage online_sticky;
	online_sticky.UpdateStatus(EXTSTATUS_NAME_LAST_ONLINE_TIME, change_time);
	db->GetExtendedStatus(call_id.c_str(), online_sticky);
	auto deleted = online_sticky.GetDeletedStatuses();
	ASSERT_GE(deleted.count(EXTSTATUS_NAME_LAST_ONLINE_TIME), 0u);
	VS_ExtendedStatusStorage::StatusValueType out;
	ASSERT_FALSE(online_sticky.GetExtStatus(EXTSTATUS_NAME_LAST_ONLINE_TIME, out));

	VS_Container cnt;
	VS_ExtendedStatusStorage set_sticky_modify = set_sticky;
	set_sticky_modify.DeleteStatus(EXTSTATUS_NAME_DESCRIPTION); // delete status;
	db->SetUserStatus(call_id.c_str(), VS_UserPresence_Status::USER_LOGOFF, set_sticky_modify, false, VS_SimpleStr());
	set_sticky_modify = set_sticky;
	set_sticky_modify.DeleteStatus(EXTSTATUS_NAME_FWD_TYPE);
	set_sticky_modify.DeleteStatus(EXTSTATUS_NAME_FWD_CALL_ID);
	set_sticky_modify.DeleteStatus(EXTSTATUS_NAME_FWD_TIMEOUT);
	set_sticky_modify.DeleteStatus(EXTSTATUS_NAME_FWD_TIMEOUT_CALL_ID);
	// debug cause == 1
	set_sticky_modify.ToContainer(cnt, true);
	set_sticky_modify.Clear();
	set_sticky_modify.UpdateStatus(cnt);
	db->SetUserStatus(call_id.c_str(), VS_UserPresence_Status::USER_LOGOFF, set_sticky_modify, false, VS_SimpleStr());
	ASSERT_TRUE(set_sticky.GetExtStatus(EXTSTATUS_NAME_LAST_ONLINE_TIME, out));
	/**
		GetSticky
	*/
	online_sticky.ResetChangesInfo();
	VS_ExtendedStatusStorage sticky(std::move(online_sticky));
	db->GetExtendedStatus(call_id.c_str(), sticky);
	deleted = sticky.GetDeletedStatuses();
	ASSERT_EQ(deleted.count(EXTSTATUS_NAME_LAST_ONLINE_TIME), 0u);
	auto set_statuses = set_sticky.GetAllStatuses();
	auto got_statuses = sticky.GetAllStatuses();
	auto device_type = got_statuses.find(EXTSTATUS_NAME_DEVICE_TYPE);
	if (device_type != got_statuses.end())
		got_statuses.erase(device_type);
	g_dbStorage.reset();
}
TEST(DISABLED_VSDBStorageDebug, OfflineStatusCacheTest)
{
	const std::string bs_name = "bs.t.trueconf.com#bs";
	const std::string call_id = "mtv@t.trueconf.com";
	const std::string old_reg_root(VS_RegistryKey::GetDefaultRoot());
	VS_RegistryKey::SetDefaultRoot(VS_BASE_SERVER_WS_ROOT_KEY_NAME);
	VS_SCOPE_EXIT{ VS_RegistryKey::SetDefaultRoot(old_reg_root); };
	g_dbStorage = std::make_shared<VS_DBStorage_TrueConf>();
	auto db = g_dbStorage;
	db->Init(bs_name.c_str());
	OfflineStatusCache::SetWorkMode(OfflineStatusCache::WorkMode::async);
	bool last_offline_appered(false);
	bool last_offline_deleted(false);
	vs::condition_variable	cond_v;
	OfflineStatusCache offline_statuses_cache([&](OfflineStatusCache::ExtStatusUpdateInfo && out)
	{
		if (std::find_if(out.begin(), out.end(), [&](const OfflineStatusCache::ExtStatusUpdateInfo::value_type &i)
		{return i.first == call_id && !i.second.IsStatusExist(EXTSTATUS_NAME_LAST_ONLINE_TIME);}) != out.end())
		{
			last_offline_deleted = true;
			cond_v.notify_all();
		}
		else if(last_offline_deleted)
		{
			if (std::find_if(out.begin(), out.end(), [&](const OfflineStatusCache::ExtStatusUpdateInfo::value_type &i)
			{return i.first == call_id && i.second.IsStatusExist(EXTSTATUS_NAME_LAST_ONLINE_TIME);}) != out.end())
			{
				last_offline_appered = true;
				cond_v.notify_all();
			}
		}
	}
	);
	// clean my status
	db->SetUserStatus(call_id.c_str(), VS_UserPresence_Status::USER_LOGOFF, VS_ExtendedStatusStorage(), false, VS_SimpleStr());
	VS_CallIDInfo status;
	std::mutex m;
	bool sticky_is_read(false);
	offline_statuses_cache.ReadStickyToCache(std::list<std::string>{call_id}, [&]
	{
		auto item = offline_statuses_cache.GetOfflineStatus(call_id);
		if (item.second)
			status.m_extStatusStorage += item.first.ext_status_;
		ASSERT_TRUE(status.m_extStatusStorage.IsStatusExist(EXTSTATUS_NAME_LAST_ONLINE_TIME));
		offline_statuses_cache.Notify();
		sticky_is_read = true;
		cond_v.notify_all();
	}
	);
	std::unique_lock<std::mutex> l(m);
	while (!cond_v.wait_for(l, std::chrono::milliseconds(10), [&sticky_is_read]() { return sticky_is_read; }))
		;
	cond_v.wait(l, [&sticky_is_read]() { return sticky_is_read; });
	sticky_is_read = false;
	//Online mtv
	status.m_status = VS_UserPresence_Status::USER_AVAIL;
	db->SetUserStatus(call_id.c_str(), VS_UserPresence_Status::USER_AVAIL, status.m_extStatusStorage, true, "matvey.pca.ru#as");
	offline_statuses_cache.clock().add_diff(std::chrono::minutes(2));
	offline_statuses_cache.Notify();
	ASSERT_TRUE(cond_v.wait_for(l, std::chrono::seconds(1), [&]() {return last_offline_deleted;}));
	db->SetUserStatus(call_id.c_str(), VS_UserPresence_Status::USER_LOGOFF, VS_ExtendedStatusStorage(), false, VS_SimpleStr());
	offline_statuses_cache.clock().add_diff(std::chrono::minutes(2));
	offline_statuses_cache.Notify();
	ASSERT_TRUE(cond_v.wait_for(l, std::chrono::seconds(1), [&]() {return last_offline_appered;}));
	offline_statuses_cache.StopThread();
	g_dbStorage.reset();
}
//debug
TEST_F(StatusTestFixture, DISABLED_UpdateExtStatusFromDB)
{
	const std::string bs_name = "bs.t.trueconf.com#bs";
	const std::string as_name = "as1.trueconf.com#as";
	const std::string call_id1 = "matv@t.trueconf.com";
	const std::string call_id2 = "mtv@t.trueconf.com";
	const std::string old_reg_root(VS_RegistryKey::GetDefaultRoot());
	VS_RegistryKey::SetDefaultRoot(VS_BASE_SERVER_WS_ROOT_KEY_NAME);
	VS_SCOPE_EXIT{ VS_RegistryKey::SetDefaultRoot(old_reg_root); };
	g_dbStorage = std::make_shared<VS_DBStorage_TrueConf>();
	auto db = g_dbStorage;
	db->Init(bs_name.c_str());
	//Add BS
	AddBS(bs_name);
	SetResolveServerForDomain("t.trueconf.com", bs_name);
	// Add as
	AddAS(as_name);
	// Add use1 and user2
	AddUser(call_id1, {}, "", "cid1", as_name, bs_name);
	AddUser(call_id2, {}, "", "cid2", as_name, bs_name);
	//clean
	LogOutUser(call_id1);
	LogOutUser(call_id2);
	// login user1
	LoginUser(call_id1);
	// subscribe user1->user2 (offlie status got)
	Subscribe(call_id1, call_id2);
	auto status = GetStatus(call_id1, call_id2);
	ASSERT_EQ(status.m_status,VS_UserPresence_Status::USER_LOGOFF);
	ASSERT_TRUE(status.m_extStatusStorage.IsStatusExist(EXTSTATUS_NAME_LAST_ONLINE_TIME));
	// user2 login => online status, and status by update
	LoginUser(call_id2);
	offline_status_cache(bs_name).UpdateExtStatusesFromDB();
	status = GetStatus(call_id1, call_id2);
	ASSERT_FALSE(status.m_extStatusStorage.IsStatusExist(EXTSTATUS_NAME_LAST_ONLINE_TIME));
	LogOutUser(call_id1);
	LogOutUser(call_id2);
}
}
#endif
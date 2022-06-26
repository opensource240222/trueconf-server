#ifdef _WIN32
#include "VS_DBCallLogSqliteTest.h"

#include "../../common/std/statistics/TConferenceStatistics.h"

#include <Windows.h>

namespace tc3_test
{
TEST_F(DBCallLogSqliteTest, MakeCall){

	ASSERT_EQ(true, callLog->LogParticipantInvite(pd1.m_conf_id, pd1.m_user_id, app_id, pd2.m_user_id, std::chrono::system_clock::now(), VS_ParticipantDescription::PRIVATE_MEMBER));

	pd2.m_joinTime = std::chrono::system_clock::now();
	ASSERT_EQ(true, callLog->LogParticipantJoin(pd2, pd1.m_user_id));

	// Suppose they are tallking now...
	ASSERT_TRUE(TalkAndLeave(callLog,pd2));
	ASSERT_TRUE(MakeStatisticsLog(callLog, pd1));
}

TEST_F(DBCallLogSqliteTest, GetMissedCalls){
	char file1[32], file2[32];

	GetCurrentAndPrevMounthFile(file1, file2);

	std::remove(file1);	std::remove(file2);

	// make 10 missed calls to pd1
	for (size_t i = 1; i < 11; i++)
	{
		std::string pd2id = id2 + std::to_string(i);
		std::string conference = std::to_string(i) + conf;
		bool do_log_reject = (i <= 5)? true: false;	// 5 calls by callee reject, then 5 calls by caller hangup
		ASSERT_TRUE(MakeMissedCall_PD1_to_PD2(pd2id, conference, do_log_reject));
	}

	ASSERT_TRUE(IsLogExists(file1));

	// make it happen in previous month
	std::rename(file1, file2);

	ASSERT_TRUE(IsLogExists(file2));

	// make 10 missed calls to pd1 again
	for (size_t i = 1; i < 11; i++)
	{
		std::string pd2id = id2 + std::to_string(i);
		std::string conference = std::to_string(i) + conf;
		bool do_log_reject = (i <= 5) ? true : false;	// 5 calls by callee reject, then 5 calls by caller hangup
		ASSERT_TRUE(MakeMissedCall_PD1_to_PD2(pd2id, conference, do_log_reject));
	}

	ASSERT_TRUE(IsLogExists(file1));

	VS_Container cont;
	ASSERT_EQ(SEARCH_DONE, callLog->GetCalls(cont, entries, AB_MISSED_CALLS, pd1.m_user_id, 0L));
	ASSERT_EQ(20, entries);

	/* For new container format */
	std::vector<curr_container> select_values;

	GetSelectValues(cont, select_values);
	ASSERT_EQ(entries, select_values.size());

	std::remove(file1);	std::remove(file2);
}

TEST_F(DBCallLogSqliteTest, Get_Placed_and_Received_Calls){
	char file1[32], file2[32];

	GetCurrentAndPrevMounthFile(file1, file2);

	std::remove(file1);	std::remove(file2);


	// make 10 placed calls from pd2 to pd1
	for (size_t i = 1; i < 11; i++)
	{
		std::string pd2id = id2 + std::to_string(i);
		std::string conference = std::to_string(i) + conf;
		ASSERT_TRUE(MakePlacedCall_PD1_to_PD2(pd2id, conference));
	}


	// make sure log was created
	ASSERT_TRUE(IsLogExists(file1));

	// make it happen in previous month
	std::rename(file1, file2);

	ASSERT_TRUE(IsLogExists(file2));

	// make 10 placed calls from pd2 to pd1 again
	for (size_t i = 1; i < 11; i++)
	{
		std::string pd2id = id2 + std::to_string(i);
		std::string conference = std::to_string(i) + conf;
		ASSERT_TRUE(MakePlacedCall_PD1_to_PD2(pd2id, conference));
	}

	ASSERT_TRUE(IsLogExists(file1));

	VS_Container cont;
	ASSERT_EQ(SEARCH_DONE, callLog->GetCalls(cont, entries, AB_PLACED_CALLS, pd2.m_user_id, 0L));
	ASSERT_EQ(2, entries);

	/* For new container format */
	//vector<selected_container> select_values;

	std::vector<curr_container> select_values;

	GetSelectValues(cont, select_values);
	ASSERT_EQ(entries, select_values.size());

	VS_Container cont1; entries = 0;
	ASSERT_EQ(SEARCH_DONE, callLog->GetCalls(cont1, entries, AB_RECEIVED_CALLS, pd1.m_user_id, 0L));
	ASSERT_EQ(20, entries);

	select_values.clear();

	GetSelectValues(cont1, select_values);
	ASSERT_EQ(entries, select_values.size());

	std::remove(file1);	std::remove(file2);
}

bool IsLogExists(const char *log_file_name){
	std::ifstream ifs;

	ifs.open(log_file_name);
	if (!ifs.good()) return false;
	ifs.close();

	return true;
}

bool DBCallLogSqliteTest::MakePlacedCall_PD1_to_PD2(std::string pd2_id, std::string conference_id){
	confD.m_name = conference_id.c_str();

	if (!callLog->LogConferenceStart(confD)) return false;

	pd2.m_user_id = pd2_id.c_str();
	pd1.m_conf_id = pd2.m_conf_id = conference_id.c_str();
	MakeCall(callLog, pd2, pd1);

	confD.m_logEnded = std::chrono::system_clock::now();
	confD.m_logCause = VS_ConferenceDescription::TERMINATED_BY_ADMIN;
	if (!callLog->LogConferenceEnd(confD)) return false;

	return true;
}

bool DBCallLogSqliteTest::MakeMissedCall_PD1_to_PD2(std::string pd2_id, std::string conference_id, bool do_log_reject){
	confD.m_name = conference_id.c_str();

	if (!callLog->LogConferenceStart(confD)) return false;

	pd2.m_user_id = pd2_id.c_str();
	pd1.m_conf_id = pd2.m_conf_id = conference_id.c_str();
	MakeNonJoinableCall(callLog, pd2, pd1, do_log_reject);

	confD.m_logEnded = std::chrono::system_clock::now();
	confD.m_logCause = VS_ConferenceDescription::TERMINATED_BY_ADMIN;
	if (!callLog->LogConferenceEnd(confD)) return false;

	return true;
}

void GetSelectValues(VS_Container& container, std::vector <selected_container> &v){
	container.Reset();
	VS_SimpleStr call_id, display_name, broker_id, conference, topic, owner;
	int32_t type, subtype;
	std::chrono::system_clock::time_point call_time, start_time, end_time;

	while (container.Next()){
		call_id.Assign(container.GetStrValueRef());

		if (!container.Next()) break;
		display_name.Assign(container.GetStrValueRef());

		if (!container.Next()) break;
		container.GetValue(call_time);

		if (!container.Next()) break;
		broker_id.Assign(container.GetStrValueRef());

		if (!container.Next()) break;
		conference.Assign(container.GetStrValueRef());

		if (!container.Next()) break;
		container.GetValue(start_time);

		if (!container.Next()) break;
		container.GetValue(end_time);

		if (!container.Next()) break;
		container.GetValue(type);

		if (!container.Next()) break;
		container.GetValue(subtype);

		if (!container.Next()) break;
		topic.Assign(container.GetStrValueRef());

		if (!container.Next()) break;
		owner.Assign(container.GetStrValueRef());

		v.emplace_back(call_id, display_name, call_time, broker_id, conference, start_time, end_time, type, subtype, topic, owner);
	}
}

void GetSelectValues(VS_Container& container, std::vector <curr_container> &v){
	container.Reset();
	VS_SimpleStr call_id;
	std::chrono::system_clock::time_point call_time;

	while (container.Next()){
		call_id.Assign(container.GetStrValueRef());

		if (!container.Next()) break;
		container.GetValue(call_time);

		v.emplace_back(call_id, call_time);
	}
}

void MakeCall(VS_DB_CallLogSQLite *callLog, VS_ParticipantDescription& pd1, VS_ParticipantDescription& pd2){
	ASSERT_TRUE(callLog->LogParticipantInvite(pd1.m_conf_id, pd1.m_user_id, pd1.m_appID, pd2.m_user_id, std::chrono::system_clock::now(), VS_ParticipantDescription::PRIVATE_MEMBER));

	pd1.m_joinTime = std::chrono::system_clock::now();;
	ASSERT_TRUE(callLog->LogParticipantJoin(pd1));

	pd2.m_joinTime = std::chrono::system_clock::now();;
	ASSERT_TRUE(callLog->LogParticipantJoin(pd2));

	// Suppose they are tallking now...
	ASSERT_TRUE(TalkAndLeave(callLog, pd1));
	ASSERT_TRUE(TalkAndLeave(callLog, pd2));
	ASSERT_TRUE(MakeStatisticsLog(callLog, pd1));
	ASSERT_TRUE(MakeStatisticsLog(callLog, pd2));
}

void MakeNonJoinableCall(VS_DB_CallLogSQLite *callLog, VS_ParticipantDescription& pd1, VS_ParticipantDescription& pd2, bool do_log_reject){
	ASSERT_TRUE(callLog->LogParticipantInvite(pd1.m_conf_id, pd1.m_user_id, pd1.m_appID, pd2.m_user_id, std::chrono::system_clock::now(), VS_ParticipantDescription::PRIVATE_MEMBER));

	if (do_log_reject)
		ASSERT_TRUE(callLog->LogParticipantReject(pd1.m_conf_id, pd2.m_user_id, "", REJECTED_BY_PARTICIPANT));

	boost::shared_ptr<TConferenceStatistics> pConfStat(new TConferenceStatistics());
	pConfStat->size_of_stat = sizeof(TConferenceStatistics);
	pConfStat->participant_time = 0;
	pConfStat->broadcast_time = 0;
	pConfStat->video_w = pConfStat->video_h = 400;
	pConfStat->loss_rcv_packets = 0;
	pConfStat->avg_send_bitrate = pConfStat->avg_rcv_bitrate = 0;
	pConfStat->avg_cpu_load = 0;
	pConfStat->avg_jitter = 0;
	pConfStat->avg_send_fps = 0.0;
	pConfStat->start_part_gmt = pConfStat->end_part_gmt = std::chrono::system_clock::now();

	VS_Container input_cont;
	input_cont.AddValue(CONFERENCE_PARAM, pd1.m_conf_id);
	input_cont.AddValue(CALLID_PARAM, pd1.m_user_id);

	ASSERT_TRUE(callLog->LogParticipantStatistics(input_cont, pConfStat.get(), pd1.m_displayName));
}

bool MakeStatisticsLog(VS_DB_CallLogSQLite *callLog, VS_ParticipantDescription& pd){
	boost::shared_ptr<TConferenceStatistics> pConfStat = boost::make_shared<TConferenceStatistics>();
	pConfStat->size_of_stat = sizeof(TConferenceStatistics);
	pConfStat->participant_time = 123;
	pConfStat->broadcast_time = 321;
	pConfStat->video_w = pConfStat->video_h = 400;
	pConfStat->loss_rcv_packets = 1000;
	pConfStat->avg_send_bitrate = pConfStat->avg_rcv_bitrate = 100;
	pConfStat->avg_cpu_load = 5;
	pConfStat->avg_jitter = 105;
	pConfStat->avg_send_fps = 2.2;
	pConfStat->start_part_gmt = pConfStat->end_part_gmt = std::chrono::system_clock::now();

	VS_Container input_cont;
	input_cont.AddValue(CONFERENCE_PARAM, pd.m_conf_id);
	input_cont.AddValue(CALLID_PARAM, pd.m_user_id);

	return callLog->LogParticipantStatistics(input_cont, pConfStat.get(), pd.m_displayName);
}

bool TalkAndLeave(VS_DB_CallLogSQLite *callLog, VS_ParticipantDescription& pd){
	pd.m_cause = VS_ParticipantDescription::DISCONNECT_UNKNOWN;
	pd.m_bytesSnd = 110;
	pd.m_bytesRcv = 110;
	pd.m_reconSnd = 210;
	pd.m_reconRcv = 210;
	pd.m_leaveTime = std::chrono::system_clock::now();
	return callLog->LogParticipantLeave(pd);
}

void GetCurrentAndPrevMounthFile(char *curent, char *prevMounth){
	SYSTEMTIME st; GetSystemTime(&st);
	int y = st.wYear % 100;
	int m = st.wMonth;
	int prevMonthCount = 1;

	sprintf(curent, "calls_%2.2d%2.2d.sqlite", y, m);
	m = m <= prevMonthCount ? --y, 12 + m - prevMonthCount : m - prevMonthCount;	// get previous mounth
	sprintf(prevMounth, "calls_%2.2d%2.2d.sqlite", y, m);
}
}
#endif
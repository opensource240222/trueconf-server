#include "std/CallLog/VS_DBCallLogPostgres.h"
#include "std/CallLog/VS_ConferenceDescription.h"
#include "std/statistics/TConferenceStatistics.h"

#include <gtest/gtest.h>
#include <memory>

namespace callLogTest {
	/*define your connection string like:
		#define CONNECTION_STRING "postgresql:host=192.168.0.2;dbname=tcs;user=postgres;password=qweASD123"
		database MUST be inited with LAST script provided here https://bug-tracking.trueconf.com/show_bug.cgi?id=43530 in attachment
	*/
#ifdef CONNECTION_STRING
	TEST(Posgres, FullConferenceCycle) {
		callLog::Postgres log;
		ASSERT_TRUE(log.Init(CONNECTION_STRING));

		VS_ConferenceDescription cd;
		cd.m_public = true;
		cd.m_name = "12345677@server.name.com#vcs";
		cd.m_appID = "133444555";
		cd.m_owner = "owner";
		cd.m_type = 1;
		cd.m_SubType = 2;
		cd.m_MaxParticipants = 3;
		cd.m_topic = "TestTopic";
		EXPECT_TRUE(log.LogConferenceStart(cd, false));

		EXPECT_TRUE(log.LogParticipantInvite(cd.m_name, "call_id1", "app_id1", "call_id2", std::chrono::system_clock::now()));

		const char* call_id_for_missed_call("call_id3");
		VS_ParticipantDescription joined_pd1, joined_pd2;
		joined_pd2.m_conf_id = joined_pd1.m_conf_id = cd.m_name;
		joined_pd1.m_appID = "joined_app_id"; joined_pd2.m_appID = "joined_app_id2";
		joined_pd1.m_user_id = "call_id1"; joined_pd2.m_user_id = "call_id2";
		joined_pd1.m_displayName = joined_pd2.m_displayName = "TestDN";
		EXPECT_TRUE(log.LogParticipantJoin(joined_pd1, ""));
		EXPECT_TRUE(log.LogParticipantJoin(joined_pd2, ""));

		EXPECT_TRUE(log.LogParticipantInvite(cd.m_name, joined_pd1.m_user_id, joined_pd1.m_appID, call_id_for_missed_call, std::chrono::system_clock::now()));
		EXPECT_TRUE(log.LogParticipantReject(cd.m_name, call_id_for_missed_call, "", REJECTED_BY_PARTICIPANT));

		EXPECT_TRUE(log.LogParticipantLeave(joined_pd1));
		EXPECT_TRUE(log.LogParticipantLeave(joined_pd2));

		joined_pd1.m_bytesSnd = joined_pd1.m_bytesRcv = joined_pd2.m_bytesSnd = joined_pd2.m_bytesRcv = 100;
		joined_pd1.m_reconSnd = joined_pd1.m_reconRcv = joined_pd2.m_reconSnd = joined_pd2.m_reconRcv = 10;
		EXPECT_TRUE(log.LogParticipantStatistics(joined_pd1));
		EXPECT_TRUE(log.LogParticipantStatistics(joined_pd2));

		VS_Container conf_info;
		conf_info.AddValue(CONFERENCE_PARAM,cd.m_name.m_str);
		conf_info.AddValue(CALLID_PARAM, joined_pd1.m_user_id.m_str);
		TConferenceStatistics c_stat;
		c_stat.participant_time = c_stat.broadcast_time = c_stat.video_w = c_stat.video_h = c_stat.loss_rcv_packets
			= c_stat.avg_cpu_load = c_stat.avg_jitter = 10;
		c_stat.avg_send_fps = 10.0;
		EXPECT_TRUE(log.LogParticipantStatistics(conf_info, &c_stat, "DN"));

		VS_Container conf_info1;
		conf_info1.AddValue(CONFERENCE_PARAM, cd.m_name.m_str);
		conf_info1.AddValue(CALLID_PARAM, joined_pd2.m_user_id.m_str);
		EXPECT_TRUE(log.LogParticipantStatistics(conf_info1, &c_stat, "DN"));

		EXPECT_TRUE(log.LogConferenceRecordStart(cd.m_name, "test_file.log", std::chrono::system_clock::now()));
		cd.m_logCause = VS_ConferenceDescription::TERMINATED_BY_ADMIN;
		EXPECT_TRUE(log.LogConferenceEnd(cd));

		VS_Container res;
		int entries(0);
		EXPECT_EQ(SEARCH_DONE, log.GetCalls(res, entries, AB_PLACED_CALLS, joined_pd1.m_user_id, time(0)));
		EXPECT_EQ(entries, 2);	// one successfull call to joined_pd2 and one rejected to "call_id3"

		entries = 0;
		res.Clear();
		EXPECT_EQ(SEARCH_DONE, log.GetCalls(res, entries, AB_RECEIVED_CALLS, joined_pd2.m_user_id, time(0)));
		EXPECT_EQ(entries, 1);

		entries = 0;
		res.Clear();
		EXPECT_EQ(SEARCH_DONE, log.GetCalls(res, entries, AB_MISSED_CALLS, call_id_for_missed_call, time(0)));
		EXPECT_EQ(entries, 1);
	}
#endif
}
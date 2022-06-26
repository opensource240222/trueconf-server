#pragma once

#include <gtest\gtest.h>
#include <gmock\gmock.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <chrono>
#include <fstream>
#include <tuple>

#include "../../common/std/CallLog/VS_DB_CallLogSQLite.h"
#include "../../common/std/CallLog/VS_ConferenceDescription.h"
#include "../../common/std/CallLog/VS_ParticipantDescription.h"
#include "../std/cpplib/VS_UserData.h"

namespace tc3_test
{
/* for new container type */
typedef std::tuple<VS_SimpleStr, VS_SimpleStr, std::chrono::system_clock::time_point, VS_SimpleStr, VS_SimpleStr, std::chrono::system_clock::time_point, std::chrono::system_clock::time_point, long, long, VS_SimpleStr, VS_SimpleStr> selected_container;
typedef std::pair<VS_SimpleStr, std::chrono::system_clock::time_point> curr_container;

void MakeCall(VS_DB_CallLogSQLite *callLog, VS_ParticipantDescription& pd1, VS_ParticipantDescription& pd2);
void MakeNonJoinableCall(VS_DB_CallLogSQLite *callLog, VS_ParticipantDescription& pd1, VS_ParticipantDescription& pd2, bool do_log_reject);
void GetSelectValues(VS_Container& container, std::vector <selected_container> &v);	/* for new container type */
void GetSelectValues(VS_Container& container, std::vector <curr_container> &v);
void GetCurrentAndPrevMounthFile(char *curent, char *prevMounth);
bool TalkAndLeave(VS_DB_CallLogSQLite *callLog, VS_ParticipantDescription& pd);
bool MakeStatisticsLog(VS_DB_CallLogSQLite *callLog, VS_ParticipantDescription& pd);
bool IsLogExists(const char *log_file_name);

struct DBCallLogSqliteTest : public testing::Test
{
	VS_DB_CallLogSQLite *callLog;
	VS_SimpleStr app_id;
	VS_ParticipantDescription pd1, pd2;
	std::string id1, id2, conf;
	int entries;
	VS_ConferenceDescription confD;

	bool MakeMissedCall_PD1_to_PD2(std::string pd2_id, std::string conference_id, bool do_log_reject);
	bool MakePlacedCall_PD1_to_PD2(std::string pd2_id, std::string conference_id);

	void SetUp(){
		callLog = new VS_DB_CallLogSQLite();
		app_id = "10102002";
		id1 = "10";
		id2 = "20";
		conf = "75575575512345676533@server.ru";
		pd1.m_user_id = id1.c_str();
		pd2.m_user_id = id2.c_str();
		pd2.m_displayName = "Participant2";
		pd1.m_appID = pd2.m_appID = app_id;
		pd1.m_conf_id = pd2.m_conf_id = conf.c_str();
		pd1.m_displayName = "Participant1";
		entries = 0;

		//confD.m_appID = app_id;
		confD.m_name = conf.c_str();
		confD.m_topic = vs::WideCharToUTF8Convert(L"Название Конференции");
		confD.m_owner = "conf_owner";
		//confD.m_type = 1;
		//confD.m_SubType = 2;
		//confD.m_public = true;
		//confD.m_MaxParticipants = 100;
		confD.m_logStarted = std::chrono::system_clock::now();
		confD.m_call_id = "5c2e568b7e";
	}
	void TearDown(){
		delete callLog;
	}
};
}
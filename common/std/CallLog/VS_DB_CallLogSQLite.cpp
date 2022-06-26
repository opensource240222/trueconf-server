#include "VS_DB_CallLogSQLite.h"
#include "VS_ConferenceDescription.h"
#include "../../std/cpplib/VS_Replace.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"
#include "std-generic/cpplib/utf8.h"
#include "std-generic/clib/vs_time.h"
#include "std-generic/sqlite/CppSQLite3.h"
#include "../../std/debuglog/VS_Debug.h"
#include "../../std/VS_ProfileTools.h"
#include "../../std/statistics/TConferenceStatistics.h"

#define DEBUG_CURRENT_MODULE VS_DM_LOGSERVICE

const char VS_DB_CallLogSQLite::m_SQLITE_TABLE_CALLS[] = "TB_PARTICIPANT_LOG";
const char VS_DB_CallLogSQLite::m_SQLITE_CONF_TABLE_CALLS[] = "TB_CONFERENCE_LOG";
const char VS_DB_CallLogSQLite::m_CALL_ID2_PARAM[] = "invited_call_id";	// changed
const char VS_DB_CallLogSQLite::m_DB_CONFERENCE_PARAM[] = "conference";
const char VS_DB_CallLogSQLite::m_DB_SERVER_PARAM[] = "broker_id";
const char VS_DB_CallLogSQLite::m_DISPLAY_NAME_PARAM[] = "display_name";
const char VS_DB_CallLogSQLite::m_CALL_ID_PARAM[] = "call_id";
const char VS_DB_CallLogSQLite::m_APP_ID_PARAM[] = "app_id";
const char VS_DB_CallLogSQLite::m_DB_TIME_PARAM[] = "invite_time";
const char VS_DB_CallLogSQLite::m_PART_JOIN_DB_TIME_PARAM[] = "join_time";
const char VS_DB_CallLogSQLite::m_PART_LEAVE_DB_TIME_PARAM[] = "leave_time";
const char VS_DB_CallLogSQLite::m_DB_TYPE_PARAM[] = "type";
const char VS_DB_CallLogSQLite::m_PART_LEAVE_REASON_PARAM[] = "leave_reason";
const char VS_DB_CallLogSQLite::m_PART_BYTES_SENT_PARAM[] = "bytes_sent";
const char VS_DB_CallLogSQLite::m_PART_BYTES_RECEIVED_PARAM[] = "bytes_received";
const char VS_DB_CallLogSQLite::m_PART_RECON_SND_PARAM[] = "reconnect_snd";
const char VS_DB_CallLogSQLite::m_PART_RECON_RCV_PARAM[] = "reconnect_rcv";
const char VS_DB_CallLogSQLite::m_PART_ID_PARAM[] = "part_id";
const char VS_DB_CallLogSQLite::m_PART_TIME_PARAM[] = "participant_time";
const char VS_DB_CallLogSQLite::m_BROADCAST_TIME_PARAM[] = "broadcast_time";
const char VS_DB_CallLogSQLite::m_VIDEO_WIDTH_PARAM[] = "video_w";
const char VS_DB_CallLogSQLite::m_VIDEO_HIGHT_PARAM[] = "video_h";
const char VS_DB_CallLogSQLite::m_LOSS_RECEIVE_PARAM[] = "loss_rcv_packets";
const char VS_DB_CallLogSQLite::m_AVG_CPU_LOAD_PARAM[] = "avg_cpu_load";
const char VS_DB_CallLogSQLite::m_AVG_JITTER_PARAM[] = "avg_jitter";
const char VS_DB_CallLogSQLite::m_AVG_SEND_FPS_PARAM[] = "avg_send_fps";

const char VS_DB_CallLogSQLite::m_DB_OWNER_PARAM[] = "owner";
const char VS_DB_CallLogSQLite::m_DB_SUBTYPE_PARAM[]  = "subtype";
const char VS_DB_CallLogSQLite::m_DB_CONF_START_TIME_PARAM[]  = "start_time";
const char VS_DB_CallLogSQLite::m_DB_CONF_END_TIME_PARAM[] = "end_time";
const char VS_DB_CallLogSQLite::m_DB_MAX_USERS_PARAM[]  = "max_participants";
const char VS_DB_CallLogSQLite::m_DB_PUBLIC_PARAM[]  = "is_public";
const char VS_DB_CallLogSQLite::m_DB_TOPIC_PARAM[]  = "topic";
const char VS_DB_CallLogSQLite::m_CONF_TERM_REASON_PARAM[] = "term_reason";
const char VS_DB_CallLogSQLite::m_CONF_RECORD_FILE_PARAM[] = "record_file";
const char VS_DB_CallLogSQLite::m_CONF_GUEST_FLAGS_PARAM[] = "guest_flags";

#define EMPTY_TIME std::chrono::system_clock::time_point()


bool VS_DB_CallLogSQLite::LogConferenceStart(const VS_ConferenceDescription& conf, bool remote /*Unused*/){
	if (!conf.m_name || !conf.m_owner || conf.m_logStarted == EMPTY_TIME){	// this params can't be null ( see CREATE TABLE "TB_CONFERENCE_LOG" script)
		return false;
	}

	CppSQLite3DB db;
	if (!this->Calls_InitSQLiteDB(db)){
		return false;
	}

	bool is_public = conf.m_public;
	int guest_flags = 0;
	VS_ConferenceDescription::GetMultyConfInfo(conf.m_call_id, is_public, guest_flags);


	int conference; std::string broker_id;
	VS_ConferenceDescription::SplitConfID(conf.m_name, conference, broker_id);

	try
	{
		char conf_started[50];
		tu::TimeToStr_ISO8601_Z(conf.m_logStarted, conf_started, 50);
		CppSQLite3Buffer bufSQL;

		std::string format = "insert into %s (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s) values"
			/* conference, broker_id, owner, start_time */
			"(%d, %Q, %Q, %Q, "
			/* type, subtype, app_id, is_public,call_id, max_participants,topic,guest_flags */
			"%d, %d, %Q, %Q, __call_id__, %d, __topic__, %d);";
		VS_ReplaceAll(format, "__call_id__", (conf.m_call_id != conf.m_name) ? "%Q" : "%s");
		VS_ReplaceAll(format, "__topic__", (!conf.m_topic.empty()) ? "%Q" : "%s");
		bufSQL.format(format.c_str(),
			m_SQLITE_CONF_TABLE_CALLS,
			/* specify names we want to insert*/
			m_DB_CONFERENCE_PARAM, m_DB_SERVER_PARAM, m_DB_OWNER_PARAM, m_DB_CONF_START_TIME_PARAM,
			m_DB_TYPE_PARAM, m_DB_SUBTYPE_PARAM, m_APP_ID_PARAM, m_DB_PUBLIC_PARAM, "named_conf_id", m_DB_MAX_USERS_PARAM, m_DB_TOPIC_PARAM,
			m_CONF_GUEST_FLAGS_PARAM,
			/* values to insert */
			conference, broker_id.c_str(), conf.m_owner.m_str, conf_started,
			conf.m_type, conf.m_SubType,
			(!!conf.m_appID ? conf.m_appID.m_str : "null"),
			(is_public ? "true" : "false"),
			(conf.m_call_id != conf.m_name) ? conf.m_call_id.m_str : "null", conf.m_MaxParticipants,
			((!conf.m_topic.empty())? conf.m_topic.c_str() : "null"), guest_flags);
		db.execDML(bufSQL);
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}

	return true;
}
bool VS_DB_CallLogSQLite::LogConferenceEnd(const VS_ConferenceDescription& conf){
	CppSQLite3DB db;
	if (!this->Calls_InitSQLiteDB(db) || !conf.m_name || !conf.m_owner)
		return false;

	int conference; std::string broker_id;
	VS_ConferenceDescription::SplitConfID(conf.m_name, conference, broker_id);

	try
	{
		char conf_ended[50];
		if (conf.m_logEnded == EMPTY_TIME || tu::TimeToStr_ISO8601_Z(conf.m_logEnded, conf_ended, 50) == 0) strcpy(conf_ended, "null");
		CppSQLite3Buffer bufSQL;

		bufSQL.format("update %s set \"%s\"=%d, \"%s\"=%Q  where \"%s\"=%d AND \"%s\"=%Q AND \"%s\"=%Q;",
			/* table to update */
			m_SQLITE_CONF_TABLE_CALLS,
			/* params to set */
			m_CONF_TERM_REASON_PARAM, conf.m_logCause,
			m_DB_CONF_END_TIME_PARAM, conf_ended,
			/* where part*/
			m_DB_CONFERENCE_PARAM, conference,
			m_DB_SERVER_PARAM, broker_id.c_str(),
			m_DB_OWNER_PARAM, conf.m_owner.m_str);
		db.execDML(bufSQL);
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}

	return true;
}

bool VS_DB_CallLogSQLite::LogParticipantInvite(const vs_conf_id& conf_id, const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,
	const std::chrono::system_clock::time_point time, VS_ParticipantDescription::Type type)
{
	dprint3("$LogParticipantInvite %s\n", conf_id.m_str);

	if (!conf_id || !call_id1 || !call_id2)
		return false;

	CppSQLite3DB db;
	if (!this->Calls_InitSQLiteDB(db)){
		return false;
	}

	int conference; std::string broker_id;
	VS_ConferenceDescription::SplitConfID(conf_id, conference, broker_id);

	try
	{
		char timeBuff[50];
		auto now = std::chrono::system_clock::now();
		tu::TimeToStr_ISO8601_Z(now, timeBuff, 50);

		// close old records
		CppSQLite3Buffer bufSQL_close_old;
		bufSQL_close_old.format("update %s set leave_time=%Q, reject_cause=%d where join_time is null AND leave_time is null AND conference=%d AND broker_id=%Q",
			m_SQLITE_TABLE_CALLS, timeBuff, (long)UNDEFINED_CAUSE, conference, broker_id.c_str());
		db.execDML(bufSQL_close_old);

		if (time == EMPTY_TIME || tu::TimeToStr_ISO8601_Z(time, timeBuff, 50) == 0) strcpy(timeBuff, "null");
		CppSQLite3Buffer bufSQL;
		bufSQL.format("insert into %s (%s, %s, %s, %s, %s, %s, %s) values("
			/*call_id, invited_call_id, app_id, type, invite_time, conference, broker_id */
			"%Q, %Q, %Q, %d, %Q, %d, %Q);",
			/* table to insert */
			m_SQLITE_TABLE_CALLS,
			/* specify names we want to insert*/
			m_CALL_ID_PARAM, m_CALL_ID2_PARAM, m_APP_ID_PARAM, m_DB_TYPE_PARAM, m_DB_TIME_PARAM, m_DB_CONFERENCE_PARAM, m_DB_SERVER_PARAM,
			/* values */
			call_id1.m_str, call_id2.m_str, (!!app_id ? app_id.m_str : "null"),
			type,
			timeBuff,
			conference, broker_id.c_str());
		db.execDML(bufSQL);
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}
	return true;
}

bool VS_DB_CallLogSQLite::LogParticipantJoin(const VS_ParticipantDescription& pd, const VS_SimpleStr& callid2){
	dprint3("$LogParticipantJoin\n");

	if (!pd.m_conf_id || !pd.m_user_id){
		return false;
	}

	CppSQLite3DB db;
	if (!this->Calls_InitSQLiteDB(db)){
		return false;
	}

	char callid2_sql[50];
	sqlite3_snprintf(50, callid2_sql, "AND \"%s\"=%Q", m_CALL_ID_PARAM, callid2.m_str);

	char display_name_sql[1024] = { 0 };
	char display_name_sql_insert[1024] = { 0 };
	if (!pd.m_displayName.empty())
	{
		sqlite3_snprintf(1024, display_name_sql, ", \"%s\"=%Q", m_DISPLAY_NAME_PARAM, pd.m_displayName.c_str());
		sqlite3_snprintf(1024, display_name_sql_insert, "%Q", pd.m_displayName.c_str());
	}

	int conference; std::string broker_id;
	VS_ConferenceDescription::SplitConfID(pd.m_conf_id, conference, broker_id);

	char joinTime_buff[50] = { 0 };
	if (pd.m_joinTime == EMPTY_TIME || tu::TimeToStr_ISO8601_Z(pd.m_joinTime, joinTime_buff, 50) == 0) strcpy(joinTime_buff, "null");

	try
	{
		CppSQLite3Buffer bufSQL;

		bufSQL.format("select type from %s where conference=%d AND broker_id=%Q;",
			m_SQLITE_CONF_TABLE_CALLS,
			/* where part */
			conference, broker_id.c_str());
		db.execDML(bufSQL);
		CppSQLite3Query q = db.execQuery(bufSQL);
		auto conf_type = fetch_long(q, "type");
		bool IsGConf = (conf_type.is_initialized() && ((VS_Conference_Type)conf_type.get() == CT_MULTISTREAM || (VS_Conference_Type)conf_type.get() == CT_INTERCOM)) ? true : false;
		VS_SimpleStr param_call_id = (IsGConf) ? "invited_call_id" : "call_id";

		bufSQL.format("select part_id from %s where \"%s\"=%Q AND conference=%d AND broker_id=%Q order by part_id desc",
			m_SQLITE_TABLE_CALLS,
			/* where part */
			param_call_id.m_str,
			pd.m_user_id.m_str,
			conference, broker_id.c_str());
		q = db.execQuery(bufSQL);
		auto part_id = fetch_long(q, "part_id");

		if (part_id.is_initialized() || IsGConf)
		{
			bufSQL.format("update %s set join_time=%Q %s where join_time is null AND leave_time is null AND \"%s\"=%Q AND conference=%d AND broker_id=%Q %s;",
				m_SQLITE_TABLE_CALLS,
				joinTime_buff,
				(!pd.m_displayName.empty())? display_name_sql : "",
				/* where part */
				param_call_id.m_str,
				pd.m_user_id.m_str,
				conference, broker_id.c_str(),
				(!!callid2 ? callid2_sql : ""));
			db.execDML(bufSQL);
		}

		if (IsGConf || !part_id.is_initialized())
		{
			bufSQL.format("insert into %s (call_id, app_id, type, invite_time, join_time, leave_time, conference, broker_id, display_name) values(%Q, %Q, %d, null, %Q, null, %d, %Q, %s);",
				m_SQLITE_TABLE_CALLS,
				pd.m_user_id.m_str,
				pd.m_appID.m_str,
				pd.m_type,
				joinTime_buff,
				conference,
				broker_id.c_str(),
				(display_name_sql_insert[0]) ? display_name_sql_insert : "null"
			);
			db.execDML(bufSQL);
		}
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}

	return true;
}

bool VS_DB_CallLogSQLite::LogParticipantReject(const vs_conf_id& conf_id, const vs_user_id& user, const vs_user_id& invited_from, const VS_Reject_Cause cause){
	dprint3("$LogParticipantReject\n");

	if (!conf_id || !user)
		return false;

	CppSQLite3DB db;
	if (!this->Calls_InitSQLiteDB(db)){
		return false;
	}

	int conference; std::string broker_id;
	VS_ConferenceDescription::SplitConfID(conf_id, conference, broker_id);

	try
	{
		char timeBuff[50];
		CppSQLite3Buffer bufSQL;
		tu::TimeToStr_ISO8601_Z(std::chrono::system_clock::now(), timeBuff, sizeof(timeBuff));

		bufSQL.format("update %s set leave_time=%Q, reject_cause=%d where join_time is null AND leave_time is null AND invited_call_id=%Q AND conference=%d AND broker_id=%Q;",
			m_SQLITE_TABLE_CALLS,
			/* values */
			timeBuff,
			cause,
			/* where part */
			user.m_str,
			conference, broker_id.c_str());
		db.execDML(bufSQL);
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}
	return true;
}

bool VS_DB_CallLogSQLite::LogParticipantLeave(const VS_ParticipantDescription& pd){
	dprint3("$LogParticipantLeave\n");

	CppSQLite3DB db;
	if (!this->Calls_InitSQLiteDB(db)){
		return false;
	}

	if (!pd.m_conf_id || !pd.m_user_id)
		return false;

	int conference; std::string broker_id;
	VS_ConferenceDescription::SplitConfID(pd.m_conf_id, conference, broker_id);

	try
	{
		char leaveTime[50];
		if (pd.m_leaveTime == EMPTY_TIME || tu::TimeToStr_ISO8601_Z(pd.m_leaveTime, leaveTime, 50) == 0) strcpy(leaveTime, "null");
		CppSQLite3Buffer bufSQL;

		bufSQL.format("update %s set leave_reason=%d, "
			"bytes_sent=CASE WHEN bytes_sent>0 AND bytes_sent IS NOT NULL THEN bytes_sent ELSE %d END, "
			"bytes_received=CASE WHEN bytes_received>0 AND bytes_received IS NOT NULL THEN bytes_received ELSE %d END, "
			"reconnect_snd=CASE WHEN reconnect_snd>0 AND reconnect_snd IS NOT NULL THEN reconnect_snd ELSE %d END, "
			"reconnect_rcv=CASE WHEN reconnect_rcv>0 AND reconnect_rcv IS NOT NULL THEN reconnect_rcv ELSE %d END, "
			"leave_time=%Q  where leave_time is null AND call_id=%Q AND conference=%d AND broker_id=%Q;",
			m_SQLITE_TABLE_CALLS,
			/* values */
			pd.m_cause,
			pd.m_bytesSnd, pd.m_bytesRcv,
			pd.m_reconSnd, pd.m_reconRcv,
			leaveTime,
			/* where part */
			pd.m_user_id.m_str,
			conference, broker_id.c_str());
		db.execDML(bufSQL);
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}
	return true;
}

bool VS_DB_CallLogSQLite::LogParticipantStatistics(const VS_ParticipantDescription& pd){
	dprint3("$LogParticipantStatistics\n");

	CppSQLite3DB db;
	if (!this->Calls_InitSQLiteDB(db)){
		return false;
	}

	if (!pd.m_conf_id || !pd.m_user_id)
		return false;

	int conference; std::string broker_id;
	VS_ConferenceDescription::SplitConfID(pd.m_conf_id, conference, broker_id);

	try
	{
		CppSQLite3Buffer bufSQL;
		bufSQL.format("select part_id from %s where call_id=%Q AND conference=%d AND broker_id=%Q AND invited_call_id is null order by part_id desc limit 1",
			m_SQLITE_TABLE_CALLS,
			/* where part */
			pd.m_user_id.m_str,
			conference, broker_id.c_str());
		auto q = db.execQuery(bufSQL);
		auto part_id = fetch_long(q, "part_id");

		std::string format = "update %s set \"%s\"=%d, \"%s\"=%d, \"%s\"=%d, \"%s\"=%d  where \"%s\"=%d AND \"%s\"=%Q";
		if (part_id.is_initialized()) {
			format += " AND part_id=";
			format += std::to_string(*part_id);
		}

		bufSQL.format(format.c_str(),
			m_SQLITE_TABLE_CALLS,
			/* values */
			m_PART_BYTES_SENT_PARAM, pd.m_bytesSnd,
			m_PART_BYTES_RECEIVED_PARAM, pd.m_bytesRcv,
			m_PART_RECON_SND_PARAM, pd.m_reconSnd,
			m_PART_RECON_RCV_PARAM, pd.m_reconRcv,
			/* where part */
			m_DB_CONFERENCE_PARAM, conference,
			m_CALL_ID_PARAM, pd.m_user_id.m_str);
		db.execDML(bufSQL);
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}
	return true;
}

bool VS_DB_CallLogSQLite::LogParticipantStatistics(VS_Container& cnt, const TConferenceStatistics* stat, const std::string& displayName){
	const char* conf_id = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char* call_id = cnt.GetStrValueRef(CALLID_PARAM);

	if (!stat || !conf_id || !call_id){
		return false;
	}

	int conference; std::string broker_id;
	VS_ConferenceDescription::SplitConfID(conf_id, conference, broker_id);

	CppSQLite3DB db;
	if (!this->Calls_InitSQLiteDB(db)){
		return false;
	}

	char display_name_sql[1024] = {0};
	if (!displayName.empty())
		sqlite3_snprintf(1024, display_name_sql, " \"%s\"=%Q, ", m_DISPLAY_NAME_PARAM, displayName.c_str());

	try
	{
		CppSQLite3Buffer bufSQL;
		bufSQL.format("select part_id from %s where call_id=%Q AND conference=%d AND broker_id=%Q AND invited_call_id is null order by part_id desc limit 1",
			m_SQLITE_TABLE_CALLS,
			/* where part */
			call_id,
			conference, broker_id.c_str());
		auto q = db.execQuery(bufSQL);
		auto part_id = fetch_long(q, "part_id");

		std::string format = "update %s set %s \"%s\"=%d, \"%s\"=%d, \"%s\"=%d, \"%s\"=%d, \"%s\"=%d, \"%s\"=%d, \"%s\"=%d, \"%s\"=%lf "
			"where \"%s\"=%d AND \"%s\"=%Q";
		if (part_id.is_initialized()) {
			format += " AND part_id=";
			format += std::to_string(*part_id);
		}

		bufSQL.format(format.c_str(),
			/* table name */
			m_SQLITE_TABLE_CALLS,
			/* values to update */
			!displayName.empty() ? display_name_sql : "",
			m_PART_TIME_PARAM, stat->participant_time,
			m_BROADCAST_TIME_PARAM, stat->broadcast_time,
			m_VIDEO_WIDTH_PARAM, stat->video_w,
			m_VIDEO_HIGHT_PARAM, stat->video_h,
			m_LOSS_RECEIVE_PARAM, stat->loss_rcv_packets,
			m_AVG_CPU_LOAD_PARAM, stat->avg_cpu_load,
			m_AVG_JITTER_PARAM, stat->avg_jitter,
			m_AVG_SEND_FPS_PARAM, stat->avg_send_fps,
			/* where part */
			m_DB_CONFERENCE_PARAM, conference,
			m_CALL_ID_PARAM, call_id);
		db.execDML(bufSQL);
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}
	return true;
}

int VS_DB_CallLogSQLite::GetCalls(VS_Container& out_cnt, int& entries, VS_AddressBook ab, const vs_user_id& call_owner, int_fast64_t last_deleted_call, int prevMonthCount) const{
	const static int MIN_ENTRIES = 24;
	const char db_null[] = "null";
	entries = entries < 0 ? 0 : entries;

	if (!call_owner){
		return SEARCH_FAILED;
	}

	CppSQLite3Buffer bufSQL;
	std::string time_comp;
	if (last_deleted_call != -1) {
		time_comp = " AND Coalesce(tpl.invite_time,tpl.join_time) > strftime('%time_fmt%', datetime(%last_deleted_call%, 'unixepoch'))"; // later than last_deleted_call
		const std::string iso8601_z_timefmt = "%Y-%m-%dT%H:%M:%SZ";	// ISO8601_Z_TIME_FMT

		VS_ReplaceAll(time_comp, "%last_deleted_call%", std::to_string(last_deleted_call));
		VS_ReplaceAll(time_comp, "%time_fmt%", iso8601_z_timefmt);
	}

	if (ab == AB_MISSED_CALLS) {
		bufSQL.format(
			"SELECT tpl.%s, tpl.%s, tpl.%s as call_time, tpl.%s, tpl.%s, l.%s, l.%s, l.%s, l.%s, l.%s, l.%s"
			" FROM \"%s\" tpl	JOIN \"%s\" l ON "
			"tpl.%s = l.%s AND	tpl.%s = l.%s "
			"WHERE tpl.%s = %Q AND tpl.%s IS NULL %s"
			"ORDER BY tpl.%s DESC limit 24;",
			/* select params*/
			m_CALL_ID_PARAM, m_DISPLAY_NAME_PARAM, m_DB_TIME_PARAM, m_DB_SERVER_PARAM, m_DB_CONFERENCE_PARAM, m_DB_CONF_START_TIME_PARAM,
			m_DB_CONF_END_TIME_PARAM, m_DB_TYPE_PARAM, m_DB_SUBTYPE_PARAM, m_DB_TOPIC_PARAM, m_DB_OWNER_PARAM,
			/* tables */
			m_SQLITE_TABLE_CALLS, m_SQLITE_CONF_TABLE_CALLS,
			/* join places*/
			m_DB_SERVER_PARAM, m_DB_SERVER_PARAM, m_DB_CONFERENCE_PARAM, m_DB_CONFERENCE_PARAM,
			/* where part*/
			m_CALL_ID2_PARAM, call_owner.m_str, m_PART_JOIN_DB_TIME_PARAM, time_comp.c_str(),
			/* order by part*/
			m_DB_TIME_PARAM);
	}
	else if (ab == AB_RECEIVED_CALLS) {
		bufSQL.format(
			"SELECT tpl.%s, tpl.%s, tpl.%s as call_time, tpl.%s, tpl.%s, l.%s, l.%s, l.%s, l.%s, l.%s, l.%s"
			" FROM \"%s\" tpl	JOIN \"%s\" l ON "
			"tpl.%s = l.%s AND	tpl.%s = l.%s "
			"WHERE tpl.%s = %Q AND tpl.%s IS NOT NULL AND reject_cause is null %s"
			"ORDER BY tpl.%s DESC limit 24;",
			/* select params*/
			m_CALL_ID_PARAM, m_DISPLAY_NAME_PARAM, m_DB_TIME_PARAM, m_DB_SERVER_PARAM, m_DB_CONFERENCE_PARAM, m_DB_CONF_START_TIME_PARAM,
			m_DB_CONF_END_TIME_PARAM, m_DB_TYPE_PARAM, m_DB_SUBTYPE_PARAM, m_DB_TOPIC_PARAM, m_DB_OWNER_PARAM,
			/* tables */
			m_SQLITE_TABLE_CALLS, m_SQLITE_CONF_TABLE_CALLS,
			/* join places*/
			m_DB_SERVER_PARAM, m_DB_SERVER_PARAM, m_DB_CONFERENCE_PARAM, m_DB_CONFERENCE_PARAM,
			/* where part*/
			m_CALL_ID2_PARAM, call_owner.m_str, m_PART_JOIN_DB_TIME_PARAM, time_comp.c_str(),
			/* order by part*/
			m_DB_TIME_PARAM);
	}
	else if (ab == AB_PLACED_CALLS) {
		bufSQL.format(
			"SELECT tpl.%s as %s, tpl.%s, Coalesce(tpl.%s,tpl.%s) as call_time, tpl.%s, tpl.%s, l.%s, l.%s, l.%s, l.%s, l.%s, l.%s"
			" FROM \"%s\" tpl	JOIN \"%s\" l ON "
			"tpl.%s = l.%s AND	tpl.%s = l.%s "
			"WHERE tpl.%s = %Q AND tpl.%s IS NOT NULL %s"
			"ORDER BY Coalesce(tpl.%s,tpl.%s) DESC limit 24;",
			/* select params*/
			m_CALL_ID2_PARAM, m_CALL_ID_PARAM, m_DISPLAY_NAME_PARAM, m_DB_TIME_PARAM, m_PART_JOIN_DB_TIME_PARAM, m_DB_SERVER_PARAM, m_DB_CONFERENCE_PARAM, m_DB_CONF_START_TIME_PARAM,
			m_DB_CONF_END_TIME_PARAM, m_DB_TYPE_PARAM, m_DB_SUBTYPE_PARAM, m_DB_TOPIC_PARAM, m_DB_OWNER_PARAM,
			/* tables */
			m_SQLITE_TABLE_CALLS, m_SQLITE_CONF_TABLE_CALLS,
			/* join places*/
			m_DB_SERVER_PARAM, m_DB_SERVER_PARAM, m_DB_CONFERENCE_PARAM, m_DB_CONFERENCE_PARAM,
			/* where part*/
			m_CALL_ID_PARAM, call_owner.m_str, m_CALL_ID2_PARAM, time_comp.c_str(),
			/* order by part*/
			m_DB_TIME_PARAM, m_PART_JOIN_DB_TIME_PARAM);
	}
	else{
		return SEARCH_FAILED;
	}

	CppSQLite3DB db;
	if (!this->Calls_InitSQLiteDB(db, prevMonthCount, false)){
		if (HaveMonthLog(prevMonthCount + 1))
			return GetCalls(out_cnt, entries, ab, call_owner, last_deleted_call, prevMonthCount + 1);
		else{
			return SEARCH_FAILED;
		}
	}

	try{
		CppSQLite3Query q = db.execQuery(bufSQL);

		const char *call_id, *display_name, *broker_id, *conference, *topic, *owner;
		int type, subtype;
		time_t call_time, start_time, end_time;
		const char* ptr;

		while (!q.eof()){
			try{
				call_id = q.fieldValue(m_CALL_ID_PARAM);
				if (!call_id) throw 1;						// can't be null

				display_name = q.fieldValue(m_DISPLAY_NAME_PARAM);
				if (!display_name) display_name = db_null;

				ptr = q.fieldValue("call_time");
				if (!ptr) throw 1;
				else call_time = tu::ISO8601_ZStrToTimeT(ptr);

				broker_id = q.fieldValue(m_DB_SERVER_PARAM);
				if (!broker_id) throw 1;					// can't be null

				conference = q.fieldValue(m_DB_CONFERENCE_PARAM);
				if (!conference) throw 1;					// can't be null

				//ptr = q.fieldValue(m_DB_CONF_START_TIME_PARAM);
				//if (!ptr) throw 1;							// can't be null
				//else start_time = tu::ISO8601_ZStrToStructTM(ptr);

				//ptr = q.fieldValue(m_DB_CONF_END_TIME_PARAM);
				//if (!ptr) memset(&end_time, 0, sizeof(end_time));
				//else end_time = tu::ISO8601_ZStrToStructTM(ptr);

				ptr = q.fieldValue(m_DB_TYPE_PARAM);
				if (!ptr) throw 1;							// can't be null
				else type = (int)strtol(ptr, NULL, 10);

				ptr = q.fieldValue(m_DB_SUBTYPE_PARAM);
				if (!ptr) throw 1;							// can't be null
				else subtype = (int)strtol(ptr, NULL, 10);

				topic = q.fieldValue(m_DB_TOPIC_PARAM);
				if (!topic) topic = db_null;

				owner = q.fieldValue(m_DB_OWNER_PARAM);
				if (!owner) throw 1;						// can't be null

				/* New container format breaf realization */
				//if (out_cnt.AddValue(CALLID_PARAM, call_id) &&
				//	out_cnt.AddValue(DISPLAYNAME_PARAM, display_name) &&
				//	out_cnt.AddValue(TIME_PARAM, &call_time, sizeof(call_time)) &&
				//	out_cnt.AddValue(BROKER_PARAM, broker_id) &&
				//	out_cnt.AddValue(CONFERENCE_PARAM, conference) &&
				//	out_cnt.AddValue(CONF_START_ST_PARAM, &start_time, sizeof(start_time)) &&	/* not found in VS_protocol*/
				//	out_cnt.AddValue(m_DB_CONF_END_TIME_PARAM, &end_time, sizeof(end_time)) &&	/* not found in VS_protocol*/
				//	out_cnt.AddValue(TYPE_PARAM, (long)type) &&
				//	out_cnt.AddValue(SUBTYPE_PARAM, (long)subtype) &&
				//	out_cnt.AddValue(TOPIC_PARAM, topic) &&
				//	out_cnt.AddValue(OWNER_PARAM, owner))
				//	++entries;

				if (out_cnt.AddValue(USERNAME_PARAM, call_id))
					++entries;

				if (call_time != static_cast<time_t>(-1))
					out_cnt.AddValue(TIME_PARAM, std::chrono::system_clock::from_time_t(call_time));

				q.nextRow();
			}
			catch (int){
				q.nextRow();
				continue;
			}
		}
		if (entries < MIN_ENTRIES){
			GetCalls(out_cnt, entries, ab, call_owner, last_deleted_call, prevMonthCount + 1);
		}
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return SEARCH_FAILED;
	}

	return SEARCH_DONE;
}

bool VS_DB_CallLogSQLite::HaveMonthLog(int count) const{
	CppSQLite3DB db;
	return Calls_InitSQLiteDB(db, count, false);
}

bool VS_DB_CallLogSQLite::Calls_InitSQLiteDB(CppSQLite3DB& db, int prevMonthCount, bool do_create) const
{
	if (prevMonthCount > 12)		// we search only for one year back
		return false;
	bool res(false);
	try
	{
		char file_name[32] = { 0 };
		{
			auto now = time(0);
			tm now_tm;
			gmtime_r(&now, &now_tm);

			int y = now_tm.tm_year % 100;
			int m = now_tm.tm_mon + 1;	// count mounthes from 1 to 12, not 0..11
			prevMonthCount = prevMonthCount % 13;

			if (prevMonthCount > 0){
				m = m <= prevMonthCount ? --y, 12 + m - prevMonthCount : m - prevMonthCount;
			}

			sprintf(file_name, "calls_%2.2d%2.2d.sqlite", y, m);
		}

		if (!do_create)			// check if file is exists
		{
			FILE* f = fopen(file_name, "r");
			if (!f){
				return false;
			}
			else
				fclose(f);
		}

		db.open(file_name);
		if ((!db.tableExists(m_SQLITE_TABLE_CALLS) || !db.tableExists(m_SQLITE_CONF_TABLE_CALLS) )&& do_create)
		{
			/* harcoded fields of table SQLITE_TABLE_CALLS */
			CppSQLite3Buffer bufSQL;
			const char *creation_script =
				"PRAGMA encoding = \"UTF-8\";"
				"CREATE TABLE IF NOT EXISTS \"TB_PARTICIPANT_LOG\"("
				"part_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
				"call_id character varying(256) NOT NULL,"
				"invited_call_id character varying(256),"
				"app_id character(32),"
				"type smallint NOT NULL DEFAULT 0,"
				"invite_time timestamp without time zone,"
				"join_time timestamp without time zone,"
				"leave_time timestamp without time zone,"
				"leave_reason smallint NOT NULL DEFAULT 0,"
				"reject_cause smallint,"
				"conference integer NOT NULL,"
				"broker_id character varying(128) NOT NULL,"
				"display_name character varying(256),"
				"bytes_sent integer NOT NULL DEFAULT 0,"
				"bytes_received integer NOT NULL DEFAULT 0,"
				"reconnect_snd integer NOT NULL DEFAULT 0,"
				"reconnect_rcv integer NOT NULL DEFAULT 0,"
				"participant_time int,"
				"broadcast_time int,"
				"video_w int,"
				"video_h int,"
				"loss_rcv_packets int,"
				"avg_cpu_load int,"
				"avg_jitter int,"
				"avg_send_fps double precision"
				/*",deleted character varying(256)"*/	// without deleted filed
				");"

				"CREATE INDEX IF NOT EXISTS \"IX_TB_PARTICIPANT_LOG_call_id_conference_broker\""
				"ON \"TB_PARTICIPANT_LOG\" (conference, broker_id, call_id);"
				"CREATE INDEX IF NOT EXISTS \"IX_TB_PARTICIPANT_LOG_invited_call_id\""
				"ON \"TB_PARTICIPANT_LOG\" (invited_call_id, join_time);"
				"CREATE INDEX  IF NOT EXISTS  \"IX_TB_PARTICIPANT_LOG_call_id_invited_call_id\""
				"ON \"TB_PARTICIPANT_LOG\" (call_id, invited_call_id);"

				"CREATE TABLE IF NOT EXISTS \"TB_CONFERENCE_LOG\""
				"("
				"conference integer NOT NULL DEFAULT 0,"
				"broker_id character varying(128) NOT NULL DEFAULT 0,"
				"owner character varying(256) NOT NULL,"
				"start_time timestamp without time zone NOT NULL,"
				"end_time timestamp without time zone,"
				"term_reason smallint NOT NULL DEFAULT 0,"
				"type smallint NOT NULL DEFAULT 0,"
				"subtype smallint NOT NULL DEFAULT 0,"
				"app_id character(32),"
				"is_public boolean,"
				"named_conf_id character varying(256),"
				"max_participants integer NOT NULL DEFAULT 0,"
				"topic character varying(256),"
				"record_file character varying(64),"
				"guest_flags integer DEFAULT 0,"
				"PRIMARY KEY(conference, broker_id)"
				");"

				"CREATE INDEX  IF NOT EXISTS  \"IX_TB_CONFERENCE_LOG_owner_type\""
				"ON \"TB_CONFERENCE_LOG\" (owner, type);";

			bufSQL.format(creation_script);
			db.execDML(bufSQL);
		}
		res = true;
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}
	return res;
}

boost::optional<long> VS_DB_CallLogSQLite::fetch_long(CppSQLite3Query& q, const char* name)
{
	boost::optional<long> value;
	while (!q.eof()) {
		try {
			const char* ptr = q.fieldValue(name);
			if (ptr)
			{
				value = (int)strtol(ptr, NULL, 10);
				break;
			}
			q.nextRow();
		}
		catch (int) {
			q.nextRow();
			continue;
		}
	}
	return value;
};

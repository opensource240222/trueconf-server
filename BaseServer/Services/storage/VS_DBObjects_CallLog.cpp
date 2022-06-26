#ifdef _WIN32 // not ported
#include "VS_DBObjects_CallLog.h"
#include "../../../common/std/cpplib/VS_FileTime.h"
#include "../../../common/std/CallLog/VS_ConferenceDescription.h"

#define DEBUG_CURRENT_MODULE VS_DM_DBSTOR

VS_DBObjects_CallLog::VS_DBObjects_CallLog() : m_inited(false)
{

}

VS_DBObjects_CallLog::~VS_DBObjects_CallLog()
{

}

bool VS_DBObjects_CallLog::Init(const VS_SimpleStr& conn_str1, const VS_SimpleStr& username1, const VS_SimpleStr& password1)
{
	if (!conn_str1)
		return false;

	m_inited = false;

	try
	{
		db1.CreateInstance("ADODB.Connection");
	}
	catch (_com_error e)
	{
//		ProcessCOMError(e);
		return false;
	}
	for (int i = 0; i<reconnect_max; i++)
	{
		try
		{
			db1->Open((const char *)conn_str1, (const char *)username1, (const char *)password1, ADODB::adConnectUnspecified);
			m_inited = true;
			break;
		}
		catch (_com_error e)
		{
			m_inited = false;
			dprint1(".%d.\n", reconnect_max - i);
//			ProcessCOMError(e, db1, dbs);
			fflush(stdout);
			SleepEx(reconnect_timeout, false);
		}
	};

	if (!m_inited)
		return false;

	dprint4("sp_log_conf_start\n");
	log_conf_start.CreateInstance("ADODB.Command");
	log_conf_start->ActiveConnection = db1; //
	log_conf_start->CommandText = "sp_log_conf_start";
	log_conf_start->CommandType = ADODB::adCmdStoredProc;
	log_conf_start->Parameters->Refresh();

	dprint4("sp_log_conf_end\n");
	log_conf_end.CreateInstance("ADODB.Command");
	log_conf_end->ActiveConnection = db1;
	log_conf_end->CommandText = "sp_log_conf_end";
	log_conf_end->CommandType = ADODB::adCmdStoredProc;
	log_conf_end->Parameters->Refresh();

	//dprint4("sp_update_conf_info\n");
	//update_conf_info.CreateInstance("ADODB.Command");
	//update_conf_info->ActiveConnection = db1;
	//update_conf_info->CommandText = "sp_update_conf_info";
	//update_conf_info->CommandType = ADODB::adCmdStoredProc;
	//update_conf_info->Parameters->Refresh();
	//if (IsPostgreSQL)
	//	update_conf_info->Parameters->Item[DB_CONFERENCE_PIC_PARAM]->Size = 10000;

	dprint4("sp_log_participant_join\n");
	log_part_join.CreateInstance("ADODB.Command");
	log_part_join->ActiveConnection = db1;
	log_part_join->CommandText = "sp_log_participant_join";
	log_part_join->CommandType = ADODB::adCmdStoredProc;
	log_part_join->Parameters->Refresh();

	dprint4("sp_log_participant_leave\n");
	log_part_leave.CreateInstance("ADODB.Command");
	log_part_leave->ActiveConnection = db1;
	log_part_leave->CommandText = "sp_log_participant_leave";
	log_part_leave->CommandType = ADODB::adCmdStoredProc;
	log_part_leave->Parameters->Refresh();

	dprint4("sp_log_participant_invite\n");
	log_part_invite.CreateInstance("ADODB.Command");
	log_part_invite->ActiveConnection = db1;
	log_part_invite->CommandText = "sp_log_participant_invite";
	log_part_invite->CommandType = ADODB::adCmdStoredProc;
	log_part_invite->Parameters->Refresh();

	dprint4("sp_log_participant_statistics\n");
	log_part_stats.CreateInstance("ADODB.Command");
	log_part_stats->ActiveConnection = db1;
	log_part_stats->CommandText = "sp_log_participant_statistics";
	log_part_stats->CommandType = ADODB::adCmdStoredProc;
	log_part_stats->Parameters->Refresh();

	m_inited = true;
	return true;
}

bool VS_DBObjects_CallLog::LogConferenceStart(const VS_ConferenceDescription& conf, bool remote)
{
	if (!m_inited)
		return false;
	try
	{
		char buf[256];
		tu::TimeToNStr(conf.m_logStarted, buf, 256);
		dprint2("$LogConfStart conf(%s) start time(%s)\n", (const char*)conf.m_name, buf);
		//WaitForCommand(log_conf_start);
		ADODB::ParametersPtr p = log_conf_start->Parameters;

		int conference; VS_SimpleStr broker_id;
		SplitConfID(conf.m_name, conference, broker_id);


		p->Item[DB_CONFERENCE_PARAM]->Value = conference;
		p->Item[DB_SERVER_PARAM]->Value = broker_id.m_str;
		p->Item[DB_OWNER_PARAM]->Value = conf.m_owner.m_str;
		p->Item[DB_TYPE_PARAM]->Value = conf.m_type;

		p->Item[DB_SUBTYPE_PARAM]->Value = conf.m_SubType;

		const VS_FileTime ft(conf.m_logStarted);
		if (ft != NULL)
			p->Item[DB_TIME_PARAM]->Value = ft;
		else
			p->Item[DB_TIME_PARAM]->Value = db_null;
		p->Item[DB_REMOTE_PARAM]->Value = remote;
		p->Item[APP_ID_PARAM]->Value = conf.m_appID.m_str;
		p->Item[DB_MAX_USERS_PARAM]->Value = conf.m_MaxParticipants;
		p->Item[DB_PUBLIC_PARAM]->Value = conf.m_public;

		VS_WideStr wTopic; wTopic.AssignUTF8(conf.m_topic.c_str());
		p->Item[DB_TOPIC_PARAM]->Value = wTopic.m_str;
		p->Item[DB_LANG_PARAM]->Value = conf.m_lang.m_str;

		log_conf_start->Execute(0, 0, ADODB::adCmdUnspecified | ADODB::adExecuteNoRecords);
	}
	catch (_com_error e)
	{
		_bstr_t	add_str(log_conf_start->GetCommandText());
		//		ProcessCOMError(e, log_conf_start->ActiveConnection, this, add_str);
		//		ReleaseDBO(dboitem);
		return false;
	}
	return true;
}

bool VS_DBObjects_CallLog::LogConferenceEnd(const VS_ConferenceDescription& conf)
{
	if (!m_inited)
		return false;
	try
	{
		char buf[256];
		tu::TimeToNStr(conf.m_logEnded, buf, 256);
		dprint2("$LogConfEnd conf(%s) end time (%s) \n", (const char*)conf.m_name, buf);

		//WaitForCommand(log_conf_end);
		ADODB::ParametersPtr p = log_conf_end->Parameters;

		int conference; VS_SimpleStr broker_id;
		SplitConfID(conf.m_name, conference, broker_id);

		p->Item[DB_CONFERENCE_PARAM]->Value = conference;
		p->Item[DB_SERVER_PARAM]->Value = broker_id.m_str;
		p->Item[DB_OWNER_PARAM]->Value = conf.m_owner.m_str;
		p->Item[CONF_TERM_PARAM]->Value = conf.m_logCause;
		p->Item[DB_TYPE_PARAM]->Value = conf.m_type;
		p->Item[DB_SUBTYPE_PARAM]->Value = conf.m_SubType;

		const VS_FileTime ft(conf.m_logEnded);
		if (ft != NULL)
			p->Item[DB_TIME_PARAM]->Value = ft;
		else
			p->Item[DB_TIME_PARAM]->Value = db_null;

		log_conf_end->Execute(0, 0, ADODB::adCmdUnspecified | ADODB::adExecuteNoRecords);

	}
	catch (_com_error e)
	{
		_bstr_t	add_str(log_conf_end->GetCommandText());
//		dbo->ProcessCOMError(e, dbo->log_conf_end->ActiveConnection, this, add_str);
//		ReleaseDBO(dboitem);
		return false;
	}
	return true;
}

bool VS_DBObjects_CallLog::LogParticipantInvite(const vs_conf_id& conf_id, const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,
	const std::chrono::system_clock::time_point time, VS_ParticipantDescription::Type type)
{
	if (!m_inited)
		return false;
	try
	{

		char buf[256];
		const VS_FileTime ftime(time);
		dprint2("$LogPartInvite from '%s' to '%s', conf(%s) time(%s)\n", (const char*)call_id1, (const char*)call_id2, (const char*)conf_id, ftime == 0 ? "current" : ftime.ToNStr(buf));

		ADODB::ParametersPtr p = log_part_invite->Parameters;

		int conference; VS_SimpleStr broker_id;
		SplitConfID(conf_id, conference, broker_id);

		p->Item[DB_CONFERENCE_PARAM]->Value = conference;
		p->Item[DB_SERVER_PARAM]->Value = broker_id.m_str;
		p->Item[CALL_ID_PARAM]->Value = call_id1.m_str;
		p->Item[CALL_ID2_PARAM]->Value = call_id2.m_str;
		p->Item[APP_ID_PARAM]->Value = app_id.m_str;
		if (time != std::chrono::system_clock::time_point()) {
			p->Item[DB_TIME_PARAM]->Value = ftime;
		}
		else
			p->Item[DB_TIME_PARAM]->Value = db_null;

		p->Item[DB_TYPE_PARAM]->Value = type;

		log_part_invite->Execute(NULL, NULL, ADODB::adCmdStoredProc | ADODB::adExecuteNoRecords);
	}
	catch (_com_error e)
	{
		_bstr_t	add_str(log_part_invite->GetCommandText());
//		dbo->ProcessCOMError(e, dbo->log_part_invite->ActiveConnection, this, add_str);
//		ReleaseDBO(dboitem);
		return false;
	}
	return true;
}

bool VS_DBObjects_CallLog::LogParticipantJoin(const VS_ParticipantDescription& pd, const VS_SimpleStr& call_id2)
{
	if (!m_inited)
		return false;
	try
	{
		char buf[256];
		const VS_FileTime joinTime(pd.m_joinTime);
		dprint2("$LogPartJoin conf(%s) user(%s) join time(%s)\n", (const char*)pd.m_conf_id, (const char*)pd.m_user_id, joinTime.ToNStr(buf));

		//WaitForCommand(log_part_join);
		ADODB::ParametersPtr p = log_part_join->Parameters;

		int conference; VS_SimpleStr broker_id;
		SplitConfID(pd.m_conf_id, conference, broker_id);

		p->Item[DB_CONFERENCE_PARAM]->Value = conference;
		p->Item[DB_SERVER_PARAM]->Value = broker_id.m_str;
		p->Item[CALL_ID_PARAM]->Value = pd.m_user_id.m_str;
		p->Item[DB_TYPE_PARAM]->Value = pd.m_type;
		p->Item[APP_ID_PARAM]->Value = pd.m_appID.m_str;

		if (joinTime != NULL)
		{
			p->Item[PART_JOIN_DB_TIME_PARAM]->Value = joinTime;
		}
		else
			p->Item[PART_JOIN_DB_TIME_PARAM]->Value = db_null;

		if (call_id2 != NULL)
		{
			p->Item[CALL_ID2_PARAM]->Value = call_id2.m_str;
		}
		else
			p->Item[CALL_ID2_PARAM]->Value = db_null;
		//leave time - here
		//p->Item[PART_LEAVE_DB_TIME_PARAM]->Value	=leaveTime;
		p->Item[PART_PRICE_PARAM]->Value = pd.m_decLimit;
		p->Item[PART_CHARGE1_PARAM]->Value = pd.m_charge1;
		p->Item[PART_CHARGE2_PARAM]->Value = pd.m_charge2;
		p->Item[PART_CHARGE3_PARAM]->Value = pd.m_charge3;



		//app_id should be taken internally

		log_part_join->Execute(NULL, NULL, ADODB::adCmdUnspecified | ADODB::adExecuteNoRecords);
	}
	catch (_com_error e)
	{
		_bstr_t	add_str(log_part_join->GetCommandText());
//		dbo->ProcessCOMError(e, dbo->log_part_join->ActiveConnection, this, add_str);
//		ReleaseDBO(dboitem);
		return false;
	}
	return true;
}

bool VS_DBObjects_CallLog::LogParticipantReject(const vs_conf_id& conf_id, const vs_user_id& user, const vs_user_id& invited_from, const VS_Reject_Cause cause)
{
	dprint4("VS_DBObjects_CallLog::LogParticipantReject() not implemented yet! Ask kt\n");
	return false;
}

bool VS_DBObjects_CallLog::LogParticipantLeave(const VS_ParticipantDescription& pd)
{
	if (!m_inited)
		return false;
	try
	{
		char buf[256];
		const VS_FileTime leaveTime(pd.m_leaveTime);
		dprint2("$LogPartLeave conf(%s) user(%s) leave time(%s)\n", (const char*)pd.m_conf_id, (const char*)pd.m_user_id, leaveTime.ToNStr(buf));

		//WaitForCommand(log_part_leave);
		ADODB::ParametersPtr p = log_part_leave->Parameters;

		int conference; VS_SimpleStr broker_id;
		SplitConfID(pd.m_conf_id, conference, broker_id);

		p->Item[DB_CONFERENCE_PARAM]->Value = conference;
		p->Item[DB_SERVER_PARAM]->Value = broker_id.m_str;
		p->Item[CALL_ID_PARAM]->Value = pd.m_user_id.m_str;
		p->Item[DB_TYPE_PARAM]->Value = pd.m_type;

		p->Item[PART_LEAVE_REASON_PARAM]->Value = pd.m_cause;
		p->Item[PART_BYTES_SENT_PARAM]->Value = pd.m_bytesSnd;
		p->Item[PART_BYTES_RECEIVED_PARAM]->Value = pd.m_bytesRcv;
		p->Item[PART_RECON_SND_PARAM]->Value = pd.m_reconSnd;
		p->Item[PART_RECON_RCV_PARAM]->Value = pd.m_reconRcv;

		//p->Item[PART_JOIN_DB_TIME_PARAM]->Value	=pd.m_joinTime;
		//leave time - here
		if (leaveTime != NULL)
		{
			p->Item[PART_LEAVE_DB_TIME_PARAM]->Value = leaveTime;
		}
		else
			p->Item[PART_LEAVE_DB_TIME_PARAM]->Value = db_null;



		//app_id should be taken internally

		log_part_leave->Execute(NULL, NULL, ADODB::adCmdUnspecified | ADODB::adExecuteNoRecords);

	}
	catch (_com_error e)
	{
		_bstr_t	add_str(log_part_leave->GetCommandText());
//		dbo->ProcessCOMError(e, dbo->log_part_leave->ActiveConnection, this, add_str);
//		ReleaseDBO(dboitem);
		return false;
	}
	return true;
}

bool VS_DBObjects_CallLog::LogParticipantStatistics(const VS_ParticipantDescription& pd)
{
	if (!m_inited)
		return false;
	try
	{
		dprint3("$LogPartStats conf(%s) user(%s)\n", (const char*)pd.m_conf_id, (const char*)pd.m_user_id);

		//WaitForCommand(log_part_leave);
		ADODB::ParametersPtr p = log_part_stats->Parameters;

		int conference; VS_SimpleStr broker_id;
		SplitConfID(pd.m_conf_id, conference, broker_id);

		p->Item[DB_CONFERENCE_PARAM]->Value = conference;
		p->Item[DB_SERVER_PARAM]->Value = broker_id.m_str;
		p->Item[CALL_ID_PARAM]->Value = pd.m_user_id.m_str;

		p->Item[PART_BYTES_SENT_PARAM]->Value = pd.m_bytesSnd;
		p->Item[PART_BYTES_RECEIVED_PARAM]->Value = pd.m_bytesRcv;
		p->Item[PART_RECON_SND_PARAM]->Value = pd.m_reconSnd;
		p->Item[PART_RECON_RCV_PARAM]->Value = pd.m_reconRcv;

		log_part_stats->Execute(NULL, NULL, ADODB::adCmdUnspecified | ADODB::adExecuteNoRecords);
	}
	catch (_com_error e)
	{
		_bstr_t	add_str(log_part_stats->GetCommandText());
//		dbo->ProcessCOMError(e, dbo->log_part_stats->ActiveConnection, this, add_str);
	}
	return false;
}

void VS_DBObjects_CallLog::SplitConfID(const vs_conf_id& conf_id, int& conference, VS_SimpleStr& broker_id)
{
	if (conf_id.Length()<15) {
		conference = 0;
		broker_id.Empty();
	}
	else {
		if (sscanf(conf_id, "%08x", &conference) != 1) {
			conference = 0;
			broker_id.Empty();
		}
		else
			broker_id = (const char*)conf_id + 9;
	}
}
#endif
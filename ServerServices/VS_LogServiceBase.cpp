/**
 ****************************************************************************
 * (c) 2007 Visicron Systems, Inc.  http://www.visicron.net/
 *
 * Project: Visicron Base Server
 *
 * \file VS_LogService.cpp
 *
 * Log Service definition file
 *
 * $Revision: 28 $
 * $History: VS_LogService.cpp $
 *
 * *****************  Version 28  *****************
 * User: Mushakov     Date: 16.07.12   Time: 16:29
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - g_dbStorage was wrapped to shared_ptr
 *
 * *****************  Version 27  *****************
 * User: Ktrushnikov  Date: 9.06.11    Time: 14:59
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - clear NamedConf server when LogConfEnd come to BS
 *
 * *****************  Version 26  *****************
 * User: Ktrushnikov  Date: 28.03.11   Time: 23:06
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - email offline participant at VS_InvitesStorage
 * - new template added
 *
 * *****************  Version 25  *****************
 * User: Ktrushnikov  Date: 15.12.10   Time: 16:00
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - conf stat to file in AS (was onlyin VCS)
 * - "Save Conf Stat" dword reg key added
 *
 * *****************  Version 24  *****************
 * User: Ktrushnikov  Date: 1.04.10    Time: 14:12
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - process templates for subj
 *
 * *****************  Version 23  *****************
 * User: Ktrushnikov  Date: 7.03.10    Time: 17:20
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - All storages are inherited from VS_DBStorageInterface (DB, Reg, LDAP,
 * Trial)
 * - g_vcs_storage changed to g_dbStorage
 * - TConferenceStatistics added
 * - Process of LogPartStat added for VCS(file) & BS(null)
 * - fixed with d78 TransportRoute::DeleteService (dont delete deleted
 * service)
 * BS::LogSrv: suppress_missed_call_mail property added
 *
 * *****************  Version 22  *****************
 * User: Mushakov     Date: 16.09.09   Time: 19:05
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - %subscribed_email added
 *
 * *****************  Version 21  *****************
 * User: Mushakov     Date: 15.09.09   Time: 20:28
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - unsubscribe key added
 *
 * *****************  Version 20  *****************
 * User: Mushakov     Date: 20.02.09   Time: 19:26
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - AppId added in ParticipantDescr (invite, join log)
 * - 2 DBs supported
 *
 * *****************  Version 19  *****************
 * User: Mushakov     Date: 26.09.08   Time: 20:03
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - New Group Conf's atributes supported;
 *
 * *****************  Version 18  *****************
 * User: Mushakov     Date: 10.09.08   Time: 19:08
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - sending missed call mail for email of existing user (call_id ==
 * email)
 *
 * *****************  Version 17  *****************
 * User: Mushakov     Date: 22.08.08   Time: 12:18
 * Updated in $/VSNA/Servers/BaseServer/Services
 *
 * *****************  Version 16  *****************
 * User: Mushakov     Date: 25.07.08   Time: 14:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - logging app_ID added
 * - logging multi_conf
 * - bug 4602 fixed
 *
 * *****************  Version 15  *****************
 * User: Mushakov     Date: 15.07.08   Time: 18:12
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - LogConfStart LogConfEnd time corrected
 * - SrvRespond added to registry
 *
 * *****************  Version 14  *****************
 * User: Mushakov     Date: 18.06.08   Time: 17:00
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - VS_MemoryLeak included
 * - Logging to smtp service added
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 10.06.08   Time: 12:02
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - AV when BS exit  fixed
 * - SmtpMailService memory Leak fixed
 * - logging to Get Invate Mail  added
 *
 * *****************  Version 12  *****************
 * User: Mushakov     Date: 15.05.08   Time: 15:04
 * Updated in $/VSNA/Servers/BaseServer/Services
 * missed call mail corrcted
 *
 * *****************  Version 11  *****************
 * User: Stass        Date: 25.04.08   Time: 17:10
 * Updated in $/VSNA/Servers/BaseServer/Services
 * bs fix
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 25.04.08   Time: 14:17
 * Updated in $/VSNA/Servers/BaseServer/Services
 * dprint3 to LogService added
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 25.04.08   Time: 14:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 * dprintf3 added to MissedCallMail
 *
 * *****************  Version 8  *****************
 * User: Ktrushnikov  Date: 15.03.08   Time: 17:11
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - send mail changed: AS just send request to BS::LOG_SRV from 2 places
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 15.02.08   Time: 20:21
 * Updated in $/VSNA/Servers/BaseServer/Services
 * sending missed call mail and invate mail added
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 12.02.08   Time: 19:13
 * Updated in $/VSNA/Servers/BaseServer/Services
 * GetAppProperties method realized
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 8.02.08    Time: 20:19
 * Updated in $/VSNA/Servers/BaseServer/Services
 * rename
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 7.02.08    Time: 20:22
 * Updated in $/VSNA/Servers/BaseServer/Services
 * Missed call handle changed
 *
 * *****************  Version 3  *****************
 * User: Dront78      Date: 23.01.08   Time: 17:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * _malloca redefinition warning fixed.
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 12.12.07   Time: 20:40
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fixed init
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 6.12.07    Time: 18:58
 * Created in $/VSNA/Servers/BaseServer/Services
 ****************************************************************************/
#include "VS_LogServiceBase.h"
#include "BaseServer/Services/storage/VS_DBStorageInterface.h"
#include "transport/Router/VS_RouterMessage.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/VS_Utils.h"

#define DEBUG_CURRENT_MODULE VS_DM_LOGSERVICE

////////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
////////////////////////////////////////////////////////////////////////////////
VS_LogServiceBase::VS_LogServiceBase(void) :
    m_recvMess(NULL)
{
}


bool VS_LogServiceBase::Init(const char *our_endpoint, const char *our_service, const bool permittedAll)
{
	auto dbStorage = g_dbStorage;
	if(!!dbStorage)
	{
		dbStorage->GetServerProperty("notify_calls_url", m_notify_calls_url);
		dprint1("LogSRV: notify_calls_url=%s\n", m_notify_calls_url.c_str());

		dbStorage->GetServerProperty("notify_conf_invite_url", m_notify_conf_invite_url);
		dprint1("LogSRV: m_notify_conf_invite_url=%s\n", m_notify_conf_invite_url.c_str());

		dbStorage->GetServerProperty("notify_wait_for_letter_url", m_notify_wait_for_letter_url);
		dprint1("LogSRV: notify_wait_for_letter_url=%s\n", m_notify_wait_for_letter_url.c_str());

		dbStorage->GetServerProperty("notify_chat_url", m_notify_chat_url);
		dprint1("LogSRV: notify_chat_url=%s\n", m_notify_chat_url.c_str());
	}
	return true;
}

bool VS_LogServiceBase::Processing(std::unique_ptr<VS_RouterMessage>&&recvMess)
{
    if (recvMess == 0)  return true;
    switch (recvMess->Type())
    {
	case transport::MessageType::Invalid:
        break;
	case transport::MessageType::Request:
        m_recvMess = recvMess.get();
        {
            VS_Container cnt;
            if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
                const char* method = 0;
                if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0) {
                // Process methods
				if (strcasecmp(method, LOGCONFSTART_METHOD) == 0)
				{
					VS_ConferenceDescription cd;
					ParseConference(cnt,cd);
					size_t size = 0;
					const void * time = cnt.GetBinValueRef(TIME_PARAM, size);
					if (time && size==sizeof(int64_t))
					{
						cd.m_logStarted = tu::WindowsTickToUnixSeconds(*static_cast<const int64_t*>(time));
					};

					LogConferenceStart_Method(cd);
				}
				else if (strcasecmp(method, LOGCONFEND_METHOD) == 0)
				{
					VS_ConferenceDescription cd;
					ParseConference(cnt,cd);
					size_t size = 0;
					const void * time = cnt.GetBinValueRef(TIME_PARAM, size);
					if (time && size==sizeof(int64_t))
					{
						cd.m_logEnded = tu::WindowsTickToUnixSeconds(*static_cast<const int64_t*>(time));
					};

					LogConferenceEnd_Method(cd);
				}
				else if (strcasecmp(method, LOGPARTINVITE_METHOD) == 0)
				{
					std::chrono::system_clock::time_point ftime;

					size_t size = 0;
					const void* time = cnt.GetBinValueRef(TIME_PARAM, size);
					if (time && size==sizeof(int64_t)) ftime = tu::WindowsTickToUnixSeconds(*static_cast<const int64_t*>(time));

					int32_t type = 0;
					cnt.GetValue(TYPE_PARAM,type);

					LogParticipantInvite_Method(
						cnt.GetStrValueRef(CONFERENCE_PARAM),
						cnt.GetStrValueRef(CALLID_PARAM),
						cnt.GetStrValueRef(APPID_PARAM),
						cnt.GetStrValueRef(CALLID2_PARAM),
						ftime,
						(VS_ParticipantDescription::Type)type);
				}
				else if (strcasecmp(method, LOGPARTJOIN_METHOD) == 0)
				{
					VS_ParticipantDescription pd;

					ParseParticipant(cnt,pd);

					size_t size = 0;
					const void* time = cnt.GetBinValueRef(TIME_PARAM, size);
					if (time && size==sizeof(int64_t))
					{
						pd.m_joinTime = tu::WindowsTickToUnixSeconds(*static_cast<const int64_t*>(time));
					};
					if (VS_GetServerType(OurEndpoint()) == ST_VCS)		// only for TCS, not BS
					{
						if (!VS_RealUserLogin(SimpleStrToStringView(pd.m_user_id)).IsOurSID())		// do not save appID of roaming user
							pd.m_appID.Empty();
					}

					LogParticipantJoin_Method(pd,cnt.GetStrValueRef(CALLID2_PARAM));
				}
				else if (strcasecmp(method, LOGPARTREJECT_METHOD) == 0)
				{
					//unsigned long size = 0;
					//const void* time = cnt.GetBinValueRef(TIME_PARAM, size);
					//if (time && size == sizeof(FILETIME))
					//{
					//	memcpy(&pd.m_leaveTime, time, size);
					//};
					VS_Reject_Cause cause = VS_Reject_Cause::UNDEFINED_CAUSE;
					auto ptr = cnt.GetLongValueRef(CAUSE_PARAM);
					if (ptr)
						cause = (VS_Reject_Cause)*ptr;

					LogParticipantReject_Method(cnt.GetStrValueRef(CONFERENCE_PARAM), cnt.GetStrValueRef(CALLID_PARAM), cnt.GetStrValueRef(FROM_PARAM), cause);
				}
				else if (strcasecmp(method, LOGPARTLEAVE_METHOD) == 0)
				{
					VS_ParticipantDescription pd;
					ParseParticipant(cnt,pd);

					size_t size = 0;
					const void* time = cnt.GetBinValueRef(TIME_PARAM, size);
					if (time && size==sizeof(int64_t))
					{
						pd.m_leaveTime = tu::WindowsTickToUnixSeconds(*static_cast<const int64_t*>(time));
					};

					LogParticipantLeave_Method(pd);
				}
				else if (strcasecmp(method, LOGPARTSTAT_METHOD) == 0)
				{
					auto dbStorage = g_dbStorage;
					if(!!dbStorage)
						dbStorage->ProcessConfStat(cnt);

					VS_ParticipantDescription pd;
					ParseParticipant(cnt,pd);

					LogParticipantStatistics_Method(pd);

					bool from_user = !m_recvMess->SrcUser_sv().empty();
					auto src = m_recvMess->SrcServer();
					if (from_user && src && strcasecmp(src, OurEndpoint()) != 0 && VS_GetServerType(src) == ST_VCS)
					{
						auto h = cnt.GetLongValueRef(HOPS_PARAM);
						if (!h)
						{
							cnt.AddValueI32(HOPS_PARAM, 1);
							PostRequest(src, nullptr, cnt);
						}
					}
				}
				else if (strcasecmp(method, LOGRECORDSTART_METHOD) == 0)
				{
					LogRecordStart_Method(cnt);
				}
				else if (strcasecmp(method, LOGRECORDSTOP_METHOD) == 0)
				{
					LogRecordStop_Method(cnt);
				}
#ifdef _WIN32	// no emails at linux server
				else if(strcasecmp(method, SENDMAIL_METHOD) == 0)
				{
					SendMail_Method(cnt);
				}
#endif
				else if(strcasecmp(method,UPDATECONFERENCEPIC_METHOD) == 0)
				{
					UpdateConferencePic_Method(cnt);
				}
				else if (strcasecmp(method, LOGPARTICIPANTDEVICE_METHOD) == 0)
				{
					LogParticipantDevice_Method(cnt);
				}
				else if (strcasecmp(method, LOGPARTICIPANTROLE_METHOD) == 0)
				{
					LogParticipantRole_Method(cnt);
				}
			}
			}
        }
        break;
	case transport::MessageType::Reply:
        break;
	case transport::MessageType::Notify:
        break;
    }
    m_recvMess = nullptr;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Methods
////////////////////////////////////////////////////////////////////////////////
void VS_LogServiceBase::LogConferenceStart_Method  (const VS_ConferenceDescription& cd)
{
	auto dbStorage = g_dbStorage;
	if(!!dbStorage)
		dbStorage->LogConferenceStart(cd);
};


void VS_LogServiceBase::LogConferenceEnd_Method    (const VS_ConferenceDescription& cd)
{
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return;
	dbStorage->LogConferenceEnd(cd);
};

void VS_LogServiceBase::LogParticipantInvite_Method	(const vs_conf_id& conf_id,const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,const std::chrono::system_clock::time_point time,VS_ParticipantDescription::Type type)
{
	auto dbStorage = g_dbStorage;
	if(!!dbStorage)
	{
		std::string call_id1_;
		if (call_id1)
			call_id1_ = std::string(VS_RemoveTranscoderID_sv(call_id1.m_str));
		std::string call_id2_;
		if (call_id2)
			call_id2_ = std::string(VS_RemoveTranscoderID_sv(call_id2.m_str));
		dbStorage->LogParticipantInvite(conf_id, call_id1_.c_str(), app_id, call_id2_.c_str(), time, type);
	}
}

void VS_LogServiceBase::LogParticipantJoin_Method  (VS_ParticipantDescription& pd, const VS_SimpleStr& call_id2)
{
	auto dbStorage = g_dbStorage;
	if(!!dbStorage)
	{
		std::string call_id2_;
		if (call_id2)
			call_id2_ = std::string(VS_RemoveTranscoderID_sv(call_id2.m_str));
		dbStorage->LogParticipantJoin(pd, call_id2_.c_str());
	}
}

void VS_LogServiceBase::LogParticipantReject_Method(const vs_conf_id& conf_id, const vs_user_id& user, const vs_user_id& invited_from, const VS_Reject_Cause cause)
{
	auto dbStorage = g_dbStorage;
	if (!!dbStorage)
	{
		std::string user_;
		if (user)
			user_ = std::string(VS_RemoveTranscoderID_sv(user.m_str));;
		std::string invited_from_;
		if (invited_from)
			invited_from_ = std::string(VS_RemoveTranscoderID_sv(invited_from.m_str));
		dbStorage->LogParticipantReject(conf_id, user_.c_str(), invited_from_.c_str(), cause);
	}
}

void VS_LogServiceBase::LogParticipantLeave_Method (VS_ParticipantDescription& pd)
{
	auto dbStorage = g_dbStorage;
	if(!!dbStorage)
	{
		dbStorage->LogParticipantLeave(pd);
	}
}

void VS_LogServiceBase::LogParticipantStatistics_Method(const VS_ParticipantDescription& pd)
{
	auto dbStorage = g_dbStorage;
	if(dbStorage)
	{
		dbStorage->LogParticipantStatistics(pd);
	}
}

void VS_LogServiceBase::LogRecordStart_Method(VS_Container& cnt)
{
	auto conf_id = cnt.GetStrValueRef(CONFERENCE_PARAM);
	auto filename = cnt.GetStrValueRef(FILENAME_PARAM);

	std::chrono::system_clock::time_point started_at{};
	if (!cnt.GetValue(START_TIME_PARAM, started_at))
		dstream3 << "LogRecordStart_Method: failed to retrieve record start time";

	auto dbStorage = g_dbStorage;
	if (dbStorage && conf_id && filename)
		dbStorage->LogConferenceRecordStart(conf_id, filename, started_at);
}

void VS_LogServiceBase::LogRecordStop_Method(VS_Container& cnt)
{
	auto conf_id = cnt.GetStrValueRef(CONFERENCE_PARAM);

	std::chrono::system_clock::time_point stopped_at{};
	if (!cnt.GetValue(STOP_TIME_PARAM, stopped_at))
		dstream3 << "LogRecordStop_Method: failed to retrieve record stop time";

	uint64_t file_size = 0;
	if (!cnt.GetValueI64(SIZE_PARAM, file_size))
		dstream3 << "LogRecordStop_Method: failed to retrieve file size";

	auto dbStorage = g_dbStorage;
	if (dbStorage && conf_id)
		dbStorage->LogConferenceRecordStop(conf_id, stopped_at, file_size);
}

void VS_LogServiceBase::UpdateConferencePic_Method(VS_Container &cnt)
{
	auto dbStorage = g_dbStorage;
	if(!!dbStorage)
		dbStorage->UpdateConferencePic(cnt);
}

void VS_LogServiceBase::LogParticipantDevice_Method(VS_Container &cnt)
{
	auto dbStorage = g_dbStorage;
	if (!dbStorage)
		return;

	VS_ParticipantDeviceParams pr;
	pr.conference_id = cnt.GetStrValueRef(CONFERENCE_PARAM);
	pr.user_instance = cnt.GetStrValueRef(USERNAME_PARAM);
	pr.action = cnt.GetStrValueRef(FUNC_PARAM);
	pr.device_id = cnt.GetStrValueRef(ID_PARAM);
	pr.device_type = cnt.GetStrValueRef(TYPE_PARAM);
	pr.device_name = cnt.GetStrValueRef(NAME_PARAM);
	cnt.GetValue(ACTIVE_PARAM, pr.is_active);

	bool mute(false);
	if (cnt.GetValue(MUTE_PARAM, mute))
		pr.pmute = &mute;

	int32_t volume(0);
	if (cnt.GetValue(VOLUME_PARAM, volume))
		pr.pvolume = &volume;

	dbStorage->LogParticipantDevices(pr);
}

void VS_LogServiceBase::LogParticipantRole_Method(VS_Container &cnt)
{
	auto dbStorage = g_dbStorage;
	if (!dbStorage)
		return;

	VS_ParticipantDescription pd;
	pd.m_conf_id = cnt.GetStrValueRef(CONFERENCE_PARAM);
	pd.m_user_id = cnt.GetStrValueRef(USERNAME_PARAM);
	int32_t role = 0;
	cnt.GetValueI32(ROLE_PARAM, role);
	pd.m_role = role & 0xff;
	pd.m_brStatus = (role >> 8) & 0xffff;

	dbStorage->LogParticipantRole(pd);
}

////////////////////////////////////////////////////////////////////////////////
// helpers
////////////////////////////////////////////////////////////////////////////////

void VS_LogServiceBase::ParseConference(VS_Container& cnt,VS_ConferenceDescription& cd)
{
	int32_t lval;
	bool bval;

	cd.m_name = cnt.GetStrValueRef(CONFERENCE_PARAM);
	cd.m_call_id = cnt.GetStrValueRef(CALLID_PARAM);
	if(cnt.GetValue(TYPE_PARAM,lval))
		cd.m_type=lval;
	if(cnt.GetValue(SUBTYPE_PARAM,lval))
		cd.m_SubType=lval;
	cd.m_owner=cnt.GetStrValueRef(OWNER_PARAM);
	if(cnt.GetValue(CAUSE_PARAM,lval))
		cd.m_logCause=(VS_ConferenceDescription::TerminationCause)lval;
	cd.m_appID = cnt.GetStrValueRef(APPID_PARAM);
	if(cnt.GetValue(MAXPARTISIPANTS_PARAM,lval))
		cd.m_MaxParticipants = lval;
	if(cnt.GetValue(PUBLIC_PARAM,bval))
		cd.m_public = bval;

	auto pTopic = cnt.GetStrValueRef(TOPIC_PARAM);
	if (pTopic) cd.m_topic = pTopic;
	cd.m_lang = cnt.GetStrValueRef(LANG_PARAM);
};

void VS_LogServiceBase::ParseParticipant(VS_Container& cnt,VS_ParticipantDescription& pd)
{
	int32_t lval;
	double dval;

	pd.m_conf_id=cnt.GetStrValueRef(CONFERENCE_PARAM);
	pd.m_user_id=cnt.GetStrValueRef(CALLID_PARAM);
	if (const char *dn = cnt.GetStrValueRef(DISPLAYNAME_PARAM)) pd.m_displayName = dn;
	pd.m_server_id =cnt.GetStrValueRef(SERVER_PARAM);
	pd.m_appID = cnt.GetStrValueRef(APPID_PARAM);
	if(cnt.GetValue(TYPE_PARAM,lval))
		pd.m_type=lval;

	if(cnt.GetValue(PRICE_PARAM,lval))
		pd.m_decLimit=lval;

	if(cnt.GetValue(CHARGE1_PARAM,dval))
		pd.m_charge1=dval;
	if(cnt.GetValue(CHARGE2_PARAM,dval))
		pd.m_charge2=dval;
	if(cnt.GetValue(CHARGE3_PARAM,dval))
		pd.m_charge3=dval;

	if(cnt.GetValue(CAUSE_PARAM,lval))
		pd.m_cause=(VS_ParticipantDescription::Disconnect_Cause)lval;

	if(cnt.GetValue(BYTES_SENT_PARAM,lval))
		pd.m_bytesSnd=lval;

	if(cnt.GetValue(BYTES_RECEIVED_PARAM,lval))
		pd.m_bytesRcv=lval;

	if(cnt.GetValue(RECON_SND_PARAM,lval))
		pd.m_reconSnd=lval;

	if(cnt.GetValue(RECON_RCV_PARAM,lval))
		pd.m_reconRcv=lval;

	size_t size = 0;
	const void* time = cnt.GetBinValueRef(JOIN_TIME_PARAM, size);
	if (time && size == sizeof(int64_t)) pd.m_joinTime = tu::WindowsTickToUnixSeconds(*static_cast<const int64_t*>(time));
};

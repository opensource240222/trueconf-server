/**
 ****************************************************************************
 * (c) 2007 Visicron Systems, Inc.  http://www.visicron.net/
 *
 * Project: Visicron Base Server
 *
 * \file VS_LogService.h
 *
 * $Revision: 11 $
 * $History: VS_LogService.h $
 *
 * *****************  Version 11  *****************
 * User: Ktrushnikov  Date: 28.03.11   Time: 23:06
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - email offline participant at VS_InvitesStorage
 * - new template added
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 15.09.09   Time: 20:29
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - unsubscribe key added
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 20.02.09   Time: 19:26
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - AppId added in ParticipantDescr (invite, join log)
 * - 2 DBs supported
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 26.09.08   Time: 20:03
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - New Group Conf's atributes supported;
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 22.08.08   Time: 12:18
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - chat message about closing multi conf by owner
 *  - mailing about missing of invate to multiconf
 *  - Calls logging fixed
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 15.05.08   Time: 15:04
 * Updated in $/VSNA/Servers/BaseServer/Services
 * missed call mail corrcted
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 25.04.08   Time: 17:10
 * Updated in $/VSNA/Servers/BaseServer/Services
 * bs fix
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 15.02.08   Time: 20:21
 * Updated in $/VSNA/Servers/BaseServer/Services
 * sending missed call mail and invate mail added
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 7.02.08    Time: 20:22
 * Updated in $/VSNA/Servers/BaseServer/Services
 * Missed call handle changed
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 12.12.07   Time: 20:40
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fixed init
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 6.12.07    Time: 18:58
 * Created in $/VSNA/Servers/BaseServer/Services
 *
 ****************************************************************************/
#pragma once

#include "Common.h"
#include "transport/Router/VS_TransportRouterServiceHelper.h"

class VS_LogServiceBase :
	virtual public VS_TransportRouterServiceReplyHelper
{
	static void ParseConference (VS_Container& cnt,VS_ConferenceDescription& cd);
	static void ParseParticipant(VS_Container& cnt,VS_ParticipantDescription& pd);

protected:
	std::string m_notify_calls_url;
	std::string m_notify_conf_invite_url;
	std::string m_notify_chat_url;
	std::string m_notify_wait_for_letter_url;

public:
	VS_LogServiceBase(void);
	~VS_LogServiceBase(void) { }

	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	// Service implementation
	VS_RouterMessage *m_recvMess;

	void LogConferenceStart_Method  (const VS_ConferenceDescription& cd);
	void LogConferenceEnd_Method    (const VS_ConferenceDescription& cd);
	void LogParticipantInvite_Method	(const vs_conf_id& conf_id,const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,
		const std::chrono::system_clock::time_point time = std::chrono::system_clock::time_point(), VS_ParticipantDescription::Type type = VS_ParticipantDescription::PRIVATE_HOST);
	void LogParticipantJoin_Method  (VS_ParticipantDescription& pd, const VS_SimpleStr& call_id2=NULL);
	void LogParticipantReject_Method(const vs_conf_id& conf_id, const vs_user_id& user, const vs_user_id& invited_from, const VS_Reject_Cause cause);
	void LogParticipantLeave_Method (VS_ParticipantDescription& pd);
	void LogParticipantStatistics_Method (const VS_ParticipantDescription& pd);
	void LogRecordStart_Method(VS_Container& cnt);
	void LogRecordStop_Method(VS_Container& cnt);
	void UpdateConferencePic_Method(VS_Container &cnt);
	void LogParticipantDevice_Method(VS_Container &cnt);
	void LogParticipantRole_Method(VS_Container &cnt);

	//void MissedCallMail(const VS_FileTime missed_call_time, const char*app_name, const char* fromId, const wchar_t* fromDn, const char* toId, const wchar_t* toDn);
	//void InviteMail(const VS_FileTime missed_call_time, const char*app_name, const char *fromId, const wchar_t *fromDn, const char *toId);
	//void MultiInviteMail(const VS_FileTime missed_call_time, const char*app_name,const char* fromId, const wchar_t* fromDn, const char* toId, const wchar_t* toDn, const char *conf_name, const char *pass);
	//void MissedNamedConfMail(const VS_FileTime missed_call_time, const char *app_name, const char* fromId, const wchar_t* fromDn, const char* toId, const wchar_t* toDn);

#ifdef _WIN32	// no emails at linux server
	virtual void SendMail_Method(VS_Container &cnt) = 0;
#endif
};



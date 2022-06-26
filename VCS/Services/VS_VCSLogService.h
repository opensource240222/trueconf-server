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
#include "../../ServerServices/Common.h"
#include "../../ServerServices/VS_LogServiceBase.h"
#include "../../ServerServices/VS_SmtpMailerService.h"

#include <memory>

class VS_VCSLogService : public VS_LogServiceBase, public VS_SmtpMailerBase
{
	void ReplaceGuestUrl(std::string& body, const char* confid = nullptr) const;
public:
	VS_VCSLogService(void);
	virtual ~VS_VCSLogService(void);

	bool InitEventStorage(const std::string& config);

	bool Init(const char *our_endpoint, const char *our_service, const bool permittedAll) override;
	void AsyncDestroy() override;
	bool Processing(std::unique_ptr<VS_RouterMessage> &&recvMess) override;

#ifdef _WIN32	// no emails at linux server
	void MissedCallMail(const std::chrono::system_clock::time_point missed_call_time, const char*app_name, const char* fromId, const char* fromDn, const char* toId, const char* toDn);
	void InviteMail(const std::chrono::system_clock::time_point  missed_call_time, const char * app_name, const char * fromId, const char * fromDn, const char * toId);
	void MultiInviteMail(const std::chrono::system_clock::time_point  missed_call_time, const char * app_name, const char * fromId, const char * fromDn, const char * toId, const char * toDn);
	void MissedNamedConfMail(const std::chrono::system_clock::time_point  missed_call_time, const char *app_name, const char* fromId, const char* fromDn, const char* toId, const char* toDn, const char* conf_id, const std::string& topic, const char* file_ics_utf8);

	virtual void SendMail_Method(VS_Container &cnt);
#endif

private:
	class EventLogger;
	std::unique_ptr<EventLogger> m_ev_log;
};

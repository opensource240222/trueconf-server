/*
 * $Revision: 29 $
 * $History: VS_ConferenceService.h $
 *
 * *****************  Version 29  *****************
 * User: Mushakov     Date: 17.07.12   Time: 23:09
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - LoginConfigurator() was removed
 * - messages from configurator are handled by SessionID
 * - fix TransportMessage::IsFromServer()
 *
 * *****************  Version 28  *****************
 * User: Ktrushnikov  Date: 5.02.12    Time: 15:58
 * Updated in $/VSNA/Servers/AppServer/Services
 * - IsKickUser(): check not only steam_id, but named_conf or VCS regsitry
 * conf_id
 * - if VCS user kicked from named_conf, send REJECT response directly to
 * user (not to BS)
 *
 * *****************  Version 27  *****************
 * User: Ktrushnikov  Date: 12.08.10   Time: 18:37
 * Updated in $/VSNA/Servers/AppServer/Services
 * [#7592]
 * - VS_ConfKick interface for store kicked users and deny them to connect
 * to conference in server
 *
 * *****************  Version 26  *****************
 * User: Smirnov      Date: 30.06.10   Time: 16:50
 * Updated in $/VSNA/Servers/AppServer/Services
 * delete conference on owner hungup (#7493)
 *
 * *****************  Version 25  *****************
 * User: Mushakov     Date: 21.06.10   Time: 14:39
 * Updated in $/VSNA/Servers/AppServer/Services
 * - VS_ConfLog
 *
 * *****************  Version 24  *****************
 * User: Mushakov     Date: 15.06.10   Time: 15:32
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - sharding
 *
 * *****************  Version 23  *****************
 * User: Ktrushnikov  Date: 14.04.10   Time: 17:57
 * Updated in $/VSNA/Servers/AppServer/Services
 * - dprint3 log added
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 15.02.10   Time: 18:50
 * Updated in $/VSNA/Servers/AppServer/Services
 * - licence restrictions reorganization
 * - SECUREBEGIN_A temporally removed
 *
 * *****************  Version 21  *****************
 * User: Dront78      Date: 18.01.10   Time: 13:04
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 20  *****************
 * User: Mushakov     Date: 8.12.09    Time: 20:23
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - ConfRestriction Interface added
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 10.08.09   Time: 12:37
 * Updated in $/VSNA/Servers/AppServer/Services
 * - added reject causes
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 5.06.09    Time: 20:19
 * Updated in $/VSNA/Servers/AppServer/Services
 * - LOG mesasges living time increased
 * - group conference living  time increased to 10 days
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 24.10.08   Time: 21:12
 * Updated in $/VSNA/Servers/AppServer/Services
 * - transport ping increased
 * - loging in conference corrected
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 16.10.08   Time: 20:26
 * Updated in $/VSNA/Servers/AppServer/Services
 * - conference logging improved
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 13.10.08   Time: 21:37
 * Updated in $/VSNA/Servers/AppServer/Services
 * - some cleaning
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 3.10.08    Time: 15:09
 * Updated in $/VSNA/Servers/AppServer/Services
 * - conference messages
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 22.08.08   Time: 12:18
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - chat message about closing multi conf by owner
 *  - mailing about missing of invate to multiconf
 *  - Calls logging fixed
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 25.04.08   Time: 14:36
 * Updated in $/VSNA/Servers/AppServer/Services
 * - removing mail message for group conference
 *
 * *****************  Version 11  *****************
 * User: Ktrushnikov  Date: 15.03.08   Time: 17:11
 * Updated in $/VSNA/Servers/AppServer/Services
 * - send mail changed: AS just send request to BS::LOG_SRV from 2 places
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 18.02.08   Time: 16:43
 * Updated in $/VSNA/Servers/AppServer/Services
 * - added net config for servers
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 15.02.08   Time: 20:21
 * Updated in $/VSNA/Servers/AppServer/Services
 * sending missed call mail and invate mail added
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 7.02.08    Time: 20:22
 * Updated in $/VSNA/Servers/AppServer/Services
 * Missed call handle changed
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 12.12.07   Time: 20:40
 * Updated in $/VSNA/Servers/AppServer/Services
 * fixed init
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 5.12.07    Time: 11:09
 * Updated in $/VSNA/Servers/AppServer/Services
 * BS - new iteration
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 30.11.07   Time: 20:36
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 26.11.07   Time: 17:51
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 ***********************************************/

#ifndef VS_CONFERENCE_SERVICE_H
#define VS_CONFERENCE_SERVICE_H

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "../../common/streams/Statistics.h"
#include "../../common/streams/Router/ConferencesConditions.h"
#include "../../common/streams/Router/Types.h"

#include "AppServer/Services/VS_Storage.h"
#include "../../ServerServices/VS_SmtpMailerService.h"
#include "../../common/std/cpplib/VS_Lock.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/synchronized.h"
#include "transport/VS_SystemMessage.h"
#include "ServerServices/VS_ConfRestrictInterface.h"
#include "../../ServerServices/VS_AutoInviteService.h"
#include "VS_ConfLog.h"
#include "AppServer/Services/VS_PresenceService.h"

#include "std-generic/compat/map.h"
#include <unordered_map>
#include <memory>

namespace ts { struct IPool; }

struct VS_ConfPartStat
{
	vs_conf_id						conf;
	VS_SimpleStr					part;
	stream::ParticipantStatistics	stat;
};

class VS_ConfKick
{
public:
	void AddConfKick(string_view conf_id, string_view user_id);
	void RemoveConfKick(string_view conf_id, string_view user_id);
	void RemoveConfKicks(string_view conf_id);
	bool IsKickedUser(string_view conf_id, string_view user_id, const VS_ConfRestrictInterface* confRestrict);
private:
	using data_t = vs::map<std::string /*conf*/, vs::set<std::string /*user*/, vs::str_less>, vs::str_less>;
	vs::Synchronized<data_t> m_data;
};
namespace vs_relaymodule { class VS_StreamsRelay; }
class VS_ConfRecorderModuleCtrl;
class VS_RTSPBroadcastModuleCtrl;

class VS_ConferenceService :
	public VS_AutoInviteService,
	public stream::ConferencesConditions,
	public VS_TransportRouterServiceReplyHelper,
	public VS_SmtpMailerBase,
	public VS_SystemMessage,
	public VS_ConfLog,
	//public VS_LogHelper,
	public VS_ConfKick,
	virtual public VS_PresenceServiceMember,
	protected VS_Lock
{
	void onRecordStartInfo(const vs_conf_id& conf_id, const std::string& filename, std::chrono::system_clock::time_point started_at);
	void onRecordStopInfo(const vs_conf_id& conf_id, std::chrono::system_clock::time_point stopped_at, uint64_t file_size);
public:
	VS_ConferenceService();
	virtual ~VS_ConferenceService(void);
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll) override;
	void AsyncDestroy() override;
	bool Timer(unsigned long tickcount) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	// Protocol Methods
	virtual void CreateConference_Method(long duration, long maxPartisipants, long type, VS_BinBuff &caps, const char* pass, const std::string& transcReserveToken);
	virtual void DeleteConference_Method(const char *name, long cause, const char *pass);
	virtual void Invite_Method	(VS_Container &cnt);
	virtual void Accept_Method	(VS_Container &cnt);
	virtual void Reject_Method	(VS_Container &cnt);
	virtual void Hangup_Method	(const char *conferenceName, const char *name, long del, HangupFlags hflags);
	virtual void Kick_Method	(const VS_Container &cnt);
	virtual void Ignore_Method	(const char *conferenceName, const char *name);
	virtual void Join_Method	(VS_Container &cnt);
	// Timer Events
	void OnTimer_CheckConferences();
	void OnTimer_CheckPartLimit(int dif);
	void OnTimer_CheckInvites();
	// helpers
	void SendPartsList(VS_ConferenceDescription &cd);
	void SendJoinMessage(VS_ConferenceDescription &cd, vs_user_id &user_id, int type);

	void SetStreamRouter(stream::Router* sr)
	{
		m_sr = sr;
	};
	void SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface>& confRestrict);
	void SetIOservice(boost::asio::io_service& ios);
	void SetTransceiversPool(const std::shared_ptr<ts::IPool> &pool);

	// Stream Router CallBacks
	void CreateConference	(const char *conferenceName) override;
	void RemoveConference	(const char *conferenceName, const stream::ConferenceStatistics& cs) override;
	void AddParticipant		(const char *conferenceName, const char *participantName) override;
	void RemoveParticipant	(const char *conferenceName, const char *participantName, const stream::ParticipantStatistics& ps) override;
	// Change State Events
	virtual void RemoveConference_Event(VS_ConferenceDescription& cd, long cause = 0);
	virtual void RemoveParticipant_Event(const char* user, long cause, VS_ConfPartStat* ps = NULL);

private:
	unsigned long		m_conf_check_time;
	unsigned long		m_part_check_time;
	unsigned long		m_invite_check_time;

protected:
	boost::asio::io_service* m_pios;
	stream::Router*	m_sr;
	boost::shared_ptr<VS_ConfRestrictInterface> m_confRestriction;
///////
	std::shared_ptr<vs_relaymodule::VS_StreamsRelay> m_streamsRelay;
	std::weak_ptr<ts::IPool> m_transceiversPool;
	std::shared_ptr<VS_ConfRecorderModuleCtrl>	m_confWriterModule;
	boost::signals2::connection m_signal_RecordStartInfo;
	boost::signals2::connection m_signal_RecordStopInfo;
	std::shared_ptr<VS_RTSPBroadcastModuleCtrl> m_RTSPBroadcastModule;

	boost::signals2::connection m_keyReqConn, m_bitrateRestricConn;

	bool IsBS(const char* server) const;
public:
	// definition of conf expiration times in different states
	const static int TIME_EXP_CREATE;
	const static int TIME_EXP_INVITE;
	const static int TIME_EXP_ACCEPT; // 10 days
	const static int TIME_EXP_END;
};

extern VS_ConferenceService *g_conferenceService;

#endif // VS_CONFERENCE_SERVICE_H

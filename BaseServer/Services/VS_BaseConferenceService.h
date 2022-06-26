/*
* $Revision: 6 $
* $History: VS_BaseConferenceService.h $
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 1.03.11    Time: 22:53
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - pass current stream_conf_id of a named_conf_id from AS to BS->DB
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 7.07.10    Time: 21:12
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - named confs procedures & parameters fixed
 * - init_named_conf_invitations processing added
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 1.07.10    Time: 20:56
 * Updated in $/VSNA/Servers/BaseServer/Services
 * Arch 3.1: NamedConfs
 * - DB interfaces created (with dummy & untested imp)
 * - Processing of InvitationUpdate_Method added
 * - BS Event "NAMED_CONF" rewritten
 * - bug fix with storing changed in struct (in map)
 * - some reject messages added
 *
 * *****************  Version 3  *****************
 * User: Ktrushnikov  Date: 30.06.10   Time: 20:31
 * Updated in $/VSNA/Servers/BaseServer/Services
 * Arch 3.1: named confs
 * - send emails at N minutes before named conf invitation
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 29.06.10   Time: 12:21
 * Updated in $/VSNA/Servers/BaseServer/Services
 * Arch 3.1: NamedConfs invitation:
 * - RESOLVEALL_METHOD to get server & status of users
 * - start resolve in separate thread (PoolThreads)
 * - ask SM for RS in BS
 * - access to RS name from all BS services via locks
 * - reconnect to RS
 * - calc is_time fixed
 *
 * *****************  Version 1  *****************
 * User: Ktrushnikov  Date: 22.06.10   Time: 23:40
 * Created in $/VSNA/Servers/BaseServer/Services
 * Arch 3.1: NamedConfs added
 * - BS has Conference service
 * - Join_Method can create/join conf (if came from BS) or post request to
 * BS (if came from user)
*
***********************************************/
#pragma once

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "../../common/transport/Router/VS_PoolThreadsService.h"
#include "storage/VS_DBStorage.h"

#include "../../common/std/cpplib/VS_Map.h"
#include "../../common/std/cpplib/VS_MapTpl.h"
#include "../../common/std/cpplib/VS_Lock.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

class VS_BaseConferenceService :
	public VS_TransportRouterServiceReplyHelper,
	public std::enable_shared_from_this<VS_BaseConferenceService>
{
	boost::asio::io_service::strand m_strand;
	VS_Lock			m_confs_lock;
	VS_Map			m_confs;		// хранит конференции для инвайта

	std::chrono::steady_clock::time_point m_last_as_check;
	std::map<std::string /*as_server_name*/, uint32_t /*server_responce_time_ms*/> m_AS_online;
	std::set<std::string> m_AS_my_domain;

	unsigned long	m_invitation_tick;
	unsigned long	INVITATION_PERIOD;

	void StartInviteParticipants(ConferenceInvitation& cd);
	void CheckConfInviteTime(ConferenceInvitation& cd);
	void StartSendEmail( ConferenceInvitation& cd );
	void ScheduleInvitaion(const char* conf_id);

public:
	VS_BaseConferenceService(boost::asio::io_service& ios);
	virtual ~VS_BaseConferenceService(void);

	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	bool Timer(unsigned long tickcount) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	bool OnPointConnected_Event(const VS_PointParams* prm) override;
	bool OnPointDisconnected_Event(const VS_PointParams* prm) override;

	// Service implementation
	void Join_Method(VS_Container &cnt);
	void InvitationUpdate_Method(VS_Container &cnt);
	void ConferenceCreated_Method(const char* named_conf_id, const char* stream_conf_id);
	void GetASOfMyDomain_Method(VS_Container& cnt);
};
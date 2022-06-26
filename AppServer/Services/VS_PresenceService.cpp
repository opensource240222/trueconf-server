
/*************************************************
 * $Revision: 58 $
 * $History: VS_PresenceService.cpp $
 *
 * *****************  Version 58  *****************
 * User: Mushakov     Date: 18.07.12   Time: 20:28
 * Updated in $/VSNA/Servers/AppServer/Services
 * - call by alias when users are on separate servers
 *
 * *****************  Version 57  *****************
 * User: Mushakov     Date: 4.05.12    Time: 20:33
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - onConnect (err) handled in transcoder
 *  - refactoring subscribtion (VS_ExternalPresenceInterface added)
 *
 * *****************  Version 56  *****************
 * User: Mushakov     Date: 6.04.12    Time: 13:57
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - shared_ptr used for m_sipProtocol
 *
 * *****************  Version 55  *****************
 * User: Ktrushnikov  Date: 6.04.12    Time: 10:50
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Authorized INVITE support added (login-pass from Registru "SIPAuth")
 * - IsSIPCallID in one singleton
 *
 * *****************  Version 54  *****************
 * User: Mushakov     Date: 28.03.12   Time: 20:27
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - resolve and subscribe terminals
 *
 * *****************  Version 53  *****************
 * User: Mushakov     Date: 21.03.12   Time: 19:32
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - gateway included in vcs (for use uncomment #define INCLUDE_GATEWAY
 * in VS_GatewayStarter.cpp)
 *
 * *****************  Version 52  *****************
 * User: Ktrushnikov  Date: 13.12.11   Time: 16:59
 * Updated in $/VSNA/Servers/AppServer/Services
 * - always do NetworkResolve for SIP call_id
 *
 * *****************  Version 51  *****************
 * User: Ktrushnikov  Date: 11/17/11   Time: 2:44p
 * Updated in $/VSNA/Servers/AppServer/Services
 * - "sip:" support in NetworkResolve()
 *
 * *****************  Version 50  *****************
 * User: Mushakov     Date: 20.07.11   Time: 16:21
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - VS_ResolveServerFinder added
 *
 * *****************  Version 49  *****************
 * User: Mushakov     Date: 13.07.11   Time: 17:35
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Roaming
 *
 * *****************  Version 48  *****************
 * User: Mushakov     Date: 8.07.11    Time: 19:02
 * Updated in $/VSNA/Servers/AppServer/Services
 * - getting DN by vcs from bs (roaming)
 *
 * *****************  Version 47  *****************
 * User: Mushakov     Date: 7.07.11    Time: 18:08
 * Updated in $/VSNA/Servers/AppServer/Services
 * - getting rs address thru DNS
 * - Get server name by address (in server)
 * - fix bugs in start services sequence (AS)
 *  - Roaming in service added;
 *
 *
 * *****************  Version 46  *****************
 * User: Mushakov     Date: 31.05.11   Time: 21:23
 * Updated in $/VSNA/Servers/AppServer/Services
 * - roaming
 *
 * *****************  Version 45  *****************
 * User: Mushakov     Date: 31.05.11   Time: 18:15
 * Updated in $/VSNA/Servers/AppServer/Services
 * - broken roaming fixed
 *
 * *****************  Version 44  *****************
 * User: Ktrushnikov  Date: 11.03.11   Time: 17:56
 * Updated in $/VSNA/Servers/AppServer/Services
 * VS_PresenceService::Resolve()
 * - default status is from func CheckOfflineStatus() (not just
 * USER_INVALID)
 *
 * *****************  Version 43  *****************
 * User: Mushakov     Date: 10.03.11   Time: 20:20
 * Updated in $/VSNA/Servers/AppServer/Services
 * - add USER_INVALID status
 * - revoke/assign ROAMING handled
 *
 * *****************  Version 42  *****************
 * User: Mushakov     Date: 2.03.11    Time: 17:43
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - roaming
 *
 * *****************  Version 41  *****************
 * User: Mushakov     Date: 9.02.11    Time: 19:51
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - subscribtions between vcs
 *
 * *****************  Version 40  *****************
 * User: Ktrushnikov  Date: 28.01.11   Time: 22:24
 * Updated in $/VSNA/Servers/AppServer/Services
 * VCS3.2
 * - VS_PresenceService::NetworkResolve() support for VCS (using
 * confRestrick with AS)
 * - common func VS_GetServerFromCallID() to VS_Utils
 *
 * *****************  Version 39  *****************
 * User: Mushakov     Date: 24.01.11   Time: 21:30
 * Updated in $/VSNA/Servers/AppServer/Services
 * - server added in status (via VCS)
 *
 * *****************  Version 38  *****************
 * User: Mushakov     Date: 20.01.11   Time: 18:16
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - Subscriptions via vcs servers supported
 *
 * *****************  Version 37  *****************
 * User: Ktrushnikov  Date: 16.09.10   Time: 15:17
 * Updated in $/VSNA/Servers/AppServer/Services
 * Arch 3.1 Conf Loggin duplicates remove
 * - RS store BS server of user
 * - ConfLog: uses BS got from RS to send logs
 * - ConfLog: dprint loggin added
 *
 * *****************  Version 36  *****************
 * User: Ktrushnikov  Date: 28.01.10   Time: 17:32
 * Updated in $/VSNA/Servers/AppServer/Services
 * VCS:
 * - Interface added to ConfRestriction for SetUserStatus in Registry
 * - don't delete MultiConf registry key at startup of VCS (AS)
 *
 * *****************  Version 35  *****************
 * User: Ktrushnikov  Date: 27.01.10   Time: 13:59
 * Updated in $/VSNA/Servers/AppServer/Services
 * VCS:
 * - Fix user status in Registry
 *
 * *****************  Version 34  *****************
 * User: Mushakov     Date: 5.10.09    Time: 19:27
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - VS_SyncPool synchronized
 *
 * *****************  Version 33  *****************
 * User: Smirnov      Date: 4.05.09    Time: 20:36
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - remove doubled status update from user
 *
 * *****************  Version 32  *****************
 * User: Mushakov     Date: 30.01.09   Time: 16:57
 * Updated in $/VSNA/Servers/AppServer/Services
 * - time added in dprint#
 * - reset flag added to sync
 * - wait reset timeout added to unsubscribe
 * - logging large msg (>25000 b)
 *
 * *****************  Version 31  *****************
 * User: Mushakov     Date: 23.01.09   Time: 19:44
 * Updated in $/VSNA/Servers/AppServer/Services
 * - statuses and subs pack size decreased
 * - set g_AppServer to 0 when ManagerService was destroyed
 * - SERVER_PARAM removed from status pack AS-> RS
 *
 * *****************  Version 30  *****************
 * User: Smirnov      Date: 27.12.08   Time: 20:19
 * Updated in $/VSNA/Servers/AppServer/Services
 * - fixed error with null user in resolve
 *
 * *****************  Version 29  *****************
 * User: Mushakov     Date: 30.10.08   Time: 20:33
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - Alias status sending removed (AS ->RS )
 * - Logging of subType added
 *
 * *****************  Version 28  *****************
 * User: Smirnov      Date: 3.06.08    Time: 21:46
 * Updated in $/VSNA/Servers/AppServer/Services
 * - bugfix #4365
 *
 * *****************  Version 27  *****************
 * User: Mushakov     Date: 22.05.08   Time: 19:09
 * Updated in $/VSNA/Servers/AppServer/Services
 * - исправлено залипание статусов
 * - login time added to statuses
 * - commets added
 *
 * *****************  Version 26  *****************
 * User: Stass        Date: 2.04.08    Time: 21:52
 * Updated in $/VSNA/Servers/AppServer/Services
 * fixed seq_id
 *
 * *****************  Version 25  *****************
 * User: Stass        Date: 2.04.08    Time: 21:44
 * Updated in $/VSNA/Servers/AppServer/Services
 * added set server to BS presense
 *
 * *****************  Version 24  *****************
 * User: Stass        Date: 18.02.08   Time: 22:26
 * Updated in $/VSNA/Servers/AppServer/Services
 * fixed send to uplink
 *
 * *****************  Version 23  *****************
 * User: Stass        Date: 18.02.08   Time: 21:43
 * Updated in $/VSNA/Servers/AppServer/Services
 * added alias support (first ver)
 *
 * *****************  Version 22  *****************
 * User: Stass        Date: 15.02.08   Time: 13:04
 * Updated in $/VSNA/Servers/AppServer/Services
 * unified statues to container
 *
 * *****************  Version 21  *****************
 * User: Mushakov     Date: 12.02.08   Time: 19:20
 * Updated in $/VSNA/Servers/AppServer/Services
 * SeApptProperties realized
 *
 * *****************  Version 20  *****************
 * User: Dront78      Date: 8.02.08    Time: 20:01
 * Updated in $/VSNA/Servers/AppServer/Services
 * Resolve method updated.
 *
 * *****************  Version 19  *****************
 * User: Stass        Date: 5.02.08    Time: 21:31
 * Updated in $/VSNA/Servers/AppServer/Services
 * changed push source to local in login and logoff events
 *
 * *****************  Version 18  *****************
 * User: Dront78      Date: 23.01.08   Time: 17:51
 * Updated in $/VSNA/Servers/AppServer/Services
 * _malloca redefinition warning fixed.
 *
 * *****************  Version 17  *****************
 * User: Stass        Date: 22.01.08   Time: 22:01
 * Updated in $/VSNA/Servers/AppServer/Services
 * added home server subscribtion to status
 *
 * *****************  Version 16  *****************
 * User: Stass        Date: 15.01.08   Time: 22:02
 * Updated in $/VSNA/Servers/AppServer/Services
 * fixes while logout
 *
 * *****************  Version 15  *****************
 * User: Dront78      Date: 28.12.07   Time: 20:08
 * Updated in $/VSNA/Servers/AppServer/Services
 * bool Init( const char *our_endpoint, const char *our_service, const
 * bool permittedAll = false ); added.
 *
 * *****************  Version 14  *****************
 * User: Stass        Date: 14.12.07   Time: 18:30
 * Updated in $/VSNA/Servers/AppServer/Services
 * done register/unregister processing with queue
 *
 * *****************  Version 13  *****************
 * User: Stass        Date: 10.12.07   Time: 18:41
 * Updated in $/VSNA/Servers/AppServer/Services
 * fixed RS set to myself
 *
 * *****************  Version 12  *****************
 * User: Ktrushnikov  Date: 5.12.07    Time: 13:13
 * Updated in $/VSNA/Servers/AppServer/Services
 * - valid overload of Init()-method
 *
 * *****************  Version 11  *****************
 * User: Stass        Date: 30.11.07   Time: 20:36
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 28.11.07   Time: 18:43
 * Updated in $/VSNA/Servers/AppServer/Services
 * fixed subscribe
 *
 * *****************  Version 9  *****************
 * User: Stass        Date: 28.11.07   Time: 18:34
 * Updated in $/VSNA/Servers/AppServer/Services
 * added subscribe
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 28.11.07   Time: 13:28
 * Updated in $/VSNA/Servers/AppServer/Services
 * fix for get subs
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 27.11.07   Time: 19:18
 * Updated in $/VSNA/Servers/AppServer/Services
 * bugfix
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 26.11.07   Time: 16:08
 * Updated in $/VSNA/Servers/AppServer/Services
 * fixed storage
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 26.11.07   Time: 15:17
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 22.11.07   Time: 20:34
 * Updated in $/VSNA/Servers/AppServer/Services
 * new iteration
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 20.11.07   Time: 16:13
 * Updated in $/VSNA/Servers/AppServer/Services
 * renamed services
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 15.11.07   Time: 15:28
 * Updated in $/VSNA/Servers/AppServer/Services
 * updates
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 13.11.07   Time: 17:32
 * Created in $/VSNA/Servers/AppServer/Services
 * new services
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:50
 * Created in $/VSNA/Servers/MediaBroker/BrokerServices
 *
 * ***********************************************/
#include "std-generic/cpplib/VS_Container.h"
#include "std/cpplib/VS_Map.h"
#include "std/cpplib/VS_SimpleStr.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/VS_RegServer.h"
#include "std/cpplib/VS_JsonConverter.h"
#include "std/cpplib/VS_CallIDUtils.h"

#include "../../ServerServices/Common.h"
#include "std-generic/cpplib/string_view.h"


#include "AppServer/Services/VS_Storage.h"

#include "VS_PresenceService.h"
#include "VS_AppServerData.h"
#include "../../ServerServices/VS_ReadLicense.h"
#include "statuslib/VS_ExternalPresenceInterface.h"
#include "transport/Router/VS_RouterMessage_io.h"
#include "TrueGateway/VS_GatewayStarter.h"
#include "std/debuglog/VS_Debug.h"

#include <boost/asio/io_service.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_USPRS

////////////////////////////////////////////////////////////////////////////////
// Global
////////////////////////////////////////////////////////////////////////////////

static const auto c_reset_timeout = std::chrono::seconds(12);

////////////////////////////////////////////////////////////////////////////////
// Init
////////////////////////////////////////////////////////////////////////////////

VS_PresenceService::VS_PresenceService(boost::asio::io_service& ios)
	: m_ios(ios)
{
	m_TimeInterval = std::chrono::seconds(10);
}

bool VS_PresenceService::Init(const char *our_endpoint, const char *our_service, const bool permittedAll )
{
	std::lock_guard<decltype(m_lock)> lock(m_lock);
	m_AliasesChangedConn = m_confRestriction->Connect_AliasesChanged([this](std::multimap<std::string, std::string> new_aliases) {OnAliasesChanged(std::move(new_aliases));});
	m_confRestriction->UpdateAliasList();

	VS_RegistryKey key(false, "Federation");
	if (key.IsValid())
	{
		long value(0);
		key.GetValue(&value, sizeof(long), VS_REG_INTEGER_VT, "Enabled");
		eRoamingMode_t mode = (eRoamingMode_t)value;

		if (mode <= RM_INVALID || mode > RM_BLACKLIST)
			mode = RM_BLACKLIST;

		std::string list;
		const char* list_name = 0;
		if (mode == RM_DISABLED)
		{
		}
		else if (mode == RM_WHITELIST) {
			list_name = "WhiteList";
		}
		else if (mode == RM_BLACKLIST) {
			list_name = "BlackList";
		}

		if (list_name && *list_name)
			key.GetString(list, list_name);

		m_confRestriction->SetRoamingSettings(mode, list);
	}
	return true;
};

////////////////////////////////////////////////////////////////////////////////
// Message Processing
////////////////////////////////////////////////////////////////////////////////

bool VS_PresenceService::Processing( std::unique_ptr<VS_RouterMessage> &&recvMess)
{
	if (recvMess == 0)	return true;
	switch (recvMess->Type()) {
	case transport::MessageType::Invalid:	// Skip
		break;
	case transport::MessageType::Request:
		m_recvMess = recvMess.get();
		{
			VS_Container cnt;
			string_view hub_id = OurEndpoint();

			if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
				const char* method = 0;
				if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0)
				{
					dprint3("PresenceService: Processing request %20s; cid:%s serv:%s user:%s bytes:%zu\n", method, recvMess->SrcCID(), recvMess->SrcServer(), recvMess->SrcUser(), recvMess->BodySize());
				// Process methods
					if (strcasecmp(method, USERREGISTRATIONINFO_METHOD) == 0)
					{
						UserRegistrationInfo_Method();
					}
					else if (strcasecmp(method, UPDATESTATUS_METHOD) == 0)
					{
						if (!CheckHops(cnt))
							return true;
						if (hub_id == recvMess->SrcServer())
						{
							UpdateClientStatus_Method(m_recvMess->SrcUser(),cnt);
						}
						else
						{
							int32_t cause = 0;
							cnt.GetValue(CAUSE_PARAM,cause);

							VS_FullID source(recvMess->SrcServer(), recvMess->SrcUser());
							UpdateStatus_Method(cnt, source, cause==1);
						}
					}
					else if (strcasecmp(method,PUSHSTATUSDIRECTLY_METHOD) == 0)
					{
						if (!CheckHops(cnt))
							return true;
						PushStatusDirectly_Method(cnt);
					}
					else if (strcasecmp(method, GETUSERSTATUS_METHOD) == 0)
					{
						GetUserStatus_Method(cnt.GetStrValueRef(CALLID_PARAM));
					}
					else if (strcasecmp(method, GETALLUSERSTATUS_METHOD) == 0)
					{
						GetAllUserStatus_Method(VS_FullID(recvMess->SrcServer(),recvMess->SrcUser()));
					}
					else if (strcasecmp(method, GETSUBSCRIPTIONS_METHOD) == 0)
					{
						GetSubscriptions_Method(recvMess->SrcServer());
					}
					else if (strcasecmp(method, SUBSCRIBE_METHOD) == 0)
					{
						if((!m_confRestriction || !m_confRestriction->IsVCS()) && hub_id!= recvMess->SrcServer() )
						{
							dprint3("not local subscribe for %s is denied in AS\n", recvMess->SrcServer());
						}
						else if(m_confRestriction && m_confRestriction->IsVCS() &&
							hub_id!= recvMess->SrcServer() &&
							!IsRoamingAllowed())
						{
							dprint3("not local subscribe for %s in denied. Roaming is not allowed\n",recvMess->SrcServer());
							VS_Container status_cnt;
							status_cnt.AddValue(METHOD_PARAM, UPDATESTATUS_METHOD);
							long hops = VS_SubscriptionHub::DEFAULT_HOPS - 1;		// minus one for myself
							status_cnt.AddValueI32(HOPS_PARAM, hops);
							unsigned long status_count(0);
							cnt.Reset();
							while(cnt.Next())
							{
								if(strcmp(cnt.GetName(),CALLID_PARAM)==0)
								{
									status_cnt.AddValue(CALLID_PARAM, cnt.GetStrValueRef());
									status_cnt.AddValueI32(USERPRESSTATUS_PARAM, USER_INVALID);
									status_cnt.AddValue(SERVER_PARAM, "");
									status_count++;
								}
							}
							VS_FullID subscriber(recvMess->SrcServer(),0);
							if(status_count>0)
								SeqPost(subscriber,status_cnt);
						}
						else
						{
							if(!recvMess->IsFromServer())
								SubscribeClient_Method(cnt);
							else
								SubscribeServer_Method(cnt);
						}
					}
					else if (strcasecmp(method, UNSUBSCRIBE_METHOD) == 0)
					{
						if((!m_confRestriction || !m_confRestriction->IsVCS()) && hub_id!=recvMess->SrcServer())
						{
							dprint3("not local subscribe for %s is denied in AS\n", recvMess->SrcServer());
						}
						else if(m_confRestriction && m_confRestriction->IsVCS() &&
							hub_id!= recvMess->SrcServer() &&
							!IsRoamingAllowed())
						{
							dprint3("not local unsubscribe for %s in denied. Roaming is not allowed\n",recvMess->SrcServer());
						}
						else
						{
							if(!recvMess->IsFromServer())
								UnsubscribeClient_Method(cnt);
							else
								UnsubscribeServer_Method(cnt);
						}
					}
					else if (strcasecmp(method, RESOLVE_METHOD) == 0)
					{
						Resolve_Method(cnt.GetStrValueRef(CALLID_PARAM));
					}
					else
						VS_SubscriptionHub::Processing(std::move(recvMess));
				}
			}
		}
		break;
	case transport::MessageType::Reply:
		m_recvMess = recvMess.get();
		{
			VS_Container cnt;
			if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize()))
			{
				const char* method = 0;
				if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0)
				{
					dprint3("PresenceService: Processing reply %20s; cid:%s serv:%s user:%s \n", method, recvMess->SrcCID(), recvMess->SrcServer(), recvMess->SrcUser());
					if (strcasecmp(method, GETALLUSERSTATUS_METHOD) == 0)
					{
						int32_t cause;
						cnt.GetValue(CAUSE_PARAM,cause);

						UpdateStatus_Method(cnt,VS_FullID(recvMess->SrcServer(),recvMess->SrcUser()), cause==1);
					}
					else
						VS_SubscriptionHub::Processing(std::move(recvMess));
				}
			}
		}
		break;
	case transport::MessageType::Notify:
		break;
	}
	m_recvMess = nullptr;
	return true;
}

void VS_PresenceService::GetAllUserStatus_Method(const VS_FullID& subscriber)
{
	int users(0);
	auto sent_statuses = users;
	if(m_recvMess->IsFromServer())
	{// request from server
		OnUplinkConnected_Event(subscriber.m_serverID);
	}
	else
	{//endpoint request
		dprint3("@GetAllUserStatus from %s:%s\n",subscriber.m_serverID.m_str,subscriber.m_userID.m_str);

		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, GETALLUSERSTATUS_METHOD);
		rCnt.AddValueI32(CAUSE_PARAM, 1);
		int32_t seq_id = m_out_sync.Reset(subscriber);
		rCnt.AddValueI32(SEQUENCE_ID_PARAM, seq_id);
		PostReply(rCnt);
	}
	// fix bug #49315
	do {
		VS_Container cnt;
		cnt.AddValue(METHOD_PARAM, UPDATESTATUS_METHOD);
		long hops = VS_SubscriptionHub::DEFAULT_HOPS - 1;		// minus one for myself
		cnt.AddValueI32(HOPS_PARAM, hops);
		users = GetNextSubscriberStatuses(subscriber, cnt, true, true, 200);
		sent_statuses += users;
		if (users > 0)
			SeqPost(subscriber, cnt);
	} while (IsCurrentSubscriberStatusesExist());
	dstream3 << " resend " << sent_statuses << " statuses by subscription";
}

void VS_PresenceService::GetSubscriptions_Method(const char* server)
{
	if(IsRoamingAllowed())
	{
		dprint1("Server %s requested subscription update\n",server);
		ServerConnected_Method(server);
	}
	else
	{
		dprint1("@GetSubscriptions from %s not allowed\n", server);
	}
}

void VS_PresenceService::GetUserStatus_Method(const char* call_id)
{
	if (!call_id || !*call_id) return;

	VS_CallIDInfo ci = GetStatus(call_id);
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, GETUSERSTATUS_METHOD);
	cnt.AddValue(CALLID_PARAM, call_id);
	ci.ToContainer(cnt, true, true);
	PostReply(cnt);
}

void VS_PresenceService::UserRegistrationInfo_Method()
{
	VS_RouterMessage *msg = new VS_RouterMessage(m_recvMess->SrcService(), m_recvMess->AddString(), 0, 0, m_recvMess->SrcUser(), 0, OurEndpoint(), 30000, m_recvMess->Body(), m_recvMess->BodySize());
	if (!PostMes(msg))
			delete msg;
}

void VS_PresenceService::UpdateStatus_Method(const VS_Container& cnt, const VS_FullID& source_ep, bool erase)
{
	if (!VS_SyncPool::CheckExistenceParamsForSync(cnt))
		return;
	VS_CallIDInfo::Source source;
	if(!m_recvMess->IsFromServer())
	{
		dstream4 << "denied roaming status update not from server: " << *m_recvMess;
		return;
	}

	source=VS_CallIDInfo::UPLINK;
	if(erase)
		CleanUplink(source_ep.m_serverID.m_str);

	if (!m_in_sync.ConsistentCheck(source_ep, cnt))
	{
		VS_Container reset_cnt;
		reset_cnt.AddValue(METHOD_PARAM, GETALLUSERSTATUS_METHOD);
		PostRequest(source_ep.m_serverID, source_ep.m_userID, reset_cnt, NULL);
	}

	int32_t hops = VS_SubscriptionHub::DEFAULT_HOPS;
	cnt.GetValue(HOPS_PARAM, hops);

	int i=0;
	VS_SimpleStr call_id;
	VS_CallIDInfo ci;
	bool set_server=false;

	int32_t lval(0);
	cnt.Reset();
	while(cnt.Next())
	{
		const char *tmp;

		if(strcasecmp(cnt.GetName(),CALLID_PARAM)==0 || strcasecmp(cnt.GetName(),USERNAME_PARAM)==0)
		{
			if(i>0) //not first item
			{
				std::lock_guard<decltype(m_lock)> lock(m_lock);
				VS_CallIdDataMap::Iterator	alias_i = m_callIdMap[call_id];
				if(!!alias_i)
				{
					for(VS_StrI_IntMap::Iterator ii = alias_i->data->Begin();!!ii;ii++)
					{
						VS_SimpleStr	alias = ii->key;
						VS_CallIDInfo	push_ci = ci;
						PushStatus(alias,push_ci,source,set_server,source_ep.m_serverID,source_ep.m_userID, hops);
					}
					PushStatus(call_id,ci,source,set_server,source_ep.m_serverID,source_ep.m_userID, hops);
				}
				else
					PushStatus(call_id,ci,source,set_server,source_ep.m_serverID,source_ep.m_userID, hops);
			}

			i++;
			call_id=cnt.GetStrValueRef();
			ci.Empty();
			set_server=false;
		}
		else if(strcasecmp(cnt.GetName(),USERPRESSTATUS_PARAM)==0)
		{
			if(cnt.GetValue(lval))
				ci.m_status=(VS_UserPresence_Status)lval;
		}
		else if(strcasecmp(cnt.GetName(), EXTSTATUS_PARAM) == 0)
		{
			VS_Container st_cnt;
			if (cnt.GetValue(st_cnt))
				ci.m_extStatusStorage.UpdateStatus(st_cnt);
		}
		else if(strcasecmp(cnt.GetName(),SERVER_PARAM)==0)
		{
			if(source == ci.UPLINK)
			{
				tmp = cnt.GetStrValueRef();
				ci.m_serverID=!!tmp ? tmp : std::string{};
				set_server=true;
			};
		}
		else if(strcasecmp(cnt.GetName(),TYPE_PARAM)==0)
		{
			if(cnt.GetValue(lval))
				ci.m_type=lval;
		}
		else if(strcasecmp(cnt.GetName(),REALID_PARAM)==0)
		{
			tmp = cnt.GetStrValueRef();
			ci.m_realID=!!tmp ? tmp : std::string{};
		}
		else if(strcasecmp(cnt.GetName(),LOCATORBS_PARAM)==0)
		{
			tmp = cnt.GetStrValueRef();
			ci.m_homeServer=!!tmp ? tmp : std::string{};
		}
		else if (strcasecmp(cnt.GetName(), DISPLAYNAME_PARAM) == 0)
		{
			if (cnt.GetStrValueRef())
				ci.m_displayName = cnt.GetStrValueRef();
		}
		else if (strcasecmp(cnt.GetName(), MULTI_LOGIN_CAPABILITY_PARAM) == 0)
		{
			cnt.GetValueI32(ci.m_ml_cap);
		}
	};

	if(i>0) //found items
	{
		std::lock_guard<decltype(m_lock)> lock(m_lock);

		VS_CallIdDataMap::Iterator alias_i = m_callIdMap[call_id];
		if(!!alias_i)
		{
			for (VS_StrI_IntMap::Iterator ii = alias_i->data->Begin(); !!ii; ii++)
			{
				VS_SimpleStr	alias = ii->key;
				VS_CallIDInfo	push_ci = ci;
				PushStatus(alias,push_ci,source,set_server,source_ep.m_serverID,source_ep.m_userID, hops);
			}
			PushStatus(call_id,ci,source,set_server,source_ep.m_serverID,source_ep.m_userID, hops);
		}
		else
			PushStatus(call_id,ci,source,set_server,source_ep.m_serverID,source_ep.m_userID, hops);
	}
};

void VS_PresenceService::PushStatusDirectly_Method(VS_Container &cnt)
{
	VS_FullID source_ep(m_recvMess->SrcServer(),m_recvMess->SrcUser());
	VS_FullID our_server(OurEndpoint(),0);
	if(source_ep!=our_server)
		return;
	VS_CallIDInfo::Source source = VS_CallIDInfo::LOCAL;

	int32_t hops = VS_SubscriptionHub::DEFAULT_HOPS;
	cnt.GetValue(HOPS_PARAM, hops);

	int i=0;
	VS_SimpleStr call_id;
	VS_CallIDInfo ci;
	bool set_server=false;

	int32_t lval;

	cnt.Reset();

	while(cnt.Next())
	{
		const char *tmp;
		if(strcasecmp(cnt.GetName(),CALLID_PARAM)==0 || strcasecmp(cnt.GetName(),USERNAME_PARAM)==0)
		{
			if(i>0) //not first item
			{
				std::lock_guard<decltype(m_lock)> lock(m_lock);
				VS_CallIdDataMap::Iterator	alias_i = m_callIdMap[call_id];
				if(!!alias_i)
				{
					for (VS_StrI_IntMap::Iterator ii = alias_i->data->Begin(); !!ii; ii++)
					{
						VS_SimpleStr	alias = ii->key;
						VS_CallIDInfo	push_ci = ci;
						PushStatus(alias, push_ci, source, set_server, source_ep.m_serverID, source_ep.m_userID, hops);
					}
					PushStatus(call_id, ci, source, set_server, source_ep.m_serverID, source_ep.m_userID, hops);
				}
				else
					PushStatus(call_id, ci, source, set_server, source_ep.m_serverID, source_ep.m_userID, hops);
			}

			i++;
			call_id=cnt.GetStrValueRef();
			ci.Empty();
			set_server=false;
		}
		else if(strcasecmp(cnt.GetName(),USERPRESSTATUS_PARAM)==0)
		{
			if(cnt.GetValue(lval))
				ci.m_status=(VS_UserPresence_Status)lval;
		}
		else if(strcasecmp(cnt.GetName(), EXTSTATUS_PARAM) == 0)
		{
			VS_Container st_cnt;
			if (cnt.GetValue(st_cnt))
				ci.m_extStatusStorage.UpdateStatus(st_cnt);
		}
		else if(strcasecmp(cnt.GetName(),SERVER_PARAM)==0)
		{
			if(source == ci.UPLINK)
			{
				tmp = cnt.GetStrValueRef();
				ci.m_serverID=!!tmp ? tmp : std::string{};
				set_server=true;
			};
		}
		else if(strcasecmp(cnt.GetName(),TYPE_PARAM)==0)
		{
			if(cnt.GetValue(lval))
				ci.m_type=lval;
		}
		else if(strcasecmp(cnt.GetName(),REALID_PARAM)==0)
		{
			tmp = cnt.GetStrValueRef();
			ci.m_realID=!!tmp ? tmp : std::string{};
		}
		else if(strcasecmp(cnt.GetName(),LOCATORBS_PARAM)==0)
		{
			tmp = cnt.GetStrValueRef();
			ci.m_homeServer=!!tmp ? tmp : std::string{};
		}
		else if (strcasecmp(cnt.GetName(), MULTI_LOGIN_CAPABILITY_PARAM) == 0)
		{
			cnt.GetValueI32(ci.m_ml_cap);
		}

	};

	if(i>0) //found items
	{
		std::lock_guard<decltype(m_lock)> lock(m_lock);
		VS_CallIdDataMap::Iterator alias_i = m_callIdMap[call_id];
		if(!!alias_i)
		{
			for (VS_StrI_IntMap::Iterator ii = alias_i->data->Begin(); !!ii; ii++)
			{
				VS_SimpleStr	alias = ii->key;
				VS_CallIDInfo	push_ci = ci;
				PushStatus(alias, push_ci, source, set_server, source_ep.m_serverID, source_ep.m_userID, hops);
			}
			PushStatus(call_id, ci, source, set_server, source_ep.m_serverID, source_ep.m_userID, hops);
		}
		else
			PushStatus(call_id, ci, source, set_server, source_ep.m_serverID, source_ep.m_userID, hops);
	}
	/**
		TODO: асинхронный пуш
	*/

}
void VS_PresenceService::UpdateClientStatus_Method(const vs_user_id &user_id, VS_Container &cnt)
{
	int32_t status;
	cnt.GetValue(USERPRESSTATUS_PARAM, status);
	dprint3("Update status from local %s status=%d\n",user_id.m_str, status);
	if(!user_id)
		return;
	VS_SimpleStr real_call_id = user_id;
	VS_ExtendedStatusStorage user_ext_st;
	VS_RealUserLogin login(SimpleStrToStringView(user_id));
	/* Skip writing extended statuses for guests. */
	if (!login.IsGuest())
	{
		VS_Container extStatusCnt;
		if (cnt.GetValue(EXTSTATUS_PARAM, extStatusCnt))
			user_ext_st.UpdateStatus(extStatusCnt);
	}
	std::lock_guard<decltype(m_lock)> lock(m_lock);
	VS_StatusMap::Iterator st = m_statusCache[real_call_id];
	if ((!!st && st->data->m_status==status)&&(st->data->m_extStatusStorage == user_ext_st))
		return;
	VS_CallIDInfo ci;
	if (!!st)
		ci = st.data();
	int32_t hops = VS_SubscriptionHub::DEFAULT_HOPS;
	cnt.GetValue(HOPS_PARAM, hops);
	ci.m_extStatusStorage += user_ext_st;
	VS_UserPresence_Status old_status = ci.m_status;
	ci.m_status = (VS_UserPresence_Status)status;
	ci.m_ml_cap = VS_MultiLoginCapability::SINGLE_USER;
	VS_CallIdDataMap::Iterator alias_i = m_callIdMap[real_call_id];
	if(!!alias_i) {
		for (VS_StrI_IntMap::Iterator ii = alias_i->data->Begin(); !!ii; ii++) {
			VS_SimpleStr	alias = ii->key;
			VS_CallIDInfo push_ci = ci;
			PushStatus(alias, push_ci, ci.LOCAL, false, OurEndpoint(), user_id, hops);
		}
	}
	PushStatus(user_id, ci, ci.LOCAL, false, OurEndpoint(), user_id, hops);

	if (old_status != ci.m_status) { // Log
		VS_Container cnt2;
		cnt2.AddValue(DISPLAYNAME_PARAM, ci.m_displayName);
		cnt2.AddValue(NEW_STATUS_PARAM, ci.m_status);
		cnt2.AddValue(OLD_STATUS_PARAM, old_status);

		std::string payload = ConvertToJsonStr(cnt2);
		VS_Container log_cnt;
		log_cnt.AddValue(OBJECT_TYPE_PARAM, USER_OBJECT_TYPE);
		log_cnt.AddValue(OBJECT_NAME_PARAM, user_id);
		log_cnt.AddValue(EVENT_TYPE_PARAM, STATUS_USER_EVENT_TYPE);
		log_cnt.AddValue(PAYLOAD_PARAM, payload.c_str());
		PostRequest(OurEndpoint(), 0, log_cnt, 0, LOG_SRV);
	};

	m_ios.post([real_call_id, status = ci.m_status, ext_status = ci.m_extStatusStorage]() {
		if (g_storage)
			g_storage->SetUserStatus(real_call_id, status, ext_status);
	});
}

////////////////////////////////////////////////////////////////////////////////
// Events
////////////////////////////////////////////////////////////////////////////////
void VS_PresenceService::OnUserLoginEnd_Event(const VS_UserData& ud, const std::string &cid)
{
	if (!IsInProcessingThread())
	{
		CallInProcessingThread([this,ud,cid]()
		{
			OnUserLoginEnd_Event(ud,cid);
		});
		return;
	}

	const vs_user_id& user_id = ud.m_name;
	int user_type = ud.m_type;
	const auto & aliases = ud.m_aliases;

	if (!user_id) return;
	RegisterAliases(user_id,aliases);

	VS_CallIDInfo ci;
	ci.m_status=USER_AVAIL;
	const auto tmp = OurEndpoint();
	ci.m_serverID = !!tmp ? tmp : std::string{};
	ci.m_realID = std::string{ user_id.m_str, (size_t)user_id.Length() };
	ci.m_type=user_type;
	ci.m_homeServer = std::string{ ud.m_homeServer.m_str,  (size_t)ud.m_homeServer.Length() };
	ci.m_displayName=ud.m_displayName;
	ci.m_logginTime = std::chrono::system_clock::now();
	ci.m_ml_cap = VS_MultiLoginCapability::SINGLE_USER;

	VS_SimpleStr	real_call_id = user_id;
	long hops = VS_SubscriptionHub::DEFAULT_HOPS - 1;	// minus one for my server

	VS_CallIdDataMap::Iterator alias_i = m_callIdMap[real_call_id];
	if(!!alias_i)
	{
		for (VS_StrI_IntMap::Iterator ii = alias_i->data->Begin(); !!ii; ii++)
		{
			VS_SimpleStr	alias = ii->key;
			VS_CallIDInfo	push_ci = ci;
			PushStatus(alias,push_ci,VS_CallIDInfo::LOCAL,true,OurEndpoint(),0,hops);
		}
		PushStatus(user_id,ci,VS_CallIDInfo::LOCAL,true,OurEndpoint(),0,hops);
	}
	else
	{
		PushStatus(user_id,ci,VS_CallIDInfo::LOCAL,true,OurEndpoint(),0,hops);
	}

	{ // Log
		VS_Container cnt;
		cnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
		cnt.AddValue(NEW_STATUS_PARAM, ci.m_status);
		cnt.AddValue(OLD_STATUS_PARAM, USER_STATUS_UNDEF);

		std::string payload = ConvertToJsonStr(cnt);
		VS_Container log_cnt;
		log_cnt.AddValue(OBJECT_TYPE_PARAM, USER_OBJECT_TYPE);
		log_cnt.AddValue(OBJECT_NAME_PARAM, ud.m_name);
		log_cnt.AddValue(EVENT_TYPE_PARAM, STATUS_USER_EVENT_TYPE);
		log_cnt.AddValue(PAYLOAD_PARAM, payload.c_str());
		PostRequest(OurEndpoint(), 0, log_cnt, 0, LOG_SRV);
	}

	m_ios.post([real_call_id, status = ci.m_status]() {
		if (g_storage)
			g_storage->SetUserStatus(real_call_id, status, VS_ExtendedStatusStorage());
	});
}


void VS_PresenceService::OnUserLogoff_Event(const VS_UserData& ud, const std::string &cid)
{
	if (!IsInProcessingThread())
	{
		CallInProcessingThread([this, ud,cid]()
		{
			OnUserLogoff_Event(ud,cid);
		});
		return;
	}

	const vs_user_id &user_id = ud.m_name;

	if (!user_id) return;
	long hops = VS_SubscriptionHub::DEFAULT_HOPS - 1;		// minus one for myself
	VS_SimpleStr	real_call_id = user_id;
	VS_CallIdDataMap::Iterator	alias_i = m_callIdMap[real_call_id];
	if(!!alias_i)
	{
		for (VS_StrI_IntMap::Iterator ii = alias_i->data->Begin(); !!ii; ii++)
		{
			VS_SimpleStr	alias = ii->key;
			PushStatus(alias,USER_LOGOFF,VS_ExtendedStatusStorage() ,VS_CallIDInfo::LOCAL,0,true,-1,0,0,hops);
		}
		PushStatus(user_id,USER_LOGOFF,VS_ExtendedStatusStorage(),VS_CallIDInfo::LOCAL,0,true,-1,0,0,hops);
	}
	else
		PushStatus(user_id,USER_LOGOFF,VS_ExtendedStatusStorage(),VS_CallIDInfo::LOCAL,0,true,-1,0,0,hops);

	UnsubscribeID(VS_FullID(OurEndpoint(), user_id));
	UnregisterAliases(user_id);

	m_ios.post([real_call_id]() {
		if (g_storage)
			g_storage->SetUserStatus(real_call_id, USER_LOGOFF, VS_ExtendedStatusStorage());
	});
}
void VS_PresenceService::OnUplinkConnected_Event(const char *server)
{
	VS_Container cnt;
	dstream4 << "VS_PresenceService::OnUplinkConnected_Event(" << server;
	cnt.AddValue(METHOD_PARAM, UPDATESTATUS_METHOD);
	cnt.AddValueI32(CAUSE_PARAM, 1);
	long hops = VS_SubscriptionHub::DEFAULT_HOPS - 1;		// minus one for myself
	cnt.AddValueI32(HOPS_PARAM, hops);
	auto resolve_finder = VS_ResolveServerFinder::Instance();
	auto users = resolve_finder->GetUsersForUplink(server);
	for (const auto&i : users)
	{
		auto st = m_statusCache[i.c_str()];
		const auto our_endpoint = OurEndpoint();
		if (!!st && (our_endpoint == nullptr || *our_endpoint == 0 ? st->data->m_serverID.empty() : st->data->m_serverID == our_endpoint))
		{
			auto alias_iter = m_aliasesMap[i.c_str()];
			string_view real_call_id;
			if (!!alias_iter && (alias_iter->data && *alias_iter->data))
				real_call_id = alias_iter->data;
			else
				real_call_id = i;
			if(real_call_id != i)
				continue;
			auto aliases = m_callIdMap[i.c_str()];
			if (!!aliases)
			{
				cnt.AddValue(CALLID_PARAM, i.c_str());
				for (auto iter = aliases->data->Begin(); !!iter; ++iter)
					cnt.AddValue(ALIAS_PARAM, iter->key);
			}
			else
			{
				dstream4 << " no aliases for call_id = " << i;
			}
			st->data->ToContainer(cnt, true, true); // send all
		}
	}
	SeqPost(VS_FullID(server, 0), cnt);
}

/////////////////////////////////////////////////
VS_UserPresence_Status VS_PresenceService::NetworkResolve(const char* uplink, const VS_SimpleStr& call_id,VS_CallIDInfo& ci)
{
	VS_SCOPE_EXIT{ dstream4 << "VS_PresenceService::NetworkResolve: finished; ci.m_status = " << ci.m_status; };
	dstream4 << "VS_PresenceService::NetworkResolve: uplink = " << uplink << "; call_id = " << call_id.m_str << ";\n";
	ci.m_status = USER_INVALID;
	VS_Container rCnt;
	unsigned long time = 30000;
	// send req
	rCnt.AddValue(METHOD_PARAM, RESOLVE_METHOD);
	rCnt.AddValue(CALLID_PARAM, call_id);
	rCnt.AddValueI32(MULTI_LOGIN_CAPABILITY_PARAM, VS_MultiLoginCapability::SINGLE_USER);
	auto req_lt = std::chrono::milliseconds(time);
	auto f = RequestResponse(uplink, 0, rCnt, req_lt, 0, PRESENCE_SRV, 10000);
	//wait for reply
	VS_RouterMessage *mes = nullptr;
	try
	{
		if (f.valid() && std::future_status::ready == f.wait_for(req_lt) && (mes = f.get()))
		{
			if (rCnt.Deserialize(mes->Body(), mes->BodySize()))
			{
				int32_t lval;
				if (rCnt.GetValue(USERPRESSTATUS_PARAM, lval))
					ci.m_status = (VS_UserPresence_Status)lval;
				const char *tmp = rCnt.GetStrValueRef(SERVER_PARAM);
				ci.m_serverID = !!tmp ? tmp : std::string{};
				tmp = rCnt.GetStrValueRef(REALID_PARAM);
				ci.m_realID = !!tmp ? tmp : std::string{};
				tmp = rCnt.GetStrValueRef(LOCATORBS_PARAM);
				ci.m_homeServer = !!tmp ? tmp : std::string{};
				auto pDn = rCnt.GetStrValueRef(DISPLAYNAME_PARAM);
				if (pDn) ci.m_displayName = pDn;
				if (rCnt.GetValue(TYPE_PARAM, lval))
					ci.m_type = (VS_UserPresence_Status)lval;
				VS_Container extStatusCnt;
				if (rCnt.GetValue(EXTSTATUS_PARAM, extStatusCnt))
					ci.m_extStatusStorage.UpdateStatus(extStatusCnt);
				rCnt.GetValueI32(MULTI_LOGIN_CAPABILITY_PARAM, ci.m_ml_cap);
			} //error in container
			delete mes;
		}//no reply - return INVALID
	}
	catch(const std::exception &e)
	{
		dstream0 << "Exception was raised while getting RequestResponse result. Message: "<<e.what();
	}
	return ci.m_status;
}

VS_UserPresence_Status VS_PresenceService::Resolve(VS_SimpleStr& call_id,VS_CallIDInfo& ci, bool use_cache, VS_UserData* from_ude,bool do_ext_resolve)
{
	bool found = false;
	bool found_ext_presence = false;
	dstream4 << "VS_PresenceService::Resolve: call_id = " << call_id.m_str << "; use_cache = " << use_cache << "; do_ext_resolve = " << do_ext_resolve;
	VS_SCOPE_EXIT{ dstream4 << "VS_PresenceService::Resolve: finished, found=" << found << ", found_ext_presence=" << found_ext_presence << ", call_id=" << call_id.m_str << ", ci.m_status=" << ci.m_status; };

	{// resolve alias > real_id
		std::lock_guard<decltype(m_lock)> lock(m_lock);
		auto ii = m_aliasesMap[call_id];
		if(!!ii)
			call_id = ii;
	}

	VS_SCOPE_EXIT
	{
		if (!ci.m_realID.empty())
			call_id = VS_SimpleStr{ (int)ci.m_realID.length(), ci.m_realID.c_str() };
	};

	VS_ResolveServerFinder	*resolve_srv = VS_ResolveServerFinder::Instance();

	{// try external GatewayPresence
		std::shared_ptr<VS_ExternalPresenceInterface> ext_presence;
		if (do_ext_resolve)
			ext_presence = resolve_srv->GetExternalPresence(call_id);
		if (ext_presence)
		{
			if (from_ude && !ext_presence->CanICall(from_ude, call_id, (m_confRestriction) ? m_confRestriction->IsVCS() : true))
				return USER_STATUS_UNDEF;
			std::string call_id_tmp = call_id.m_str;
			found_ext_presence = ext_presence->Resolve(call_id_tmp, ci, from_ude);
			call_id = call_id_tmp.c_str();
			if (found_ext_presence)
				return ci.m_status;
		}
	}

	call_id = vs::UTF8ToLower(call_id.m_str).c_str();		// we can lower call_id only after extPresence (see bug#23299)

	// todo: do CheckOfflineStatus() if not FindStatus()
	ci = CheckOfflineStatus(call_id);
	if (!call_id)
		return ci.m_status;

	auto cache = FindStatus(call_id.m_str);
	if (cache.found) {
		VS_ExtendedStatusStorage tmp = ci.m_extStatusStorage;
		ci = cache.info;
		ci.m_extStatusStorage = tmp;
		found = true;
	}
	if (!use_cache
		&& !found_ext_presence)
	{
		std::string server;
		if (resolve_srv->GetServerForResolve(call_id, server, false))
		{
			if (!server.empty()
				&& strcasecmp(server.c_str(), OurEndpoint()) == 0)
			{
				ci.m_homeServer = server;
			}
			else if (!found
				&& IsRoamingAllowed())
			{
				NetworkResolve(server.c_str(), call_id, ci);
			}
		}
	}
	else
		ci.m_realID = std::string{ call_id.m_str, call_id.Length() };

	return ci.m_status;
}

VS_UserPresence_Status VS_PresenceService::ResolveWithForwarding(const VS_SimpleStr& from, VS_SimpleStr &name, unsigned int *fwd_limit, bool force_fwd,VS_CallIDInfo *ci,
																 VS_UserData *ud, unsigned *fwd_timeout)
{
	dstream4 << "VS_PresenceService::ResolveWithForwarding: from = " << from.m_str << "; name = " << name.m_str << "; fwd_limit = " << *fwd_limit << "; force_fwd = " << force_fwd << ";\n";
	if (*fwd_limit == 0) return USER_INVALID;
	if (!force_fwd) (*fwd_limit)--;
	*fwd_timeout = 0;

	VS_UserPresence_Status status = this->Resolve(name, *ci, false, ud);
	VS_Container cnt, cnt2;
	VS_ExtendedStatusStorage & user_ext_st = ci->m_extStatusStorage;

	int32_t ext_status = 0;
	VS_ExtendedStatusStorage::StatusValueType ext_st_val;
	if(user_ext_st.GetExtStatus(EXTSTATUS_NAME_EXT_STATUS, ext_st_val))
	{
		if (int32_t *p = boost::get<int32_t>(&ext_st_val))
			ext_status = *p;
	}
	if (ext_status == 0x021) status = USER_LOGOFF; // busy extended status
	int32_t fwd_type(0);
	std::string fwd_call_id;
	std::string fwd_timeout_call_id;

	if (user_ext_st.GetExtStatus(EXTSTATUS_NAME_FWD_TYPE, ext_st_val))
	{
		if (int32_t *p = boost::get<int32_t>(&ext_st_val))
			fwd_type = *p;
		if (0 == fwd_type)
			return status;
	}
	if (user_ext_st.GetExtStatus(EXTSTATUS_NAME_FWD_TIMEOUT, ext_st_val))
		if (int32_t* p = boost::get<int32_t>(&ext_st_val))
			*fwd_timeout = *p;
	if (force_fwd || fwd_type == 1 ||
		(fwd_type == 2 && (status == USER_BUSY || status == USER_LOGOFF))
		)
	{
		if (force_fwd)
		{
			if (user_ext_st.GetExtStatus(EXTSTATUS_NAME_FWD_TIMEOUT_CALL_ID, ext_st_val))
				if (std::string *p = boost::get<std::string>(&ext_st_val))
					name = p->c_str();
		}
		else
			if (user_ext_st.GetExtStatus(EXTSTATUS_NAME_FWD_CALL_ID, ext_st_val))
				if (std::string *p = boost::get<std::string>(&ext_st_val))
					name = p->c_str();
		return ResolveWithForwarding(from, name, fwd_limit, false, ci, ud, fwd_timeout);
	}
	dstream4 << "VS_PresenceService::ResolveWithForwarding: finished. Status = " << status;
	return status;
}

bool VS_PresenceService::IsRegisteredTransId(const char *call_id)
{
	if (!call_id || !*call_id)
		return false;

	string_view s(call_id);
	auto pos = s.find_first_of('/');
	if (pos == string_view::npos)
		return false;
	auto trans_id = s.substr(pos + 1);
	auto new_call_id = s.substr(0, pos);

	VS_ResolveServerFinder	*resolve_srv = VS_ResolveServerFinder::Instance();
	if (!resolve_srv) return false;

	auto resolver = resolve_srv->GetExternalPresence(std::string(new_call_id).c_str());
	return resolver && resolver->IsRegisteredTransId(std::string(trans_id).c_str());
}

class Resolve_Task: public VS_PoolThreadsTask, public VS_TransportRouterServiceHelper
{
	boost::shared_ptr<VS_PresenceService> m_presenceService;
public:
	VS_SimpleStr			m_call_id;
	VS_SimpleStr			m_server;
	VS_SimpleStr			m_user;
	VS_SimpleStr			m_service;

	Resolve_Task(const boost::shared_ptr<VS_PresenceService>& PresSRV, const char* server, const char* user, const char* call_id, const char* service)
		: m_presenceService(PresSRV)
		, m_call_id(call_id)
		, m_server(server)
		, m_user(user)
		, m_service(service)
	{
	}

	void Run() {
		VS_CallIDInfo ci;
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, RESOLVE_METHOD);
		rCnt.AddValue(CALLID_PARAM, m_call_id.m_str);
		VS_UserPresence_Status status = m_presenceService->Resolve(m_call_id, ci, false, 0, false);
		ci.ToContainer(rCnt,true,true);
		PostRequest(m_server, m_user, rCnt, 0, m_service);
	}
};

void VS_PresenceService::Resolve_Method(const char* call_id)
{
	if (!call_id)
		return;
	if(m_confRestriction&&(string_view(OurEndpoint())!= m_recvMess->SrcServer()) && (!IsRoamingAllowed()))
		return;
	dprint1("Resolve from %s:%s for %s\n", m_recvMess->SrcServer(), m_recvMess->SrcUser(), call_id);
	PutTask(new Resolve_Task(shared_from_this(), m_recvMess->SrcServer(), m_recvMess->SrcUser(), call_id, m_recvMess->SrcService()), "Resolve", 25);
}
////////////////////////////////////////////////////////////////////////////////
// SUBSCRIBE_METHOD
////////////////////////////////////////////////////////////////////////////////
void VS_PresenceService::SubscribeClient_Method( VS_Container& cnt )
{
	VS_FullID sub(m_recvMess->SrcServer(), m_recvMess->SrcUser());
	int32_t cause = 0;
	cnt.GetValue(CAUSE_PARAM,cause);
	if(cause==1)
	{
		UnsubscribeID(sub);
		dprint2("subs reset for %s-%s \n",sub.m_userID.m_str,sub.m_serverID.m_str);
	}

	dprint3("subscribe from local:%s-%s\n", sub.m_userID.m_str,sub.m_serverID.m_str);
	Subscribe(cnt, sub, cause==1);
}

void VS_PresenceService::SubscribeServer_Method(VS_Container &cnt)
{
	VS_FullID server_id(m_recvMess->SrcServer(),m_recvMess->SrcUser());

	int32_t cause = 0;
	cnt.GetValue(CAUSE_PARAM,cause);

	int32_t seq_id;
	if(!cnt.GetValue(SEQUENCE_ID_PARAM,seq_id))
		seq_id=-1;

	int32_t seq_test = -1;
	if(seq_id>0)
		seq_test=m_sub_sync.Inc(server_id);

	if(cause==1)
	{
		UnsubscribeID(server_id);
		dprint2("subs reset for %s, init seq_id=%08x\n",m_recvMess->SrcServer(),seq_id);
		m_sub_sync.Init(server_id, seq_id);
		seq_test=seq_id;
	}
	if((seq_id>0 && seq_test!=seq_id)||(m_sub_sync.IsWaitForReset(server_id)))
	{
		if(seq_id>0 && seq_test!=seq_id)
		{
			m_sub_sync.SetSeqId(server_id,seq_id);
			m_sub_sync.MarkForReset(server_id);
			dprint1("sync lost in sub for %s L%08x!=R%08x\n",m_recvMess->SrcServer(), seq_test, seq_id);
		}

		auto now = std::chrono::system_clock::now();
		auto it=m_sub_reset_time[server_id];
		if (!!it && now - *it->data < c_reset_timeout)
		{
		  dprint3("Reset subs supressed\n") ;
		}
		else
		{
		  it=now;

			  VS_Container reset_cnt;
			  reset_cnt.AddValue(METHOD_PARAM,GETSUBSCRIPTIONS_METHOD);
			  PostRequest(server_id.m_serverID,server_id.m_userID,reset_cnt,NULL,PRESENCE_SRV);
		}
	};

	dprint3("subscribe from %s, seq_id=%08x\n", m_recvMess->SrcServer(),seq_id);
	Subscribe(cnt,server_id,cause==1);
}

////////////////////////////////////////////////////////////////////////////////
// UNSUBSCRIBE_METHOD(CALLID_PARAM[])
////////////////////////////////////////////////////////////////////////////////
void VS_PresenceService::UnsubscribeClient_Method( VS_Container& cnt )
{
	VS_FullID sub(m_recvMess->SrcServer(), m_recvMess->SrcUser());
	dprint3("unsubscribe from %s-%s\n", sub.m_userID.m_str,sub.m_serverID.m_str);

	Unsubscribe(cnt,sub);
}
void VS_PresenceService::UnsubscribeServer_Method(VS_Container &cnt)
{
	VS_FullID server_id(m_recvMess->SrcServer(),m_recvMess->SrcUser());
	int32_t seq_id;
	if(!cnt.GetValue(SEQUENCE_ID_PARAM,seq_id))
		seq_id=-1;

	int32_t seq_test = -1;
	if(seq_id>0)
		seq_test=m_sub_sync.Inc(server_id);

	dprint3("unsubscribe from %s, seq_id=%08x\n", m_recvMess->SrcServer(),seq_id);

	if((seq_id>0 && seq_test!=seq_id) ||(m_sub_sync.IsWaitForReset(server_id)))
	{
		if(seq_id>0 && seq_test!=seq_id)
		{
			m_sub_sync.SetSeqId(server_id,seq_id);
			m_sub_sync.MarkForReset(server_id);
			dprint1("sync lost in unsub for %s L%08x!=R%08x\n",m_recvMess->SrcServer(), seq_test, seq_id);
		}
		auto now = std::chrono::system_clock::now();
		auto it=m_sub_reset_time[server_id];
		if (!!it && now - *it->data < c_reset_timeout)
		{
			dprint3("Reset subs supressed\n") ;
		}
		else
		{
			it=now;
			VS_Container reset_cnt;
			reset_cnt.AddValue(METHOD_PARAM,GETSUBSCRIPTIONS_METHOD);
			PostRequest(server_id.m_serverID,server_id.m_userID,reset_cnt,NULL,PRESENCE_SRV);
		}
	};
	Unsubscribe(cnt,server_id);
}

void VS_PresenceService::SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface> &confRestrict)
{
	m_confRestriction = confRestrict;
}

const boost::shared_ptr<VS_ConfRestrictInterface> &VS_PresenceService::GetConfRestrict() const
{
	return m_confRestriction;
}

bool VS_PresenceService::Timer(unsigned long tickcount)
{
	//std::lock_guard<decltype(m_lock)> lock(m_lock); VS_SubscriptionHub::Timer has own lock
	return VS_SubscriptionHub::Timer(tickcount);
}

VS_CallIDInfo VS_PresenceService::CheckOfflineStatus( const VS_SimpleStr& call_id )
{
	VS_ExtendedStatusStorage extStatus;
	auto st = m_confRestriction->CheckOfflineStatus(call_id, extStatus);
	return VS_CallIDInfo(st, extStatus);
}

bool VS_PresenceService::IsRoamingAllowed(const char *for_server_name)
{
	if(!m_confRestriction)
		return VS_SubscriptionHub::IsRoamingAllowed(for_server_name);
	else
		return m_confRestriction->IsRoamingAllowed(for_server_name);
}

void VS_PresenceService::OnAliasesChanged(std::multimap<std::string,std::string>&& new_aliases)
{
	if (!IsInProcessingThread())
	{
		CallInProcessingThread( [this, new_aliases]() mutable
		{
			OnAliasesChanged(std::move(new_aliases));
		});
		return;
	}
	std::multimap<std::string,std::string> to_unreg;
	for(auto it=m_registry_aliases.begin(); it!=m_registry_aliases.end(); ++it)
	{
		bool found(false);
		for(auto it2=new_aliases.begin(); it2!=new_aliases.end(); ++it2)
		{
			if ( (it->first == it2->first) &&
				 (it->second == it2->second) )
				 found = true;
		}
		if (!found)
			to_unreg.emplace(it->first, it->second);
	}
	// UnReg old aliases
	{
		auto it=to_unreg.begin();
		while (it!=to_unreg.end())
		{
			VS_StrI_IntMap a;
			std::string key = it->first;
			for(; it!=to_unreg.upper_bound(key); ++it)
			{
				VS_SimpleStr alias = it->second.c_str();
				a.Insert(alias,1);		// SystemAliases=1
			}

			UnregisterAliases(key.c_str(),a);
		}
	}
	m_registry_aliases = std::move(new_aliases);
	auto it= m_registry_aliases.begin();
	while (it!= m_registry_aliases.end())
	{
		VS_StrI_IntMap a;
		std::string key = it->first;
		for(; it!= m_registry_aliases.upper_bound(key); ++it)
		{
			VS_SimpleStr alias = it->second.c_str();
			a.Insert(alias,1);		// SystemAliases=1
		}
		RegisterAliases(key.c_str(),a);
	}
}

bool VS_PresenceService::UsersStatuses(UsersList &users)
{
	return VS_SubscriptionHub::FindStatuses(users, VS_GatewayStarter::GetInstance()->IsStarted());
}

void VS_PresenceService::ListOfOnlineUsers(UsersList &users)
{
	users = VS_SubscriptionHub::ListOfOnlineUsers();
}

bool VS_PresenceService::Subscribe( const VS_SimpleStr &call_id, const VS_FullID& subscriber )
{
	if (IsInProcessingThread())
		return VS_SubscriptionHub::Subscribe(call_id, subscriber);
	else
	{
		CallInProcessingThread([this,call_id, subscriber]()
			{
				Subscribe(call_id,subscriber);
			});
		return true;
	}
}

bool VS_PresenceService::Subscribe(VS_Container& in_cnt, const VS_FullID&  subscriber, bool reset )
{
	if (IsInProcessingThread())
		return VS_SubscriptionHub::Subscribe(in_cnt, subscriber, reset);
	else
	{
		CallInProcessingThread([this,in_cnt,subscriber,reset]() mutable
		{
			Subscribe(in_cnt,subscriber,reset);
		});
		return true;
	}
}

bool VS_PresenceService::Unsubscribe( const VS_SimpleStr &call_id, const VS_FullID& subscriber )
{
	if (IsInProcessingThread())
		return VS_SubscriptionHub::Unsubscribe(call_id, subscriber);
	else
	{
		CallInProcessingThread([this, call_id, subscriber]()
		{
			Unsubscribe(call_id,subscriber);
		});
		return true;
	}
}

bool VS_PresenceService::Unsubscribe( VS_Container& in_cnt, const VS_FullID&  subscriber)
{
	if (IsInProcessingThread())
		return VS_SubscriptionHub::Unsubscribe(in_cnt, subscriber);
	else
	{
		CallInProcessingThread([this, in_cnt, subscriber]() mutable
		{
			Unsubscribe(in_cnt, subscriber);
		});
		return true;
	}
}

bool VS_PresenceService::CheckHops(VS_Container& cnt)
{
	int32_t* hops_ptr = cnt.GetLongValueRef(HOPS_PARAM);
	if (!hops_ptr)
		cnt.AddValueI32(HOPS_PARAM, VS_SubscriptionHub::DEFAULT_HOPS - 1);	// i am first hop, so minus one
	else{
		if (*hops_ptr <= 0) {
			dstream1 << "Drop " << cnt.GetStrValueRef(METHOD_PARAM) << " due to hops=" << *hops_ptr << " (msg: " << *m_recvMess << ")\n";
			return false;
		}
		else
			--(*hops_ptr);
	}
	return true;
}

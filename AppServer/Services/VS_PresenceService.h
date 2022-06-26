/*************************************************
 * $Revision: 18 $
 * $History: VS_PresenceService.h $
 *
 * *****************  Version 18  *****************
 * User: Mushakov     Date: 28.03.12   Time: 20:27
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - resolve and subscribe terminals
 *
 * *****************  Version 17  *****************
 * User: Mushakov     Date: 7.07.11    Time: 18:08
 * Updated in $/VSNA/Servers/AppServer/Services
 * - getting rs address thru DNS
 * - Get server name by address (in server)
 * - fix bugs in start services sequence (AS)
 *  - Roaming in service added;
 *
 *
 * *****************  Version 16  *****************
 * User: Mushakov     Date: 31.05.11   Time: 18:15
 * Updated in $/VSNA/Servers/AppServer/Services
 * - broken roaming fixed
 *
 * *****************  Version 15  *****************
 * User: Mushakov     Date: 10.03.11   Time: 20:20
 * Updated in $/VSNA/Servers/AppServer/Services
 * - add USER_INVALID status
 * - revoke/assign ROAMING handled
 *
 * *****************  Version 14  *****************
 * User: Mushakov     Date: 9.02.11    Time: 19:51
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - subscribtions between vcs
 *
 * *****************  Version 13  *****************
 * User: Ktrushnikov  Date: 28.01.11   Time: 22:24
 * Updated in $/VSNA/Servers/AppServer/Services
 * VCS3.2
 * - VS_PresenceService::NetworkResolve() support for VCS (using
 * confRestrick with AS)
 * - common func VS_GetServerFromCallID() to VS_Utils
 *
 * *****************  Version 12  *****************
 * User: Mushakov     Date: 30.10.08   Time: 20:33
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - Alias status sending removed (AS ->RS )
 * - Logging of subType added
 *
 * *****************  Version 11  *****************
 * User: Stass        Date: 2.04.08    Time: 21:44
 * Updated in $/VSNA/Servers/AppServer/Services
 * added set server to BS presense
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 22.01.08   Time: 22:01
 * Updated in $/VSNA/Servers/AppServer/Services
 * added home server subscribtion to status
 *
 * *****************  Version 9  *****************
 * User: Ktrushnikov  Date: 5.12.07    Time: 13:13
 * Updated in $/VSNA/Servers/AppServer/Services
 * - valid overload of Init()-method
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 30.11.07   Time: 20:36
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 28.11.07   Time: 18:34
 * Updated in $/VSNA/Servers/AppServer/Services
 * added subscribe
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 28.11.07   Time: 13:28
 * Updated in $/VSNA/Servers/AppServer/Services
 * fix for get subs
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
 **************************************************/

#ifndef VS_PRESENCE_SERVICE_H
#define VS_PRESENCE_SERVICE_H

#include <mutex>

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "../../common/std/cpplib/VS_Map.h"
#include "../../ServerServices/VS_SubscriptionHub.h"
#include "AppServer/Services/VS_Storage.h"
#include "std-generic/asio_fwd.h"

#include <boost/enable_shared_from_this.hpp>

class VS_PresenceService :
	public VS_TransportRouterServiceReplyHelper,
	public VS_SubscriptionHub,
	public UsersStatusesInterface,
	public boost::enable_shared_from_this<VS_PresenceService>
{
	std::recursive_mutex m_lock;
	boost::asio::io_service& m_ios;
	VS_IDTimeMap       m_reset_time;
	typedef VST_Map<VS_FullID, std::chrono::system_clock::time_point> IDTimeMap;
	VS_SyncPool		m_sub_sync;
    VS_IDTimeMap    m_sub_reset_time;
	boost::signals2::scoped_connection	m_AliasesChangedConn;
	boost::shared_ptr<VS_ConfRestrictInterface>	m_confRestriction;
	// UsersStatusesInterface
	void ListOfOnlineUsers(UsersList &users) override;
	bool UsersStatuses(UsersList &users) override;
	bool Timer(unsigned long tickcount) override;
	// VS_SubscriptionHub override
	VS_CallIDInfo CheckOfflineStatus(const VS_SimpleStr& call_id) override;
	bool IsUplink() const override { return false; }
	//helper funcs:
	//resolver:
	VS_UserPresence_Status NetworkResolve(const char* uplink, const VS_SimpleStr& call_id, VS_CallIDInfo& ci);
	//Methods
	void GetAllUserStatus_Method(const VS_FullID& subscriber);
	void GetSubscriptions_Method(const char* server);
	void UserRegistrationInfo_Method();
	void UpdateStatus_Method(const VS_Container& cnt, const VS_FullID& source, bool erase);
	void UpdateClientStatus_Method(const vs_user_id& source_user, VS_Container &cnt);
	void GetUserStatus_Method(const char* call_id);
	void Resolve_Method(const char* call_id);
	void SubscribeClient_Method(VS_Container& cnt);
	void SubscribeServer_Method(VS_Container& cnt);
	void UnsubscribeServer_Method(VS_Container& cnt);
	void UnsubscribeClient_Method(VS_Container& cnt);
	void PushStatusDirectly_Method(VS_Container& cnt);
	void OnUplinkConnected_Event(const char* server);
	void OnAliasesChanged(std::multimap<std::string, std::string>&& new_aliases);
	bool CheckHops(VS_Container& cnt);
public:
	VS_PresenceService(boost::asio::io_service& ios);
	void SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface> &confRestrict);
	const boost::shared_ptr<VS_ConfRestrictInterface> &GetConfRestrict() const;
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;
	bool IsRoamingAllowed(const char *for_server_name = 0) override;
	VS_UserPresence_Status Resolve(VS_SimpleStr& call_id,VS_CallIDInfo& ci,bool use_cache=false, VS_UserData* from_ude=NULL,bool do_ext_resolve=true);
	VS_UserPresence_Status ResolveWithForwarding(const VS_SimpleStr& from, VS_SimpleStr &name, unsigned int *fwd_limit,
		bool force_fwd,VS_CallIDInfo *ci, VS_UserData *ud, unsigned *fwd_timeout);
// Events
	void OnUserLoginEnd_Event(const VS_UserData& ud, const std::string &cid);
	void OnUserLogoff_Event(const VS_UserData& ud, const std::string &cid);
	bool Subscribe	        ( const VS_SimpleStr &call_id, const VS_FullID& subscriber );
	bool Subscribe	        ( VS_Container& in_cnt, const VS_FullID&  subscriber, bool reset=false );
	bool Unsubscribe	    ( const VS_SimpleStr &call_id, const VS_FullID& subscriber );
	bool Unsubscribe	    ( VS_Container& in_cnt, const VS_FullID&  subscriber);
	bool IsRegisteredTransId(const char* call_id);
};

class VS_PresenceServiceMember
{
protected:
	boost::shared_ptr<VS_PresenceService> m_presenceService;
public:
	void SetPresenceService(const boost::shared_ptr<VS_PresenceService>& PresSRV)
	{
		m_presenceService = PresSRV;
	}
};

#endif // VS_USER_PRESENCE_SERVICE_H

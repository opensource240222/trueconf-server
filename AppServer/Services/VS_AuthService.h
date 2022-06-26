/*************************************************
 * $Revision: 34 $
 * $History: VS_AuthService.h $
 *
 * *****************  Version 34  *****************
 * User: Ktrushnikov  Date: 6.09.10    Time: 17:18
 * Updated in $/VSNA/servers/appserver/services
 * BS Event "USER" supported [#6978]
 * - when user deleted from DB - logout him from AS
 * - cleanup m_loginData at this AS
 *
 * *****************  Version 33  *****************
 * User: Ktrushnikov  Date: 10.08.10   Time: 21:27
 * Updated in $/VSNA/Servers/AppServer/Services
 * [#7561]: GenerateSessionKey() at LoginUser and at ReqUpdateAccount()
 * [#7562]: update login session key at timer timeout in Visicron.dll
 *
 * *****************  Version 32  *****************
 * User: Mushakov     Date: 21.06.10   Time: 15:36
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - doman -> domain
 *
 * *****************  Version 31  *****************
 * User: Mushakov     Date: 15.06.10   Time: 15:32
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - sharding
 *
 * *****************  Version 30  *****************
 * User: Ktrushnikov  Date: 15.09.09   Time: 16:39
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Rating added
 * - display_name type changed to char* (from wchar_t)
 * - send BS Events to different service (AUTH,FWD)
 *
 * *****************  Version 29  *****************
 * User: Smirnov      Date: 4.05.09    Time: 19:07
 * Updated in $/VSNA/Servers/AppServer/Services
 * - cache for login
 *
 * *****************  Version 28  *****************
 * User: Ktrushnikov  Date: 26.03.09   Time: 19:28
 * Updated in $/VSNA/Servers/AppServer/Services
 * - cleanup
 *
 * *****************  Version 27  *****************
 * User: Ktrushnikov  Date: 26.03.09   Time: 19:28
 * Updated in $/VSNA/Servers/AppServer/Services
 * - VS_TempUserData moved to other file
 *
 * *****************  Version 26  *****************
 * User: Ktrushnikov  Date: 4.03.09    Time: 21:09
 * Updated in $/VSNA/Servers/AppServer/Services
 * no BS implementation:
 * - AS login user if have previous cached answer from BS
 * - BS add SERVER_PARAM = OurEndpoint() to rCnt
 * - HomeFWDSrv answers by itself if no BS and mess from ABOOK (emulate
 * SearchAB with "no changes" result)
 *
 * *****************  Version 25  *****************
 * User: Smirnov      Date: 3.10.08    Time: 18:21
 * Updated in $/VSNA/Servers/AppServer/Services
 * - bugfix, decrease average login time if it value more than noSB
 * threshold
 *
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 18.06.08   Time: 20:11
 * Updated in $/VSNA/Servers/AppServer/Services
 * - client startup warning messages removed
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 4.06.08    Time: 17:25
 * Updated in $/VSNA/Servers/AppServer/Services
 * - debug log
 *
 * *****************  Version 22  *****************
 * User: Dront78      Date: 16.05.08   Time: 15:15
 * Updated in $/VSNA/Servers/AppServer/Services
 * - BS offload algorithm added
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 21.04.08   Time: 17:15
 * Updated in $/VSNA/Servers/AppServer/Services
 * - login of logged in user, transport interfaces cleaning
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 14.04.08   Time: 16:57
 * Updated in $/VSNA/Servers/AppServer/Services
 * - added wan_Ip property
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 26.03.08   Time: 15:32
 * Updated in $/VSNA/Servers/AppServer/Services
 * - hashed login procedure
 *
 * *****************  Version 18  *****************
 * User: Ktrushnikov  Date: 6.03.08    Time: 17:36
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Use VS_Container params to pass params for LocalRequest
 * - Don't use a specific function for LocalRequest's
 * - VS_Protocol: LOCALREQUEST_METHOD added
 *
 * *****************  Version 17  *****************
 * User: Ktrushnikov  Date: 5.03.08    Time: 21:26
 * Updated in $/VSNA/Servers/AppServer/Services
 * - lock for LocalRequest params added
 * - VS_TransportRouter_MessageQueue::DeleteMsg(): don't create notify
 * message, cause it is useless and would produce memory leak in
 * TR::ProcessingMessage()
 * - __dbg_printf() changed to dprint4() with _DEBUG definition
 * - method added: VS_TransportMessage::ReleaseMessage(): to clean this
 * with no delete of mess
 *
 * *****************  Version 16  *****************
 * User: Ktrushnikov  Date: 3.03.08    Time: 18:37
 * Updated in $/VSNA/Servers/AppServer/Services
 * - switch context from TransportRouter_Imp Thread to Service Thread by
 * using VS_ServiceHelper::PostLocalRequest()
 * - VS_Storage::FindUser(): check of input params added
 *
 * *****************  Version 15  *****************
 * User: Stass        Date: 18.02.08   Time: 21:43
 * Updated in $/VSNA/Servers/AppServer/Services
 * added alias support (first ver)
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 15.02.08   Time: 21:18
 * Updated in $/VSNA/Servers/AppServer/Services
 * - safe access to g_AppDAta in services
 * - new broker info structure
 *
 * *****************  Version 13  *****************
 * User: Dront78      Date: 13.02.08   Time: 11:01
 * Updated in $/VSNA/Servers/AppServer/Services
 * Memory Leak build fixed.
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 31.01.08   Time: 22:14
 * Updated in $/VSNA/Servers/AppServer/Services
 * - conditions rewrited, added to AUTH_SRV_
 * - logoff user on point disconnect
 *
 * *****************  Version 11  *****************
 * User: Stass        Date: 22.01.08   Time: 22:01
 * Updated in $/VSNA/Servers/AppServer/Services
 * added home server subscribtion to status
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 17.12.07   Time: 15:31
 * Updated in $/VSNA/Servers/AppServer/Services
 * fixes in login step 3
 *
 * *****************  Version 9  *****************
 * User: Stass        Date: 14.12.07   Time: 18:30
 * Updated in $/VSNA/Servers/AppServer/Services
 * done register/unregister processing with queue
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 12.12.07   Time: 18:11
 * Updated in $/VSNA/Servers/AppServer/Services
 * fixes
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 7.12.07    Time: 13:22
 * Updated in $/VSNA/Servers/AppServer/Services
 * added checkstatus
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 5.12.07    Time: 13:13
 * Updated in $/VSNA/Servers/AppServer/Services
 * - valid overload of Init()-method
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
 ************************************************/

#ifndef VS_AUTH_SERVICE_H
#define VS_AUTH_SERVICE_H

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "std/cpplib/VS_Policy.h"
#include "../../ServerServices/Common.h" /// for id
#include "transport/VS_SystemMessage.h"
#include "../../common/std/cpplib/VS_MapTpl.h"
#include "VS_AppServerData.h"
#include "AppServer/Services/VS_Storage.h"
#include "../../common/std/cpplib/VS_MemoryLeak.h"
#include <vector>
#include <algorithm>
#include <boost/signals2.hpp>

typedef VST_StrIMap<VS_TempUserData> VS_TempUserDataMap;
class VS_TranscoderLogin;

const unsigned long nAuthLogin		= 10;
const unsigned long LoginBarrier	= 10*1000;	// 10 sec
const unsigned long noBSbarrier		= 60*1000;	// 60 sec

class VS_AuthService :
	public VS_TransportRouterServiceReplyHelper,
	public VS_SystemMessage
{
	VS_TempUserDataMap		m_loginData;

	// login stat params
	std::vector<int>	m_loginTime;
	long				m_login_stat[nAuthLogin];
	long				m_login_count;
	long				m_avetime;
	VS_SimpleStr		m_debugHomeBS;
	std::string			m_defaultDomain = "trueconf.com";
	std::weak_ptr<VS_TranscoderLogin> m_transLogin;

	void GetUserDomain(const VS_SimpleStr& call_id,VS_SimpleStr& domain);

	void OnUserLoginEnd_Event(const VS_UserData& ud, const std::string &cid);
	void OnUserLogoff_Event(const VS_UserData& ud, const std::string &cid);
	VS_Policy m_policy;

public:
	VS_AuthService(void): m_login_count(0), m_avetime(0), m_policy( "TC" ) { memset(m_login_stat, 0, sizeof(m_login_stat)); m_loginTime.reserve(1024); }
	virtual ~VS_AuthService(void) {
		m_OnUserLoggedIn.disconnect_all_slots();
		m_OnUserLoggedOut.disconnect_all_slots();
	}
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;
	void SetTransLogin(const std::weak_ptr<VS_TranscoderLogin>& transLogin);

	void LoginUser_Method (VS_Container& cnt);
	void LoginUser2_Method (VS_Container& cnt);
	void UserLoggedIn_Method (VS_Container& cnt);

	void LogoutUser_Method (const vs_user_id& user, const char* uplink);

	void CheckUserLoginStatus_Method();

	bool OnPointConnected_Event(const VS_PointParams* prm) override;
	bool OnPointDisconnected_Event(const VS_PointParams* prm) override;
	bool OnPointDeterminedIP_Event(const char* uid, const char* ip) override;

	void UpdateAccount_Method(VS_Container& cnt);
	void ReqUpdateAccount_Method(VS_Container& cnt);

	void OnUserChange_Method(VS_Container& cnt);
	void SetRegID_Method(VS_Container& cnt);
	void UpdatePeerCfg_Method(VS_Container& cnt);
	boost::signals2::signal< void (const VS_UserData &ud, const std::string &cid) > m_OnUserLoggedIn;
	boost::signals2::signal< void (const VS_UserData &ud, const std::string &cid) > m_OnUserLoggedOut;
	boost::signals2::signal< void (const char* call_id, std::vector<VS_ExternalAccount>& v) > m_OnNewPeerCfg;

};

#endif // VS_AUTH_SERVICE_H

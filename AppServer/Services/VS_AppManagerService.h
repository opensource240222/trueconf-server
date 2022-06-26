/************************************************
 * $Revision: 26 $
 * $History: VS_AppManagerService.h $
 *
 * *****************  Version 26  *****************
 * User: Mushakov     Date: 24.06.10   Time: 16:15
 * Updated in $/VSNA/Servers/AppServer/Services
 * - opt disabled when SECUREBEGIN_
 * - locator bs removed
 *
 * *****************  Version 25  *****************
 * User: Mushakov     Date: 15.06.10   Time: 15:32
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - sharding
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 15.02.10   Time: 18:50
 * Updated in $/VSNA/Servers/AppServer/Services
 * - licence restrictions reorganization
 * - SECUREBEGIN_A temporally removed
 *
 * *****************  Version 23  *****************
 * User: Mushakov     Date: 23.01.09   Time: 19:44
 * Updated in $/VSNA/Servers/AppServer/Services
 * - statuses and subs pack size decreased
 * - set g_AppServer to 0 when ManagerService was destroyed
 * - SERVER_PARAM removed from status pack AS-> RS
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 3.06.08    Time: 21:55
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Redirect v.3
 *
 * *****************  Version 21  *****************
 * User: Ktrushnikov  Date: 16.05.08   Time: 19:41
 * Updated in $/VSNA/Servers/AppServer/Services
 * - SM & AS: LoadBalance algorithm added as MSC_LOADBALANCE
 *
 * *****************  Version 20  *****************
 * User: Ktrushnikov  Date: 14.05.08   Time: 21:25
 * Updated in $/VSNA/Servers/AppServer/Services
 * - added ktrushnikov_Redirect
 *
 * *****************  Version 19  *****************
 * User: Ktrushnikov  Date: 28.04.08   Time: 16:39
 * Updated in $/VSNA/Servers/AppServer/Services
 * - SendPeriod for average-stat fixed
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 25.04.08   Time: 19:25
 * Updated in $/VSNA/Servers/AppServer/Services
 * - RS and BS online UP fixed
 *
 * *****************  Version 17  *****************
 * User: Ktrushnikov  Date: 21.04.08   Time: 21:14
 * Updated in $/VSNA/Servers/AppServer/Services
 * - file VS_UsageStatService.h placed from AS to ServerServices
 * - return types for stat funcs changed from bool to void
 * - stat class is inited by -1 (not by 0)
 * - new light struct for stat: VS_UsageStatOther (not so big like for AS)
 * - Avarage & Current statistics added to BS and RS (like it is in AS)
 *
 * *****************  Version 16  *****************
 * User: Ktrushnikov  Date: 12.04.08   Time: 17:29
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Manager commands for AS from SM added
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 11.04.08   Time: 17:32
 * Updated in $/VSNA/Servers/AppServer/Services
 * - protocol cleaning
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 31.03.08   Time: 20:34
 * Updated in $/VSNA/Servers/AppServer/Services
 * - event based connects to server
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 11.03.08   Time: 13:16
 * Updated in $/VSNA/Servers/AppServer/Services
 * - query for BS
 *
 * *****************  Version 12  *****************
 * User: Ktrushnikov  Date: 22.02.08   Time: 16:47
 * Updated in $/VSNA/Servers/AppServer/Services
 * - bugfix: set correct status in UpdateServerStatusInVector()
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 18.02.08   Time: 20:41
 * Updated in $/VSNA/Servers/AppServer/Services
 * - simple redirect
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 15.02.08   Time: 21:18
 * Updated in $/VSNA/Servers/AppServer/Services
 * - safe access to g_AppDAta in services
 * - new broker info structure
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 14.02.08   Time: 17:21
 * Updated in $/VSNA/Servers/AppServer/Services
 * - auto connect
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 13.02.08   Time: 13:43
 * Updated in $/VSNA/Servers/AppServer/Services
 * - properties corrected
 * - sturtup sequence of server connects rewrited
 * - code redused
 *
 * *****************  Version 7  *****************
 * User: Ktrushnikov  Date: 25.01.08   Time: 12:37
 * Updated in $/VSNA/Servers/AppServer/Services
 * - code cleanup from OnPointConnect/Disconnect tests
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 24.01.08   Time: 17:25
 * Updated in $/VSNA/Servers/AppServer/Services
 * - delete calls to ConnectPoint from AS
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 22.01.08   Time: 17:25
 * Updated in $/VSNA/Servers/AppServer/Services
 * - if no messages - delete endpoint
 * - ControlInquiry type in Forced disconnect endpoint was set to All (not
 * to delete one endpoint)
 * - OnPointConnect/Disconnet conditions added in AppServer
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 16.01.08   Time: 19:15
 * Updated in $/VSNA/Servers/AppServer/Services
 * - OnPointConnect_Event() added
 *
 * *****************  Version 3  *****************
 * User: Ktrushnikov  Date: 4.12.07    Time: 11:50
 * Updated in $/VSNA/Servers/AppServer/Services
 * - VS_TransportMessage::Set(): checks fixed for new arch
 * - VS_AppManagerService: Init() overload fixed
 * - VS_AppManagerService: Message processing fixed
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 28.11.07   Time: 16:48
 * Updated in $/VSNA/Servers/AppServer/Services
 * - rename VS_MagaerService from AS
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 26.11.07   Time: 19:55
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 26.11.07   Time: 17:51
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 22.11.07   Time: 20:34
 * Updated in $/VSNA/Servers/AppServer/Services
 * new iteration
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 13.11.07   Time: 17:32
 * Created in $/VSNA/Servers/AppServer/Services
 * new services

 ************************************************/

#ifndef VS_APPMANAGER_SERVICE_H
#define VS_APPMANAGER_SERVICE_H

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "tools/Watchdog/VS_RoutersWatchdog.h"
#include "../../LicenseLib/VS_License.h"

#include "../../ServerServices/Common.h"
#include "../../ServerServices/VS_UsageStatService.h"
#include "VS_AppServerData.h"
#include "AppServer/Services/VS_PresenceService.h"

class VS_AppManagerService :
	public VS_Testable,
	public VS_AppServerData,
	public VS_TransportRouterServiceHelper,
	private VS_UsageStatAS
	, public VS_PresenceServiceMember
{
	VS_RoutersWatchdog*		m_watchdog;
	stream::Router*			m_sr;
	VS_RouterMessage*		m_recvMess;						// received message
	VS_SimpleStr			m_SM;

	VS_SimpleStr			m_broker_ver;

	static const unsigned long	RefreshPeriod = 60*1000;
	static const unsigned long	SendPeriod = 60*60*1000;
	static const unsigned long	DirPingPeriod = 29*1050;	//< Directory ping period
	bool CountStats(unsigned long ticks);
	void SendStats();

	unsigned long			m_RefTime;					//< refresh stats timer
	unsigned long			m_SendTime;					//< send stats timer
	unsigned long			m_DirPingTime;				//< Directory ping time

	VS_Server_States		m_state;

	void ManageServer_Method(long cmd, long param, const void* param2 = 0);

	void MSC_Redirect(const unsigned int N, const char** sids, int NumOfSid);

//	void GetBSList(VS_Container &cnt);

public:
	VS_AppManagerService() :
		m_watchdog(0), m_sr(0), m_recvMess(0),
		m_RefTime(0), m_SendTime(0), m_DirPingTime(0),
		m_state(SSTATE_NONE)
	{
		m_TimeInterval = std::chrono::seconds(5);
	}
	virtual ~VS_AppManagerService() {g_appServer = 0;}
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	void AsyncDestroy() override;
	bool Timer(unsigned long tickcount) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	// VS_Testable implementation
	bool Test() {return m_state!=SSTATE_OFFLINE;};
	// VS_PointConditions
	bool OnPointConnected_Event(const VS_PointParams* prm) override;
	bool OnPointDisconnected_Event(const VS_PointParams* prm) override;
	// Service implementation
	void SetComponents(stream::Router* sr, VS_RoutersWatchdog* watchdog, const char* ver) {
		m_sr = sr; m_watchdog = watchdog; m_broker_ver = ver;
	}
	void UpdateInfo_Method(VS_Container &cnt);
	////void UpdateInfo_Method(const char* locatorBS, const char* RS);
	void CheckServers();
	bool CheckAllBS();
	void DefinedServerStatusChanged(const char* server, bool status, VS_ServerTypes type);
};

#endif // VS_APPMANAGER_SERVICE_H

#pragma once
#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "ServerServices/VS_UsageStatService.h"
#include "tools/Watchdog/VS_RoutersWatchdog.h"
#include "ServerServices/VS_SubscriptionHub.h"


class VS_ManagerService :
	public VS_Testable,
	public VS_TransportRouterServiceReplyHelper,
	private VS_UsageStatOther
{
private:
	void ManageServer_Method(long cmd, long param);

	VS_SimpleStr			m_SM;
	VS_RoutersWatchdog*		m_watchdog;

	static const int RefreshPeriod = 60*1000;
	static const int SendPeriod = 60*60*1000;
	static const unsigned long	DirPingPeriod=29*1050;	//< Directory ping period

	unsigned long			m_RefTime;					//< refresh stats timer
	unsigned long			m_SendTime;					//< send stats timer
	unsigned long			m_DirPingTime;				//< Directory ping time

	VS_SimpleStr			m_server_version;


protected:

public:
	VS_ManagerService():
		m_RefTime(0), m_SendTime(0), m_DirPingTime(0)
	{
		m_TimeInterval = std::chrono::seconds(5);
	}
	~VS_ManagerService()
	{}

	bool Init(const char* our_endpoint, const char *our_service, const bool permitAll) override;
	bool Timer(unsigned long tickcount) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	void SetComponents(VS_RoutersWatchdog* watchdog, const char* ver) {
		m_watchdog = watchdog;
		m_server_version = ver;
	}

	bool Test() {return true;};

	bool CountStats(unsigned long tickcount);

	bool OnPointConnected_Event(const VS_PointParams* prm) override;
	bool OnPointDisconnected_Event(const VS_PointParams* prm) override;
};
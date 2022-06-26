/**
 *************************************************
 * $Revision: 9 $
 * $History: VS_VCSConfigurationService.h $
 *
 * *****************  Version 9  *****************
 * User: Ktrushnikov  Date: 17.05.12   Time: 14:58
 * Updated in $/VSNA/Servers/VCS/Services
 * VS_VCSConfigurationService::OnPointDeterminedIP_Event:
 * - processing modex to Service thread from TransportRouter thread
 * (because, g_dbStorage->FindUser() can take long time at LDAP)
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 24.12.10   Time: 19:39
 * Updated in $/VSNA/Servers/VCS/Services
 *
 * *****************  Version 7  *****************
 * User: Ktrushnikov  Date: 6.10.10    Time: 19:19
 * Updated in $/VSNA/Servers/VCS/Services
 * VCS: CountStats method fixed
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 8.09.10    Time: 12:38
 * Updated in $/VSNA/Servers/VCS/Services
 * LogStats fix:
 * - send to REGISTRATION_SRV instead of MANAGER_SRV
 * - set m_SendTime to null at ctor of service
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 7.07.10    Time: 17:04
 * Updated in $/VSNA/Servers/VCS/Services
 * [VCS 3.0.7]
 * - Endpoint properites didn't come: Local Ip, IP, Logged User
 * - save m_app_id at login at auto_login too (not just by login/pass)
 * - UPDATECONFIGURATION_METHOD: don't send ourservice, cause there will
 * be no SrcUser. call directly.
 * - PointConditions added to CONFIGURATION_SRV
 * - g_dbStorage->SetEndpointProperty support empty string
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 25.05.10   Time: 21:18
 * Updated in $/VSNA/Servers/VCS/Services
 * - Send stats to RegServer
 *
 * *****************  Version 3  *****************
 * User: Ktrushnikov  Date: 16.04.10   Time: 17:42
 * Updated in $/VSNA/Servers/VCS/Services
 * #7262: send to user UPDATECONFIGURATION_METHOD with server connect info
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 11.01.10   Time: 14:20
 * Updated in $/VSNA/Servers/VCS/Services
 * - new certificate
 * - refactoring
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 3.01.10    Time: 15:16
 * Created in $/VSNA/Servers/VCS/Services
 * - VCS refactoried
 *
 * *****************  Version 1  *****************
 * User: Ktrushnikov  Date: 2.12.09    Time: 18:50
 * Created in $/VSNA/Servers/VCS
 * Configuration service added
 * Guest: don't add to m_users map
 * VS_StorageUserData
 * - save AppID-Key pairs per User
 * VSAuthService
 * - process second login of user (logout first user)
 * VCSStorage
 * - added FindUserExtended: get Key param by login
 * - interface cleanup (confs)
 * VS_ConfMemStorage
 * - removed from inheritance
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 1.10.09    Time: 20:04
 * Updated in $/VS2005/Servers/MediaBroker/BrokerServices
 *  - read TRIAL flag bug fixed
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 18.09.09   Time: 16:51
 * Updated in $/VS2005/Servers/MediaBroker/BrokerServices
 * - trial flag
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 7.05.09    Time: 19:52
 * Updated in $/VS2005/Servers/MediaBroker/BrokerServices
 * - bug fix with: *(int*) bitrate;
 * - bug #5922: Send Bitrate and WxH params to user
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 18.09.06   Time: 14:47
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * - async all storage working services
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 29.09.04   Time: 14:24
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * pragma_once removed
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 22.03.04   Time: 18:52
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * UTF8 system chat messages + chat message protocol  ver 11,12,13 support
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 5.01.04    Time: 19:19
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * users reloading
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 5.09.03    Time: 14:40
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * Moved types out of BrokerServices project
 *
 * *****************  Version 1  *****************
 * User: Slavetsky    Date: 9/04/03    Time: 5:36p
 * Created in $/VS/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 26.08.03   Time: 16:23
 * Updated in $/VS/Servers/ServerServices
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 10.07.03   Time: 20:18
 * Updated in $/VS/Servers/ServerServices
 * minimum client version warning
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 23.05.03   Time: 17:59
 * Updated in $/VS/Servers/ServerServices
 * Removed old client version message, added reading of protocol version
 * message from registry
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 20.05.03   Time: 15:38
 * Updated in $/VS/Servers/ServerServices
 * Serisies iterfaces rewrited
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 14.05.03   Time: 18:07
 * Created in $/VS/Servers/ServerServices
 * renamed NetworkConfigurationService -> ConfigurationService
 * added GetAppPropwrties method
 *************************************************/
#ifndef VS_VCSCONFIGURATION_SERVICE_H
#define VS_VCSCONFIGURATION_SERVICE_H

#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../../ServerServices/VS_UsageStatService.h"

#include <map>

class VS_VCSConfigurationService :
	public VS_TransportRouterServiceReplyHelper,
	private VS_UsageStatAS
{
	stream::Router*		m_sr;
	std::string				m_broker_ver;

	int		m_width;
	int		m_height;
	long	m_bitrate;

	static const unsigned long	RefreshPeriod = 60*1000;
	static const unsigned long	SendPeriod = 60*60*1000;
	bool CountStats(unsigned long ticks);
	void SendStats();

	unsigned long			m_RefTime;					//< refresh stats timer
	unsigned long			m_SendTime;					//< send stats timer

	std::map<std::string,std::string> m_connected_cids;		// [key=cid,value=app_name]

	bool PointDisconnected_Handler(const char* cid);
	bool PointDeterminedIP_Handler(const char* uid, const char* ip );

public:
	VS_VCSConfigurationService()
		: m_sr(0)
		, m_width(-1)
		, m_height(-1)
		, m_bitrate(-1)
		, m_SendTime(0)
	{
		m_TimeInterval = std::chrono::seconds(5);
	}
	virtual ~VS_VCSConfigurationService(void) { }
	void SetComponents(stream::Router* sr, string_view product_version) {
		m_sr = sr;
		m_broker_ver = (std::string)product_version;
	}
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	bool Timer(unsigned long ticks) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	// UPDATECONFIGURATION_METHOD(IPCONFIG_PARAM, ...)
	void UpdateConfiguration_Method(VS_Container &cnt);
	void GetAppProperties_Method(const char* app_name, const char* ver);
	void SetEpProperties_Method(VS_Container &cnt);
	void OnPropertiesChange_Method(const char* pass);

	bool OnPointDisconnected_Event(const VS_PointParams* prm) override;
	bool OnPointDeterminedIP_Event(const char* uid, const char* ip ) override;
};

#endif

/**
 ****************************************************************************
 * (c) 2002-2003 Visicron Inc.  http://www.visicron.net/
 *
 * Project: Visicron Registration Server
 *
 * \file
 *
 * Directory Service definition file
 *
 * $Revision: 15 $
 * $History: VS_RegistrationService.h $
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 16.08.12   Time: 21:36
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * -crypt mgateway
 * - arm futures ver added
 *
 * *****************  Version 14  *****************
 * User: Mushakov     Date: 16.03.12   Time: 15:54
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - Add OS and CPU info in Regiatration Server
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 23.03.11   Time: 17:49
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - Server Name Verification added at registration
 *
 * *****************  Version 12  *****************
 * User: Mushakov     Date: 1.11.10    Time: 21:00
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - cert update added
 * - registration from server added
 * - authorization servcer added
 *
 * *****************  Version 11  *****************
 * User: Ktrushnikov  Date: 30.09.10   Time: 15:34
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * RegServer
 * - MANAGER_SRV added
 * - Post LogStats_Method to REGISTRATION_SRV
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 7.04.10    Time: 19:41
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - reg offline
 *
 * *****************  Version 9  *****************
 * User: Dront78      Date: 6.04.10    Time: 16:50
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - new license algorithm
 *
 * *****************  Version 8  *****************
 * User: Dront78      Date: 24.02.10   Time: 19:05
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * updated license manager
 *
 * *****************  Version 7  *****************
 * User: Dront78      Date: 18.02.10   Time: 10:09
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * Armadillo Security Passport added
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 30.10.09   Time: 14:49
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - Sign added in TransportHandshake
 *  - Sign Verify added in TransportRouter
 *  - hserr_verify_failed transport result code added
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 28.10.09   Time: 17:58
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - vs_ep_id removed
 * - registration corrected
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 26.10.09   Time: 20:32
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - sign verification added
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 23.10.09   Time: 15:05
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - VCS 3
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 7.08.09    Time: 15:14
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 31.03.09   Time: 17:35
 * Created in $/VSNA/Servers/RegistryServer/Services
 * RegistryServer added
 *
 * *****************  Version 3  *****************
 * User: Ktrushnikov  Date: 25.11.07   Time: 13:47
 * Updated in $/VS2005/Servers/DirectoryServer/Services
 * - Offline Registration added
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 22.03.07   Time: 16:18
 * Updated in $/VS2005/Servers/DirectoryServer/Services
 * добавлена верификация ProductType при регистрации брокера
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:51
 * Created in $/VS2005/Servers/DirectoryServer/Services
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 6.12.06    Time: 12:31
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added signed  license encode for server protocol version 3
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 5.06.06    Time: 12:47
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 25.04.06   Time: 15:54
 * Updated in $/VS/Servers/DirectoryServer/Services
 * Added certificate issue
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 29.09.04   Time: 14:24
 * Updated in $/VS/Servers/DirectoryServer/Services
 * pragma_once removed
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 28.01.04   Time: 20:09
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added license update logic
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 12.01.04   Time: 12:52
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added reply
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 29.12.03   Time: 18:27
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added RegisterBroker method
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 25.12.03   Time: 21:22
 * Created in $/VS/Servers/DirectoryServer/Services
 * added registration service and storage
 *
 ****************************************************************************/


#ifndef VS_REGISTRATION_SERVICE_H
#define VS_REGISTRATION_SERVICE_H

#include "../../ServerServices/Common.h"
#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "../../common/transport/Router/VS_PoolThreadsService.h"
#include "../../LicenseLib/VS_License.h"
#include "../../common/std/cpplib/VS_WorkThread.h"
#include "../../common/std/cpplib/VS_MessageHandler.h"
#include "../../common/std/cpplib/VS_MessageData.h"
#include "transport/Client/VS_DNSFunction.h"

#include "VS_RegStorage.h"

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class VS_RegistrationService :
	public VS_TransportRouterServiceReplyHelper,
	public VS_MessageHandler,
	public boost::enable_shared_from_this<VS_RegistrationService>
{
private:
	VS_RegStorage*                       m_storage;
	VS_License*		                     m_demolicense;
	boost::shared_ptr<VS_WorkThread>     m_worker;
	std::map<unsigned long, std::string> m_secures;
	std::vector<boost::shared_ptr<VS_DDNS>> m_ddns_servers;

private:
	VS_RegistrationService(void);

	bool CheckCAAvailability();
	unsigned long GetArmTemplateIndex(const VS_License* lic, int ver);
	void LoadArmSecureSections();

	bool UpdateServerCertificate(const char *server_name, const char *cert_request, VS_SimpleStr &out_cert, long &regRes);

	bool VerifyServerCert(const char *server_name, const char *server_cert, const bool use_new_cert = true);
	void GetPrivateKeyForEncryption(const char *server_name, std::vector<char> &p_key);
	void HandleConnectionData(const char *conn_data, const char *srv_name);
	void DoDDNSUpdate(std::string &conn_data, std::string &srv_dns_name);

public:
	enum {
		MSG_DDNS_UPDATE = 1
	};

	class DDNSUpdateMessageData : public VS_MessageData {
	public:
		DDNSUpdateMessageData(const char *conn_data, const char *srv_name);
		virtual ~DDNSUpdateMessageData();

		std::string &GetConnData(void);
		std::string &GetSrvName(void);
	private:
		std::string m_conn_data;
		std::string m_srv_name;
	};

	static boost::shared_ptr<VS_RegistrationService> Make(void)
	{
		boost::shared_ptr<VS_RegistrationService> ptr(new VS_RegistrationService);

		return ptr;
	}

	~VS_RegistrationService(void);

	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll) override;
	void Destroy(const char* our_endpoint, const char* our_service) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

    bool OnPointConnected_Event( const VS_PointParams* prm ) override;
    bool OnPointDeterminedIP_Event(const char* uid, const char* ip) override;
	bool PointDeterminedIP_Handler(const char* uid, const char* ip);
    bool OnPointDisconnected_Event( const VS_PointParams* prm) override;

    // Service implementation

    void SetStorage(VS_RegStorage* ds)
    { m_storage=ds; };

    // LOGEVENT_METHOD(TYPE_PARAM,[F1,F2,F3])
	void LogEvent_Method(VS_BrokerEvents type, const char* f1 = 0, const char* f2 = 0, const char* f3 = 0, VS_RegStorage::BrokerStates state = VS_RegStorage::BS_VALID);

	// LOGSTATS_METHOD(STATS)
	void LogStats_Method(const void* stats, size_t size, const char* from_server = 0);

	bool RegisterServer_Method(const char* server_id, const char * server_name, const char* key, const char* pass,
		const char *cert_request, const char *version, const char *src_server, const char *os_info, const char *cpu_info, const char *product_version, bool IsOffline, VS_Container &rCnt);
	bool VerifyServerName(const char *server_name, const char *src_server);

	bool RegisterServerFromFile_Method(VS_Container& cnt);
	bool CertificateUpdate_Method(VS_Container &cnt);
	bool DecryptContentAlloc(const unsigned char *encr_buf, const unsigned long encr_sz, unsigned char *&buf_out, unsigned long &out_buf_sz, long &arm_hw_key);

	// UPDATELICENSE_METHOD([NAME_PARAM]+)
	void UpdateLicense_Method(const VS_SimpleStr& src_ep, VS_Container& cnt, int version, bool IsOffline = false);

	// UPDATECONFIGURATION_METHOD
	void UpdateConfiguration_Method(const char *conn_data, const char *srv_name);

	// UPDATE_CALLCFGCORRECTOR_METHOD
	bool UpdateCallCfgCorrector_Method(const char* server_name, const char *hash, VS_Container &cnt);
	void VerifySignFailedReplay(const VS_SimpleStr& server_id);
	virtual void HandleMessage(const boost::shared_ptr<VS_MessageData> &message);
	void Post(const boost::shared_ptr<VS_MessageHandler> &h, const boost::shared_ptr<VS_MessageData> &data);
};

#endif

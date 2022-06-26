/**
 ****************************************************************************
 * (c) 2002-2003 Visicron Inc.  http://www.visicron.net/
 *
 * Project: Visicron Registration Server
 *
 * \file VS_RegistrationService.cpp
 *
 * $Revision: 62 $
 * $History: VS_RegistrationService.cpp $
 *
 * *****************  Version 62  *****************
 * User: Smirnov      Date: 16.08.12   Time: 21:36
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * -crypt mgateway
 * - arm futures ver added
 *
 * *****************  Version 61  *****************
 * User: Mushakov     Date: 16.03.12   Time: 19:13
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - log
 *
 * *****************  Version 60  *****************
 * User: Mushakov     Date: 16.03.12   Time: 19:04
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - Logging Offline
 *
 * *****************  Version 59  *****************
 * User: Mushakov     Date: 16.03.12   Time: 16:10
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - interfaces corrected
 *
 * *****************  Version 58  *****************
 * User: Mushakov     Date: 16.03.12   Time: 15:54
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - Add OS and CPU info in Regiatration Server
 *
 * *****************  Version 57  *****************
 * User: Mushakov     Date: 6.10.11    Time: 21:58
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *
 * *****************  Version 56  *****************
 * User: Mushakov     Date: 6.10.11    Time: 21:35
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - update cert fix
 *
 * *****************  Version 55  *****************
 * User: Mushakov     Date: 6.10.11    Time: 16:30
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - fix VerifyServerCert
 *
 * *****************  Version 54  *****************
 * User: Mushakov     Date: 5.10.11    Time: 22:19
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - debug log added
 *
 * *****************  Version 53  *****************
 * User: Mushakov     Date: 5.10.11    Time: 22:09
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - log added
 *
 * *****************  Version 52  *****************
 * User: Mushakov     Date: 5.10.11    Time: 21:36
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - ssl refactoring (SetCert interfaces)
 *
 * *****************  Version 51  *****************
 * User: Ktrushnikov  Date: 8.08.11    Time: 14:24
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - FromVariant_NoTZ() for valid_until and valid_after
 * - include path fixed
 *
 * *****************  Version 50  *****************
 * User: Dront78      Date: 25.05.11   Time: 18:53
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - armadillo optimizations disabled totally
 *
 * *****************  Version 49  *****************
 * User: Mushakov     Date: 24.05.11   Time: 20:52
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - Reg Offline
 *
 * *****************  Version 48  *****************
 * User: Mushakov     Date: 24.05.11   Time: 18:23
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - offline Registration Supported
 *
 * *****************  Version 47  *****************
 * User: Mushakov     Date: 6.05.11    Time: 20:43
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - new reg; new reg cert; cert chain supported in tc_server
 *
 * *****************  Version 46  *****************
 * User: Mushakov     Date: 25.03.11   Time: 15:30
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - check conn required lic in offline registration
 *
 * *****************  Version 45  *****************
 * User: Mushakov     Date: 24.03.11   Time: 16:55
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *
 * *****************  Version 44  *****************
 * User: Mushakov     Date: 23.03.11   Time: 17:49
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - Server Name Verification added at registration
 *
 * *****************  Version 43  *****************
 * User: Mushakov     Date: 11.03.11   Time: 16:26
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - cert Update fix
 *
 * *****************  Version 42  *****************
 * User: Mushakov     Date: 9.03.11    Time: 20:06
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - Roaming
 *
 * *****************  Version 41  *****************
 * User: Mushakov     Date: 2.03.11    Time: 17:43
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - roaming
 *
 * *****************  Version 40  *****************
 * User: Mushakov     Date: 23.02.11   Time: 4:05
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - SecureHandshake ver. 2 added
 *
 * *****************  Version 39  *****************
 * User: Mushakov     Date: 26.01.11   Time: 19:48
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - log added
 *
 * *****************  Version 38  *****************
 * User: Mushakov     Date: 11.01.11   Time: 16:36
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - redundant arm_key generation removed
 *
 * *****************  Version 37  *****************
 * User: Mushakov     Date: 29-11-10   Time: 20:55
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - registration 3.1 (sm)
 * - auto update cert for as, bs, rs 3.1 (sm)
 *
 * *****************  Version 36  *****************
 * User: Mushakov     Date: 11.11.10   Time: 16:19
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - update certificate on server start
 * - add logs to reg
 *
 * *****************  Version 35  *****************
 * User: Mushakov     Date: 2.11.10    Time: 15:18
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *
 * *****************  Version 34  *****************
 * User: Mushakov     Date: 1.11.10    Time: 21:00
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - cert update added
 * - registration from server added
 * - authorization servcer added
 *
 * *****************  Version 33  *****************
 * User: Ktrushnikov  Date: 30.09.10   Time: 15:34
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * RegServer
 * - MANAGER_SRV added
 * - Post LogStats_Method to REGISTRATION_SRV
 *
 * *****************  Version 32  *****************
 * User: Mushakov     Date: 30.09.10   Time: 15:13
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *
 * *****************  Version 31  *****************
 * User: Mushakov     Date: 10.09.10   Time: 20:24
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - Registration on SM added
 *
 * *****************  Version 30  *****************
 * User: Mushakov     Date: 18.08.10   Time: 14:18
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - x.509 fields restrictions added
 *
 * *****************  Version 29  *****************
 * User: Mushakov     Date: 28.05.10   Time: 16:48
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - waiting for connect reg result 100 sec
 * - log
 *
 * *****************  Version 28  *****************
 * User: Mushakov     Date: 13.05.10   Time: 19:17
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - 7399
 *
 * *****************  Version 27  *****************
 * User: Mushakov     Date: 22.04.10   Time: 19:15
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - install default key for trial mode, restart when key changed
 *
 * *****************  Version 26  *****************
 * User: Dront78      Date: 8.04.10    Time: 17:55
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - armadillo upgraded to maximum keys
 *
 * *****************  Version 25  *****************
 * User: Mushakov     Date: 7.04.10    Time: 20:34
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - offline registration supported
 * - arm_key encrypted
 *
 * *****************  Version 24  *****************
 * User: Ktrushnikov  Date: 7.04.10    Time: 20:12
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - hotfix: arm secured sections index bit shift
 *
 * *****************  Version 23  *****************
 * User: Dront78      Date: 7.04.10    Time: 19:43
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *
 * *****************  Version 22  *****************
 * User: Mushakov     Date: 7.04.10    Time: 19:41
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - reg offline
 *
 * *****************  Version 21  *****************
 * User: Dront78      Date: 6.04.10    Time: 16:50
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - new license algorithm
 *
 * *****************  Version 20  *****************
 * User: Mushakov     Date: 2.04.10    Time: 17:24
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *
 * *****************  Version 19  *****************
 * User: Mushakov     Date: 2.04.10    Time: 14:15
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *
 * *****************  Version 18  *****************
 * User: Mushakov     Date: 2.04.10    Time: 14:11
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *
 * *****************  Version 17  *****************
 * User: Mushakov     Date: 1.04.10    Time: 17:32
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - work without reg server supported
 *
 * *****************  Version 16  *****************
 * User: Mushakov     Date: 23.03.10   Time: 17:40
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - log events added
 *
 * *****************  Version 15  *****************
 * User: Mushakov     Date: 19.03.10   Time: 17:59
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - ClientType is received from client
 * - Endpoint function removed from code
 * - arr_key logged inRegistryServer
 *
 * *****************  Version 14  *****************
 * User: Dront78      Date: 26.02.10   Time: 19:54
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - updated secures to latest Armadillo versions
 *
 * *****************  Version 13  *****************
 * User: Dront78      Date: 24.02.10   Time: 19:05
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * updated license manager
 *
 * *****************  Version 12  *****************
 * User: Dront78      Date: 18.02.10   Time: 10:09
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * Armadillo Security Passport added
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 12.02.10   Time: 15:56
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - ARM Keys added
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 8.02.10    Time: 18:04
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - some commemts removed
 * - readingtrial minutes added
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 28.01.10   Time: 19:53
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - offline registration supported (VCS)
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 18.12.09   Time: 18:04
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - Removed VCS_BUILD somewhere
 * - Add new field to license
 * - Chat service for bsServer renamed
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 16.11.09   Time: 15:58
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - new Certificate
 *  - added error code in Cert Verify
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
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 25.11.07   Time: 19:24
 * Updated in $/VS2005/Servers/DirectoryServer/Services
 * - bug fix: don't check broker state if it is offline-registration
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 25.11.07   Time: 13:47
 * Updated in $/VS2005/Servers/DirectoryServer/Services
 * - Offline Registration added
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 22.03.07   Time: 16:18
 * Updated in $/VS2005/Servers/DirectoryServer/Services
 * добавлена верификация ProductType при регистрации брокера
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 5.02.07    Time: 21:31
 * Updated in $/VS2005/Servers/DirectoryServer/Services
 * - project configuration
 * - depricated functions warnings suppressed
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:51
 * Created in $/VS2005/Servers/DirectoryServer/Services
 *
 * *****************  Version 28  *****************
 * User: Stass        Date: 6.12.06    Time: 12:31
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added signed  license encode for server protocol version 3
 *
 * *****************  Version 27  *****************
 * User: Smirnov      Date: 28.11.06   Time: 16:44
 * Updated in $/VS/Servers/DirectoryServer/Services
 * - more complex random generation functions
 *
 * *****************  Version 26  *****************
 * User: Mushakov     Date: 20.06.06   Time: 14:43
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 25  *****************
 * User: Mushakov     Date: 5.06.06    Time: 12:47
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 24  *****************
 * User: Mushakov     Date: 1.06.06    Time: 18:56
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 23  *****************
 * User: Mushakov     Date: 1.06.06    Time: 18:33
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 22  *****************
 * User: Mushakov     Date: 1.06.06    Time: 18:12
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 21  *****************
 * User: Stass        Date: 31.05.06   Time: 19:45
 * Updated in $/VS/Servers/DirectoryServer/Services
 * reg server key  pass moved
 *
 * *****************  Version 20  *****************
 * User: Mushakov     Date: 4.05.06    Time: 16:06
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 19  *****************
 * User: Mushakov     Date: 3.05.06    Time: 15:52
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 18  *****************
 * User: Mushakov     Date: 2.05.06    Time: 17:56
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 17  *****************
 * User: Mushakov     Date: 27.04.06   Time: 18:02
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 16  *****************
 * User: Mushakov     Date: 27.04.06   Time: 13:10
 * Updated in $/VS/Servers/DirectoryServer/Services
 * updated manage of certificate
 *
 * *****************  Version 15  *****************
 * User: Mushakov     Date: 26.04.06   Time: 16:33
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 14  *****************
 * User: Mushakov     Date: 25.04.06   Time: 15:54
 * Updated in $/VS/Servers/DirectoryServer/Services
 * Added certificate issue
 *
 * *****************  Version 13  *****************
 * User: Stass        Date: 21.04.06   Time: 17:05
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 12  *****************
 * User: Stass        Date: 9.07.05    Time: 14:57
 * Updated in $/VS/Servers/DirectoryServer/Services
 * logging connect
 *
 * *****************  Version 11  *****************
 * User: Stass        Date: 30.01.04   Time: 18:39
 * Updated in $/VS/Servers/DirectoryServer/Services
 * добавил правильное поведение в случае поломки базы
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 30.01.04   Time: 16:31
 * Updated in $/VS/Servers/DirectoryServer/Services
 * buxfix fo non-existant broker
 *
 * *****************  Version 9  *****************
 * User: Stass        Date: 28.01.04   Time: 20:09
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added license update logic
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 26.01.04   Time: 16:21
 * Updated in $/VS/Servers/DirectoryServer/Services
 * IP logging changed
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 15.01.04   Time: 17:19
 * Updated in $/VS/Servers/DirectoryServer/Services
 * changed debug printout
 * updated temp ep behavior
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 12.01.04   Time: 12:52
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added reply
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 9.01.04    Time: 17:21
 * Updated in $/VS/Servers/DirectoryServer/Services
 * fixed bugs with register
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 6.01.04    Time: 20:33
 * Updated in $/VS/Servers/DirectoryServer/Services
 * module based debug print
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 30.12.03   Time: 18:17
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added key check (still commented)
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

#include "VS_RegistrationService.h"
#include "../ddns_utils.h"

#include "../../ServerServices/VS_MediaBrokerStats.h"
#include "../../common/std/cpplib/VS_IntConv.h"
#include "../../common/std/cpplib/VS_Utils.h"
#include "std/cpplib/md5.h"
#include "SecureLib/VS_CertificateIssue.h"
#include "SecureLib/VS_CryptoInit.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "SecureLib/VS_SecureConstants.h"
#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_Sign.h"
#include "SecureLib/VS_UtilsLib.h"
#include "../../common/transport/Lib/VS_TransportLib.h"
#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "../../common/acs/Lib/VS_AcsLib.h"
#include "../../common/acs/connection/VS_ConnectionTCP.h"
#include "ProtectionLib/Protection.h"
#include "../../GetLicenseKey/KeyGen.h"

#include "../../common/std/cpplib/VS_SimpleWorkThread.h"
#include "../../common/std/cpplib/netutils.h"
#include "../../common/std/cpplib/VS_FileTime.h"
#include "std/debuglog/VS_Debug.h"
#include "std/RegistrationStatus.h"
#include <time.h>

#include <string>
#include <chrono>

#define DEBUG_CURRENT_MODULE VS_DM_REGS

using namespace ddns_utils;
using namespace netutils;
using namespace std::chrono;

struct md5_hw_assign {
	unsigned short ex1;
	unsigned short ex2;
	unsigned short ex3;
	unsigned short ex4;
};

//const char PRIVATE_KEY_PASS[]					= "P[sDubdfgPLk|d)d#4cmdfa658HhdsflvFe5^&*Xl";

class RegisterServer_Task : public VS_PoolThreadsTask, public VS_TransportRouterServiceReplyHelper
{
protected:
	VS_SimpleStr		m_server_id;
	VS_SimpleStr		m_server_name;
	VS_SimpleStr		m_key;
	VS_SimpleStr		m_serial;
	VS_SimpleStr		m_cert_request;
	VS_SimpleStr		m_version;
	VS_SimpleStr		m_src_server;
	VS_SimpleStr		m_os_info;
	VS_SimpleStr		m_cpu_info;
	VS_SimpleStr		m_product_version;
	VS_RegistrationService	*m_reg_service;
	std::unique_ptr<VS_RouterMessage> m_mess;
public:
	RegisterServer_Task(VS_RegistrationService *reg_srv, const char* server_id, const char * server_name,const char *src_server, const char* key,const char* pass,
		const char *cert_request, const char *version, const char *os_info, const char* cpu_info, const char *product_version, std::unique_ptr<VS_RouterMessage>&& recvMess):m_server_id(server_id),m_server_name(server_name),m_src_server(src_server),m_key(key),
		m_serial(pass),m_cert_request(cert_request), m_version(version), m_reg_service(reg_srv),m_os_info(os_info),m_cpu_info(cpu_info), m_product_version(product_version),m_mess(std::move(recvMess))
	{
		m_recvMess = m_mess.get();
	}
	void Run()
	{
		if(!m_server_id || !m_server_name || !m_key || !m_src_server || !m_reg_service)
			return;
		VS_Container cnt;
		m_reg_service->RegisterServer_Method(m_server_id,m_server_name, m_key, m_serial,m_cert_request,m_version,m_src_server,m_os_info,m_cpu_info, m_product_version, false, cnt);
		PostReply(cnt);
	}
};

////////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
////////////////////////////////////////////////////////////////////////////////
/*
void TimetToFileTime( time_t t, LPFILETIME pft )
{
    LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD) ll;
    pft->dwHighDateTime = ll >>32;
}
*/

VS_RegistrationService::VS_RegistrationService(void)
	: m_storage(NULL), m_demolicense(NULL), m_worker(new VS_SimpleWorkThread())
{
	vs::InitOpenSSL();
	m_worker->Start("RegService");
}

VS_RegistrationService::~VS_RegistrationService()
{
	m_worker->Stop();
	m_worker->WaitStopped();
}

void VS_RegistrationService::LoadArmSecureSections()
{
	VS_RegistryKey secures(false, "Secures", false, false);
	if (secures.IsValid()) {
		char name[32] = {0};
		for (unsigned i = 1; i <= 2048; ++i) {
			sprintf(name, "Key%d", i);
			std::string key;
			if (secures.GetString(key, name))
				m_secures[i] = std::move(key);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// VS_TransportRouterCallService implementation
////////////////////////////////////////////////////////////////////////////////
bool VS_RegistrationService::Init(const char *our_endpoint, const char *our_service,const bool permittedAll)
{
	if(!m_storage)
		return false;

	m_storage->CleanUp();
	LoadArmSecureSections();


	char demo_lic_name[128];
	sprintf(demo_lic_name,"%s\\DEMO",LICENSE_KEY);

	VS_RegistryKey	demo_key(false, demo_lic_name, false, true);
	if (demo_key.IsValid())
	{
		m_demolicense=new VS_License(demo_key);
		if (!m_demolicense->IsValid())
		{
			dprint0("REGS: DEMO license invalid\n");
			delete m_demolicense;
			m_demolicense=NULL;
		};
	}
	else
		dprint0("REGS: Can't load DEMO license from registry\n");
	if(!CheckCAAvailability())
	{
		dprint0("CA: Test CA failed\n");
		return false;
	}

	// read DDNS servers configuration from registry
	for (int i = 0;; i++)
	{
		char key_name[MAX_PATH] = { 0 };
		_snprintf(key_name, MAX_PATH, "%s\\DDNS%d", CONFIGURATION_KEY, i);

		if (i > 0)
		{
			VS_RegistryKey dns_key(false, key_name);

			if (!dns_key.IsValid())
				break;
		}

		{
			boost::shared_ptr<VS_DDNS> pddns(new VS_DDNS);

			if (!pddns->SetParamsFromReg(i))
				continue;

			m_ddns_servers.push_back(pddns);
		}
	}

	return true;
}

void VS_RegistrationService::Destroy(const char* /*our_endpoint*/, const char* /*our_service*/)
{
	m_storage->CleanUp();
}

bool VS_RegistrationService::CheckCAAvailability()
{
	std::unique_ptr<char, free_deleter> CA_PrivateKey;
	std::unique_ptr<char, free_deleter> CA_cert;
	char			*pem_buf(0);
	uint32_t		sz(0);
	VS_CertificateRequest	req;
	VS_PKey					pkey;
	VS_CertificateCheck	certCheck, certCheck1;
	VS_CertAuthority	ca;
	char				*request_buf(0);


	VS_GET_PEM_CACERT

	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);

	rkey.GetValue(CA_PrivateKey, VS_REG_BINARY_VT, SRV_PRIVATE_KEY);

	if(!CA_PrivateKey)
	{
		dprint0("CA: PrivateKey is absent!\n");
		return false;
	}
	int ca_cert_len = rkey.GetValue(CA_cert, VS_REG_BINARY_VT, SRV_CERT_KEY);

	if(!CA_cert || ca_cert_len<=0)
	{
		dprint0("CA: Certificate is absent!\n");
		return false;
	}

	if (!certCheck.SetCert(CA_cert.get(), ca_cert_len, store_PEM_BUF) || !certCheck.SetCertToChain(PEM_CACERT, strlen(PEM_CACERT) + 1, store_PEM_BUF))
	{
		dprint0("CA: CA_Cert is invalid!\n");
		return false;
	}

	if(!certCheck.VerifyCert())
	{
		dprint0("CA: CA Certificate is not root certificate!\n");
		return false;
	}

	char broker[] = "TEST:1";
	if((!pkey.GenerateKeys(2048,alg_pk_RSA))||
		(!req.SetPKeys(&pkey,&pkey))||(!req.SetEntry("commonName",broker))||
		(!req.SignRequest()))
	{
		dprint0("CA: Test Generate request failed\n");
		return false;
	}
	req.SaveTo(request_buf,sz,store_PEM_BUF);
	if(!sz)
	{
		dprint0("CA: save test request failed!\n");
		return false;
	}
	request_buf = new char[sz+1];
	if(!req.SaveTo(request_buf,sz,store_PEM_BUF))
	{
		delete [] request_buf;
		dprint0("CA: szve test request failed!\n");
		return false;
	}
	if (!ca.SetCACertificate(CA_cert.get(), strlen(CA_cert.get()) + 1, store_PEM_BUF))
	{
		delete [] request_buf;
		dprint0("CA: Invalid CA certificate!\n");
		return false;
	}

	if ((!ca.SetCAPrivateKey(CA_PrivateKey.get(), store_PEM_BUF)))
	{
		delete [] request_buf;
		dprint0("CA: Invalid private key\n");
		return false;
	}
	if((!ca.SetCertRequest(request_buf,(unsigned long)strlen(request_buf)+1,store_PEM_BUF))||
		(!ca.VerifyRequestSign()))
	{
		delete [] request_buf;
		dprint0("CA: Invalid private key pass!\n");
		return false;
	}
	delete [] request_buf;
	request_buf = 0;
	if((!ca.SetSerialNumber(1))||(!ca.SetExpirationTime(60*60*24)))
	{
		dprint0("CA: Test Set Expiration time and serialNumber failed!\n");
		return false;
	}
	sz = 0;
	ca.IssueCertificate(pem_buf,sz,store_PEM_BUF);
	if(!sz)
	{
		dprint0("CA: Test IssueCertificate failed.\n");
		return false;
	}
	pem_buf = new char[sz];
	if(!ca.IssueCertificate(pem_buf,sz,store_PEM_BUF))
	{
		delete [] pem_buf;
		dprint0("CA: Test IssueCertificate failed\n");
		return false;
	}

	if((!certCheck1.SetCert(pem_buf,sz,store_PEM_BUF))||(!certCheck1.SetCertToChain(PEM_CACERT,strlen(PEM_CACERT)+1,store_PEM_BUF)) ||
		!certCheck1.SetCertToChain(CA_cert.get(), ca_cert_len, store_PEM_BUF))
	{
		delete [] pem_buf;
		dprint0("CA: CA_Cert is invalid!\n");
		return false;
	}
	delete [] pem_buf;
	if(!certCheck1.VerifyCert())
	{
		dprint0("CA: Verificateion Issued Certificate failed!\n");
		return false;
	}
	return true;
}
bool VS_RegistrationService::Processing(std::unique_ptr<VS_RouterMessage>&&recvMess)
{
    if (recvMess == 0)  return true;
    switch (recvMess->Type())
    {
	case transport::MessageType::Invalid:
        break;
	case transport::MessageType::Request:
        m_recvMess = recvMess.get();
        {
            VS_Container cnt;
            if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
                const char* method = 0;
                if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0) {
                // Process methods
                    if (_stricmp(method, LOGEVENT_METHOD) == 0)
                    {
						if(IsAuthorized(m_recvMess->SrcServer()))
						{
							int32_t type;
							if(!cnt.GetValue(TYPE_PARAM,type))
								type=0;
							LogEvent_Method((VS_BrokerEvents)type,
								cnt.GetStrValueRef(FIELD1_PARAM),
								cnt.GetStrValueRef(FIELD2_PARAM),
								cnt.GetStrValueRef(FIELD3_PARAM));
						}

					}
					else if (_stricmp(method, LOGSTATS_METHOD) == 0)
					{
						if(IsAuthorized(m_recvMess->SrcServer()))
						{
							size_t size = 0;
							const void* stats = cnt.GetBinValueRef(MEDIABROKERSTATS_PARAM, size);
							const char* server = cnt.GetStrValueRef(SERVER_PARAM);
							const char* FromServer = (server && *server)? server: m_recvMess->SrcServer();
							LogStats_Method(stats,size,FromServer);
						}
					}
					else if (_stricmp(method, REGISTERSERVER_METHOD) == 0)
					{
						PutTask(new RegisterServer_Task(this,
							cnt.GetStrValueRef(SERVERID_PARAM),
							cnt.GetStrValueRef(SERVERNAME_PARAM),
							m_recvMess->SrcServer(),
							cnt.GetStrValueRef(KEY_PARAM),
							cnt.GetStrValueRef(PASSWORD_PARAM),
							cnt.GetStrValueRef(CERT_REQUEST_PARAM),
							cnt.GetStrValueRef("Srv Version"),
							cnt.GetStrValueRef("OS Info"),
							cnt.GetStrValueRef("CPU Info"),
							cnt.GetStrValueRef(SERVER_VERSION_PARAM),
							std::move(recvMess)
						));
					}
					else if(_stricmp(method,REGISTERSERVEROFFLINEFROMFILE_METHOD) == 0)
					{
						RegisterServerFromFile_Method(cnt);
					}
					else if (_stricmp(method, REGISTERSERVEROFFLINE_METHOD) == 0)
					{
						VS_Container rCnt;
						unsigned int armversion = atou_s(m_recvMess->AddString());
						if (!armversion) // i don't understand where AddString() filled
							armversion = atou_s(cnt.GetStrValueRef("Version"));
						bool reg_ok = RegisterServer_Method(cnt.GetStrValueRef(SERVERID_PARAM),
											cnt.GetStrValueRef(SERVERNAME_PARAM),
											cnt.GetStrValueRef(KEY_PARAM),
											cnt.GetStrValueRef(PASSWORD_PARAM),
											cnt.GetStrValueRef(CERT_REQUEST_PARAM),
											cnt.GetStrValueRef("Srv Version"),
											m_recvMess->SrcServer(),
											cnt.GetStrValueRef("OS Info"),
											cnt.GetStrValueRef("CPU Info"),
											cnt.GetStrValueRef(SERVER_VERSION_PARAM),
											true,rCnt);
						PostReply(rCnt);

						if ( reg_ok )
							UpdateLicense_Method(cnt.GetStrValueRef(SERVERNAME_PARAM), cnt, armversion, true);
					}
					else if(_stricmp(method,CERTIFICATEUPDATE_METHOD) == 0)
					{
						CertificateUpdate_Method(cnt);
					}
					else if (_stricmp(method, UPDATELICENSE_METHOD) == 0)
					{
						if(IsAuthorized(m_recvMess->SrcServer())) {
							unsigned int armversion = atou_s(m_recvMess->AddString());
							UpdateLicense_Method(recvMess->SrcServer(), cnt, armversion);
						}
					}
					else if (_stricmp(method, UPDATECONFIGURATION_METHOD) == 0)
					{
						int32_t type = 0;
						cnt.GetValue(TYPE_PARAM, type);
						if (type == BE_UPDATE_DDNS)
							UpdateConfiguration_Method(cnt.GetStrValueRef(FIELD1_PARAM), recvMess->SrcServer());
					}
					else if (_stricmp(method, POINTDETERMINEDIP_METHOD) == 0) {
							PointDeterminedIP_Handler(cnt.GetStrValueRef(USERNAME_PARAM), cnt.GetStrValueRef(IPCONFIG_PARAM));
					}
					else if (_stricmp(method, UPDATE_CALLCFGCORRECTOR_METHOD) == 0)
					{
						VS_Container rCnt;

						rCnt.AddValue(METHOD_PARAM, UPDATE_CALLCFGCORRECTOR_METHOD);
						bool update_ok = UpdateCallCfgCorrector_Method(
							recvMess->SrcServer(),
							cnt.GetStrValueRef(HASH_PARAM),
							rCnt);

						// send reply back
						if (update_ok)
							PostReply(rCnt);
					}
                }
            }
        }
        break;
	case transport::MessageType::Reply:
        break;
	case transport::MessageType::Notify:
        break;
    }
    m_recvMess = nullptr;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// LOGEVENT_METHOD(TYPE_PARAM)
////////////////////////////////////////////////////////////////////////////////
void VS_RegistrationService::LogEvent_Method(VS_BrokerEvents type,const char* f1,const char* f2,const char* f3,VS_RegStorage::BrokerStates state)
{
	m_storage->LogEvent(type,m_recvMess->SrcServer(),f1,f2,f3,state == m_storage->BS_VALID ? m_storage->GetState(m_recvMess->SrcServer()) : state);
}

////////////////////////////////////////////////////////////////////////////////
// SENDSTATS_METHOD(TYPE_PARAM)
////////////////////////////////////////////////////////////////////////////////
void VS_RegistrationService::LogStats_Method(const void* stats, size_t size, const char* from_server)
{
		if (!stats || size!=sizeof(VS_MediaBrokerStats))
			return;
		m_storage->LogStats(from_server,(const VS_MediaBrokerStats*)stats);
}

////////////////////////////////////////////////////////////////////////////////
// REGISTERBROKER_METHOD(ENDPOINT_PARAM,KEY_PARAM,PASSWORD_PARAM)
////////////////////////////////////////////////////////////////////////////////

bool VS_RegistrationService::VerifyServerName(const char *server_name, const char *src_server)
{
	if(!server_name || !*server_name || !src_server || !*src_server)
		return false;
	VS_ConnectionTCP   conn;
	unsigned long mills(5000);
	unsigned char	resultCode = hserr_antikyou, fatalSilenceCoef = 0, retHops = 0;
	unsigned short  maxConnSilenceMs = 0;
	char *server_id = 0;
	char *user_id = 0;
	bool tcpKeepAliveSupport;

	net::HandshakeHeader* hs = VS_FormTransportHandshake(OurEndpoint(), server_name, 1, false);
	if(!hs)
		return false;
	char *tmp_serv_name = strdup(server_name);
	char *p = strchr(tmp_serv_name,'#');
	if(p)
		*p = 0;
	bool res(false);
	if(conn.Connect(tmp_serv_name,4307,mills)&&conn.CreateOvReadEvent() &&
		conn.CreateOvWriteEvent() && VS_WriteZeroHandshake( &conn, hs, mills ))
	{
		free((void*)hs);
		hs = 0;
		if(VS_ReadZeroHandshake( &conn, &hs, mills ))
		{
			if(VS_TransformTransportReplyHandshake( hs,
				resultCode, maxConnSilenceMs, fatalSilenceCoef,
				retHops, server_id,
				user_id, tcpKeepAliveSupport) &&
				hserr_alienserver == resultCode &&
				!_stricmp(server_id,src_server))
			{
				free((void*)hs);
				hs = 0;
				res = true;
			}
		}
	}
	if(hs)
		free((void*)hs);
	free(tmp_serv_name);
	return res;
}
bool VS_RegistrationService::RegisterServer_Method(const char* server_id,const char * server_name, const char* key,
												   const char* serial, const char *cert_request,
												   const char *version, const char *src_server, const char *os_info,
												   const char *cpu_info, const char *product_version,
												   bool IsOffline, VS_Container &rCnt)
{
	if(!m_storage || !server_id || !server_name || !key || (!src_server && !IsOffline))
	{
		rCnt.AddValue(METHOD_PARAM, (IsOffline)? REGISTERSERVEROFFLINE_METHOD: REGISTERSERVER_METHOD);
		rCnt.AddValue(SERVERID_PARAM,server_id);
		rCnt.AddValue(SERVERNAME_PARAM,server_name);
		rCnt.AddValueI32(RESULT_PARAM, RegStatus::failed);
		return true;
	}
	bool isServerNameVerified = IsOffline ? false : VerifyServerName(server_name, src_server);

	VS_CertAuthority	ca;
	unsigned long		certSerialNumber(0);
	VS_FileTime notBefore, notAfter;

	char				*pem_buf(0);
	uint32_t			sz(0);
	unsigned int		len(0);
	VS_WideStr			organization_name(0);
	VS_WideStr			country(0);
	VS_WideStr			contact_person(0);
	VS_WideStr			contact_email(0);

	std::unique_ptr<char, free_deleter> CA_PrivateKey;
	std::unique_ptr<char, free_deleter> reg_certificate;

	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	rkey.GetValue(CA_PrivateKey, VS_REG_BINARY_VT, SRV_PRIVATE_KEY);
	rkey.GetValue(reg_certificate, VS_REG_BINARY_VT, SRV_CERT_KEY);

	dprint3("REGS: trying to register broker %s,key='%s',serial='%s', src_srv='%s'\n",server_id,(const char*)key,(const char*)serial,(IsOffline)?"offline":src_server);
	VS_SimpleStr tmp(256);
	VS_SimpleStr key2(256);
	VS_GenKeyByMD5(key2);
	rCnt.AddValue(METHOD_PARAM, (IsOffline)? REGISTERSERVEROFFLINE_METHOD: REGISTERSERVER_METHOD);
	rCnt.AddValue(SERVERID_PARAM,server_id);
	rCnt.AddValue(SERVERNAME_PARAM,server_name);
	VS_PKey reqPubKey;
	VS_SimpleStr reqPubKeyBuf;

	RegStatus result(RegStatus::succeeded);
	if(cert_request)
	{
		if (!ca.SetCACertificate(reg_certificate.get(), strlen(reg_certificate.get()) + 1, store_PEM_BUF)||
			!ca.SetCAPrivateKey(CA_PrivateKey.get(), store_PEM_BUF)||
			(!ca.SetCertRequest(cert_request,(unsigned long)strlen(cert_request)+1,store_PEM_BUF))||
			(!ca.VerifyRequestSign()))
		{
			dprint3("CA: Verification Request failed. Src_srv='%s'\n",(IsOffline)?"offline":src_server);
			result = RegStatus::failed;
		}
		else
		{
			if(!ca.GetRequestPublicKey(&reqPubKey))
			{
				dprint3("CA: GetRequestPublicKey faild. Src_srv='%s'\n",(IsOffline)?"offline":src_server);
				result = RegStatus::failed;
			}
			else
			{
				char *pk_buf(0);
				uint32_t pk_buf_sz(0);
				reqPubKey.GetPublicKey(store_PEM_BUF,pk_buf,&pk_buf_sz);
				if(pk_buf_sz)
				{
					pk_buf = new char[pk_buf_sz];
					if(!reqPubKey.GetPublicKey(store_PEM_BUF,pk_buf,&pk_buf_sz))
					{
						dprint3("CA: GetRequestPublicKey faild. 2. Src_srv='%s'\n", (IsOffline)?"offline":src_server);
						result = RegStatus::failed;
					}
					else
						reqPubKeyBuf = pk_buf;
					delete [] pk_buf;
				}
				else
				{
					dprint3("CA: GetRequestPublicKey faild. 1. Src_srv='%s'\n", (IsOffline)?"offline":src_server);
					result = RegStatus::failed;
				}
			}
		}
	}
	if(IsOffline)
	{
		std::map<uint64_t, VS_License> licenses;
		if(m_storage->GetLicenses(server_id,licenses,false))
		{
			// Success if at least one license doesn't require a connection to reg server.
			result = std::any_of(licenses.begin(), licenses.end(), [](const auto& x) {
				return !(x.second.m_restrict & VS_License::REG_CONNECTED);
			}) ? RegStatus::succeeded : RegStatus::validLicenseIsNotAvailable;
		}
		else
			result = RegStatus::validLicenseIsNotAvailable;
	}
	VS_SimpleStr hw_md5;
	int	regRes(0);
	if((RegStatus::succeeded == result)&& (m_storage->RegisterServer(server_id,server_name,key,key2,serial,reqPubKeyBuf, organization_name, country,
		contact_person, contact_email, certSerialNumber, notBefore, notAfter, hw_md5, regRes)))
	{
		VS_SimpleStr hw_key_ext_value;
		VS_SimpleStr version_ext_value = "ASN1:GeneralString:";
		VS_SimpleStr server_id_ext_value = "ASN1:GeneralString:";
		VS_SimpleStr isServerNameVerified_ext_value = "ASN1:GeneralString:";

		rCnt.AddValue(KEY_PARAM,(const char*)key2);
		if(!version)
			version_ext_value+="0";
		else
			version_ext_value+= version;

		if(!!hw_md5)
		{
			hw_key_ext_value = "ASN1:GeneralString:";
			hw_key_ext_value += hw_md5;
		}

		if(isServerNameVerified)
			isServerNameVerified_ext_value+="1";
		else
			isServerNameVerified_ext_value+="0";

		if(organization_name.Length()>64)
		{
			wchar_t buf[64] ={0};
			wcsncpy_s(buf,64,organization_name,63);
			wcsncpy_s(buf+(63-3),4,L"...",3);
			organization_name = buf;
		}

		if(contact_person.Length()>64)
		{
			wchar_t buf[40] = {0};
			wcsncpy_s(buf,40,contact_person,39);
			contact_person = buf;
		}
		if(contact_email.Length()>255)
		{
			wchar_t buf[255] = {0};
			wcsncpy_s(buf,255,contact_email,254);
			contact_email = buf;
		}

		server_id_ext_value += server_id;
		if(cert_request)
		{
			if(result == RegStatus::succeeded)
			{
				unsigned cond_res(0);
				std::string org_string;
				std::string cntr_string;
				std::string person_string;
				std::string email_string;
				organization_name.ToUTF8(org_string);
				country.ToUTF8(cntr_string);
				contact_person.ToUTF8(person_string);
				contact_email.ToUTF8(email_string);
				if(notBefore >= notAfter)
					result = RegStatus::failed;
				else if((++cond_res&&!certSerialNumber)||
					(++cond_res&&!!organization_name&&!ca.SetEntry("organizationName",
							{org_string.data(), org_string.size()}))	||
					(++cond_res&&!!country&&!ca.SetEntry("countryName",
							{cntr_string.data(), cntr_string.size()}))			||
					(++cond_res&&!!contact_person&&!ca.SetEntry("surname",
							{person_string.data(), person_string.size()}))		||
					(++cond_res&&!!contact_email&&!ca.SetEntry("emailAddress",
							{email_string.data(), email_string.size()}))	||
					(++cond_res&&!ca.SetSerialNumber(certSerialNumber))||
					(++cond_res&&!ca.SetExpirationTime(notBefore.chrono_system_clock_time_point(),
														notAfter.chrono_system_clock_time_point()))||
					(++cond_res&&!!hw_key_ext_value ? !ca.SetExtension(HWKEY_HASH_CERT_EXTENSIONS,hw_key_ext_value):false) ||
					(++cond_res&&!ca.SetExtension(SERVER_ID_EXTENSION,server_id_ext_value))	||
					(++cond_res&&!ca.SetExtension(SERVER_VERSION_EXTENSIONS,version_ext_value))||
					(++cond_res&&!ca.SetExtension(SERVER_NAME_IS_VERIFIED_EXTENSION,isServerNameVerified_ext_value))
					)
				{
					dprint3("CA: Set serial or Expiration Time failed. Serial = %ld; notBefore = %s; notAfter = %s; cond_res =%d. Src_srv='%s'\n", certSerialNumber,
						tu::TimeToString(notBefore.chrono_system_clock_time_point(), "%Y%m%d%H%M%SZ", true).c_str(),
						tu::TimeToString(notAfter.chrono_system_clock_time_point(), "%Y%m%d%H%M%SZ", true).c_str(),
						cond_res, (IsOffline) ? "offline" : src_server);
					result = RegStatus::failed;
				}
				else
				{
					ca.IssueCertificate(pem_buf,sz,store_PEM_BUF);
					if(!sz)
					{
						dprint3("CA: IssueCertificate failed. sz = 0; Src_srv='%s'\n",(IsOffline)?"offline":src_server);
						result = RegStatus::failed;
					}
					else
					{
						pem_buf = new char[sz+1];
						if(!ca.IssueCertificate(pem_buf,sz,store_PEM_BUF))
						{
							dprint3("CA: Issue Certificate failed. Src_srv='%s'\n",(IsOffline)?"offline":src_server);
							result = RegStatus::failed;
						}
						else
						{
							pem_buf[sz] = 0;
							rCnt.AddValue(CERTIFICATE_PARAM,pem_buf);
							rCnt.AddValue(CERTIFICATE_CHAIN_PARAM, reg_certificate.get());
							rCnt.AddValue(PARENT_CERT_PARAM, reg_certificate.get());
							m_storage->SetServerCertificate(server_name,pem_buf);
							result = RegStatus::succeeded;
						}
						delete [] pem_buf;
					}
				}
			}
		}else
			result = RegStatus::failed;
	}
	else
	{
		dprint3("REGS: RegisterServer return false. Src_srv='%s'",(IsOffline)?"offline":src_server);
		result = result == RegStatus::succeeded ? static_cast<RegStatus>(regRes) : result;
	}
	VS_SimpleStr tmp_field1;
	tmp_field1 += "ENDP: ";
	if(IsOffline)
		tmp_field1 += " OFFLINE_REGISTRATION, ";
	else
		tmp_field1 += src_server;
	tmp_field1 += "RegResult = ";
	char tmp_buf[256]={0};
	_itoa_s(static_cast<int32_t>(result),tmp_buf,256,10);
	tmp_field1 += tmp_buf;

	m_storage->LogEvent(result==RegStatus::succeeded?BE_REGISTER:BE_REGISTER_FAIL,server_id,tmp_field1,key,serial,m_storage->BS_VALID,false);
	VS_SimpleStr reg_result_field;
	reg_result_field = result==RegStatus::succeeded? "Registration OK" : "Registration failed";
	m_storage->LogEvent(BE_HWINFO,server_id,os_info,cpu_info,reg_result_field,m_storage->BS_VALID,false);
	rCnt.AddValueI32(RESULT_PARAM, result);

	// log offline registration
	if (IsOffline && result == RegStatus::succeeded && product_version && *product_version)
	{
		m_storage->LogEvent(BE_OFFLINE_REGISTER, server_name, server_name, product_version);
	}

	// add terminals dumb data
	{
		VS_SimpleStr data;
		if (result == RegStatus::succeeded && m_storage->GetConfigCorrectorData(server_id, data))
		{
			rCnt.AddValue(SCRIPT_PARAM, data);
		}
	}

	return true;
}
bool VS_RegistrationService::CertificateUpdate_Method(VS_Container &cnt)
{
	long result(0);
	VS_Container rCnt;
	VS_SimpleStr	out_cert;
	rCnt.AddValue(METHOD_PARAM,CERTIFICATEUPDATE_METHOD);
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::unique_ptr<char, free_deleter> reg_certificate;
	rkey.GetValue(reg_certificate, VS_REG_BINARY_VT, SRV_CERT_KEY);

	const char *cert_buf = cnt.GetStrValueRef(CERTIFICATE_PARAM);
	const char *cert_req = cnt.GetStrValueRef(CERT_REQUEST_PARAM);

	dprint3("REGS: CertificateUpdate_Method\n");

	if(!m_recvMess->SrcServer() || !IsAuthorized(m_recvMess->SrcServer()))
		result = 0;
	else if(UpdateServerCertificate(m_recvMess->SrcServer(),cert_req,out_cert,result))
	{
		result = 1;
		rCnt.AddValue(CERTIFICATE_PARAM,out_cert);
		rCnt.AddValue(CERTIFICATE_CHAIN_PARAM, reg_certificate.get());
		rCnt.AddValue(PARENT_CERT_PARAM, reg_certificate.get());
	}
	dprint3("REGS: Certificate Update result = %ld\n", result);
	rCnt.AddValueI32(RESULT_PARAM, result);
	PostReply(rCnt);
	return (result == 1);
}

bool VS_RegistrationService::UpdateServerCertificate(const char *server_name, const char *cert_request,VS_SimpleStr &out_cert, long &regRes)
{
//VS_GET_PEM_CACERT

	VS_Certificate	currCert;
	VS_CertAuthority	ca;
	VS_SimpleStr	certPEM;
	VS_FileTime		maxLicTime;
	VS_FileTime		notBeforeTime;
	std::string certNotBeforeBuf, certNotAfterBuf;
	unsigned long	cert_id(0);
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::unique_ptr<char, free_deleter> CA_PrivateKey;
	rkey.GetValue(CA_PrivateKey, VS_REG_BINARY_VT, SRV_PRIVATE_KEY);
	std::unique_ptr<char, free_deleter> reg_certificate;
	int ca_cert_len = rkey.GetValue(reg_certificate, VS_REG_BINARY_VT, SRV_CERT_KEY);

	dprint3("REGS: UpdateServerCertificate request for %s\n",server_name);
	unsigned cond_res(0);
	if( (++cond_res
			&& VS_ServCertInfoInterface::get_info_res::ok
			!= m_storage->GetServerCertificate(server_name,certPEM)) ||
		(++cond_res && !m_storage->GetMaxLicTime(server_name,maxLicTime)) ||
		(++cond_res && !VerifyServerCert(server_name,certPEM) )||
		(++cond_res && !currCert.SetCert(certPEM,certPEM.Length()+1,store_PEM_BUF) )||
		(++cond_res &&!currCert.GetExpirationTime(certNotBeforeBuf, certNotAfterBuf, false)) ||
		(++cond_res && ca_cert_len<=0)||
		(++cond_res && !ca.SetCACertificate(reg_certificate.get(), ca_cert_len, store_PEM_BUF))||
		(++cond_res && !ca.SetCAPrivateKey(CA_PrivateKey.get(), store_PEM_BUF))||
		(++cond_res && !ca.SetCertRequest(cert_request,(unsigned long)strlen(cert_request)+1,store_PEM_BUF))||
		(++cond_res && !ca.VerifyRequestSign()))
	{
		dprint3("REGS: UpdateServerCertificate verify or init faild for %s. Code = %d\n",server_name,cond_res );
		regRes = 0;
		return false;
	}
	char year[5] ={0};
	char month[3] = {0};
	char day[3] = {0};
	char hours[3] = {0};
	char mins[3] = {0};
	char sec[3] = {0};

	strncpy(year,certNotAfterBuf.data(),4);
	strncpy(month,certNotAfterBuf.data()+4,2);
	strncpy(day,certNotAfterBuf.data()+6,2);
	strncpy(hours,certNotAfterBuf.data()+8,2);
	strncpy(mins,certNotAfterBuf.data()+10,2);
	strncpy(sec,certNotAfterBuf.data()+12,2);

	time_t notAfterT;
	time_t nowT;
	time(&nowT);
	tm notAtm;
	notAtm.tm_sec = atoi(sec);
	notAtm.tm_min = atoi(mins);
	notAtm.tm_hour = atoi(hours);
	notAtm.tm_mday = atoi(day);
	notAtm.tm_mon = atoi(month) - 1;
	notAtm.tm_year = atoi(year) - 1900;
	notAfterT = mktime(&notAtm);
	if(5<(notAfterT - nowT)/60/60/24)
	{
		dprint3("REGS: UpdateServerCertificate old cert still valid for %s\n",server_name);
		regRes = 0;
		return false;
	}
	VS_FileTime nowFT;
	nowFT.Now();
	VS_FileTime difime = maxLicTime;
	difime -= nowFT;

	if((maxLicTime<=nowFT)||
		(5>= (difime.GetTimeInSec())/60/60/24))
	{
		dprint3("REGS: UpdateServerCertificate new cert is absent for %s\n",server_name);
		regRes = 0;
		return false;
	}
	if(!ca.SetEntry("commonName",server_name))
	{
		dprint3("REGS: UpdateServerCertificate ca.SetEntry failed for %s\n",server_name);
		regRes = 0;
		return false;
	}
	std::string buf;
	const char* exts[] = { SERVER_ID_EXTENSION, HWKEY_HASH_CERT_EXTENSIONS, SERVER_VERSION_EXTENSIONS, SERVER_NAME_IS_VERIFIED_EXTENSION };
	for(int i = 0;i<4;i++)
	{
		bool isSucess(true);
		if(currCert.GetExtension(exts[i],buf))
		{
			VS_SimpleStr currExt = "ASN1:GeneralString:";
			currExt += buf.c_str();
			if(!ca.SetExtension(exts[i],currExt))
				isSucess = false;
		}
		if(!isSucess)
		{
			dprint3("REGS: UpdateServerCertificate !isSuccess for %s\n",server_name);
			regRes = 0;
			return false;
		}
	}
	const char* entries[] = { "organizationName", "countryName", "surname", "emailAddress" };
	for(int i =0;i<4;i++)
	{
		if(currCert.GetSubjectEntry(entries[i],buf))
			ca.SetEntry(entries[i],buf.c_str());
	}

	if(!m_storage->GetNewCertificateId(server_name,maxLicTime,cert_id) || !cert_id || !ca.SetSerialNumber(cert_id))
	{
		dprint3("REGS: UpdateServerCertificate getting certificate id error for %s\n",server_name);
		regRes = 0; return false;
	}

	if (!ca.SetExpirationTime(system_clock::from_time_t(nowT), maxLicTime.chrono_system_clock_time_point()))
	{
		dprint3("REGS: UpdateServerCertificate ca.SetExpirationTime failed for %s\n",server_name);
		regRes = 0; return false;
	}

	char* oldBuf;
	uint32_t sz;

	ca.IssueCertificate(oldBuf,sz,store_PEM_BUF);
	if(sz)
	{
		oldBuf = new char[sz+1];
		oldBuf[sz]=0;
		if(ca.IssueCertificate(oldBuf,sz,store_PEM_BUF)&& m_storage->SetServerCertificate(server_name,oldBuf))
		{
			out_cert = oldBuf;
			delete [] oldBuf;
			regRes = 1;
			dprint3("UpdateServerCertificate IssueCertificate success for %s\n", server_name);
			return true;
		}
		delete [] oldBuf;
		sz = 0;
	}
	dprint3("REGS: UpdateServerCertificate IssueCertificate failed for %s\n", server_name);
	regRes = 0;
	return false;
}

bool VS_RegistrationService::VerifyServerCert(const char *server_name, const char *server_cert, const bool use_new_cert)
{
	VS_GET_PEM_CACERT

	dprint3("REGS: VerifyServerCert for server %s\n",server_name);
	if(!server_name || !*server_name ||
		!server_cert || !*server_cert)
		return false;

	VS_Certificate	cert;
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::unique_ptr<char, free_deleter> CA_cert;
	int reg_cert_ln = rkey.GetValue(CA_cert, VS_REG_BINARY_VT, use_new_cert ? SRV_CERT_KEY : "CertificateOld");
	if(!CA_cert || reg_cert_ln<=0)
	{
		dprint0("CA Certificates is absent!!!\n");
		return false;
	}
	VS_CertificateCheck	certCheck;
	if(!cert.SetCert(server_cert,strlen(server_cert)+1, store_PEM_BUF)||
		!certCheck.SetCert(server_cert,strlen(server_cert)+1,store_PEM_BUF) ||
		!certCheck.SetCertToChain(PEM_CACERT,strlen(PEM_CACERT)+1,store_PEM_BUF)||
		!certCheck.SetCertToChain(CA_cert.get(), reg_cert_ln, store_PEM_BUF))
	{
		return false;
	}
	std::unique_ptr<void, free_deleter> certChain;
	int chainLen = rkey.GetValue(certChain,VS_REG_BINARY_VT, SRV_CERT_CHAIN_KEY);
	VS_Container	cert_chain;
	if (certChain && cert_chain.Deserialize(certChain.get(), chainLen))
	{
		cert_chain.Reset();
		if(cert_chain.IsValid())
		{
			while(cert_chain.Next())
			{
				if(!!cert_chain.GetName() && (0 == _stricmp(cert_chain.GetName(),CERTIFICATE_CHAIN_PARAM)))
				{
					size_t sz(0);
					const char *cert_in_chain = (const char*)cert_chain.GetBinValueRef(sz);
					if(sz && cert_in_chain)
						certCheck.SetCertToChain(cert_in_chain,sz,store_PEM_BUF);
				}
			}
		}
	}


	std::string descr;
	if(certCheck.VerifyCert(nullptr,&descr))
	{
		std::string buf;
		if(cert.GetSubjectEntry("commonName",buf) && !_stricmp(buf.c_str(),server_name))
		{
			dprint3("REGS: VerifyServerCert ok. Server %s\n",server_name);
			return true;
		}
		else
		{
			dprint3("REGS: VerifyServerCert failed. commonName and ServerName is not matched %s!=%s\n",server_name,buf.c_str());
			return false;
		}
	}
	else
	{
		dprint3("REGS: VerifyServerCert failed. Reason = %s\n", descr.c_str());
		return false;
	}
}

bool VS_RegistrationService::DecryptContentAlloc(const unsigned char *encr_buf, const unsigned long encr_sz, unsigned char *&buf_out, unsigned long &buf_out_sz,long &arm_hw_key)
{
	VS_PKey private_key;
	VS_PKeyCrypt	crypt;

	VS_Container	cnt;

	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::unique_ptr<char, free_deleter> CA_PrivateKey;
	rkey.GetValue(CA_PrivateKey, VS_REG_BINARY_VT, "CAPrivateKey");
	if(!CA_PrivateKey)
		return false;
	if (!private_key.SetPrivateKey(CA_PrivateKey.get(), store_PEM_BUF) || !crypt.SetPrivateKey(&private_key) || !cnt.Deserialize(encr_buf, encr_sz))
		return false;

	size_t sess_key_ln(0);
	const unsigned char *sess_key = (const unsigned char*)cnt.GetBinValueRef("key",sess_key_ln);
	size_t iv_ln(0);
	const unsigned char	*iv = (const unsigned char *)cnt.GetBinValueRef("iv",iv_ln);
	size_t encr_data_ln(0);
	const unsigned char * encr_data = (const unsigned char *)cnt.GetBinValueRef("data",encr_data_ln);
	unsigned char *decrdata(0);
	uint32_t decrlen(0);

	crypt.Decrypt(encr_data,encr_data_ln,alg_sym_AES256,iv,sess_key,sess_key_ln,decrdata,&decrlen);
	decrdata = new unsigned char [decrlen];
	if(!crypt.Decrypt(encr_data,encr_data_ln,alg_sym_AES256,iv,sess_key,sess_key_ln,decrdata,&decrlen))
	{
		delete[] decrdata;
		return false;
	}
	buf_out = decrdata;
	buf_out_sz = decrlen;

	decrdata = 0;
	decrlen = 0;
	VS_Container	reg_cnt;
	if(!reg_cnt.Deserialize(buf_out,buf_out_sz))
	{
		return true;
	}
	size_t arm_sz(0);
	VS_Container arm_cnt;
	const void * arm_buf = reg_cnt.GetBinValueRef("arm",arm_sz);
	if(!arm_buf || (!arm_cnt.Deserialize(arm_buf,arm_sz)))
		return true;
	sess_key_ln = 0;
	sess_key = (const unsigned char*)arm_cnt.GetBinValueRef("key",sess_key_ln);
	iv_ln = 0;
	iv = (const unsigned char *)arm_cnt.GetBinValueRef("iv",iv_ln);
	encr_data_ln = 0;
	encr_data = (const unsigned char *)arm_cnt.GetBinValueRef("data",encr_data_ln);

	crypt.Decrypt(encr_data,encr_data_ln,alg_sym_AES256,iv,sess_key,sess_key_ln,decrdata,&decrlen);
	decrdata = new unsigned char [decrlen];
	if(!crypt.Decrypt(encr_data,encr_data_ln,alg_sym_AES256,iv,sess_key,sess_key_ln,decrdata,&decrlen)||(decrlen!=sizeof(long)))
	{
		delete[] decrdata;
		return true;
	}
	arm_hw_key = *reinterpret_cast<const int32_t*>(decrdata);
	delete[] decrdata;
	return true;
}

bool VS_RegistrationService::RegisterServerFromFile_Method(VS_Container &cnt)
{
	bool res(false);
	size_t sz(0);
	const unsigned char * file_cnt = (const unsigned char*)cnt.GetBinValueRef("FileContent",sz);
	uint32_t decode_sz(0);
	unsigned char *decode_buf = (unsigned char*)VS_Base64DecodeAlloc((void*)file_cnt,sz,decode_sz);


	unsigned char *decr_buf(0);
	unsigned long decr_buf_sz(0);
	long arm_hw_key(0);
	if(!DecryptContentAlloc(decode_buf,decode_sz,decr_buf,decr_buf_sz,arm_hw_key))
	{
		free(decode_buf);
		VS_Container err_cnt;
		err_cnt.AddValue(METHOD_PARAM,REGISTERSERVEROFFLINE_METHOD);
		err_cnt.AddValueI32(RESULT_PARAM, 0);
		PostReply(err_cnt);
		return false;
	}
	free(decode_buf);


	VS_Container reg_cnt;
	if(!reg_cnt.Deserialize(decr_buf,decr_buf_sz))
	{
		delete [] decr_buf;
		VS_Container err_cnt;
		err_cnt.AddValue(METHOD_PARAM,REGISTERSERVEROFFLINE_METHOD);
		err_cnt.AddValueI32(RESULT_PARAM, 0);
		PostReply(err_cnt);
		return false;
	}
	delete [] decr_buf;
	decr_buf = 0;
	decr_buf_sz = 0;
	unsigned int armversion = atou_s(reg_cnt.GetStrValueRef("Version"));
	VS_Container rCnt;
	bool reg_ok = RegisterServer_Method(reg_cnt.GetStrValueRef(SERVERID_PARAM),
						reg_cnt.GetStrValueRef(SERVERNAME_PARAM),
						reg_cnt.GetStrValueRef(KEY_PARAM),
						reg_cnt.GetStrValueRef(PASSWORD_PARAM),
						reg_cnt.GetStrValueRef(CERT_REQUEST_PARAM),
						reg_cnt.GetStrValueRef("Srv Version"),0,
						reg_cnt.GetStrValueRef("OS Info"),
						reg_cnt.GetStrValueRef("CPU Info"),
						reg_cnt.GetStrValueRef(SERVER_VERSION_PARAM),
						true,rCnt);
	PostReply(rCnt);

	reg_cnt.AddValueI32("arm_hw_key", arm_hw_key);
	if(reg_ok)
		UpdateLicense_Method(reg_cnt.GetStrValueRef(SERVERNAME_PARAM), reg_cnt, armversion, true);
	return true;
}

void VS_RegistrationService::UpdateLicense_Method(const VS_SimpleStr& server_name,VS_Container& cnt, int arm_lic_version, bool IsOffline)
{

	VS_License::Type lic_type=VS_License::SIGNED_WITH_HW;
	dprint3("REGS: Update License from %s (pv%d ltype%d) \n",m_recvMess->SrcServer(),arm_lic_version,(int)lic_type);

	VS_Container reply;
	reply.AddValue(METHOD_PARAM, (IsOffline)? UPDATELICENSEOFFLINE_METHOD: UPDATELICENSE_METHOD);

	std::map<uint64_t, VS_License> licenses;

	if(m_demolicense)
		licenses.emplace(m_demolicense->m_id, *m_demolicense);

	if ( (m_storage->GetState(server_name)==m_storage->BS_VALID) || IsOffline )
	{
		reply.AddValueI32(RESULT_PARAM, 1);
		if(!m_storage->GetLicenses(server_name,licenses))
			return;
	} else {
		reply.AddValueI32(RESULT_PARAM, 0);
	}
	int32_t received_arm_hw_key(0);
	cnt.GetValue("arm_hw_key",received_arm_hw_key);
	unsigned long arm_hw_key(0); VS_SimpleStr arm_key; unsigned long hw_lock(0);
	if(!m_storage->GetArmHwKey(server_name, arm_hw_key, arm_key, hw_lock) || !arm_key)
	{
		arm_hw_key = received_arm_hw_key;
		dprint3("REGS: sn = %s; arm_hw_key = %ld\n", server_name.m_str, arm_hw_key);
		if(!arm_hw_key)	return;
	}
	if(received_arm_hw_key!=arm_hw_key)
	{
		dprint3("REGS: WARNING! Received arm hw key is not equal registered arm_hw_key! ServerName = %s, received_arm_hw = %d, registered_arm_hw_key = %ld\n", server_name.m_str, received_arm_hw_key, arm_hw_key);
	}

	cnt.Reset();

	uint64_t id;
	const void *pnt;
	size_t size;

	unsigned long licFeatures(0);
	while(cnt.Next())
	{
		if(_stricmp(cnt.GetName(),NAME_PARAM)==0)
		{
			pnt=cnt.GetBinValueRef(size);
			if (size == sizeof(uint64_t))
			{
				id = *static_cast<const uint64_t*>(pnt);

				const auto it = licenses.find(id);
				if (it == licenses.end())
				{
					reply.AddValue(NAME_PARAM, &id, sizeof(id));
					reply.AddValueI32(TYPE_PARAM, 1);
					dprint2("REGS: command to delete license %016I64X\n", id);
				}
				else
				{
					licFeatures |= GetArmTemplateIndex(&it->second, arm_lic_version);
					licenses.erase(it);
				}
			}
		}

	};
	md5_hw_assign hw_stick = {};
	std::vector<char> p_key;
	GetPrivateKeyForEncryption(server_name,p_key);

	for (const auto& kv : licenses)
	{
		const VS_License* lic = &kv.second;
		BYTE* buf;int size;

		if(VS_EncodeLicense(*lic,lic_type,&buf,&size,p_key))
		{
			licFeatures |= GetArmTemplateIndex(lic, arm_lic_version);
			memcpy(&hw_stick, lic->m_hw_md5, sizeof(hw_stick));
			reply.AddValue(NAME_PARAM, &lic->m_id, sizeof(lic->m_id));
			reply.AddValueI32(TYPE_PARAM, 2);
			reply.AddValue(DATA_PARAM,buf,size);
			dprint2("REGS: command to add license %016I64X\n", lic->m_id);
		} else {
			dprint1("REGS: could not encode license %016I64X for server %s\n", lic->m_id, server_name.m_str);
		}
	}

	if ((!arm_key /*|| !hw_lock*/) && licFeatures) {
		char arm_key_buffer[ARM_MAX_KEY_BUFFER_LEN] = {};
		dprint3("REGS: licFeatures = %ld;\n", licFeatures);
		MakeLicenseKey(arm_key_buffer, m_secures[licFeatures].c_str(), arm_hw_key, 0, hw_stick.ex1, hw_stick.ex2, hw_stick.ex3, hw_stick.ex4);
		dprint3("REGS: secures = %s;\n", m_secures[licFeatures].c_str());
		arm_key = arm_key_buffer;
		if(!m_storage->SetArmHwKey(server_name, arm_hw_key, arm_key)) return;
	}
	dprint3("REGS: sn = %s; arm_key = %s; licF = %ld\n", server_name.m_str, arm_key.m_str, licFeatures);

	VS_Container arm_cnt;
	arm_cnt.AddValue("",arm_key);
	arm_cnt.AddValueI32("f", licFeatures);

	VS_Sign	signer;
	VS_SignArg				signarg = {alg_pk_RSA,alg_hsh_SHA1};
	uint32_t				sign_size = VS_SIGN_SIZE;
	if(signer.Init(signarg))
	{
		  unsigned char sign_buf[256] = {0};
		  uint32_t		 sign_sz(256);
		  if(!p_key.empty()&&
			  signer.SetPrivateKey(&p_key[0],store_PEM_BUF)&&
			  signer.SignData((unsigned char*)arm_key.m_str,arm_key.Length(),sign_buf,&sign_size))
		  {
			  arm_cnt.AddValue(" ",sign_buf,sign_size);
		  }
	}
	VS_PKey	pub_key;
	VS_SimpleStr pk;
	VS_PKeyCrypt	crypt;
	VS_Container	encr_cnt;

	uint32_t vcs_ver(0);
	if( VS_ServCertInfoInterface::get_info_res::ok
			== m_storage->GetPublicKey(server_name,pk,vcs_ver)
		&& pub_key.SetPublicKey(pk,store_PEM_BUF) && crypt.SetPublicKey(&pub_key))
	{
		unsigned char *encrbuf(0);
		uint32_t encrlen(0);
		unsigned char iv[16] = {0};
		unsigned char *sesskey(0);
		uint32_t lensesskey(0);
		void *in_buf(0);
		size_t in_buf_sz(0);

		arm_cnt.SerializeAlloc(in_buf,in_buf_sz);

		crypt.Encrypt((const unsigned char*)in_buf,in_buf_sz,alg_sym_AES256,encrbuf,&encrlen,
			iv,sesskey,&lensesskey);
		sesskey = new unsigned char[lensesskey];
		crypt.Encrypt((const unsigned char*)in_buf,in_buf_sz,alg_sym_AES256,encrbuf,&encrlen,
			iv,sesskey,&lensesskey);
		encrbuf = new unsigned char[encrlen];
		if(!crypt.Encrypt((const unsigned char*)in_buf,in_buf_sz,alg_sym_AES256,encrbuf,&encrlen,
			iv,sesskey,&lensesskey))
		{
			free(in_buf);
			delete [] sesskey;
			delete [] encrbuf;
		}
		else
		{
			free(in_buf);
			in_buf = 0;
			in_buf_sz = 0;
			encr_cnt.AddValue("",(void*)sesskey,lensesskey);
			encr_cnt.AddValue("@",(void*)iv,16);
			encr_cnt.AddValue("&",encrbuf,encrlen);
			delete [] sesskey;
			delete [] encrbuf;
			reply.AddValue("ak", encr_cnt);
		}
	}
	PostReply(reply);
	return;
};

void VS_RegistrationService::GetPrivateKeyForEncryption(const char *server_name, std::vector<char> &p_key)
{
	/**
		1. GetCertificate
		2. Check Cert by old PrivateKey
		if(yes)
			return oldPkey
		else
			retruen new Pkey
	*/
	VS_SimpleStr cert;
	m_storage->GetServerCertificate(server_name,cert);
	bool is_new = VerifyServerCert(server_name,cert,true);
	std::unique_ptr<char, free_deleter> value;
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	int sz = rkey.GetValue(value, VS_REG_BINARY_VT, is_new ? SRV_PRIVATE_KEY : "PrivateKeyOld");
	if(sz>0)
		p_key.insert(p_key.end(), value.get(), value.get() + sz);
}

////////////////////////////////////////////////////////////////////////////////
// Notifications from Transport
////////////////////////////////////////////////////////////////////////////////

bool VS_RegistrationService::OnPointConnected_Event( const VS_PointParams* prm)
{
	m_storage->SetState(prm->uid,m_storage->BS_VALID);
	m_storage->LogEvent(BE_CONNECT,prm->uid);
	return true;
}

bool VS_RegistrationService::OnPointDisconnected_Event( const VS_PointParams* prm )
{
	m_storage->SetState(prm->uid,m_storage->BS_DISCONNECTED);
	m_storage->LogEvent(BE_DISCONNECT,prm->uid,0,0,0,m_storage->GetState(prm->uid));
	return true;
}

bool VS_RegistrationService::OnPointDeterminedIP_Event( const char* uid, const char* ip)
{
	if (!uid || !*uid || _stricmp(uid, OurEndpoint()) == 0)
		return true;

	m_storage->LogIP(uid, ip);

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, POINTDETERMINEDIP_METHOD);
	cnt.AddValue(USERNAME_PARAM, uid);
	cnt.AddValue(IPCONFIG_PARAM, ip);
	PostRequest(OurEndpoint(), 0, cnt);
	return true;
}

bool VS_RegistrationService::PointDeterminedIP_Handler(const char* uid, const char* ip)
{
	VS_Container cnt;

	cnt.AddValue(METHOD_PARAM, EXTERNAL_IP_DETERMINED_METHOD);
	cnt.AddValue(IPCONFIG_PARAM, ip);

	PostRequest(uid, 0, cnt, 0, CHECKLIC_SRV);
	return true;
}

unsigned long VS_RegistrationService::GetArmTemplateIndex(const VS_License* lic, int arm_lic_version)
{
	unsigned long result(0);
	// SECUREBEGIN
	// |= 1

	if (!lic->m_trial_conf_minutes) {
		// server is not trial - SECUREBEGIN_A_FULL (allow stoarge and planned invites)
		result |= 2;
		if (arm_lic_version < 331) { // for servers prior 3.3.1
			result |= 64; // it was planned invites
		}
	}

	// allow guest login - SECUREBEGIN_B_GUEST
	if (lic->m_max_guests) result |= 4;

	for (int i = 0; i < (sizeof(lic->m_restrict) << 3); ++i) {
		auto flag = (1i64 << i);
		if (lic->m_restrict & flag) {
			switch (flag)
			{
			case lic->LDAP_ALLOWED:
				// allow ldap login - SECUREBEGIN_C_LDAP
				result |= 8;
				break;
			case lic->ROLECONF_ALLOWED:
				// allow role conferences - SECUREBEGIN_D_ROLE
				result |= 16;
				break;
			case lic->MULTIGATEWAY_ALLOWED:
				// allow multigateway - SECUREBEGIN_F_MGATEWAY
				result |= 64;
				break;
			//case lic->WEBRTC_BROADCAST_ALLOWED:
			//	// allow webrtc broadcast - SECUREBEGIN_G_WEBERTC
			//	result |= 128;
			//	break;
			default:
				break;
			}
			if (arm_lic_version >= 440)	{
				if (flag == lic->ENTERPRISE_MASTER ||		// allow license sharing - SECUREBEGIN_E_ENTERPRISE
					flag == lic->ENTERPRISE_SLAVE)
					result |= 32;
			} else {		// tcs version prior to tcs440
				if (flag == lic->UDPMULTICAST_ALLOWED)
					result |= 32;
			}
		}
	}

	//SECUREBEGIN_H
	// |= 256
	//SECUREBEGIN_J
	// |= 512
	//SECUREBEGIN_K
	// |= 1024
	return result;
}

/* !!! DDNS !!! */

void VS_RegistrationService::HandleMessage(const boost::shared_ptr<VS_MessageData> &message)
{
	unsigned long type = 0, len = 0;
	std::string srv_domain_name;
	char c;
	void *data = message->GetMessPointer(type, len);

	if (type != MSG_DDNS_UPDATE)
		return;

	auto msg = boost::static_pointer_cast<DDNSUpdateMessageData>(message);

	for (size_t i = 0; (c = (msg->GetSrvName().at(i))) != '#'; i++)
	{
		srv_domain_name.push_back(c);
	}

	DoDDNSUpdate(msg->GetConnData(), srv_domain_name);
}

void VS_RegistrationService::HandleConnectionData(const char *conn_data, const char *srv_name)
{
	if (conn_data == NULL || srv_name == NULL)
		return;

	boost::shared_ptr<DDNSUpdateMessageData> ddns_update_msg(new DDNSUpdateMessageData(conn_data, srv_name));
	//auto str = ddns_update_msg->GetConnData();
	Post(boost::static_pointer_cast<VS_MessageHandler>(shared_from_this()), boost::static_pointer_cast<VS_MessageData>(ddns_update_msg));
}

void VS_RegistrationService::UpdateConfiguration_Method(const char *conn_data, const char *srv_name)
{
	HandleConnectionData(conn_data, srv_name);
}

void VS_RegistrationService::Post(const boost::shared_ptr<VS_MessageHandler> &h, const boost::shared_ptr<VS_MessageData> &data)
{
	m_worker->Post(h, data);
}

void VS_RegistrationService::DoDDNSUpdate(std::string &conn_data, std::string &srv_dns_name)
{
	AddrSet gaddrs; // global IP addresses

	ConnectionStringParser conn_parser(conn_data);
	if (!conn_parser.Parse()) // bad connection string
		return;

	// find all global IPv4 addresses
	for (size_t i = 0; i < conn_parser.GetCount(); i++)
	{
		auto &a = conn_parser.GetAddr(i);

		if (!IsPrivateAddress_IPv4(a.first.c_str()))
		{
			dprint0("Global IP address has been found: %s:%s\n", a.first.c_str(), a.second.c_str());
			gaddrs.push_back(a);
		}
	}

	for (auto &v : m_ddns_servers)
	{
		VS_SimpleStr domain = v->GetDNSDomain();
		// check if we need to do DNS updating for this server
		if (IsSubdomain(srv_dns_name.c_str(), domain.m_str))
		{
			if (gaddrs.size() > 0)
				dprint0("Performing DNS entries update for \"%s\"...\n", srv_dns_name.c_str());
			UpdateDNSServerRecords(srv_dns_name, v.get(), gaddrs);
		}
	}
}

/* DDNS Update Message */
VS_RegistrationService::DDNSUpdateMessageData::DDNSUpdateMessageData(const char *conn_data, const char *srv_name)
	: VS_MessageData(MSG_DDNS_UPDATE, NULL, 0), m_conn_data(conn_data), m_srv_name(srv_name)
{}

VS_RegistrationService::DDNSUpdateMessageData::~DDNSUpdateMessageData()
{}

std::string &VS_RegistrationService::DDNSUpdateMessageData::GetConnData(void)
{
	return m_conn_data;
}

std::string &VS_RegistrationService::DDNSUpdateMessageData::GetSrvName(void)
{
	return m_srv_name;
}

bool VS_RegistrationService::UpdateCallCfgCorrector_Method(const char* server_name, const char *hash, VS_Container &cnt)
{
	VS_SimpleStr data;

	if (m_storage->GetConfigCorrectorData(server_name, data))
	{
		char md5_buf[32 + 1] = { 0 };
		VS_ConvertToMD5(SimpleStrToStringView(data), md5_buf);

		if (strlen(hash) == 32 && _stricmp(hash, (char *)md5_buf) == 0)
		{
			return false;
		}

		cnt.AddValue(SCRIPT_PARAM, data);
	}

	return true;
}

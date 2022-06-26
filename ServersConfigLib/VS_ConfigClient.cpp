/*****************************************************************************
 * (c) 2004 Visicron Inc.  http://www.visicron.net/
 *
 * Project: Broker Services - Media streaming
 *
 * Created: SMirnovK 9 Jun 2004
 *
 * $History: VS_ConfigClient.cpp $
 *
 * *****************  Version 49  *****************
 * User: Smirnov      Date: 16.08.12   Time: 21:36
 * Updated in $/VSNA/Servers/ServersConfigLib
 * -crypt mgateway
 * - arm futures ver added
 *
 * *****************  Version 48  *****************
 * User: Mushakov     Date: 17.07.12   Time: 23:09
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - LoginConfigurator() was removed
 * - messages from configurator are handled by SessionID
 * - fix TransportMessage::IsFromServer()
 *
 * *****************  Version 47  *****************
 * User: Ktrushnikov  Date: 28.06.12   Time: 12:09
 * Updated in $/VSNA/Servers/ServersConfigLib
 * #12479c4: VS_LDAPCore::Init at UpdateRegistry
 *
 * *****************  Version 46  *****************
 * User: Ktrushnikov  Date: 22.06.12   Time: 19:24
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - VS_LDAPCore: cache of users updates only by timeout (AB Refresh) OR
 * from configurator Refresh button
 * - quick logout
 * - UpdateAddressBook called at OnUserChange(type=4)
 *
 * *****************  Version 45  *****************
 * User: Ktrushnikov  Date: 15.06.12   Time: 12:09
 * Updated in $/VSNA/Servers/ServersConfigLib
 * new ldap:
 * - VS_LDAPCore split for tc_server.exe & tc_conf.dll
 *
 * *****************  Version 44  *****************
 * User: Mushakov     Date: 16.03.12   Time: 15:54
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - Add OS and CPU info in Regiatration Server
 *
 * *****************  Version 43  *****************
 * User: Mushakov     Date: 5.10.11    Time: 21:36
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - ssl refactoring (SetCert interfaces)
 *
 * *****************  Version 42  *****************
 * User: Mushakov     Date: 15.06.11   Time: 18:42
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - Get license in offline registration
 *
 * *****************  Version 41  *****************
 * User: Mushakov     Date: 24.05.11   Time: 21:41
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - offline reg
 *
 * *****************  Version 40  *****************
 * User: Mushakov     Date: 24.05.11   Time: 20:52
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - Reg Offline
 *
 * *****************  Version 39  *****************
 * User: Mushakov     Date: 11.05.11   Time: 3:49
 * Updated in $/VSNA/Servers/ServersConfigLib
 *
 * *****************  Version 38  *****************
 * User: Mushakov     Date: 10.05.11   Time: 20:37
 * Updated in $/VSNA/Servers/ServersConfigLib
 *
 * *****************  Version 37  *****************
 * User: Mushakov     Date: 20.04.11   Time: 19:46
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - DeleteConf from manager
 *
 * *****************  Version 36  *****************
 * User: Mushakov     Date: 15.03.11   Time: 16:10
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - 8553 automatic updated user info supported
 *
 * *****************  Version 35  *****************
 * User: Mushakov     Date: 2.03.11    Time: 17:43
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - roaming
 *
 * *****************  Version 34  *****************
 * User: Mushakov     Date: 15.02.11   Time: 13:56
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - registration case "no valid lic" handled
 *
 * *****************  Version 33  *****************
 * User: Mushakov     Date: 15.12.10   Time: 21:39
 * Updated in $/VSNA/Servers/ServersConfigLib
 * 8276
 *
 * *****************  Version 32  *****************
 * User: Ktrushnikov  Date: 14.12.10   Time: 12:21
 * Updated in $/VSNA/Servers/ServersConfigLib
 *
 * *****************  Version 31  *****************
 * User: Mushakov     Date: 10.12.10   Time: 14:00
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - fclose offline2.vrg
 *
 * *****************  Version 30  *****************
 * User: Mushakov     Date: 8.12.10    Time: 21:27
 * Updated in $/VSNA/Servers/ServersConfigLib
 * 8241
 *
 * *****************  Version 29  *****************
 * User: Mushakov     Date: 3-12-10    Time: 16:30
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - VSGetServerIdentifyString added (7205)
 *  - sign/verify sign code in secure lib  was little corrected
 *  - SignVerifier added
 *
 * *****************  Version 28  *****************
 * User: Mushakov     Date: 3.11.10    Time: 20:46
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - VCS manager authorization added
 * - VCS Config dead lock fixed
 *
 * *****************  Version 27  *****************
 * User: Ktrushnikov  Date: 2.11.10    Time: 10:42
 * Updated in $/VSNA/Servers/ServersConfigLib
 * #3569:
 * - vs_bc.dll: send to right place; send call_id with server_name
 * - VS_RegistryStorage: send to client UPDATEACCOUNT_METHOD when
 * OnUserChange() with type=1
 * - VS_RegistryStorage: FetchRights at Read()
 * #7454:
 * - Sleep() removed
 *
 * *****************  Version 26  *****************
 * User: Mushakov     Date: 1.11.10    Time: 21:00
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - cert update added
 * - registration from server added
 * - authorization servcer added
 *
 * *****************  Version 25  *****************
 * User: Mushakov     Date: 10.09.10   Time: 20:24
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - Registration on SM added
 *
 * *****************  Version 24  *****************
 * User: Ktrushnikov  Date: 22.04.10   Time: 15:15
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - SendInvites() and SetLayout(): call TrySetCurrentConnectionToHKCU()
 * before sending transport message
 *
 * *****************  Version 23  *****************
 * User: Mushakov     Date: 7.04.10    Time: 20:59
 * Updated in $/VSNA/Servers/ServersConfigLib
 *
 * *****************  Version 22  *****************
 * User: Mushakov     Date: 7.04.10    Time: 20:34
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - offline registration supported
 * - arm_key encrypted
 *
 * *****************  Version 21  *****************
 * User: Ktrushnikov  Date: 17.03.10   Time: 13:42
 * Updated in $/VSNA/Servers/ServersConfigLib
 * Create HKCU ConnectTCP* from CurrentConnect for vs_bc.dll
 * VS_SetLocalENameInService
 * - useless check for endpointName removed
 *
 * *****************  Version 20  *****************
 * User: Ktrushnikov  Date: 23.02.10   Time: 14:40
 * Updated in $/VSNA/Servers/ServersConfigLib
 * VCS:
 * - Services added: SMTP_MAILER, LOCATOR, LOG (two last to make work
 * smtp_mailer)
 * - OnUserChange(): changes come from configurator vs_bc.dll
 * vs_bc.dll:
 * - fix params (broker) where to send updates
 *
 * *****************  Version 19  *****************
 * User: Mushakov     Date: 28.01.10   Time: 19:53
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - offline registration supported (VCS)
 *
 * *****************  Version 18  *****************
 * User: Ktrushnikov  Date: 28.01.10   Time: 17:40
 * Updated in $/VSNA/Servers/ServersConfigLib
 * vs_bc.dll:
 * - send to conf_srv (not to multiconf_srv)
 * - InviteUsers method
 *
 * *****************  Version 17  *****************
 * User: Mushakov     Date: 18.12.09   Time: 18:04
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - Removed VCS_BUILD somewhere
 * - Add new field to license
 * - Chat service for bsServer renamed
 *
 * *****************  Version 16  *****************
 * User: Mushakov     Date: 16.11.09   Time: 15:58
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - new Certificate
 *  - added error code in Cert Verify
 *
 * *****************  Version 15  *****************
 * User: Ktrushnikov  Date: 13.11.09   Time: 17:56
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - SERVER_VERSION renamed to VCS_VERSION
 *
 * *****************  Version 14  *****************
 * User: Mushakov     Date: 6.11.09    Time: 18:09
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - version added
 * - Check commonName added
 * - Check ServerVersion added
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 28.10.09   Time: 17:59
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - vs_ep_id removed
 * - registration corrected
 *
 * *****************  Version 12  *****************
 * User: Mushakov     Date: 23.10.09   Time: 15:05
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - VCS 3
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 7.08.09    Time: 15:14
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 2.04.08    Time: 19:56
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - A lot of CID's geniration fixed
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 25.03.08   Time: 17:51
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - SSL added
 *  - fixed bug: Connect to server with another name
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 20.02.08   Time: 19:33
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - hardware key
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 6.02.08    Time: 15:54
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - client transport cleaning
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 28.01.08   Time: 21:29
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - NA principles
 *
 * *****************  Version 5  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:01
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 27.11.07   Time: 18:54
 * Updated in $/VSNA/Servers/ServersConfigLib
 *
 * *****************  Version 3  *****************
 * User: Dront78      Date: 21.11.07   Time: 19:57
 * Updated in $/VSNA/Servers/ServersConfigLib
 * Some build errors fixed.
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 20.11.07   Time: 16:13
 * Updated in $/VSNA/Servers/ServersConfigLib
 * renamed services
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:50
 * Created in $/VSNA/Servers/ServersConfigLib
 * *****************  Version 8  *****************
 * User: Dront78      Date: 5.12.07    Time: 15:03
 * Updated in $/VS2005/Servers/ServersConfigLib
 * Removed unused PASSWORD_PARAM from ONADDRESSBOOKCHANGE_METHOD
 *
 * *****************  Version 7  *****************
 * User: Ktrushnikov  Date: 25.11.07   Time: 19:03
 * Updated in $/VS2005/Servers/ServersConfigLib
 * - bug fix: save XML-file name in class variable
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 25.11.07   Time: 13:47
 * Updated in $/VS2005/Servers/ServersConfigLib
 * - Offline Registration added
 *
 * *****************  Version 5  *****************
 * User: Avlaskin     Date: 22.08.07   Time: 12:23
 * Updated in $/VS2005/Servers/ServersConfigLib
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 11.07.07   Time: 19:58
 * Updated in $/VS2005/Servers/ServersConfigLib
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 22.03.07   Time: 16:18
 * Updated in $/VS2005/Servers/ServersConfigLib
 * добавлена верификация ProductType при регистрации брокера
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 5.02.07    Time: 21:36
 * Updated in $/VS2005/Servers/ServersConfigLib
 * - project configuration
 * - depricated functions warnings suppressed
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/ServersConfigLib
 *
 * *****************  Version 12  *****************
 * User: Mushakov     Date: 16.06.06   Time: 16:50
 * Updated in $/VS/Servers/ServersConfigLib
 * Added verification log in ServerConfigDLL (debug version only)
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 9.06.06    Time: 13:57
 * Updated in $/VS/Servers/ServersConfigLib
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 1.06.06    Time: 18:12
 * Updated in $/VS/Servers/ServersConfigLib
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 31.05.06   Time: 20:57
 * Updated in $/VS/Servers/ServersConfigLib
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 31.05.06   Time: 20:44
 * Updated in $/VS/Servers/ServersConfigLib
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 27.04.06   Time: 13:10
 * Updated in $/VS/Servers/ServersConfigLib
 * updated manage of certificate
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 26.04.06   Time: 16:33
 * Updated in $/VS/Servers/ServersConfigLib
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 25.04.06   Time: 16:26
 * Updated in $/VS/Servers/ServersConfigLib
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 25.04.06   Time: 15:54
 * Updated in $/VS/Servers/ServersConfigLib
 * Added certificate issue
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 15.03.06   Time: 14:00
 * Updated in $/VS/Servers/ServersConfigLib
 * - added parametr broker to netclient functions
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 27.06.05   Time: 19:18
 * Updated in $/VS/Servers/ServersConfigLib
 * gateway registration repaired
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 5.03.04    Time: 11:56
 * Created in $/VS/Servers/ServersConfigLib
 * removed all config dlls, now it common
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 20.01.04   Time: 20:51
 * Updated in $/VS/Servers/SBSBroker/SBSBrokerLib
 * moved to "user" in registry
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 16.01.04   Time: 17:38
 * Updated in $/VS/Servers/SBSBroker/SBSBrokerLib
 * corrected sbs config
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 13.01.04   Time: 18:48
 * Updated in $/VS/Servers/SBSBroker/SBSBrokerLib
 * added read of session
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 13.01.04   Time: 13:10
 * Updated in $/VS/Servers/SBSBroker/SBSBrokerLib
 * service name is corrected
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 9.01.04    Time: 20:24
 * Updated in $/VS/Servers/SBSBroker/SBSBrokerLib
 * added check for valid
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 9.01.04    Time: 18:25
 * Updated in $/VS/Servers/SBSBroker/SBSBrokerLib
 * define messages, comment
 *
 * Updated in $/VS/Servers/SBSBroker/SBSBrokerLib
 * define messages, comment
 *
 ****************************************************************************/
/*****************************************************************************
 * \file	VS_CofigClient.h
 * \brief	Send/receive transport messages
 ****************************************************************************/

/*****************************************************************************
 * Includes
 ****************************************************************************/

#include "VS_ConfigClient.h"
#include "ServersConfigLib/VS_ServersConfigLib.h"

#include "acs/ConnectionManager/VS_ConnectionManager.h"
#include "acs/Lib/VS_AcsHttpLib.h"
#include "acs/Lib/VS_AcsLib.h"
#include "acs/Lib/VS_AcsLog.h"
#include "LicenseLib/VS_License.h"
#include "net/EndpointRegistry.h"
#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_CertificateIssue.h"
#include "SecureLib/VS_CryptoInit.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "SecureLib/VS_SecureConstants.h"
#include "SecureLib/VS_Sign.h"
#include "SecureLib/VS_UtilsLib.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std/CallLog/VS_ConferenceDescription.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/VS_Endpoint.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_UserData.h"
#include "std/cpplib/VS_Utils.h"
#include "std/RegistrationStatus.h"
#include "std/VS_RegServer.h"
#include "tools/Server/VS_Server.h"

#include <openssl/x509_vfy.h>

#include <memory>


//const char PRIVATE_KEY_PASS[]					= "P[sDubdfgPLk|d)d#4cmdfa658HhdsflvFe5^&*Xl";

/*****************************************************************************
 * Implementation
 ****************************************************************************/

DWORD WINAPI ThreadProc(void* param)
{
	vs::SetThreadName("ConfigClient");
	return static_cast<VS_ConfigClient*>(param)->ThreadCycle();
}

/*****************************************************************************
 * Constructor
 ****************************************************************************/
//long VS_ConfigClient::m_ProductID = 0;

VS_ConfigClient::VS_ConfigClient()
{
	vs::InitOpenSSL();
	m_ThreadId = 0;
	m_hThread = 0;
	m_hwnd = 0;
	m_hDie = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hLDAPUpdateFinished = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_Valid = false;
}

/*****************************************************************************
 * Destructor
 ****************************************************************************/
VS_ConfigClient::~VS_ConfigClient()
{
	Release();
	if (m_hDie) CloseHandle(m_hDie);
	if (m_hLDAPUpdateFinished) CloseHandle(m_hLDAPUpdateFinished);
}

/*****************************************************************************
 * Init before use
 ****************************************************************************/
bool VS_ConfigClient::Init(HWND hwnd)
{
	if (m_Valid)
		return false;

	if (!hwnd)
		return false;

	char buff[256] = {0};

	VS_UninstallTransportClient();
	VS_UninstallConnectionManager();

	//VS_GenerateTempEndpoint(buff);
	/**
		сгенерить имя и пароль для конфигуратора, и попытаться залогиниться
	**/

	VS_GenKeyByMD5(buff);
	m_Ep = buff;
	*buff = 0;


	net::endpoint::ClearAllConnectTCP(RegServerName);
	net::endpoint::AddConnectTCP({ RegServerHost, RegServerPort, RegServerProtocol }, RegServerName);
	/**
		msk3.pca.ru#sm
	*/

	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	rkey.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, "Server Manager");
	m_RegEp = buff;

	*buff = 0;


	m_RegEp = RegServerName;

	m_hwnd = hwnd;
	m_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, &m_ThreadId);
	if (!m_hThread)
		return false;
	if ( !VS_AcsLibInitial())
		return false;
	if ( !VS_InstallConnectionManager(m_Ep))
		return false;
	if ( !VS_InstallTransportClient( this ) )
		return false;
	if ( !VS_RegisterService(CLIENT_SRV, m_ThreadId, WM_SERVICE_CC) )
		return false;

	VS_RegistryKey configKey(false, CONFIGURATION_KEY, false, true);
	m_Valid = true;
	TrySetCurrentConnectionToHKCU();
	Sleep(0);		// Fix: ktrushnikov: Win2008 64-bit (#8156)

	const unsigned int sz = 256;
	char server[sz];
	if ( VS_GetLocalEName(server, sz) )
		VS_CreateConnect(server);

	return true;
}
void VS_ConfigClient::SetApplication(int num)
{
	//m_ProductID = num;
}

/*****************************************************************************
 * Process all messages
 ****************************************************************************/
void VS_ConfigClient::Servises(MSG *msg)
{
	switch (msg->message)
	{
	case WM_SERVICE_CC:
		ConfigClientSrv(msg);
		break;
	default:
		break; /// all other Widows messages
	}
}


/*****************************************************************************
 * main client thread recieving transport messages
 ****************************************************************************/
DWORD VS_ConfigClient::ThreadCycle()
{
    MSG msg;
	DWORD dwObj;
	// force to create message queue
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	while(TRUE) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	{
			Servises(&msg);			// check for unread messages
			if (msg.message == WM_CC_DESTROY) {
				VS_UninstallTransportClient();
				VS_UninstallConnectionManager();
				SetEvent(m_hDie);
				return NOERROR;
			}
		}
		dwObj = MsgWaitForMultipleObjects(0, NULL, FALSE, 1000, QS_ALLINPUT);
	}
	return -255;
}

/*****************************************************************************
 * Call it to release resources
 ****************************************************************************/
void VS_ConfigClient::Release()
{
	if (m_ThreadId) {
		PostThreadMessage(m_ThreadId, WM_CC_DESTROY ,0, 0);
		WaitForSingleObject(m_hDie, 10000);
		if (m_hThread) CloseHandle(m_hThread); 	m_hThread = 0;
		m_ThreadId = 0;
	}
	m_Valid = false;
}

/*****************************************************************************
 * Config Client Service (CLIENT_SRV)
 ****************************************************************************/
void VS_ConfigClient::ConfigClientSrv(MSG* msg)
{
	VS_Container cnt;
	VS_ClientMessage tMsg(msg);
	VS_AcsLog	*verify_log(0);
	VS_GET_PEM_CACERT
#ifdef _DEBUG
	verify_log = new VS_AcsLog("verify", 5000000, 1000000,"./");
#else
	verify_log = new VS_AcsEmptyLog();
#endif

	switch(tMsg.Type())
	{
	case transport::MessageType::Reply:
		if (cnt.Deserialize(tMsg.Body(), tMsg.BodySize())) {
			const char *method = cnt.GetStrValueRef(METHOD_PARAM);
			if (method && _stricmp(method, REGISTERSERVER_METHOD)==0) {
				int32_t result = false;
				cnt.GetValue(RESULT_PARAM, result);
				if (result == 1) {
					VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
					rkey.SetString(cnt.GetStrValueRef(KEY_PARAM), "Key");
					verify_log->TPrintf("-------------------new verification----------------");

					const char *cert = cnt.GetStrValueRef(CERTIFICATE_PARAM);
					const char *sm_cert = cnt.GetStrValueRef(SM_CERTIFICATE_PARAM);
					if(cert)
					{
						VS_CertificateCheck	certCheck;
						certCheck.SetCert(cert,strlen(cert)+1,store_PEM_BUF);
						certCheck.SetCertToChain(PEM_CACERT,strlen(PEM_CACERT)+1, store_PEM_BUF);
						if(sm_cert)
							certCheck.SetCertToChain(sm_cert,strlen(sm_cert) +1,store_PEM_BUF);


#ifdef _DEBUG

						FILE *fp = fopen("certificate.pem","wb");
						if(fp)
						{
							fwrite(cert,strlen(cert)+1,1,fp);
							fclose(fp);
						}
#endif
						int verify_err_code(0);
						std::string err_str;

						if(certCheck.VerifyCert(&verify_err_code, &err_str))
						{
							verify_log->TPrintf("Certificate verification is ok");
							rkey.SetValue(cert, (unsigned long)strlen(cert) + 1, VS_REG_BINARY_VT, SRV_CERT_KEY);
							rkey.SetValue(m_PrivateKey.m_str, m_PrivateKey.Length() + 1, VS_REG_BINARY_VT, SRV_PRIVATE_KEY);
							VS_Container cert_chain_cnt;
							if(sm_cert)
								cert_chain_cnt.AddValue(CERTIFICATE_CHAIN_PARAM,(void*)sm_cert,(unsigned long)strlen(sm_cert) + 1);

							void *buf_cnt(0);
							size_t buf_sz(0);

							cert_chain_cnt.SerializeAlloc(buf_cnt,buf_sz);

							rkey.SetValue(buf_cnt, buf_sz,VS_REG_BINARY_VT, SRV_CERT_CHAIN_KEY);
							if (buf_cnt)
								free(buf_cnt);
							PostNotify(VS_CC_BROKERREG_OK);
						}
						else
						{
							VS_Certificate	failed_cert;
							if(!failed_cert.SetCert(cert,strlen(cert)+1,store_PEM_BUF))
								verify_log->TPrintf("Data format failed");
							else
							{
								char buf[24];
								auto curr_time = std::chrono::system_clock::now();
								tu::TimeToLStr(curr_time, buf, 24);
								VS_SimpleStr	str;
								verify_log->TPrintf("Current time = %s\n",buf);
								PrepareCertDataToLog(&failed_cert,str);
								verify_log->TPrintf(str);
							}
							verify_log->TPrintf("Certificate verification is failed. %s", err_str.c_str());
							if (verify_err_code == X509_V_ERR_CERT_NOT_YET_VALID)
								PostNotify(VS_CC_VERIFICATION_ERR_CERT_NOT_YET_VALID);
							else if (verify_err_code == X509_V_ERR_CERT_HAS_EXPIRED)
								PostNotify(VS_CC_VERIFICATION_ERR_CERT_HAS_EXPIRED);
							else if (verify_err_code == X509_V_ERR_CERT_SIGNATURE_FAILURE || verify_err_code == X509_V_ERR_CRL_SIGNATURE_FAILURE)
								PostNotify(VS_CC_VERIFICATION_ERR_SIGNATURE_FAILED);
							else
								PostNotify(VS_CC_VERIFICATION_NEW_CERT_FAILED);
						}
					}
					else
						PostNotify(VS_CC_BROKERREG_FAIL);
				}
				else if(result == 2)
					PostNotify(VS_CC_BROKERREG_LOCKED);
				else if(result == 3)
					PostNotify(VS_CC_SERVER_NAME_USED);
				else {
					PostNotify(VS_CC_BROKERREG_FAIL);
				}
			}else if (method && _stricmp(method, UPDATELICENSEOFFLINE_METHOD)==0) {
				this->OnUpdateLicenceOfflineReply(cnt);

			}else if (method && _stricmp(method, REGISTERSERVEROFFLINE_METHOD)==0) {
				this->OnRegisterBrokerOfflineReply(cnt);
			}/*else if (method && _stricmp(method, "LDAPUpdatedEvent")==0) {
				SetEvent(m_hLDAPUpdateFinished);
			}*/
		}
		break;
	case transport::MessageType::Notify:
		if (strcmp(tMsg.DstService(), REGISTRATION_SRV)==0)
			PostNotify(VS_CC_NOREGCONNECT);
		break;
	default:
		break;
	}
	verify_log->TPrintf("-------------------end verification----------------");
	delete verify_log;
}

void VS_ConfigClient::PrepareCertDataToLog(VS_Certificate *cert, VS_SimpleStr &str)
{
	if(!cert)
		str = "Certificate is null";
	else
	{
		std::string buf;
		std::string notBeforeBuf, notAfterBuf;
		if(cert->GetSubjectEntry("commonName",buf))
		{
			str += "CommonName = ";
			str += buf.c_str();
			str += ";\n";
		}
		if(cert->GetSubjectEntry("countryName",buf))
		{
			str += "CountreName = ";
			str += buf.c_str();
			str += ";\n";
		}
		if(cert->GetSubjectEntry("organizationName",buf))
		{
			str += "organization = ";
			str += buf.c_str();
			str += ";\n";
		}
		if(cert->GetSubjectEntry("surname",buf))
		{
			str += "contact person = ";
			str += buf.c_str();
			str += ";\n";
		}
		if(cert->GetSubjectEntry("emailAddress",buf))
		{
			str += "Email = ";
			str += buf.c_str();
			str += ";\n";
		}
		if(cert->GetExpirationTime(notBeforeBuf, notAfterBuf))
		{
			str += "Not Before = ";
			str += notBeforeBuf.c_str();
			str += "; Not After = ";
			str += notAfterBuf.c_str();
			str +=";\n";
		}
	}
}
/*****************************************************************************
 * Send message to external hwnd
 ****************************************************************************/
void VS_ConfigClient::PostNotify(int notify)
{
	if (m_hwnd) PostMessage(m_hwnd, WM_CC_NOTIFY, notify, 0);
}


/*****************************************************************************
 * Request to register broker
 ****************************************************************************/
bool VS_ConfigClient::ReqLicence(const char* server_id,const char *server_name, const char* serial, const char *organization_name,
					const char *country, const char *contact_person, const char *contact_email, const char* to_file)
{
	static const char VS_SERVER_START_MODE[] = "Start Mode";
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	if (!serial) return false;

	if(!to_file)
	{
		long mode(1);
		rkey.RemoveValue("RegResult");
		if (!rkey.SetValue(&mode, sizeof(mode), VS_REG_INTEGER_VT, VS_SERVER_START_MODE)
		 || !rkey.SetString(server_name, "Server Name")
		 || !rkey.SetString(serial, "Serial")
		 || !rkey.SetString(server_id, "SrvID"))
		{
			rkey.RemoveValue(VS_SERVER_START_MODE);
			rkey.RemoveValue("Server Name");
			rkey.RemoveValue("Serial");
			rkey.RemoveValue("RegResult");
			rkey.RemoveValue("SrvID");
			PostNotify(VS_CC_REGISTRY_WRITE_FAILED);
			return true;

		}
		if((7!=VS_GetServiceState())&&(!VS_StopService()))
		{
			rkey.RemoveValue(VS_SERVER_START_MODE);
			rkey.RemoveValue("Server Name");
			rkey.RemoveValue("Serial");
			rkey.RemoveValue("RegResult");
			rkey.RemoveValue("SrvID");
			PostNotify(VS_CC_STOP_SERVICE_FAILED);
			return true;
		}
		long i =0;
		while(7!=VS_GetServiceState()&&i<120)
		{
			Sleep(1000);
			i++;
		}
		if(7!=VS_GetServiceState())
		{
			rkey.RemoveValue(VS_SERVER_START_MODE);
			rkey.RemoveValue("Server Name");
			rkey.RemoveValue("Serial");
			rkey.RemoveValue("RegResult");
			rkey.RemoveValue("SrvID");
			PostNotify(VS_CC_STOP_SERVICE_FAILED);
			return true;
		}
		if(!VS_StartService())
		{
			rkey.RemoveValue(VS_SERVER_START_MODE);
			rkey.RemoveValue("Server Name");
			rkey.RemoveValue("Serial");
			rkey.RemoveValue("RegResult");
			rkey.RemoveValue("SrvID");
			PostNotify(VS_CC_START_SERVICE_FAILED);
			return true;
		}
		while(7!=VS_GetServiceState())
			Sleep(1000);
		RegStatus reg_result(RegStatus::failed);
		if(!rkey.GetValue(&reg_result,sizeof(reg_result),VS_REG_INTEGER_VT,"RegResult"))
		{
			rkey.RemoveValue(VS_SERVER_START_MODE);
			rkey.RemoveValue("Server Name");
			rkey.RemoveValue("Serial");
			rkey.RemoveValue("RegResult");
			rkey.RemoveValue("SrvID");
			PostNotify(VS_CC_RESULT_CODE_EMPTY);
			return true;
		}
		switch(reg_result)
		{
		case RegStatus::succeeded:
			PostNotify(VS_CC_BROKERREG_OK);
			break;
		case RegStatus::changingHardwareIsNotAllowed:
			PostNotify(VS_CC_BROKERREG_LOCKED);
			break;
		case RegStatus::serverNameIsInUse:
			PostNotify(VS_CC_SERVER_NAME_USED);
			break;
		case RegStatus::certIsNotYetValid:
			PostNotify(VS_CC_VERIFICATION_ERR_CERT_NOT_YET_VALID);
			break;
		case RegStatus::certHasExpired:
			PostNotify(VS_CC_VERIFICATION_ERR_CERT_HAS_EXPIRED);
			break;
		case RegStatus::certSignatureIsInvalid:
			PostNotify(VS_CC_VERIFICATION_ERR_SIGNATURE_FAILED);
			break;
		case RegStatus::certIsInvalid:
			PostNotify(VS_CC_VERIFICATION_NEW_CERT_FAILED);
			break;
		case RegStatus::brokerIsNotAvailable:
			PostNotify(VS_CC_NOREGCONNECT);
			break;
		case RegStatus::validLicenseIsNotAvailable:
			PostNotify(VS_CC_NO_VALID_LIC);
			break;
		default:
			PostNotify(VS_CC_BROKERREG_FAIL);
			break;
		}

		rkey.RemoveValue(VS_SERVER_START_MODE);
		rkey.RemoveValue("Server Name");
		rkey.RemoveValue("Serial");
		rkey.RemoveValue("RegResult");
		rkey.RemoveValue("SrvID");
		return true;
	}

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, (to_file)? REGISTERSERVEROFFLINE_METHOD: REGISTERSERVER_METHOD);		// online/offline registration

	rCnt.AddValue(SERVERID_PARAM, server_id);

	rCnt.AddValue(PASSWORD_PARAM, serial);
	rCnt.AddValue(SERVERNAME_PARAM,server_name);

	rCnt.AddValue("Version", ARM_LICENSE_VERSION);
	rCnt.AddValue("Srv Version",VCS_SERVER_VERSION);

	if(GenerateCertRequest(/*broker*/server_name,organization_name, country, contact_person, contact_email))
		rCnt.AddValue(CERT_REQUEST_PARAM, (char*)m_CertReq);

	if(to_file)
	{
		long cmd =1;
		if(!rkey.SetValue(&cmd,sizeof(cmd),VS_REG_INTEGER_VT,"cmd"))
			return false;
		VS_StartService();
		while(7!=VS_GetServiceState())
			Sleep(1000);
		std::unique_ptr<void, free_deleter> hw_buf;
		int	hw_sz(0);
		if(!(hw_sz=rkey.GetValue(hw_buf,VS_REG_BINARY_VT,"cmd_res")))
		{
			rkey.RemoveValue("cmd");
			return false;
		}
		VS_Container hw_cnt;
		hw_cnt.Deserialize(hw_buf.get(), hw_sz);
		rCnt.AddValue(KEY_PARAM,hw_cnt.GetStrValueRef("hw"));
		rCnt.AddValue("arm", hw_buf.get(), hw_sz);
		rCnt.AddValue("OS Info",hw_cnt.GetStrValueRef("OS Info"));
		rCnt.AddValue("CPU Info",hw_cnt.GetStrValueRef("CPU Info"));

	}
	void* body = nullptr;
	size_t bodySize;
	rCnt.SerializeAlloc(body, bodySize);
	VS_SCOPE_EXIT{ if (body) ::free(body); };
	if ( !to_file || !strlen(to_file) )
	{
		VS_ClientMessage tMsg(CLIENT_SRV, 0, 0, /*MANAGER_SRV*/REGISTRATION_SRV, 30000, body, bodySize,0,0,m_RegEp);
		return tMsg.Send()!=0;

	} else {

		void *encr_buf(0);
		unsigned long encr_buf_sz(0);

		if(!PrepareCryptBufAlloc(body,bodySize,encr_buf,encr_buf_sz))
			return false;

		uint32_t b64_buf_sz(0);
		char *b64_buf = VS_Base64EncodeAlloc(encr_buf,encr_buf_sz,b64_buf_sz);

		free(encr_buf);

		FILE* f = fopen(to_file, "wb");
		if ( !f )
		{
			free(b64_buf);
			return false;
		}

		size_t n_elements = fwrite(b64_buf, 1, b64_buf_sz, f);
		free(b64_buf);
		if ( n_elements != b64_buf_sz )
		{
			fclose(f);
			return false;
		}

		rkey.SetValue(m_PrivateKey.m_str, m_PrivateKey.Length() + 1, VS_REG_BINARY_VT, SRV_PRIVATE_KEY);
		return !fclose(f);
	}
}
bool VS_ConfigClient::PrepareCryptBufAlloc(const void * in_buf, const unsigned long in_buf_sz, void *&buf_out, unsigned long &out_buf_sz)
{
	if(!in_buf || !in_buf_sz)
		return false;
	VS_PKey			key;
	VS_Certificate	ca_cert;
	VS_PKeyCrypt	crypt;
	VS_GET_PEM_CACERT


	if(!ca_cert.SetCert(PEM_CACERT,strlen(PEM_CACERT)+1, store_PEM_BUF) ||
		!ca_cert.GetCertPublicKey(&key)|| !crypt.SetPublicKey(&key))
		return false;

	unsigned char *encrbuf(0);
	uint32_t encrlen(0);
	unsigned char iv[16] = {0};
	unsigned char *sesskey(0);
	uint32_t lensesskey(0);

	crypt.Encrypt((const unsigned char*)in_buf,in_buf_sz,alg_sym_AES256,encrbuf,&encrlen,
					iv,sesskey,&lensesskey);
	sesskey = new unsigned char[lensesskey];
	crypt.Encrypt((const unsigned char*)in_buf,in_buf_sz,alg_sym_AES256,encrbuf,&encrlen,
					iv,sesskey,&lensesskey);
	encrbuf = new unsigned char[encrlen];
	if(!crypt.Encrypt((const unsigned char*)in_buf,in_buf_sz,alg_sym_AES256,encrbuf,&encrlen,
						iv,sesskey,&lensesskey))
	{
		delete [] sesskey;
		delete [] encrbuf;
		return false;
	}

	VS_Container cnt;
	cnt.AddValue("key",(void*)sesskey,lensesskey);
	cnt.AddValue("iv",(void*)iv,16);
	cnt.AddValue("data",encrbuf,encrlen);
	size_t out_buf_sz_tmp;
	cnt.SerializeAlloc(buf_out, out_buf_sz_tmp);
	out_buf_sz = out_buf_sz_tmp;
	delete [] sesskey;
	delete [] encrbuf;
	return true;
}

bool VS_ConfigClient::ReqLicence(const char* from_file_name, const char* xml_file_name)
{
	if ( !from_file_name || !strlen(from_file_name) ||
		 !xml_file_name  || !strlen(xml_file_name) )
		return false;

	m_reg_response_filename = xml_file_name;

	FILE* f = fopen(from_file_name, "rb");
	if ( !f )
		return false;

	unsigned long bodySize = 0;

	size_t n_elements = fread((void*) &bodySize, sizeof(unsigned long), 1, f);
	if ( (n_elements != 1) || !bodySize )
	{
		fclose(f);
		return false;
	}

	auto body = std::make_unique<char[]>(bodySize);

	n_elements = fread(body.get(), 1, bodySize, f);
	if ( (n_elements != bodySize) )
	{
		fclose(f);
		return false;
	}

	fclose(f);

	VS_ClientMessage tMsg(CLIENT_SRV, 0, m_RegEp, /*MANAGER_SRV*/REGISTRATION_SRV, 30000, body.get(), bodySize);

	return tMsg.Send()!=0;
}

bool VS_ConfigClient::GenerateCertRequest(const char *broker, const char *organization_name,
									const char *country, const char *contact_person, const char *contact_email)
{
	VS_CertificateRequest	req;
	VS_PKey					pkey;
	char					*pem_buf(0);
	uint32_t				sz(0);
	if((!pkey.GenerateKeys(VS_DEFAULT_PKEY_LEN,alg_pk_RSA))||
		(!req.SetPKeys(&pkey,&pkey))||(!req.SetEntry("commonName",broker)))
		return false;
	if(organization_name)
		req.SetEntry("organizationName",organization_name);
	if(country)
		req.SetEntry("countryName",country);
	if(contact_person)
		req.SetEntry("surname",contact_person);
	if(contact_email)
		req.SetEntry("emailAddress",contact_email);
	if(!req.SignRequest())
		return false;
	req.SaveTo(0,sz,store_PEM_BUF);
	if(!sz)
		return false;
	pem_buf = new char[sz+1];
	if(!req.SaveTo(pem_buf,sz,store_PEM_BUF))
	{
		delete [] pem_buf;
		return false;
	}
	pem_buf[sz] = 0;
	m_CertReq = pem_buf;
	delete [] pem_buf;
	pem_buf = 0;
	sz = 0;
	pkey.WritePrivateKey(pem_buf,sz,store_PEM_BUF);
	if(!sz)
		return false;
	pem_buf = new char [sz + 1];
	pkey.WritePrivateKey(pem_buf,sz,store_PEM_BUF);
	pem_buf[sz] = 0;
	m_PrivateKey = pem_buf;
	delete [] pem_buf;
	sz = 0;
	return true;
}
/*****************************************************************************
 * Request to update user on current broker
 ****************************************************************************/
bool VS_ConfigClient::ReqUpdateUser(const char* broker, const char * user, int type)
{
	//if (!user) return false;
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::string session;
	rkey.GetString(session, SESSIONID_TAG);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, ONUSERCHANGE_METHOD);
	rCnt.AddValue(USERNAME_PARAM, user);
	rCnt.AddValueI32(TYPE_PARAM, type);
	rCnt.AddValue(SESSION_PARAM, session);

	TrySetCurrentConnectionToHKCU();
	Sleep(0);		// Fix: ktrushnikov: Win2008 64-bit (#8156)

	void* body;
	size_t bodySize;
	rCnt.SerializeAlloc(body, bodySize);
	VS_ClientMessage tMsg(CLIENT_SRV, 0, 0, AUTH_SRV, 10000, body, bodySize, 0, 0, broker, 0);
	free(body);
	return tMsg.Send()!=0;
}

/*****************************************************************************
 * Request to update app properties on current broker
 ****************************************************************************/
bool VS_ConfigClient::ReqUpdateProp(const char* broker)
{
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::string session;
	rkey.GetString(session, SESSIONID_TAG);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, ONAPPPROPSCHANGE_METHOD);
	rCnt.AddValue(SESSION_PARAM, session);

	TrySetCurrentConnectionToHKCU();
	Sleep(0);		// Fix: ktrushnikov: Win2008 64-bit (#8156)

	void* body;
	size_t bodySize;
	rCnt.SerializeAlloc(body, bodySize);
	VS_ClientMessage tMsg(CLIENT_SRV, 0, broker, CONFIGURATION_SRV, 10000, body, bodySize, 0, 0, broker, 0);
	free(body);
	return tMsg.Send()!=0;
}

/*****************************************************************************
 * Request to update addressbook properties
 ****************************************************************************/
bool VS_ConfigClient::ReqUpdateAddressBook(const char* broker, const char * user, const char* query, int ab)
{
	if (!user) return false;

	char tmp[256] = {0};
	if (!VS_GetLocalEName(tmp, 256))
		return false;
	char* p = strchr(tmp, '#');
	if (!p)
		return false;
	*p = 0;

	VS_RealUserLogin r(user);
	//r.SetServerName(tmp);
	//r = user;
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::string session;
	rkey.GetString(session, SESSIONID_TAG);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, ONADDRESSBOOKCHANGE_METHOD);
	rCnt.AddValue(USERNAME_PARAM, (const char*)r);
	rCnt.AddValueI32(ADDRESSBOOK_PARAM, ab);
	rCnt.AddValue(QUERY_PARAM,query);
	rCnt.AddValue(SESSION_PARAM, session);

	TrySetCurrentConnectionToHKCU();
	Sleep(0);		// Fix: ktrushnikov: Win2008 64-bit (#8156)

	void* body;
	size_t bodySize;
	rCnt.SerializeAlloc(body, bodySize);
	VS_ClientMessage tMsg(CLIENT_SRV, 0, broker, ADDRESSBOOK_SRV, 10000, body, bodySize, 0, 0, broker, 0);
	free(body);
	return tMsg.Send()!=0;
}

bool VS_ConfigClient::OnRegisterBrokerOfflineReply(VS_Container &cnt)
{
	VS_AcsLog* offline_verify_log(0);
#ifdef _DEBUG
	offline_verify_log = new VS_AcsLog("verify_offline", 5000000, 1000000,"./");
#else
	offline_verify_log = new VS_AcsEmptyLog();
#endif
	offline_verify_log->TPrintf("-------------------new verification----------------");

	int32_t result = false;
	cnt.GetValue(RESULT_PARAM, result);

	offline_verify_log->TPrintf("Result from RegServer: %d", result);
	if ( result != 1 )
	{
		offline_verify_log->TPrintf("-------------------end verification----------------");
		if ( offline_verify_log ) { delete offline_verify_log; offline_verify_log = 0; }

		PostNotify(VS_CC_BROKERREG_FAIL);
		return false;
	}

	const char* key = cnt.GetStrValueRef(KEY_PARAM);
	if ( !key )
	{
		offline_verify_log->TPrintf("No KEY_PARAM in container");
		offline_verify_log->TPrintf("-------------------end verification----------------");
		if ( offline_verify_log ) { delete offline_verify_log; offline_verify_log = 0; }

		PostNotify(VS_CC_BROKERREG_FAIL);
		return false;
	}
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	rkey.SetString(key, "Key");

	const char* cert = cnt.GetStrValueRef(CERTIFICATE_PARAM);
	if ( !cert )
	{
		offline_verify_log->TPrintf("No CERTIFICATE_PARAM in container");
		offline_verify_log->TPrintf("-------------------end verification----------------");
		if ( offline_verify_log ) { delete offline_verify_log; offline_verify_log = 0; }

		PostNotify(VS_CC_BROKERREG_FAIL);
		return false;
	}


#ifdef _DEBUG
	FILE* cert_file = fopen("certificate.pem", "wb");
	if ( cert_file )
	{
		fwrite(cert, 1, strlen(cert)+1, cert_file);
		fclose(cert_file);
	}
#endif

	VS_GET_PEM_CACERT

	VS_CertificateCheck	certCheck;
	certCheck.SetCert(cert, strlen(cert) + 1, store_PEM_BUF);
	certCheck.SetCertToChain(PEM_CACERT, strlen(PEM_CACERT) +1, store_PEM_BUF);

	if( !certCheck.VerifyCert() )
	{
		offline_verify_log->TPrintf("Certificate verification is failed.");
		VS_Certificate failed_cert;
		if( !failed_cert.SetCert(cert,strlen(cert)+1, store_PEM_BUF) )
			offline_verify_log->TPrintf("Data format failed");
		else
		{
			char buf[24];
			auto curr_time = std::chrono::system_clock::now();
			tu::TimeToLStr(curr_time, buf, 24);
			VS_SimpleStr str;
			offline_verify_log->TPrintf("Current time = %s\n", buf);
			PrepareCertDataToLog(&failed_cert,str);
			offline_verify_log->TPrintf(str);
		}

		offline_verify_log->TPrintf("-------------------end verification----------------");
		if ( offline_verify_log ) { delete offline_verify_log; offline_verify_log = 0; }

		PostNotify(VS_CC_BROKERREG_FAIL);
		return false;
	}
	offline_verify_log->TPrintf("Certificate verification is ok");
	rkey.SetValue(cert, (unsigned long)strlen(cert) + 1, VS_REG_BINARY_VT, SRV_CERT_KEY);
	offline_verify_log->TPrintf("-------------------end verification----------------");
	if ( offline_verify_log ) { delete offline_verify_log; offline_verify_log = 0; }




// encode data with base64
	unsigned char key_base64[2048];					memset(key_base64, 0, 2048);
	unsigned char certificate_base64[65535];		memset(certificate_base64, 0, 65535);
	unsigned char private_key_base64[8192];			memset(private_key_base64, 0, 8192);

	VS_BASE64_Encoding_Init();
	if ( !VS_BASE64_Encode((unsigned char*) key, (unsigned long) strlen(key), key_base64) )
	{
		PostNotify(VS_CC_BROKERREG_FAIL);
		return false;
	}

	VS_BASE64_Encoding_Init();
	if ( !VS_BASE64_Encode((unsigned char*) cert, (unsigned long) strlen(cert), certificate_base64) )
	{
		PostNotify(VS_CC_BROKERREG_FAIL);
		return false;
	}

	VS_BASE64_Encoding_Init();
	if ( !VS_BASE64_Encode((unsigned char*) m_PrivateKey.m_str, m_PrivateKey.Length(), private_key_base64) )
	{
		PostNotify(VS_CC_BROKERREG_FAIL);
		return false;
	}

// make XML
	m_OfflineRegFileXML.Resize(1048576);	memset(m_OfflineRegFileXML.m_str, 0, 1048576);		// 1 MB = 1024*1024

	sprintf(m_OfflineRegFileXML.m_str,	"<?xml version=\"1.0\"?>\r\n"
										"<reg_message>\r\n"
											"<registry>\r\n"
												"<key>\r\n"
													"<base64>%s=</base64>\r\n"
												"</key>\r\n"
												"<certificate>\r\n"
													"<base64>%s=</base64>\r\n"
												"</certificate>\r\n"
												"<private_key>\r\n"
													"<base64>%s=</base64>\r\n"
												"</private_key>\r\n"
											"</registry>\r\n", key_base64, certificate_base64, private_key_base64);
	return true;
}

bool VS_ConfigClient::OnUpdateLicenceOfflineReply(VS_Container &cnt)
{
	uint64_t id = 0;
	int32_t type = 0;
	const void* data = 0;
	size_t size = 0;
	int counter = 0;

	if ( !m_OfflineRegFileXML.Length() )
		return false;

	sprintf(m_OfflineRegFileXML.m_str + m_OfflineRegFileXML.Length(), "<licenses>\r\n");

	while( cnt.Next() )
	{
		if( _stricmp(cnt.GetName(), NAME_PARAM) == 0 )
		{
			const void* d = 0;
			size_t s = 0;

			d = cnt.GetBinValueRef(s);
			if (s == sizeof(uint64_t))
			{
				if (id != 0)
				{
					ProcessLicense(id,type,data,size);
					counter++;
				}

				id = *static_cast<const uint64_t*>(d);
				type = 0;
				data = 0;
			}
		} else if(_stricmp(cnt.GetName(),TYPE_PARAM)==0) {
			cnt.GetValue(type);
		} else if(_stricmp(cnt.GetName(),DATA_PARAM)==0) {
			data = cnt.GetBinValueRef(size);
		}
	};

	if( id )
	{
		ProcessLicense(id,type,data,size);
		counter++;
	}

	sprintf(m_OfflineRegFileXML.m_str + m_OfflineRegFileXML.Length(), "</licenses>\r\n"
																	"</reg_message>\r\n");

// write to file
	if ( !m_reg_response_filename.Length() )
	{
		m_OfflineRegFileXML.Empty();
		return false;
	}

	FILE* f = fopen(m_reg_response_filename, "wt");
	if ( !f )
	{
		m_OfflineRegFileXML.Empty();
		return false;
	}

	unsigned long output_sz = m_OfflineRegFileXML.Length();
	if ( output_sz != fwrite(m_OfflineRegFileXML.m_str, 1, output_sz, f) )
	{
		fclose(f);
		m_OfflineRegFileXML.Empty();
		return false;
	}

	fclose(f);
	m_OfflineRegFileXML.Empty();

	PostNotify(VS_CC_BROKERREG_OK);
	return true;
}

bool VS_ConfigClient::ProcessLicense(uint64_t id, long type, const void* data, unsigned long size)
{
	if ( !data || !size )
		return false;

	if ( type != LIC_ADD)
		return false;

	VS_License lic(data,size);
	if (!lic.IsValid())
		return false;

	char lic_name[128];		memset(lic_name, 0, 128);
	sprintf(lic_name, "%016I64X", lic.m_id);

	unsigned char data_base64[2048];					memset(data_base64, 0, 2048);
	VS_BASE64_Encoding_Init();
	if ( !VS_BASE64_Encode((unsigned char*) data, size, data_base64) )
	{
		PostNotify(VS_CC_BROKERREG_FAIL);
		return false;
	}

	sprintf(m_OfflineRegFileXML.m_str + m_OfflineRegFileXML.Length(),	"<license>\r\n"
																			"<name>%s</name>\r\n"
																			"<data>\r\n"
																				"<base64>%s=</base64>\r\n"
																			"</data>\r\n"
																		"</license>\r\n", lic_name, data_base64);
	return true;
}

void VS_ConfigClient::SendInvites(char *conf, char *pass, char** users, int num)
{
	const unsigned int sz = 256;
	char server[sz];
	if ( !VS_GetLocalEName(server, sz) )
		return;

	if (!conf || !pass || !users || num<=0 || !server)
		return;

	VS_Container cnt;
	for (int i=0; i < num; i++)
		if (users[i]) cnt.AddValue(CALLID_PARAM, users[i]);
	if (!cnt.IsValid())
		return;

	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::string session;
	rkey.GetString(session, SESSIONID_TAG);

	cnt.AddValue(METHOD_PARAM, INVITEUSERS_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, (const char*) conf);
	cnt.AddValue(PASSWORD_PARAM, (const char*) pass);
	cnt.AddValue(SESSION_PARAM, session);

	TrySetCurrentConnectionToHKCU();

	void* body;
	size_t bodySize;
	cnt.SerializeAlloc(body, bodySize);
	VS_ClientMessage tMsg(CLIENT_SRV, 0, 0, CONFERENCE_SRV, 10000, body, bodySize, 0, 0, server, 0);
	free(body);
	tMsg.Send();
}
void VS_ConfigClient::DeleteConference(const char *conf_id)
{
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::string session;
	rkey.GetString(session, SESSIONID_TAG);

	const unsigned int sz = 256;
	char server[sz];
	if( !VS_GetLocalEName(server, sz) )
		return;

	if(!conf_id || !server)
		return;

	TrySetCurrentConnectionToHKCU();
	Sleep(0);		// Fix: ktrushnikov: Win2008 64-bit (#8156)

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, DELETECONFERENCE_METHOD);
	cnt.AddValue(NAME_PARAM,conf_id);
	cnt.AddValueI32(CAUSE_PARAM, VS_ConferenceDescription::TERMINATED_BY_ADMIN);
	cnt.AddValue(SESSION_PARAM, session);
	void* body;
	size_t bodySize;
	cnt.SerializeAlloc(body, bodySize);

	VS_ClientMessage tMsg(CLIENT_SRV, 0, 0, CONFERENCE_SRV, 10000, body, bodySize, 0, 0, server, 0);
	free(body);
	tMsg.Send();
}

void VS_ConfigClient::SetLayout(char *conf, char *caster)
{
	/**
		TODO: this message is nowhere to be handled.
	*/
	const unsigned int sz = 256;
	char server[sz];
	if (!VS_GetLocalEName(server, sz) )
		return;
	if (!conf)
		return;
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::string session;
	rkey.GetString(session, SESSIONID_TAG);

	VS_Container	cnt;
	cnt.AddValue(METHOD_PARAM,SETLAYERS_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, conf);
	cnt.AddValue(USERNAME_PARAM, caster);
	cnt.AddValue(SESSION_PARAM, session);

	TrySetCurrentConnectionToHKCU();

	void* body;
	size_t bodySize;
	cnt.SerializeAlloc(body, bodySize);
	VS_ClientMessage tMsg(CLIENT_SRV, 0, 0, CONFERENCE_SRV, 10000, body, bodySize, 0, 0, server, 0);
	free(body);
	tMsg.Send();
}

unsigned VS_ConfigClient::RegisterFromFile(const char *file_name)
{
	VS_PKey			private_key;
	VS_PKeyCrypt	crypt;
	VS_AcsLog	*verify_log(0);
#ifdef _DEBUG
	verify_log = new VS_AcsLog("verify", 5000000, 1000000,"./");
#else
	verify_log = new VS_AcsEmptyLog();
#endif


	VS_RegistryKey rkey(false, CONFIGURATION_KEY);
	std::unique_ptr<char, free_deleter> pkey_buf;
	auto pkey_size = rkey.GetValue(pkey_buf, VS_REG_BINARY_VT, SRV_PRIVATE_KEY);
	if (pkey_size <= 0 || !private_key.SetPrivateKey(pkey_buf.get(), store_PEM_BUF)
		||(!crypt.SetPrivateKey(&private_key)))
	{
		return 2; // Data is not private key
	}
	FILE *f = fopen(file_name,"rb");
	if(!f)
	{
		return 3;	//	File access error
	}
	int b_size;
	if(fseek( f, 0, SEEK_END ) || (0>=(b_size=ftell(f))))
	{
		fclose(f);
		return 3;	//	File access error
	}
	fseek(f,0,SEEK_SET);
	auto buf = std::make_unique<unsigned char[]>(b_size);
	if (1 != fread(buf.get(), b_size, 1, f))
	{
		fclose(f);
		return 3;	//	File access error
	}
	fclose(f);
	uint32_t decoded_sz(0);
	void *decoded_buf = VS_Base64DecodeAlloc(buf.get(), b_size, decoded_sz);
	VS_Container cnt;
	if((!decoded_buf) || !cnt.Deserialize(decoded_buf,decoded_sz))
	{
		free(decoded_buf);
		return 4;	// File has unsupported format;
	}
	free(decoded_buf);

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
	if(!crypt.Decrypt(encr_data,encr_data_ln,alg_sym_AES256,iv,sess_key,sess_key_ln,decrdata,&decrlen) ||
		!cnt.Deserialize(decrdata,decrlen))
	{
		delete [] decrdata;
		return 5;	// Decrypt error;
	}
	delete [] decrdata;

	VS_RegistryKey key_write(false, CONFIGURATION_KEY, false, true);
	key_write.SetString(cnt.GetStrValueRef(KEY_PARAM), "Key");
	verify_log->TPrintf("-------------------new verification----------------");


	const char *cert = cnt.GetStrValueRef(CERTIFICATE_PARAM);
	size_t ak_sz(0);
	const void * ak_buf = cnt.GetBinValueRef("ak",ak_sz);
	if(ak_buf)
		key_write.SetValue(ak_buf,ak_sz,VS_REG_BINARY_VT,"ak");
	VS_Container cert_chain_cnt;

	if(cert)
	{
		VS_GET_PEM_CACERT
		VS_CertificateCheck	certCheck;
		certCheck.SetCert(cert,strlen(cert)+1, store_PEM_BUF);
		certCheck.SetCertToChain(PEM_CACERT,strlen(PEM_CACERT)+1,store_PEM_BUF);
		cnt.Reset();
		while(cnt.Next())
		{
			if(!!cnt.GetName() && (0 == _stricmp(cnt.GetName(),CERTIFICATE_CHAIN_PARAM)))
			{
				const char *cert_in_chain = cnt.GetStrValueRef();
				if(cert_in_chain)
				{
					certCheck.SetCertToChain(cert_in_chain,strlen(cert_in_chain)+1, store_PEM_BUF);
					cert_chain_cnt.AddValue(CERTIFICATE_CHAIN_PARAM,(void*)cert_in_chain,strlen(cert_in_chain));
				}
			}
		}
		if(cnt.GetStrValueRef(PARENT_CERT_PARAM))
			cert_chain_cnt.AddValue(PARENT_CERT_PARAM,cnt.GetStrValueRef(PARENT_CERT_PARAM));

#ifdef _DEBUG
		FILE *fp = fopen("certificate.pem","wb");
		if(fp)
		{
			fwrite(cert,strlen(cert)+1,1,fp);
			fclose(fp);
		}
#endif
		if(certCheck.VerifyCert())
		{
			verify_log->TPrintf("Certificate verification is ok");
			key_write.SetValue(cert, (unsigned long)strlen(cert) + 1, VS_REG_BINARY_VT, SRV_CERT_KEY);
			void *buf_cnt(0);
			size_t buf_sz(0);

			cert_chain_cnt.SerializeAlloc(buf_cnt,buf_sz);
			VS_SCOPE_EXIT{ if (buf_cnt) ::free(buf_cnt); };

			key_write.SetValue(buf_cnt, buf_sz,VS_REG_BINARY_VT, SRV_CERT_CHAIN_KEY);
		}
		else
		{
			VS_Certificate	failed_cert;
			if(!failed_cert.SetCert(cert,strlen(cert) + 1, store_PEM_BUF))
				verify_log->TPrintf("Data format failed");
			else
			{
				char buf[24];
				auto curr_time = std::chrono::system_clock::now();
				tu::TimeToLStr(curr_time, buf, 24);
				VS_SimpleStr	str;
				verify_log->TPrintf("Current time = %s\n",buf);
				PrepareCertDataToLog(&failed_cert,str);
				verify_log->TPrintf(str);
			}
			verify_log->TPrintf("Certificate verification is failed.");
			return 7; // Certificate Verificateion failed;
		}
	}
	else
	{
		return 6;	//Certificate is absent
	}

	if(GetLicensingInfo(cnt))
	{
		return 0; // No Error;
	}
	else
	{
		return 8; // Licenses not found;
	}
}
bool VS_ConfigClient::GetLicensingInfo(VS_Container &cnt)
{
	uint64_t id = 0;
	int32_t type = 0;
	const void* data = nullptr;
	size_t size = 0;
	int counter=0;
	VS_RegistryKey l_root(VS_Server::RegistryKey(), false, LICENSE_KEY, false, false);
	cnt.Reset();

	while(cnt.Next())
	{
		if(_stricmp(cnt.GetName(),NAME_PARAM)==0)
		{
			const void* d;
			size_t s = 0;
			d=cnt.GetBinValueRef(s);
			if (s == sizeof(uint64_t))
			{
				if (id != 0)
				{
					ProcessLicense(l_root,id,type,data,size);
					counter++;
				}

				id = *static_cast<const uint64_t*>(d);
				type=0;data=0;
			}
		}
		else if(_stricmp(cnt.GetName(),TYPE_PARAM)==0)
		{	cnt.GetValue(type); }
		else if(_stricmp(cnt.GetName(),DATA_PARAM)==0)
		{ data=cnt.GetBinValueRef(size); }
	}

	if(0!=id)
	{
		ProcessLicense(l_root,id,type,data,size);
		counter++;
	}
	return counter>0;
}

void VS_ConfigClient::ProcessLicense(VS_RegistryKey &l_root, uint64_t id, long type, const void *data, int size)
{
	switch(type)
	{
	case LIC_DELETE:
		{
			char lic_name[128];
			sprintf(lic_name, "%s\\%016I64X", LICENSE_KEY, id);
			l_root.RemoveKey(lic_name);
			break;
		};
	case LIC_ADD:
		{
			//dprint3("CLS: Adding license %016I64X\n",id);
			if(data==NULL || size==0)
			{
				//dprint1("Erroneous add license request from reg srv\n");
				break;
			}

			//VS_License lic(data,size);
			/*if(!lic)
				dprint1("Erroneous license arrived from reg srv\n"); */

			char lic_name[128];

			sprintf(lic_name, "%s\\%016I64X", LICENSE_KEY, /*lic.m_id*/id);

			VS_RegistryKey	lic_key(VS_Server::RegistryKey(),false, lic_name, false, true);
			if (!lic_key.IsValid())
				break;

			lic_key.SetValue(data, size, VS_REG_BINARY_VT, LICENSE_DATA_TAG);
			break;
		}
	}
}
bool VS_ConfigClient::GetCertificateData(VS_WideStr &serverEP, VS_WideStr &country, VS_WideStr &organization, VS_WideStr &contact_person, VS_WideStr &email, VS_SimpleStr &notBefore, VS_SimpleStr &notAfter)
{
	std::string buf;
	std::string notBeforeBuf, notAfterBuf;
	VS_Certificate cert;

	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::unique_ptr<char, free_deleter> cert_buf;
	int cert_len = rkey.GetValue(cert_buf,VS_REG_BINARY_VT, SRV_CERT_KEY);
	if(!cert_buf || cert_len<=0)
		return false;
	if(!cert.SetCert(cert_buf.get(), cert_len, store_PEM_BUF))
		return false;

	if(cert.GetSubjectEntry("commonName", buf))
		serverEP.AssignUTF8(buf.c_str());
	else
		serverEP.Empty();
	if(cert.GetSubjectEntry("countryName",buf))
		country.AssignUTF8(buf.c_str());
	else
		country.Empty();
	if(cert.GetSubjectEntry("organizationName",buf))
		organization.AssignUTF8(buf.c_str());
	else
		organization.Empty();
	if(cert.GetSubjectEntry("surname",buf))
		contact_person.AssignUTF8(buf.c_str());
	else
		contact_person.Empty();
	if(cert.GetSubjectEntry("emailAddress",buf))
		email.AssignUTF8(buf.c_str());
	else
		email.Empty();
	if(cert.GetExpirationTime(notBeforeBuf, notAfterBuf, false))
	{
		notBefore = notBeforeBuf.c_str();
		notAfter = notAfterBuf.c_str();
	}
	else
	{
		notBefore.Empty();
		notAfter.Empty();
	}
	return true;
}

void VS_ConfigClient::TrySetCurrentConnectionToHKCU()
{
	char buff[1024] = {0};
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	key.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, CURRENT_CONNECT_TAG);

	if (*buff)
	{
		VS_ResetConnectTCP(true);

		char* token_ctx = 0;
		char* token = 0;
		char* p = 0;
		unsigned int port = 0;
		char seps[] = ",";

		token = strtok_s( buff, seps, &token_ctx );
		while( token != NULL )
		{
			p = strrchr(token, ':');
			p[0] = 0;		p++;
			port = atoi(p);

			// For ipv6 address [1::1:1:1]
			if(*token == '[')
			{
				char* last_bracket = strrchr(token, ']');
				if (last_bracket)
				{
					token++;
					*last_bracket = '\0';
				}
			}

			VS_SetConnectTCP(token, port, true);

			token = strtok_s( NULL, seps, &token_ctx );
		}
	}
}

void VS_ConfigClient::OnConnect(const char *to_name, const long error)
{
}

void VS_ConfigClient::OnSidChange(const char *old_name, const char *new_name)
{
}


bool VS_ConfigClient::GetIdentifyStr(VS_SimpleStr &str)
{
/**
	формат:
	serverID-timestamp-sign

	подписывается строка:
	Server_idServer_nameTimeStamp

**/


	VS_Certificate	cert;
	VS_Sign			sign;
	VS_SignArg		signarg = {alg_pk_RSA,alg_hsh_SHA1};
	VS_SimpleStr	server_name;
	VS_SimpleStr	server_id;
	VS_SimpleStr	data_for_sign;
	uint32_t sign_sz = VS_SIGN_SIZE;
	unsigned char sign_buf[VS_SIGN_SIZE];
	if(!sign.Init(signarg))
		return false;

	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::unique_ptr<char, free_deleter> cert_buf;
	int cert_len = rkey.GetValue(cert_buf,VS_REG_BINARY_VT, SRV_CERT_KEY);
	std::unique_ptr<char, free_deleter> private_key_buf;
	rkey.GetValue(private_key_buf,VS_REG_BINARY_VT, SRV_PRIVATE_KEY);
	if(!cert_buf || !private_key_buf || cert_len<=0)
		return false;
	if(!cert.SetCert(cert_buf.get(), cert_len, store_PEM_BUF) || !sign.SetPrivateKey(private_key_buf.get(), store_PEM_BUF))
		return false;

	std::string buf;
	if(cert.GetSubjectEntry("commonName", buf))
		server_name = buf.c_str();
	if(cert.GetExtension(SERVER_ID_EXTENSION,buf))
		server_id = data_for_sign = buf.c_str();
	if(!server_id || !server_name)
		return false;

	data_for_sign +=server_name;
	/**
		подписать строку
	*/
	time_t notAfter(0);
	time(&notAfter);
	char chtime[32] = {0};
	notAfter += 5*60; // + 5 min
	notAfter /=60; // mins
	data_for_sign+=_i64toa(notAfter,chtime,10);
	if(!sign.SignData((unsigned char*)data_for_sign.m_str,data_for_sign.Length(),sign_buf,&sign_sz))
		return false;

	uint32_t b64_buf_sz(0);
	char *b64_buf = VS_Base64EncodeAlloc(sign_buf,sign_sz,b64_buf_sz);

	uint32_t b64str_sz = b64_buf_sz + 1;
	char *tmp = new char[b64str_sz];
	memcpy(tmp,b64_buf,b64_buf_sz);
	tmp[b64_buf_sz] = 0;


	str = server_id;
	str+="-";
	str+=chtime;
	str+="-";
	str+=tmp;
	free(b64_buf);
	delete [] tmp;
	return true;
}

/*****************************************************************************
 * (c) 2004 Visicron Inc.  http://www.visicron.net/
 *
 * Project: Broker Services - Media streaming
 *
 * Created: SMirnovK 9 Jun 2004
 *
 * $History: VS_ConfigClient.h $
 *
 * *****************  Version 20  *****************
 * User: Mushakov     Date: 17.07.12   Time: 23:09
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - LoginConfigurator() was removed
 * - messages from configurator are handled by SessionID
 * - fix TransportMessage::IsFromServer()
 *
 * *****************  Version 19  *****************
 * User: Ktrushnikov  Date: 22.06.12   Time: 19:24
 * Updated in $/VSNA/servers/serversconfiglib
 * - VS_LDAPCore: cache of users updates only by timeout (AB Refresh) OR
 * from configurator Refresh button
 * - quick logout
 * - UpdateAddressBook called at OnUserChange(type=4)
 *
 * *****************  Version 18  *****************
 * User: Ktrushnikov  Date: 15.06.12   Time: 12:09
 * Updated in $/VSNA/Servers/ServersConfigLib
 * new ldap:
 * - VS_LDAPCore split for tc_server.exe & tc_conf.dll
 *
 * *****************  Version 17  *****************
 * User: Mushakov     Date: 20.04.11   Time: 19:46
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - DeleteConf from manager
 *
 * *****************  Version 16  *****************
 * User: Mushakov     Date: 15.03.11   Time: 16:10
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - 8553 automatic updated user info supported
 *
 * *****************  Version 15  *****************
 * User: Mushakov     Date: 15.02.11   Time: 13:56
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - registration case "no valid lic" handled
 *
 * *****************  Version 14  *****************
 * User: Mushakov     Date: 15.12.10   Time: 21:39
 * Updated in $/VSNA/Servers/ServersConfigLib
 * 8276
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 8.12.10    Time: 21:27
 * Updated in $/VSNA/Servers/ServersConfigLib
 * 8241
 *
 * *****************  Version 12  *****************
 * User: Mushakov     Date: 3-12-10    Time: 16:30
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - VSGetServerIdentifyString added (7205)
 *  - sign/verify sign code in secure lib  was little corrected
 *  - SignVerifier added
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 3.11.10    Time: 20:46
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - VCS manager authorization added
 * - VCS Config dead lock fixed
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 10.09.10   Time: 20:24
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - Registration on SM added
 *
 * *****************  Version 9  *****************
 * User: Ktrushnikov  Date: 17.03.10   Time: 13:42
 * Updated in $/VSNA/servers/serversconfiglib
 * Create HKCU ConnectTCP* from CurrentConnect for vs_bc.dll
 * VS_SetLocalENameInService
 * - useless check for endpointName removed
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 28.01.10   Time: 19:53
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - offline registration supported (VCS)
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 18.12.09   Time: 18:04
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - Removed VCS_BUILD somewhere
 * - Add new field to license
 * - Chat service for bsServer renamed
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 28.10.09   Time: 17:59
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - vs_ep_id removed
 * - registration corrected
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 23.10.09   Time: 15:05
 * Updated in $/VSNA/Servers/ServersConfigLib
 *  - VCS 3
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 7.08.09    Time: 15:14
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 1.04.08    Time: 16:39
 * Updated in $/VSNA/Servers/ServersConfigLib
 * -key lengthes modified
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:01
 * Updated in $/VSNA/Servers/ServersConfigLib
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 25.11.07   Time: 13:47
 * Updated in $/VS2005/Servers/ServersConfigLib
 * - Offline Registration added
 *
 * *****************  Version 4  *****************
 * User: Avlaskin     Date: 22.08.07   Time: 12:23
 * Updated in $/VS2005/Servers/ServersConfigLib
 *
 * *****************  Version 3  *****************
 * User: Dront78      Date: 11.07.07   Time: 19:58
 * Updated in $/VS2005/Servers/ServersConfigLib
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 22.03.07   Time: 16:18
 * Updated in $/VS2005/Servers/ServersConfigLib
 * добавлена верификация ProductType при регистрации брокера
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/ServersConfigLib
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 20.06.06   Time: 14:43
 * Updated in $/VS/Servers/ServersConfigLib
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 16.06.06   Time: 16:50
 * Updated in $/VS/Servers/ServersConfigLib
 * Added verification log in ServerConfigDLL (debug version only)
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 31.05.06   Time: 20:45
 * Updated in $/VS/Servers/ServersConfigLib
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 27.04.06   Time: 13:10
 * Updated in $/VS/Servers/ServersConfigLib
 * updated manage of certificate
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 25.04.06   Time: 15:54
 * Updated in $/VS/Servers/ServersConfigLib
 * Added certificate issue
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 15.03.06   Time: 14:00
 * Updated in $/VS/Servers/ServersConfigLib
 * - added parametr broker to netclient functions
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 5.03.04    Time: 11:56
 * Created in $/VS/Servers/ServersConfigLib
 * removed all config dlls, now it common
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
 ****************************************************************************/
/*****************************************************************************
 * \file	VS_CofigClient.h
 * \brief	Send/receive transport messages
 ****************************************************************************/

#ifndef VS_CONFIGCLIENT_H
#define VS_CONFIGCLIENT_H

/*****************************************************************************
 * Includes
 ****************************************************************************/
#include "../common/std/cpplib/VS_SimpleStr.h"
#include "../common/std/cpplib/VS_WideStr.h"
#include "transport/Client/VS_TransportClient.h"

#include <cstdint>

/*****************************************************************************
 * Defines
 ****************************************************************************/

// service messages
#define WM_SERVICE_CC			WM_APP+1
// internal messages
#define WM_CC_DESTROY			WM_USER+1
// external messages
#define WM_CC_NOTIFY			WM_USER+2
// external params
#define VS_CC_NOREGCONNECT		0x1			// message not send
#define VS_CC_BROKERREG_OK		0x101		// broker registered ok
#define VS_CC_BROKERREG_FAIL	0x102		// broker registration failed

#define VS_CC_VERIFICATION_NEW_CERT_FAILED	0x103
#define VS_CC_VERIFICATION_ERR_CERT_NOT_YET_VALID	0x105
#define VS_CC_VERIFICATION_ERR_CERT_HAS_EXPIRED		0x106
#define VS_CC_VERIFICATION_ERR_SIGNATURE_FAILED		0x107
/*
	1 неправильный CA
	2 время сертификата вышло
	3 время сертификата не наступило
*/
#define VS_CC_BROKERREG_LOCKED	0x104		//broker registration locked
#define VS_CC_SERVER_NAME_USED	0x108		//ServerName уже занято
#define VS_CC_REGISTRY_WRITE_FAILED	0x109	//Ошибка записи в реестр
#define VS_CC_START_SERVICE_FAILED	0x10A	//Не удалось запустить службу
#define VS_CC_RESULT_CODE_EMPTY	0x10B		//Код результата не найден (произошла внутренняя ошибка)
#define VS_CC_STOP_SERVICE_FAILED	0x10C
#define VS_CC_NO_VALID_LIC			0x10D


class VS_Certificate;
class VS_Container;

/*****************************************************************************
 * Interface
 ****************************************************************************/
class VS_RegistryKey;
class VS_Container;

class VS_ConfigClient : public VS_EndpointCallback
{
	// variables
	bool			m_Valid;
	DWORD			m_ThreadId;
	HANDLE			m_hThread;
	HANDLE			m_hDie;
	HWND			m_hwnd;
	VS_SimpleStr	m_Ep;		// self
	VS_SimpleStr	m_RegEp;	// registration
	VS_SimpleStr	m_CertReq;
	VS_SimpleStr	m_PrivateKey;
	VS_SimpleStr	m_OfflineRegFileXML;
	VS_SimpleStr	m_reg_response_filename;
	HANDLE			m_hLDAPUpdateFinished;

	//static long m_ProductID;
	// methods
	void		Servises(MSG *msg);
	void		ConfigClientSrv(MSG *msg);
	DWORD		ThreadCycle();
	void		PostNotify(int notify);

	bool		GenerateCertRequest(const char *broker, const char *organization_name,
									const char *country, const char *contact_person, const char *contact_email);
	bool		PrepareCryptBufAlloc(const void * in_buf, const unsigned long in_buf_sz, void *&buf_out, unsigned long &out_buf_sz);
	void		PrepareCertDataToLog(VS_Certificate* cert, VS_SimpleStr &str);

	bool		OnRegisterBrokerOfflineReply(VS_Container &cnt);
	bool		OnUpdateLicenceOfflineReply(VS_Container &cnt);
	bool		ProcessLicense(uint64_t id, long type, const void* data, unsigned long size);

	void		ProcessLicense(VS_RegistryKey& l_root,uint64_t id,long type,const void* data,int size);
	bool		GetLicensingInfo(VS_Container& cnt);
	void		TrySetCurrentConnectionToHKCU();

public:
	VS_ConfigClient();
	~VS_ConfigClient();
	bool		Init(HWND hwnd);
	void		Release();

	bool		ReqLicence(const char* server_id, const char *server_name,const char* serial, const char *organization_name, const char *country,
							const char *contact_person, const char *contact_email, const char* to_file = 0);
	bool		ReqLicence(const char* file_name, const char* xml_file_name);

	bool		ReqUpdateUser(const char* broker, const char * user, int type);
	bool		ReqUpdateProp(const char* broker);
	unsigned	RegisterFromFile(const char *file_name);
	bool		ReqUpdateAddressBook(const char* broker, const char * user, const char *query, int ab);
	void		SendInvites(char *conf, char *pass, char** users, int num);
	void		DeleteConference(const char *conf_name);

	void		SetLayout(char *conf, char *caster);

	bool GetCertificateData(VS_WideStr &serverEP,
							VS_WideStr &country,
							VS_WideStr &organization,
							VS_WideStr &contact_person,
							VS_WideStr &email,
							VS_SimpleStr &notBefore,
							VS_SimpleStr &notAfter);

	bool		GetIdentifyStr(VS_SimpleStr& str);

	static void	SetApplication(int num);

	void OnConnect(const char *to_name, const long error = 0);
	void OnSidChange(const char *old_name, const char *new_name);

	friend DWORD WINAPI ThreadProc(void* param);
};

#endif

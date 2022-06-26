/************************************************
 * $Revision: 30 $
 * $History: VS_ServerManagerService.h $
 *
 * *****************  Version 30  *****************
 * User: Ktrushnikov  Date: 5.03.12    Time: 17:37
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - #11142: update A record
 * - many domain support (for conferendo.com)
 * - "Allowed AS" registry added
 *
 * *****************  Version 29  *****************
 * User: Ktrushnikov  Date: 6.12.11    Time: 18:27
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - ddns update vcs2.tcp with hosts (was vcs.tcp)
 * - skip list of servers added
 *
 * *****************  Version 28  *****************
 * User: Mushakov     Date: 23.02.11   Time: 4:05
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - SecureHandshake ver. 2 added
 *
 * *****************  Version 27  *****************
 * User: Mushakov     Date: 1.11.10    Time: 21:00
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - cert update added
 * - registration from server added
 * - authorization servcer added
 *
 * *****************  Version 26  *****************
 * User: Ktrushnikov  Date: 6.10.10    Time: 14:58
 * Updated in $/VSNA/Servers/ServerManager/Services
 * SM:
 * - RedirectAS() & LoadBalance() by domain[vzochat.com, v-port.net]
 *
 * *****************  Version 25  *****************
 * User: Mushakov     Date: 10.09.10   Time: 20:24
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Registration on SM added
 *
 * *****************  Version 24  *****************
 * User: Ktrushnikov  Date: 26.07.10   Time: 12:59
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - 2 threads to update servers in different SRVs (split by server_name
 * *.vzochat.com)
 *
 * *****************  Version 23  *****************
 * User: Ktrushnikov  Date: 19.07.10   Time: 18:29
 * Updated in $/VSNA/servers/servermanager/services
 * AS asks SM (not BS) for AppProps
 * - by SM connected
 * - by manual request from support web
 *
 * *****************  Version 22  *****************
 * User: Ktrushnikov  Date: 10.03.10   Time: 13:22
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - update of vcs.tcp.video-port.com
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 3.06.08    Time: 21:55
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Redirect v.3
 *
 * *****************  Version 20  *****************
 * User: Ktrushnikov  Date: 22.05.08   Time: 21:39
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Implementation of REDIRECT state in AS commented
 * - Check if NetConfig Exist in AS for redirecting
 * - LoadBalance algo re-written
 * - Overload algo re-written
 *
 * *****************  Version 19  *****************
 * User: Ktrushnikov  Date: 21.05.08   Time: 17:43
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - CheckOverLoad() for users loggged in ASs at the SM
 *
 * *****************  Version 18  *****************
 * User: Ktrushnikov  Date: 16.05.08   Time: 19:41
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - SM & AS: LoadBalance algorithm added as MSC_LOADBALANCE
 *
 * *****************  Version 17  *****************
 * User: Ktrushnikov  Date: 27.04.08   Time: 19:33
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Average statistics added
 *
 * *****************  Version 16  *****************
 * User: Dront78      Date: 24.04.08   Time: 20:31
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - DNS server thread rewritten
 *
 * *****************  Version 15  *****************
 * User: Ktrushnikov  Date: 12.04.08   Time: 17:29
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Manager commands for AS from SM added
 *
 * *****************  Version 14  *****************
 * User: Dront78      Date: 4.04.08    Time: 18:46
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - DDNS algorithms updated
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 25.03.08   Time: 17:51
 * Updated in $/VSNA/Servers/ServerManager/Services
 *  - SSL added
 *  - fixed bug: Connect to server with another name
 *
 * *****************  Version 12  *****************
 * User: Ktrushnikov  Date: 21.03.08   Time: 19:42
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - support for Current stats  (not Average stats): send from AS to SM
 * and saved to Registry::Servers
 * - VS_FileTime: RUS_FMT added: dd.mm.yyyy hh:mm:ss
 * - struct VS_AppServerStats added
 *
 * *****************  Version 11  *****************
 * User: Ktrushnikov  Date: 21.02.08   Time: 16:34
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - bug fixes in Storage
 * - UPDATE_INFO only for registered servers
 * - Get BS/RS algo: 1st online BS and RS
 * - On SM start set servers statuses to zero
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 13.02.08   Time: 13:43
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - properties corrected
 * - sturtup sequence of server connects rewrited
 * - code redused
 *
 * *****************  Version 9  *****************
 * User: Dront78      Date: 12.02.08   Time: 15:40
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - unuseble functions removed
 * - updated DDNS multithreading code
 *
 * *****************  Version 8  *****************
 * User: Ktrushnikov  Date: 16.01.08   Time: 19:15
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - OnPointConnect_Event() added
 *
 * *****************  Version 7  *****************
 * User: Avlaskin     Date: 17.12.07   Time: 17:07
 * Updated in $/VSNA/Servers/ServerManager/Services
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 15.12.07   Time: 12:29
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Ping Servers from Storage
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 11.12.07   Time: 17:44
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Use only dns-name (don't use port) in DDNS SRV Updates
 * - Handle DDNS on Connect/Disconnect of broker
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 11.12.07   Time: 15:44
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - DDNS functions support hostname (not just ip)
 * - Funcs in EndpointRegistry to parse buffer and return ConnectTCP
 * (don't write to registry)
 * - Send info about AS's ConnectTCP to SM
 *
 * *****************  Version 3  *****************
 * User: Ktrushnikov  Date: 9.12.07    Time: 20:52
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Cleanup, when stop server
 * - Async DNS Update support added
 * - Storage Lock fixed
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 4.12.07    Time: 11:50
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - VS_TransportMessage::Set(): checks fixed for new arch
 * - VS_AppManagerService: Init() overload fixed
 * - VS_AppManagerService: Message processing fixed
 *
 * *****************  Version 1  *****************
 * User: Ktrushnikov  Date: 30.11.07   Time: 11:09
 * Created in $/VSNA/Servers/ServerManager/Services
 * - ServerManagerService added for SM server
 * - Storage for ServerManagerService added
 *
 ************************************************/

#pragma once

#include <vector>
#include <algorithm>
#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "transport/Client/VS_DNSFunction.h"
#include "transport/Router/VS_TransportRouter.h"
#include "streams/fwd.h"
#include "VS_ServerManagerStorage.h"
#include "VS_RegDBStorage.h"

class VS_ServerManagerService :
	public VS_ServCertInfoInterface,
	public VS_Testable,
	public VS_TransportRouterServiceReplyHelper
{
	VS_TransportRouter*		m_tr;
	VS_ServerManagerStorage	m_storage;
	VS_RegDBStorage			m_dbstorage;

	static const unsigned long	SERVERS_PING_PERIOD = 30*1000;	// AppServer ping period


private:
	bool OnUpdateInfo(VS_Container &cnt, const char* from_sid);

	bool RegisterServer_Method(const char* server_id, const char * server_name,const char* key,const char* pass,
		const char *cert_request, const char *version, bool IsOffline = false);
	bool CertificateUpdate_Method(VS_Container &cnt);

	//bool RegisterServer_Method(const char* server_id, const char* serial, /*const char* pass, */
	//							const char *cert_request,const long productID, bool IsOffline = false);
	void GetAllAppProperties_Method();
	void GetASOfMyDomain_Method(string_view src_server);

	bool OnLogStats_Method(VS_Container &cnt);
	void SendCommandToServer(const char* sid, long mgr_cmd, long mgr_cmd_param, const char* redir_server = 0);
	void SendCommandToServer(const char* sid, long mgr_cmd, long mgr_cmd_param, VS_Container &cnt);

	void LoadBalance(string_view domain);
	void RedirectAS(string_view domain);

//	VS_DDNS*	m_dns;

	HANDLE		m_timer;
//	HANDLE		m_thread;

	unsigned long	m_loadbalance_time;
	unsigned long	m_redirectas_time;

public:

	VS_ServerManagerService();
	virtual ~VS_ServerManagerService();

	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	void AsyncDestroy() override;
	bool Timer(unsigned long tick) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	// VS_Testable implementation
	bool Test() {return true;}

	bool		m_obj_deleted;

	struct UpdateThreadArgs
	{
		HANDLE						thread_handle;

		std::vector<std::string>	online_servers;
		VS_Lock						online_servers_lock;

		VS_DDNS						ddns;

		VS_ServerManagerService*	service;

		UpdateThreadArgs(): thread_handle(0), service(0)
		{	}
	};

	UpdateThreadArgs				ddns_thread[10];

	std::string						m_skip_list;

	std::multimap<std::string, UpdateThreadArgs*>		m_dns_domains;		// [sid, ptr_dns_struct]


	VS_ServCertInfoInterface::get_info_res
		GetPublicKey(
			const VS_SimpleStr& server_name,
			VS_SimpleStr &pub_key,
			uint32_t &vcs_ver) override;
	VS_ServCertInfoInterface::get_info_res
		GetServerCertificate(
			const VS_SimpleStr &server_name,
			VS_SimpleStr &cert) override;

	static unsigned __stdcall	Thread(void *arg);
	void UpdateServersThread(UpdateThreadArgs* arg);

	bool OnConnect_UpdateDNS(const char *endpoint_name);
	bool OnDisconnect_CleanUpDNS(const char* endpoint_name);

	// VS_EndpointConditions
	bool OnPointConnected_Event( const VS_PointParams* prm ) override;
	bool OnPointDisconnected_Event( const VS_PointParams* prm ) override;
};
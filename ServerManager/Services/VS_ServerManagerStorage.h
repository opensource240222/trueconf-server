#pragma once

#include <vector>
#include <map>
#include <list>
#include <string>

#include "../../common/std/cpplib/VS_Protocol.h"
#include "../../common/std/cpplib/VS_Lock.h"
#include "../../common/std/cpplib/VS_SimpleStr.h"
#include "../../common/std/cpplib/VS_WideStr.h"
#include "../../ServerServices/VS_MediaBrokerStats.h"
#include "../../common/std/cpplib/VS_Endpoint.h"
#include "../../common/std/cpplib/VS_MapTpl.h"
#include "std-generic/cpplib/VS_Container.h"
#include "SecureLib/VS_Certificate.h"
#include "transport/VS_ServCertInfoInterface.h"

class VS_ServerManagerStorage:
	public VS_ServCertInfoInterface,
	public VS_Lock
{
public:
	class VS_ServerManager_ServerInfo
	{
	public:
		VS_SimpleStr	m_dns_name;
		long			m_type;
		long			m_status;

		long			m_autoVerify;

		VS_SimpleStr			m_domain;
		std::list<std::string>	m_domains;

		VS_SimpleStr	m_serv_key;



		unsigned long	m_redirected;

		unsigned long	m_online_users;
		unsigned long	m_max_users;

		/////X.509 Certificate//////
		VS_SimpleStr	m_serial;
		unsigned long	m_registerAllowed;
		VS_SimpleStr	m_certPEM;
		///////////////////////////

		long			mgr_cmd;
		long			mgr_cmd_param;


		VS_ServerManager_ServerInfo(): m_type(0), m_status(0),m_autoVerify(0), mgr_cmd(MSC_NONE), mgr_cmd_param(0), m_online_users(0), m_max_users(-1), m_redirected(0),m_registerAllowed(0)
		{	}
	};

//	friend class VS_ServerManagerService;		// for ping of servers

	VS_ServerManagerStorage();
	virtual ~VS_ServerManagerStorage();

	bool AddServer(const char* dns_name, const char* domain, const long type, const long status, const VS_SimpleStr serial, const char* cert,const char *ser_key,
					const unsigned long m_registerAllowed,
					const long mgr_cmd = MSC_NONE, const long mgr_cmd_param = 0,
					const unsigned long max_users = -1, const unsigned long redirected = false,
					const unsigned long tmp_online_users = 0, const unsigned long tmp_auto_verify = 0);

	void GetLocatorBS(VS_SimpleStr& str);
	void GetBSByDomain(const VS_SimpleStr& domain,VS_SimpleStr& b_srv);

	std::map<VS_SimpleStr, std::list<std::string>> GetAllBS();
	void GetAllRS(VS_Container &cnt);
	void GetRSByDomain(VS_Container &cnt, const char* domain);

	void GetRS(VS_SimpleStr& str);
	bool ServerStatus(const char* dns_name, const long status);
	void CheckChangeRegistry();
	bool IsRegisteredAS(const char* sid, VS_SimpleStr *domain = 0);

	bool RegisterServer(const char* server_id,const char *server_name, const char *serial, const char *hwkey, const char *cert_request, const char *version,const bool isSrvVerified,
						VS_SimpleStr &out_key, VS_SimpleStr &out_cert, int &regRes);
	bool UpdateServerCertificate(const char *server_name, const char *cert_request, VS_SimpleStr &out_cert, long &regRes);


	void GetServerStat(const char* sid, VS_AppServerStats& stats);
	void SetServerStat(const char* sid, VS_AppServerStats* stats);
	void SetServerStat_StartTime(const char* sid);

	void ResetManageCommandIndex();
	bool GetNextManageCommand(VS_SimpleStr &sid, long &mgr_cmd, long &mgr_cmd_param);

	void SetRedirected(const char* sid, const long flag);

	void GetOverLoadedServers(VST_StrIMap<long> &servers);
	void GetAllAS(string_view domain, std::vector<VS_ServerManager_ServerInfo> &vec);
	void GetAllAS(std::vector<VS_ServerManager_ServerInfo> &vec);
	VS_ServCertInfoInterface::get_info_res
		GetPublicKey( const VS_SimpleStr& server_name,
			VS_SimpleStr &pub_key, uint32_t &vcs_ver) override;
	VS_ServCertInfoInterface::get_info_res
		GetServerCertificate(const VS_SimpleStr &server_name,
			VS_SimpleStr &cert) override;

private:
	static const char* SERVERS_ROOT;
	static const char* SERVER_TYPE;
	static const char* SERVER_STATUS;

	static const char* SERVER_START_TIME;
	static const char* SERVER_LAST_ONLINE_TIME;
	static const char* SERVER_VERSION;
	static const char* SERVER_CPU_LOAD;
	static const char* SERVER_NUM_ENDPOINTS;
	static const char* SERVER_TRANSPORT_BITRATE_IN;
	static const char* SERVER_TRANSPORT_BITRATE_OUT;
	static const char* SERVER_NUM_STREAMS;
	static const char* SERVER_STREAMS_BITRATE_IN;
	static const char* SERVER_STREAMS_BITRATE_OUT;
	static const char* SERVER_ONLINE_USERS;
	static const char* SERVER_CONFS;
	static const char* SERVER_PARTICIPANTS;

	static const char* SERVER_SERIAL;
	static const char* SERVER_REGISTRATION_ALLOWED;
	static const char* SERVER_AUTO_VERIFY;

	static const char* SERVER_MANAGECOMMAND;
	static const char* SERVER_MANAGECOMMAND_PARAM;

	static const char* SERVER_REDIRECTED;
	static const char* SERVER_MAX_USERS;
	static const char* SERVER_DOMAIN;

	std::vector<VS_ServerManager_ServerInfo>	m_BS;

	std::vector<VS_ServerManager_ServerInfo>	m_RS;
	std::vector<VS_ServerManager_ServerInfo>	m_AS;
	std::map<VS_SimpleStr,std::list<std::string>>	m_bs_by_dns;
	std::map<VS_SimpleStr,std::list<std::string>>	m_domains_for_rs; //RS->domain list



	std::vector<VS_ServerManager_ServerInfo>::iterator	m_mgr_cmd_it;
	unsigned int										m_mgr_cmd_index;

	std::string m_last_write;

	bool UpdateServerStatusInVector(const char* dns_name, const long type, const long status);
	bool Refresh();
	void OnCreate_CleanUp();



};
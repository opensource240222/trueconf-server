#include "VS_CheckLicenseService.h"
#include "ldap_core/common/VS_RegABStorage.h"
#include "VS_RegistryStorage.h"
#include "VS_TRStorageSqlite.h"
#include "ProtectionLib/Protection.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/VS_RegServer.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "SecureLib/VS_SecureConstants.h"
#include "SecureLib/VS_Sign.h"
#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "SecureLib/VS_SecureHandshake.h"
#include "std/cpplib/VS_MemoryLeak.h"
#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"
#include "ServerServices/VS_ReadLicense.h"
#include "ServerServices/Common.h"
#include "ServerServices/VS_TorrentStarter.h"
#include "VS_LDAPStorage.h"
#include "TrueGateway/VS_GatewayStarter.h"
#include "net/EndpointRegistry.h"
#include "std/cpplib/netutils.h"
#include "TransceiverLib/TransceiversPoolInterface.h"
#include "VCS/version.h"
#include "std/cpplib/VS_JsonConverter.h"

#include <chrono>
#include <string>
#include <vector>



#include "ProtectionLib/OptimizeDisable.h"
#include "ServerServices/VS_TorrentStarterBase.h"
#include "std-generic/cpplib/ThreadUtils.h"

#define DEBUG_CURRENT_MODULE VS_DM_REGS
#define EMPTY_TIME std::chrono::system_clock::time_point()

const long long ILFE_TIME_WITHOUT_REG = 12*60*60*1000; // enh #23862

const unsigned int LIC_LIMIT_LOG_TIME = 24u * 60 * 60 * 1000;

VS_CheckLicenseService::VS_CheckLicenseService(boost::asio::io_service& ios, std::unique_ptr<VS_TorrentStarterBase> torrentStarter)
	: m_watchdog(nullptr)
	, m_recvMess(nullptr)
	, m_ios(ios)
	, m_tr(nullptr)
	, m_torrent_starter(std::move(torrentStarter))
	, m_reg_disconnect_tick(0)
	, m_state(SSTATE_NONE)
	, m_CurrFlags(VS_License::DEFAULT_FLAGS)
	, m_ReloadLicense(false)
	, m_licLimitCheckTime(0)
{
	m_TimeInterval = std::chrono::seconds(5);
	VS_RegistryKey    key(false, LICENSE_KEY,false);
	key.RemoveValue(SERVER_SHUTDOWN_TIME_TAG);
	VS_Container cnt;
	WriteLicDataToReg(cnt);
}
VS_CheckLicenseService::~VS_CheckLicenseService()
{
	VS_RegistryKey    key(false, LICENSE_KEY,false);
	key.RemoveValue(SERVER_SHUTDOWN_TIME_TAG);
}

/*static bool send_conn_info_to_reg_server(void)
{
	DWORD value = 0;
	VS_RegistryKey conn_key(false, CONFIGURATION_KEY);

	if (conn_key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, DDNS_NOTIFY_REG_ABOUT_IP) <= 0)
		return false;

	if (value != 0)
		return true;

	return false;
}*/

bool VS_CheckLicenseService::Init(const char* our_endpoint, const char* /*our_service*/, const bool /*permittedAll*/)
{
	bool res(false);
	{
NANOBEGIN2;
		m_my_id = our_endpoint;

		VS_License	lic_sum;
		if(p_licWrap)
			lic_sum = p_licWrap->GetLicSum();
		if(lic_sum.m_validuntil == EMPTY_TIME)
			m_LicExpPeriod=MaxLicExpPeriod;
		else
		{
			m_LicExpPeriod=static_cast<unsigned long>(std::chrono::duration_cast<std::chrono::milliseconds>(lic_sum.m_validuntil-std::chrono::system_clock::now()).count()) + LicExpAdd;
		}
		m_LicUpdPeriod = (m_LicExpPeriod>MaxLicExpPeriod?MaxLicExpPeriod:m_LicExpPeriod);// - LicUpdShift;
		m_LicExpTime=m_LicUpdTime=1;

#ifdef _SVKS_M_BUILD_
		// dont try connect to RegServer
		m_regSrv.Empty();
		m_ReloadLicense = true;

#ifdef _DEBUG
		m_CurrFlags|=VS_License::REG_CONNECTED;			// for developers: make reg_connected licenses work
#endif

		CallInProcessingThread( boost::bind(&VS_CheckLicenseService::OnTimer_RefreshLicense, this, 0) );
#else
		m_regSrv = RegServerName;
		VS_Container cnt;
		cnt.AddValue(METHOD_PARAM, LOGEVENT_METHOD);
		cnt.AddValueI32(TYPE_PARAM, BE_START);
		cnt.AddValue(FIELD1_PARAM, m_server_version);
		PostRequest(m_regSrv, 0, cnt, 0, REGISTRATION_SRV);

		// get server current connection information and send it to reg server.
		/*if (send_conn_info_to_reg_server())
		{
			VS_Container conn_cnt;
			VS_SimpleStr conn_string;
			int count;

			count = m_acs->GetListeners(conn_string);
			if (count > 0)
			{
				conn_cnt.AddValue(METHOD_PARAM, UPDATECONFIGURATION_METHOD);
				conn_cnt.AddValue(TYPE_PARAM, (long)BE_UPDATE_DDNS);
				conn_cnt.AddValue(FIELD1_PARAM, (char *)&conn_string.m_str[0]);
				PostRequest(m_regSrv, 0, conn_cnt, 0, REGISTRATION_SRV);
			}
		}*/
#endif
		m_state = SSTATE_RUNNING;
		SendLicensingInfo();
		res = true;
NANOEND2;
	}
	return res;
}

void VS_CheckLicenseService::Destroy(const char* /*our_endpoint*/, const char* /*our_service*/)
{
	if (g_dbStorage)
		g_dbStorage->CleanUp();
}

void VS_CheckLicenseService::AsyncDestroy()
{
	m_state = SSTATE_OFFLINE;
	if (!m_regSrv.IsEmpty())
	{
		char state_str[16] = {};
		sprintf(state_str, "%u", static_cast<unsigned int>(m_state));

		VS_Container cnt;
		cnt.AddValue(METHOD_PARAM, LOGEVENT_METHOD);
		cnt.AddValueI32(TYPE_PARAM, BE_SHUTDOWN);
		cnt.AddValue(FIELD1_PARAM, state_str);
		PostRequest(m_regSrv, 0, cnt, NULL, REGISTRATION_SRV);

#ifdef _WIN32   // VS_Server not ported yet
		// pause for a message
		VS_Server::PauseDestroy(500);
#endif
		vs::SleepFor(std::chrono::milliseconds(500));
	}
    if (m_torrent_starter && m_torrent_starter->IsStarted())
		m_torrent_starter->Stop(m_tr);
}

void VS_CheckLicenseService::SetComponents(VS_RoutersWatchdog* watchdog, transport::IRouter *tr, string_view acs_listeners_tcp, string_view ver, const std::weak_ptr<VS_TranscoderLogin>& transLogin)
{
	m_watchdog = watchdog;
	m_server_version = (std::string)ver;
	m_tr = tr;
	m_acs_listeners_tcp = (std::string) acs_listeners_tcp;
	m_transLogin = transLogin;
}

void VS_CheckLicenseService::SetTransceiversPool(const std::shared_ptr<ts::IPool>& pool)
{
	m_transceiversPool = pool;
}

void VS_CheckLicenseService::SendLicensingInfo()
{
//NANOBEGIN;
	if(m_regSrv)
	{
		dprint3("updating licenses\n");
		VS_Container	cnt;
		cnt.AddValue(METHOD_PARAM, UPDATELICENSE_METHOD);
		if(p_licWrap)
			p_licWrap->PrepareDataForUpdateLic(cnt);
		cnt.AddValueI32("arm_hw_key", VS_ArmReadHardwareKey());
		PostRequest(m_regSrv, 0, cnt, ARM_LICENSE_VERSION, REGISTRATION_SRV);
	}
//NANOEND;
}
bool VS_CheckLicenseService::Processing(std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)
		return true;
	VS_Container	cnt;
	m_recvMess = recvMess.get();

	switch(recvMess->Type())
	{
	case transport::MessageType::Reply:
	case transport::MessageType::Request:
		if (cnt.Deserialize(m_recvMess->Body(), m_recvMess->BodySize()))
		{
			const char* method = 0;
			if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0)
			{
				if (strcasecmp(method, UPDATELICENSE_METHOD) == 0)
				{
					GetLicensingInfo(cnt);
				}
				else if(strcasecmp(method,"$UCA*ZA^") == 0) //  VS_ReadLicense
				{
					WriteLicDataToReg(cnt);
				}
				else if(strcasecmp(method,SERVERVERIFYFAILED_METHOD) == 0)
				{
					ServerVerificationFailed();
				}
				else if (strcasecmp(method, EXTERNAL_IP_DETERMINED_METHOD) == 0) {
					auto ip = cnt.GetStrValueRef(IPCONFIG_PARAM);
					if (ip && *ip)
					{
						ExternalIPDetermined_Handler(ip);
					}
				}
			}
		}
		break;
	}
	m_recvMess = nullptr;
	return true;
}

bool VS_CheckLicenseService::GetArmKeyFromCnt(VS_Container &cnt,VS_SimpleStr &arm_key, uint32_t &licFeatures)
{
	VS_PKeyCrypt	decrypt;
	VS_PKey			priv_key;

	int	private_key_len(0);
	std::unique_ptr<char, free_deleter> private_key_buf;
	unsigned char* cnt_buf(0);
	uint32_t cnt_buf_len(0);
	VS_Container arm_cnt;
	if (!cnt.GetValue("ak", arm_cnt))
		return false;


	VS_RegistryKey    cfg(false, CONFIGURATION_KEY);
	if(!(private_key_len = cfg.GetValue(private_key_buf,VS_REG_BINARY_VT, SRV_PRIVATE_KEY))||
		!priv_key.SetPrivateKey(private_key_buf.get(), store_PEM_BUF) ||
		!decrypt.SetPrivateKey(&priv_key))
	{
		return false;
	}

	size_t sess_key_ln(0);
	const unsigned char *sess_key = (const unsigned char*)arm_cnt.GetBinValueRef("",sess_key_ln);
	size_t iv_ln(0);
	const unsigned char	*iv = (const unsigned char *)arm_cnt.GetBinValueRef("@",iv_ln);
	size_t encr_data_ln(0);
	const unsigned char * encr_data = (const unsigned char *)arm_cnt.GetBinValueRef("&",encr_data_ln);
	decrypt.Decrypt(encr_data,encr_data_ln,alg_sym_AES256,iv,sess_key,sess_key_ln,nullptr,&cnt_buf_len);
	cnt_buf = new unsigned char[cnt_buf_len];
	VS_Container decr_cnt;
	if(!decrypt.Decrypt(encr_data,encr_data_ln,alg_sym_AES256,iv,sess_key,sess_key_ln,cnt_buf,&cnt_buf_len)||
		!decr_cnt.Deserialize((const void*)cnt_buf,cnt_buf_len))
	{
		delete [] cnt_buf;
		cnt_buf =0;
		return false;
	}
	delete [] cnt_buf;
	cnt_buf = 0;
	const char * arm = decr_cnt.GetStrValueRef("");
	size_t sign_sz(0);
	const void * sign = decr_cnt.GetBinValueRef(" ",sign_sz);
	if(!arm || !VerifyDataSign((unsigned char*)arm,strlen(arm),(unsigned char*)sign,sign_sz))
		return false;
	arm_key = arm;
	decr_cnt.GetValueI32("f", licFeatures);
	return true;
}

bool VS_CheckLicenseService::VerifyDataSign(const unsigned char *data, const unsigned long data_sz,
											const unsigned char *sign, const unsigned long sign_sz)
{
	VS_RegistryKey key(false, CONFIGURATION_KEY);

	std::unique_ptr<void, free_deleter> cert_chain_buf;
	uint32_t buf_sz = key.GetValue(cert_chain_buf,VS_REG_BINARY_VT, SRV_CERT_CHAIN_KEY);
	VS_Container cert_chain_cnt;
	const char *parent_cert(0);
	if(buf_sz)
	{
		cert_chain_cnt.Deserialize(cert_chain_buf.get(), buf_sz);
		parent_cert = cert_chain_cnt.GetStrValueRef(PARENT_CERT_PARAM);
	}


	VS_GET_PEM_CACERT
	if(parent_cert)
	{
		VS_CertificateCheck	certCheck;
		if(!certCheck.SetCert(parent_cert,strlen(parent_cert)+1,store_PEM_BUF) || !certCheck.SetCertToChain(PEM_CACERT,strlen(PEM_CACERT) +1, store_PEM_BUF))
		{
			return false;
		}
		cert_chain_cnt.Reset();
		if(cert_chain_cnt.IsValid())
		{
			while(cert_chain_cnt.Next())
			{
				if(!!cert_chain_cnt.GetName() && (0 == strcasecmp(cert_chain_cnt.GetName(),CERTIFICATE_CHAIN_PARAM)))
				{
					size_t sz(0);
					const char *cert_in_chain = (const char*)cert_chain_cnt.GetBinValueRef(sz);
					if(sz && cert_in_chain)
						certCheck.SetCertToChain(cert_in_chain,sz,store_PEM_BUF);
				}
			}
		}
		if(!certCheck.VerifyCert())
		{
			return false;
		}

	}


	VS_Sign					verifier;
	VS_SignArg				signarg = {alg_pk_RSA,alg_hsh_SHA1};



	VS_Certificate	cert;
	VS_PKey	cert_public_key;
	buf_sz = 0;
	char *pem_buf(0);
	if(!data || !data_sz || !sign || !sign_sz)
		return false;

	if(!verifier.Init(signarg)||
		!cert.SetCert(!!parent_cert?parent_cert:PEM_CACERT,strlen(!!parent_cert?parent_cert:PEM_CACERT) + 1,store_PEM_BUF)||!cert.GetCertPublicKey(&cert_public_key))
		return false;

	cert_public_key.GetPublicKey(store_PEM_BUF,0,&buf_sz);
	pem_buf = new char[buf_sz];
	if(!cert_public_key.GetPublicKey(store_PEM_BUF,pem_buf,&buf_sz))
	{
		delete [] pem_buf;
		return false;
	}
	if(!verifier.SetPublicKey(pem_buf,buf_sz,store_PEM_BUF))
	{
		delete [] pem_buf;
		return false;
	}
	delete [] pem_buf;
	if(1!=verifier.VerifySign((const unsigned char*)data,data_sz,sign,sign_sz))
		return false;
	else
		return true;
}

void VS_CheckLicenseService::GetLicensingInfo(VS_Container &cnt)
{
//NANOBEGIN;
	if(m_regSrv != m_recvMess->SrcServer())
	{
		dprint1("Licensing info arrived from wrong source\n");
		return;
	}
	if(!cnt.IsValid())
	{
		dprint1("Container is not valid\n");
		return;
	}
	dprint3("license info arrived\n");
	uint64_t id = 0;
	int32_t type = 0;
	const void* data = nullptr;
	size_t size = 0;
	int counter=0;

	VS_RegistryKey l_root(false, LICENSE_KEY, false, false);
	int32_t r;
	cnt.GetValue(RESULT_PARAM,r);
	while(cnt.Next())
	{
		if(strcasecmp(cnt.GetName(),NAME_PARAM)==0)
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
		else if(strcasecmp(cnt.GetName(),TYPE_PARAM)==0)
		{	cnt.GetValue(type); }
		else if(strcasecmp(cnt.GetName(),DATA_PARAM)==0)
		{ data=cnt.GetBinValueRef(size); }
	};

	if(id!=0)
	{
		ProcessLicense(l_root,id,type,data,size);
		counter++;
	}
	VS_SimpleStr arm_key;
	if(!GetArmKeyFromCnt(cnt,arm_key,m_licFeatures))
	{
		dprint3("arm_key is absent!\n");
	}

	if (!VS_ArmInstallKey(arm_key)) {
		VS_ArmSetDefaultKey();
		m_licFeatures = 0;
		dprint3("license info update failed (arm_key not installed arm_key = %s)\n", arm_key.m_str);
	}
	else
	{
		size_t ak_sz(0);
		m_arm_key_cnt.Clear();
		const void *ak_buf = cnt.GetBinValueRef("ak",ak_sz);
		m_arm_key_cnt.AddValue("ak",ak_buf,ak_sz);
	}
	unsigned int old_flags=m_CurrFlags;
	m_CurrFlags|=VS_License::REG_CONNECTED;

	if(m_CurrFlags!=old_flags || counter>0)
		m_ReloadLicense=true;

	if (m_lastLicRefresh == decltype(m_lastLicRefresh)())
		OnTimer_RefreshLicense(0);
//NANOEND;
}

bool VS_CheckLicenseService::Timer( unsigned long tickcount)
{
	OnTimer_RefreshLicense(tickcount);
	return true;
}

void VS_CheckLicenseService::OnTimer_RefreshLicense(unsigned long ticks)
{
	static unsigned reg_reconnect_count = 0;
	/*if(m_LicExpTime==0)
		return;*/
NANOBEGIN2;
	if ( ticks-m_LicUpdTime > m_LicUpdPeriod )
	{
		SendLicensingInfo();
		m_LicUpdTime=ticks;
		m_LicUpdPeriod=MaxLicExpPeriod;
	}
	if(m_reg_disconnect_tick == 1)
	{
		m_reg_disconnect_tick = ticks;
		reg_reconnect_count = 1;

		if(!!p_licWrap&&p_licWrap->IsConnectRequired())
		{
			auto shutdown_date = std::chrono::system_clock::now() + std::chrono::milliseconds(ILFE_TIME_WITHOUT_REG);
			auto seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(shutdown_date.time_since_epoch()).count();
			VS_RegistryKey key(false, LICENSE_KEY, false, true);
			key.SetValue(static_cast<void*>(&seconds_since_epoch), sizeof(seconds_since_epoch), VS_REG_INT64_VT, SERVER_SHUTDOWN_TIME_TAG);
		}
	}
	else if(m_reg_disconnect_tick == 0)
		reg_reconnect_count = 0;
	else if( (m_reg_disconnect_tick>0) && (ticks-m_reg_disconnect_tick>ILFE_TIME_WITHOUT_REG))		// timeout working without connection to regserver
	{
		reg_reconnect_count = 0;
		m_ReloadLicense = true;
	}
	if( (m_reg_disconnect_tick>0) && (reg_reconnect_count>0) &&
		(ticks-m_reg_disconnect_tick>reg_reconnect_count * 60 * 1000))
	{
		SendLicensingInfo();
		reg_reconnect_count++;
		m_LicUpdTime=ticks;
	}
	bool NeedConnect = (p_licWrap && p_licWrap->IsConnectRequired());
	if((m_lastLicRefresh == decltype(m_lastLicRefresh)()) && (!NeedConnect || ticks > 60000))
	{
		dprint3("Waiting for reg connect is more then 60 secs\n");
		m_ReloadLicense = true;
	}
NANOEND2;

	if ( ticks-m_LicExpTime > m_LicExpPeriod || m_ReloadLicense)
	{
		VS_License	lic_sum;
		dprint1("Refreshing licenses\n");
		VS_Container	cnt;

		if(!VS_ReadLicense(m_my_id,(VS_License::Flags)m_CurrFlags, cnt))
		{
			VS_SimpleStr err_lic = cnt.GetStrValueRef(RESULT_PARAM);
			VS_RegistryKey rKey(false, LICENSE_KEY_NAME, false, true);
			if(!err_lic)
				rKey.SetString("Unknown", "Error License");
			else
				rKey.SetString(err_lic, "Error License");
			VS_ArmSetDefaultKey();
			dprint0("license check failed, terminating\n");
			if (m_watchdog) {
				m_watchdog->Shutdown();
				m_registrationEvent.set();
			} else {
				m_registrationEvent.set();
				throw "terminated by licensing";
			};
		}
		else
		{
			if(p_licWrap)
				lic_sum = p_licWrap->GetLicSum();
			if (p_licWrap && !p_licWrap->IsConnectRequired())
			{
NANOBEGIN2;
				if(m_arm_key_cnt.IsValid())
				{
					size_t cnt_sz(0);
					const void * cnt_buf = m_arm_key_cnt.GetBinValueRef("ak",cnt_sz);
					VS_RegistryKey    cfg(false, CONFIGURATION_KEY, false, true);
					cfg.SetValue(cnt_buf,cnt_sz,VS_REG_BINARY_VT,"ak");
					m_arm_key_cnt.Clear();
				}
				else
				{
					VS_RegistryKey    cfg(false, CONFIGURATION_KEY);
					VS_Container arm_cnt;
					std::unique_ptr<void, free_deleter> cnt_buf;
					int cnt_buf_len(0);
					VS_SimpleStr	arm_key;
					if(!(cnt_buf_len = cfg.GetValue(cnt_buf,VS_REG_BINARY_VT,"ak")) ||
						!arm_cnt.AddValue("ak", cnt_buf.get(), cnt_buf_len) || !GetArmKeyFromCnt(arm_cnt, arm_key, m_licFeatures))
						m_licFeatures = 0;

					if(!arm_key ||!VS_ArmInstallKey(arm_key))
					{
						VS_ArmSetDefaultKey();
						m_licFeatures = 0;
						dprint0("arm key from regestry not installed!\n");
					}
				}
NANOEND2;
			}
			else
				m_arm_key_cnt.Clear();
			if (!VS_ArmCheckFeatures(m_licFeatures))
			{
				dprint3("restart by features\n");
				m_watchdog->Restart();
			}
			dprint4("Send cnt with licdata\n");
			WriteLicDataToReg(cnt);
			if (!lic_sum.m_max_guests && !lic_sum.m_onlineusers && (p_licWrap&&!p_licWrap->IsSlave()))
			{
				dprint0("license check failed, terminating. (conferences||onlineusers == 0)\n");
				if (m_watchdog) {
					m_watchdog->Shutdown();
					m_registrationEvent.set();
				} else {
					m_registrationEvent.set();
					throw "terminated by licensing";
				}
			}

			VS_SecureHandshake::SetImprovedSecurityState(VS_CheckLicense(LE_IMPROVED_SECURITY));

			if(!g_dbStorage)
			{
				bool ldapIsAllowed = VS_CheckLicense(LE_LDAP_ALLOWED);
				VS_SimpleStr storage_name(256);
				VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);
				cfg_root.GetValue(storage_name.m_str,256,VS_REG_STRING_VT,STORAGE_TYPE_KEY_NAME);
SECUREBEGIN_A_FULL;
				if(ldapIsAllowed&&(storage_name==STORAGE_TYPE_LDAP))
				{
					dprint3( "\tLDAP Storage \n" );
					auto ldapStorage = std::make_shared<VS_LDAPStorage>(m_ios, OurEndpoint(), VS_CheckLicense(LE_USER_GROUPS_ALLOWED), m_transLogin);
					ldapStorage->SetPresenceService(m_presenceService);
					g_dbStorage = ldapStorage;
					if (g_dbStorage->error_code )
					{
						dprint0( "failed with code %d\n",g_dbStorage->error_code);
						if (g_dbStorage->error_code == VSS_LDAP_CONNECT_SERVER_ERROR)
							m_watchdog->Restart();
						else
							m_watchdog->Shutdown();
						g_dbStorage.reset();
					}
					else if (!m_watchdog->AddTestable(g_dbStorage.get(), 12)) {
						dprint0("AddTestable storage failed\n");
						m_watchdog->Shutdown();
						g_dbStorage.reset();
					}
				}
				else
				{
					dprint3( "\tRegistry Storage \n" );

					g_dbStorage = std::make_shared<VS_RegistryStorage>(new VS_RegABStorage(), VS_CheckLicense(LE_USER_GROUPS_ALLOWED), OurEndpoint(), m_transLogin);

					if(g_dbStorage->error_code)
					{
						dprint0( "failed with code %d\n",g_dbStorage->error_code);
						m_watchdog->Shutdown();
						g_dbStorage.reset();
					}
					else if (!m_watchdog->AddTestable(g_dbStorage.get(), 12)) {
						dprint0("AddTestable storage failed\n");
						m_watchdog->Shutdown();
						g_dbStorage.reset();
					}
				}
SECUREEND_A_FULL;
				m_registrationEvent.set();
			}

			if (!m_watchdog->IsShutdown())		// Init MultiGW only if g_dbStorage inited ok (crash bug#11966)
			{
SECUREBEGIN_F_MGATEWAY
				VS_GatewayStarter *gw_starter = VS_GatewayStarter::GetInstance();
				if(gw_starter)
				{
					if (VS_CheckLicense(LE_MULTIGATEWAY) &&
						((lic_sum.m_gateways > 0 || lic_sum.TC_INFINITY == lic_sum.m_gateways) ||	// by_amount
						(p_licWrap && p_licWrap->IsSlave())))										// by slave flag
					{
						const std::string serverInfo = VS_TRUECONF_WS_DISPLAY_NAME " " STRPRODUCTVER;
						auto &&checkDigest = [this](const std::string&login, const std::string&pass) {return CheckDigest(login, pass); };
						std::function<bool(const std::string&, const std::string&)> functor(std::bind(&VS_CheckLicenseService::CheckDigest, this, std::placeholders::_1, std::placeholders::_2));
						if (!gw_starter->IsStarted()) {
							assert(m_transceiversPool.lock() != nullptr);	// transceivers pool must be set for this moment
							assert(m_transLogin.lock() != nullptr);
							auto initStatus = gw_starter->StartGateway(m_ios, m_tr, std::move(checkDigest), m_transceiversPool, m_transLogin, serverInfo);
							NotifyOnGWStartError(initStatus);
						}
						int max_gateways = p_licWrap->IsSlave() ? INT_MAX : lic_sum.m_gateways;	// allow slave to stretch gateways with LoginUser
						gw_starter->SetMaxTranscoders(max_gateways);

						if (!m_acs_listeners_tcp.empty())
							gw_starter->SetServerAddresses(m_acs_listeners_tcp.c_str());
					}
					else{
						if(gw_starter->IsStarted())
							gw_starter->StopGateway();
					}
				}
SECUREEND_F_MGATEWAY

				const char SQLITE_FILE[] = "files_download_stats.sqlite";

				if (m_torrent_starter)
				{
					if (VS_CheckLicense(LE_FILETRANSFER)) {
						if (!m_torrent_starter->IsStarted()) {
							std::shared_ptr<VS_TRStorageInterface> db_storage;
							try {
								db_storage = std::make_shared<VS_TRStorageSqlite>(VS_TorrentStarterBase::FileStorageDir(true) + SQLITE_FILE);
							} catch (std::runtime_error &e) {
								dprint0("Failed to create sqlite storage: %s\n", e.what());
							}
							m_torrent_starter->Start(m_tr, db_storage);
						}
					} else {
						if (m_torrent_starter->IsStarted()) {
							m_torrent_starter->Stop(m_tr);
						}
					}
				}
		}
		}

NANOBEGIN2;
		if(p_licWrap)
			lic_sum = p_licWrap->GetLicSum();
		if(lic_sum.m_validuntil == EMPTY_TIME)
			m_LicExpPeriod=MaxLicExpPeriod+LicExpAdd;
		else {
			m_LicExpPeriod = static_cast<unsigned long>(std::chrono::duration_cast<std::chrono::milliseconds>(lic_sum.m_validuntil - std::chrono::system_clock::now()).count()) + LicExpAdd;
		};
		m_lastLicRefresh = std::chrono::steady_clock::now();

		if(m_regSrv != 0)
		{
			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, LOGEVENT_METHOD);
			cnt.AddValueI32(TYPE_PARAM, BE_LICENSE_RELOAD);

			char buf[256] ={0};
			sprintf(buf,"lic: %dU/%dC/%dS/%dTP/%dG/%dGW/%dT R=%" PRIX64,lic_sum.m_onlineusers,lic_sum.m_conferences,lic_sum.m_symmetric_participants,
				lic_sum.m_terminal_pro_users,lic_sum.m_max_guests,lic_sum.m_gateways,lic_sum.m_trial_conf_minutes,lic_sum.m_restrict);
			cnt.AddValue(FIELD1_PARAM, (char*)buf);

			if(!PostRequest(m_regSrv,0,cnt,NULL,REGISTRATION_SRV))
				dprint0("\t\t log license reload failed\n");
		};

		m_LicExpTime=ticks;
		m_LicUpdTime=ticks;

        m_LicUpdPeriod = (m_LicExpPeriod > MaxLicExpPeriod + LicExpAdd ? MaxLicExpPeriod + LicExpAdd : m_LicExpPeriod) - LicUpdShift - LicExpAdd;
		m_reg_disconnect_tick = 0;
		m_ReloadLicense=false;
NANOEND2;
	}

	if (m_regSrv && p_licWrap && (ticks - m_licLimitCheckTime > LIC_LIMIT_LOG_TIME)) {
		m_licLimitCheckTime = ticks;

		unsigned user	= p_licWrap->GetFailCount(LE_LOGIN),
				 conf	= p_licWrap->GetFailCount(LE_NEWCONFERENCE),
				 gw		= p_licWrap->GetFailCount(LE_GATEWAYLOGIN),
				 guest	= p_licWrap->GetFailCount(LE_GUEST_LOGIN);

		if (user || conf || gw || guest) {
			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, LOGEVENT_METHOD);
			cnt.AddValueI32(TYPE_PARAM, BE_LICENSE_LIMIT);

			char buf[256] = { 0 };
			sprintf(buf, "users:%u, gw:%u, guests:%u, conf:%u", user, gw, guest, conf);
			cnt.AddValue(FIELD1_PARAM, (char*)buf);

			if (!PostRequest(m_regSrv, 0, cnt, NULL, REGISTRATION_SRV)) {
				dprint0("\t\t log license limit failed\n");
			}
		}
	}
}

void VS_CheckLicenseService::ProcessLicense(VS_RegistryKey& l_root,uint64_t id,long type,const void* data,int size)
{
NANOBEGIN2;
	switch(type)
	{
	case LIC_DELETE:
		if(p_licWrap)
		{
			dprint3("Deleting license %016" PRIX64 "\n", id);
			std::string lic_key;
			if(!p_licWrap->GetLicenseKey(id,lic_key))
			{
				dprint1("Erroneous delete license request from reg srv\n");
			}
			else if(!l_root.RemoveKey(lic_key))
			{
				dprint1("Can't delete key\n");
				VS_RegistryKey lic_err_Key(false, LICENSE_KEY_NAME, false, true);
				lic_err_Key.SetString("Can't delete License", "Error");
				m_watchdog->Shutdown();
				// todo(kt): second time? exception
				m_registrationEvent.set();
			}
		}
		break;
	case LIC_ADD:
		{
			dprint3("Adding license %016" PRIX64 "\n", id);
			if(data==NULL || size==0)
			{
				dprint1("Erroneous add license request from reg srv\n");
				break;
			}

			VS_License lic(data,size);
			if (!lic.IsValid())
				dprint1("Erroneous license arrived from reg srv\n");

			char lic_name[128];

			sprintf(lic_name, "%s\\%016" PRIX64, LICENSE_KEY, lic.m_id);

			//VS_RegistryKey	lic_key(VS_Server::RegistryKey(),false, lic_name, false, true);
			VS_RegistryKey	lic_key(false, lic_name, false, true);
			if (!lic_key.IsValid())
				break;

			lic_key.SetValue(data, size, VS_REG_BINARY_VT, LICENSE_DATA_TAG);
		}
		break;
	}
NANOEND2;
}

void VS_CheckLicenseService::WriteLicDataToReg(VS_Container &cnt)
{
	VS_RegistryKey	l_root(false, LICENSE_KEY, false, true);
NANOBEGIN2;

	dprint4("WriteLicData\n");
	VS_RegistryKey rKey(false, LICENSE_KEY_NAME, false, true);
	rKey.SetString("Unknown", "Error License");
	int zero_val = 0;
	l_root.SetValue(&zero_val, sizeof(int), VS_REG_INTEGER_VT, LIC_CONFERENCES_TAG);
	l_root.SetValue(&zero_val, sizeof(int), VS_REG_INTEGER_VT, LIC_ONLINE_USERS_TAG);
	l_root.SetValue(&zero_val, sizeof(int), VS_REG_INTEGER_VT, LIC_TERMINAL_PRO_USERS_TAG);
	l_root.SetValue(&zero_val, sizeof(int), VS_REG_INTEGER_VT, LIC_SYMMETRIC_PARTICIPANTS_TAG);
	l_root.SetValue(&zero_val, sizeof(int), VS_REG_INTEGER_VT, LIC_MAX_GUESTS_TAG);
	l_root.SetValue(&zero_val, sizeof(int), VS_REG_INTEGER_VT, LIC_GATEWAYS_TAG);
	l_root.SetValue(&zero_val, sizeof(int), VS_REG_INTEGER_VT, LIC_RESTRICTION_TAG);
	l_root.SetValue(&zero_val, sizeof(int), VS_REG_INTEGER_VT, LIC_LIMITED_TAG);
	l_root.SetValue(&zero_val, sizeof(int), VS_REG_INTEGER_VT, LIC_TRIAL_TIME_TAG);
	l_root.SetString("", LIC_VALID_UNTIL_TAG);
	l_root.SetString("", LIC_RELOAD_DATE_TAG);

	int32_t limited(0);
	if(cnt.GetValue("limited",limited))
	{
		l_root.SetValue(&limited, sizeof(int), VS_REG_INTEGER_VT, LIC_LIMITED_TAG);
		dprint4("\tl = %d\n",limited);
	}

	if(cnt.GetStrValueRef(RESULT_PARAM))
	{
		auto errStr = cnt.GetStrValueRef(RESULT_PARAM);
		rKey.SetString(errStr, "Error License");
		dprint4("\tel = %s\n", errStr);
	}
NANOEND2;

	size_t sz(0);
	auto lic_buf = cnt.GetBinValueRef(DATA_PARAM,sz);
	VS_License l;
	if (lic_buf && l.Deserialize(lic_buf, sz)) {
		dprint4("lic is exist. sz = %zu\n", sz);
		l.SaveToRegistry(l_root);	// has own NANOBEGIN2 NANOEND2

		char valid_until[128] = { 0 };
		std::chrono::system_clock::time_point time;
		const auto res = cnt.GetValue(VALID_UNTIL_PARAM, time);
		if (res)
		{
			if (time != EMPTY_TIME && tu::TimeToNStr(time, valid_until, sizeof(valid_until)) > 0)
			{
				l_root.SetString(valid_until, LIC_VALID_UNTIL_TAG);
				dprint4("\tvu = %s\n", valid_until);
			}
			else if (time == EMPTY_TIME)
			{
				l_root.SetString("forever", LIC_VALID_UNTIL_TAG);
				dprint4("\trd = %s\n", "forever");
			}
		}
	}
}

void LogEvent(string_view object_type, string_view object_name, string_view event_type, string_view message, VS_TransportRouterServiceHelper& transport, const std::string& toServer) {
	dstream0 << "Send log event to admin: type=" << object_type << ", name=" << object_name << ", event=" << event_type << ", msg=" << message;
	VS_Container msg;
	msg.AddValue("Message", message);
	std::string payload = ConvertToJsonStr(msg);

	VS_Container log_cnt;
	log_cnt.AddValue(OBJECT_TYPE_PARAM, object_type);
	log_cnt.AddValue(OBJECT_NAME_PARAM, object_name);
	log_cnt.AddValue(EVENT_TYPE_PARAM, event_type);
	log_cnt.AddValue(PAYLOAD_PARAM, payload);
	transport.PostRequest(toServer.c_str(), 0, log_cnt, 0, LOG_SRV);
}

void VS_CheckLicenseService::NotifyOnGWStartError(GWInitStatus status)
{
	if (status == GWInitStatus::started)
		return;	// no error
	if ((unsigned)(status & GWInitStatus::sipBinded) == 0)
		LogEvent(SERVER_OBJECT_TYPE, OurEndpoint(), "Gateway Error", "Failed to bind ports for SIP.\n", *this, OurEndpoint());
	if ((unsigned)(status & GWInitStatus::h323Binded) == 0)
		LogEvent(SERVER_OBJECT_TYPE, OurEndpoint(), "Gateway Error", "Failed to bind ports for H323.\n", *this, OurEndpoint());
}
void VS_CheckLicenseService::ServerVerificationFailed()
{
NANOBEGIN2;
	/**
	*   
	*   
	*    
	*   
	**/
	VS_RegistryKey	l_root(false, LICENSE_KEY, false, true);
	VS_RegistryKey	l_reg;
	if(l_root.IsValid())
	{
		std::vector<std::string> to_delete;
		l_root.ResetKey();
		while (l_root.NextKey(l_reg))
			to_delete.emplace_back(l_reg.GetName());
		for (const auto& x: to_delete)
			l_root.RemoveKey(x);
	}
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	rkey.RemoveValue(SRV_CERT_KEY);
	rkey.RemoveValue(SRV_PRIVATE_KEY);
	m_watchdog->Shutdown();
	// todo(kt): second time? exception
	m_registrationEvent.set();
NANOEND2;
}

bool VS_CheckLicenseService::WaitForRegistrationEvent()
{
	m_registrationEvent.wait();
	return !m_watchdog->IsShutdown();
}

bool VS_CheckLicenseService::OnPointConnected_Event(const VS_PointParams *prm)
{
	dprint4("OnPointConnected_Event uid=%s, cid=%s, res = %d\n", prm->uid, prm->cid, prm->reazon);
	if(m_lastLicRefresh == decltype(m_lastLicRefresh)())
	{
		if(prm && (m_regSrv  == prm->uid) && (prm->reazon<=0))
			m_ReloadLicense=true;
		return true;
	}
	if(prm && m_regSrv == prm->uid && (prm->reazon>0))
	{
		dprint3("RegServer connected;\n");
		m_reg_disconnect_tick = 0;
		if(!(m_CurrFlags&VS_License::REG_CONNECTED))
			SendLicensingInfo();
		VS_RegistryKey    key(false, LICENSE_KEY,false);
		key.RemoveValue(SERVER_SHUTDOWN_TIME_TAG);
	}
	return true;
}

bool VS_CheckLicenseService::OnPointDisconnected_Event(const VS_PointParams *prm)
{
	dprint4("OnPointDisconnected_Event uid=%s, cid=%s, res = %d\n", prm->uid, prm->cid, prm->reazon);
	if(prm && (m_regSrv == prm->uid))
	{
		dprint3("RegServer disconnected;\n");
		m_reg_disconnect_tick = 1;
		/**
			 ,  5   reloadLic
		*/
		m_CurrFlags&=~VS_License::REG_CONNECTED;
	}
	return true;
}

static bool NotifyRegAboutIP(void)
{
	int32_t value = 0;
	VS_RegistryKey conn_key(false, CONFIGURATION_KEY);

	if (conn_key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, DDNS_NOTIFY_REG_ABOUT_IP) <= 0)
		return false;

	if (value != 0)
		return true;

	return false;
}

static bool NotifyRegWithAcceptTCP(void)
{
	int32_t value = 0;
	VS_RegistryKey conn_key(false, CONFIGURATION_KEY);

	if (conn_key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, DDNS_NOTIFY_REG_WITH_ACCEPT_TCP) <= 0)
		return false;

	if (value != 0)
		return true;

	return false;
}

void VS_CheckLicenseService::ExternalIPDetermined_Handler(const char* ip)
{
	if (!NotifyRegAboutIP())
		return;
	VS_Container cnt;
	std::string conn_info;

	if (!NotifyRegWithAcceptTCP())
	{
		if (!netutils::IsPrivateAddress_IPv4(ip))
		{
			conn_info += ip;
			conn_info += ":4307";
		}
	}
	else
	{
		const unsigned n_atcp = net::endpoint::GetCountAcceptTCP(OurEndpoint(), true);
		for (unsigned i = 0; i < n_atcp; ++i)
		{
			auto accept_tcp = net::endpoint::ReadAcceptTCP(i + 1, OurEndpoint(), true);

			if (netutils::IsPrivateAddress_IPv4(accept_tcp->host.c_str()))
				continue;

			conn_info += accept_tcp->host;
			conn_info += ":";
			conn_info += std::to_string(accept_tcp->port);

			if (i != n_atcp - 1)
				conn_info += ",";
		}
	}

	if (conn_info.empty()) // nothing to send so far
		return;

	cnt.AddValue(METHOD_PARAM, UPDATECONFIGURATION_METHOD);
	cnt.AddValueI32(TYPE_PARAM, BE_UPDATE_DDNS);
	cnt.AddValue(FIELD1_PARAM, conn_info.c_str());

	PostRequest(m_regSrv, 0, cnt, 0, REGISTRATION_SRV);
}


bool VS_CheckLicenseService::CheckDigest(const std::string& login, const std::string& pass)
{
	auto dbStorage = g_dbStorage;
	if (!dbStorage)
		return false;
	return dbStorage->CheckDigestByRegistry(login.c_str(), pass.c_str());
}

std::weak_ptr<VS_TorrentStarterBase> VS_CheckLicenseService::GetTorrentStarter() const
{
	return m_torrent_starter;
}

#include "ProtectionLib/OptimizeEnable.h"

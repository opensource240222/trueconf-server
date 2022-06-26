#include "VS_ServerManagerStorage.h"
#include "../../common/std/cpplib/VS_Protocol.h"
#include "../../common/std/debuglog/VS_Debug.h"
#include "../../common/std/cpplib/VS_IntConv.h"
#include "../../common/std/cpplib/VS_Utils.h"
#include "std/cpplib/md5.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "SecureLib/VS_CertificateIssue.h"
#include "SecureLib/VS_SecureConstants.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "../../common/std/cpplib/VS_FileTime.h"
#include <time.h>
#include <chrono>



#define DEBUG_CURRENT_MODULE VS_DM_CLUSTS

const char* VS_ServerManagerStorage::SERVERS_ROOT = "Servers\\";
const char* VS_ServerManagerStorage::SERVER_TYPE = "Type";
const char* VS_ServerManagerStorage::SERVER_STATUS = "Status";

const char* VS_ServerManagerStorage::SERVER_START_TIME				= "StartTime";
const char* VS_ServerManagerStorage::SERVER_LAST_ONLINE_TIME		= "LastOnlineTime";
const char* VS_ServerManagerStorage::SERVER_VERSION					= "Version";
const char* VS_ServerManagerStorage::SERVER_CPU_LOAD				= "CPU Load";
const char* VS_ServerManagerStorage::SERVER_NUM_ENDPOINTS			= "NumEndpoints";
const char* VS_ServerManagerStorage::SERVER_TRANSPORT_BITRATE_IN	= "TransportBitrateIn";
const char* VS_ServerManagerStorage::SERVER_TRANSPORT_BITRATE_OUT	= "TransportBitrateOut";
const char* VS_ServerManagerStorage::SERVER_NUM_STREAMS				= "NumStreams";
const char* VS_ServerManagerStorage::SERVER_STREAMS_BITRATE_IN		= "StreamsBitrateIn";
const char* VS_ServerManagerStorage::SERVER_STREAMS_BITRATE_OUT		= "StreamsBitrateOut";
const char* VS_ServerManagerStorage::SERVER_ONLINE_USERS			= "OnlineUsers";
const char* VS_ServerManagerStorage::SERVER_CONFS					= "Conferences";
const char* VS_ServerManagerStorage::SERVER_PARTICIPANTS			= "Participants";
const char* VS_ServerManagerStorage::SERVER_SERIAL 					= "Serial";
const char* VS_ServerManagerStorage::SERVER_REGISTRATION_ALLOWED	= "Registration Allowed";
const char*	VS_ServerManagerStorage::SERVER_AUTO_VERIFY				= "AutoVerify";
const char* VS_ServerManagerStorage::SERVER_MANAGECOMMAND 			= "ManageCommand";
const char* VS_ServerManagerStorage::SERVER_MANAGECOMMAND_PARAM		= "ManageCommandParam";
const char* VS_ServerManagerStorage::SERVER_REDIRECTED				= "Redirected";
const char* VS_ServerManagerStorage::SERVER_MAX_USERS				= "MaxUsers";
const char* VS_ServerManagerStorage::SERVER_DOMAIN					= "Domain";


void TimetToFileTime( time_t t, LPFILETIME pft )
{
    LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD) ll;
    pft->dwHighDateTime = ll >>32;
}
bool VerifyServerCertUsingCertFrom(const char *server_name, const char *server_cert, const char *cert_location)
{
	VS_GET_PEM_CACERT

		if (!server_name || !*server_name ||
			!server_cert || !*server_cert ||
			!cert_location || !*cert_location)
			return false;

	std::unique_ptr<char, free_deleter> SM_Certificate;
	VS_Certificate	cert;
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	int len_sm_cert(0);
	if (0 >= (len_sm_cert = rkey.GetValue(SM_Certificate, VS_REG_BINARY_VT, cert_location)))
		return false;

	VS_CertificateCheck	certCheck;
	if (!cert.SetCert(server_cert, strlen(server_cert) + 1, store_PEM_BUF))
		return false;
	if (!certCheck.SetCert(server_cert, strlen(server_cert) + 1, store_PEM_BUF) ||
		!certCheck.SetCertToChain(PEM_CACERT, strlen(PEM_CACERT) + 1, store_PEM_BUF) ||
		!certCheck.SetCertToChain(SM_Certificate.get(), len_sm_cert, store_PEM_BUF))
	{
		return false;
	}

	if (certCheck.VerifyCert())
	{
		std::string buf;
		if (cert.GetSubjectEntry("commonName", buf) && !_stricmp(buf.data(), server_name))
			return true;
		else
			return false;
	}
	else
		return false;
}
bool VerifyServerCert(const char *server_name, const char *server_cert)
{
	if (VerifyServerCertUsingCertFrom(server_name, server_cert, SRV_CERT_KEY))
		return true;
	// TODO: Delete this code after 30.12.2018
	if (VerifyServerCertUsingCertFrom(server_name, server_cert, "CertificateOld"))
	{
		dstream3 << "SMStor: " << server_name << " has old certificate";
		return true;
	}
	return false;
}
VS_ServerManagerStorage::VS_ServerManagerStorage(): m_mgr_cmd_index(0)
{
	VS_AutoLock lock(this);
	this->CheckChangeRegistry();
	OnCreate_CleanUp();
}

VS_ServerManagerStorage::~VS_ServerManagerStorage()
{
	VS_AutoLock lock(this);

	m_BS.clear();
	m_RS.clear();
	m_AS.clear();

	m_bs_by_dns.clear();
	m_domains_for_rs.clear();
}

bool VS_ServerManagerStorage::AddServer(const char* dns_name, const char* domain, const long type, const long status,
										const VS_SimpleStr serial, const char * cert, const char* serv_key,const unsigned long registration_allowed,/* const char* notBefore, const char* notAfter,*/
										const long mgr_cmd, const long mgr_cmd_param, const unsigned long max_users, const unsigned long redirected, const unsigned long online_users, const unsigned long auto_verify)
{
	VS_AutoLock lock(this);
	VS_SimpleStr key_name = SERVERS_ROOT;
	key_name += dns_name;
	VS_RegistryKey key(false, key_name);
	VS_ServerManager_ServerInfo info;
	info.m_dns_name = dns_name;
	info.m_domain = domain;
	info.m_type = type;
	info.m_status = status;
	info.m_redirected = redirected;
	info.m_online_users = online_users;
	info.m_max_users = max_users;
	info.m_serv_key  = serv_key;
	info.m_autoVerify = auto_verify;

	info.m_serial = serial;
	info.m_registerAllowed = registration_allowed;
	info.mgr_cmd = mgr_cmd;
	info.mgr_cmd_param = mgr_cmd_param;
	info.m_certPEM = cert;

	std::list<std::string>	bs_list;

	std::unique_ptr<char, free_deleter> buf;
	unsigned long buf_sz(0);

	switch(type)
	{
	case ST_BS:
		if((buf_sz=key.GetValue(buf,VS_REG_STRING_VT,"domains"))>0)
		{
			char* p = buf.get();
			while(buf_sz&&*p)
			{
				info.m_domains.emplace_back(p);
				buf_sz-=strlen(p)+1;
				p+=strlen(p)+1;
			}
		}

		if (domain && *domain)
			m_bs_by_dns[domain].push_front(dns_name);

		m_BS.push_back(info);
		break;
	case ST_RS:
		//key.ResetValues();
		if((buf_sz=key.GetValue(buf,VS_REG_STRING_VT,"domains"))>0)
		{
			char* p = buf.get();
			while(buf_sz&&*p)
			{
				info.m_domains.emplace_back(p);
				m_domains_for_rs[dns_name].push_front(p);  // todo: or domain instead of dns_name?
				buf_sz-=strlen(p)+1;
				p+=strlen(p)+1;
			}
		}
		m_RS.push_back(info);
		break;
	case ST_AS:
		m_AS.push_back(info);
		break;
	}

	return true;
}

bool VS_ServerManagerStorage::Refresh()
{
	VS_AutoLock lock(this);

	m_BS.clear();
	m_RS.clear();
	m_AS.clear();

	m_bs_by_dns.clear();
	m_domains_for_rs.clear();

	// Read from registry about servers
	VS_RegistryKey key(false, "Servers");
	VS_RegistryKey key1;

	key.ResetKey();
	while( key.NextKey(key1) )
	{
		long b = 0;
		if (!key1.GetValue(&b, sizeof(long), VS_REG_INTEGER_VT, SERVER_TYPE)) {
			dprint0("SMstor: skipping server %s without type\n", key1.GetName());
			continue;
		}

		long tmp_status = 0;
		key1.GetValue(&tmp_status, sizeof(long), VS_REG_INTEGER_VT, SERVER_STATUS);

		long tmp_max_users = -1;
		key1.GetValue(&tmp_max_users, sizeof(long), VS_REG_INTEGER_VT, SERVER_MAX_USERS);

		long tmp_redirected = -1;
		key1.GetValue(&tmp_redirected, sizeof(long), VS_REG_INTEGER_VT, SERVER_REDIRECTED);

		long tmp_online_users = -1;
		key1.GetValue(&tmp_online_users, sizeof(long), VS_REG_INTEGER_VT, SERVER_ONLINE_USERS);

		long tmp_autoVerify = 0;
		key1.GetValue(&tmp_autoVerify, sizeof(tmp_autoVerify), VS_REG_INTEGER_VT, SERVER_AUTO_VERIFY);

		std::unique_ptr<char, free_deleter> serial, cert, serv_key, domain;
		unsigned long registerAllowed(0);

		key1.GetValue(cert, VS_REG_BINARY_VT, SRV_CERT_KEY);
		if (cert && !VerifyServerCert(key1.GetName(), cert.get())) {
			////key1.RemoveValue("Certificate"); // не удалять невалидный сертификат, в целях безопасности это должен контролировать администратор (пока сертификат неудален, сервер нельзя зарегать)
			cert = nullptr;
		}
		key1.GetValue(serv_key, VS_REG_BINARY_VT, "key");
		key1.GetValue(serial, VS_REG_STRING_VT, SERVER_SERIAL);
		key1.GetValue(&registerAllowed, sizeof(registerAllowed), VS_REG_INTEGER_VT, SERVER_REGISTRATION_ALLOWED);

		key1.GetValue(domain, VS_REG_STRING_VT, SERVER_DOMAIN);

		long mgr_cmd = 0;
		key1.GetValue(&mgr_cmd, sizeof(long), VS_REG_INTEGER_VT, SERVER_MANAGECOMMAND);

		long mgr_cmd_param = 0;
		key1.GetValue(&mgr_cmd_param, sizeof(long), VS_REG_INTEGER_VT, SERVER_MANAGECOMMAND_PARAM);

		this->AddServer(key1.GetName(), domain.get(), b, tmp_status, serial.get(), cert.get(), serv_key.get(), registerAllowed,
						mgr_cmd, mgr_cmd_param, tmp_max_users, tmp_redirected, tmp_online_users,tmp_autoVerify);
	}
	return true;
}

void VS_ServerManagerStorage::GetLocatorBS(VS_SimpleStr& str)
{
	VS_AutoLock lock(this);

	if ( !m_BS.size() )
		return ;

	for(unsigned int i=0; i < m_BS.size(); i++)
	{
		if ((m_BS[i].m_status == 1)&&(!m_BS[i].m_domain))	// online
		{
			str = m_BS[i].m_dns_name;
			return ;
		}
	}
	return ;
}
void VS_ServerManagerStorage::GetBSByDomain(const VS_SimpleStr &domain, VS_SimpleStr &b_srv)
{
	VS_AutoLock	lock(this);
	std::map<VS_SimpleStr, std::list<std::string>>::iterator iter = m_bs_by_dns.find(domain);
	if(!m_bs_by_dns.size() || iter==m_bs_by_dns.end())
		return;
	b_srv = iter->second.begin()->c_str();
}

void VS_ServerManagerStorage::GetRS(VS_SimpleStr& str)
{
	VS_AutoLock lock(this);

	for(unsigned int i=0; i < m_RS.size(); i++)
	{
		if ((m_RS[i].m_status == 1)&&m_RS[i].m_domains.empty())	// online
		{
			str = m_RS[i].m_dns_name;
			return ;
		}
	}

	return ;
}

bool VS_ServerManagerStorage::ServerStatus(const char* dns_name, const long status)
{
	VS_AutoLock lock(this);

	if ( !dns_name )
		return false;

	VS_SimpleStr key_name = SERVERS_ROOT;
	key_name += dns_name;

	VS_RegistryKey key(false, key_name.m_str, false, true);

	long type = 0;
	if (!key.GetValue(&type, sizeof(long), VS_REG_INTEGER_VT, SERVER_TYPE))
		return false;

	long dsa = status;
	if (!key.SetValue(&dsa, sizeof(long), VS_REG_INTEGER_VT, SERVER_STATUS))
		return false;

	this->UpdateServerStatusInVector(dns_name, type, status);

	return true;
}

bool VS_ServerManagerStorage::UpdateServerStatusInVector(const char* dns_name, const long type, const long status)
{
	VS_AutoLock lock(this);

	if ( !dns_name )
		return false;

	std::vector<VS_ServerManager_ServerInfo>* v = 0;
	switch(type)
	{
	case ST_RS:		v = &m_RS;		break;
	case ST_AS:		v = &m_AS;		break;
	case ST_BS:		v = &m_BS;		break;
	default:		return false;
	}

	for(unsigned int i=0; i < v->size(); i++)
	{
		VS_ServerManager_ServerInfo* info = &((*v)[i]);
		if (info->m_dns_name == dns_name)
		{
			info->m_status = status;
			return true;
		}
	}
	return false;
}

bool VS_ServerManagerStorage::RegisterServer(const char* server_id,const char *server_name, const char *serial,const char *hwkey, const char *cert_request, const char *version,const bool isVerified,
						VS_SimpleStr &out_key, VS_SimpleStr &out_cert, int &regRes)
{
	if(!cert_request)
	{
		regRes = 0;
		return false;
	}
	VS_FileTime			notBefore, notAfter;

	VS_AutoLock lock(this);
	Refresh();
	std::vector<VS_ServerManager_ServerInfo>::iterator iter;
	bool found(false);
	switch (server_name ? VS_GetServerType(server_name) : ST_UNKNOWN)
	{
	case ST_AS:
		for(iter = m_AS.begin();iter != m_AS.end();iter++)
		{
			if(iter->m_dns_name == server_name)
			{
				found = true;
				break;
			}
		}
		if(found)
			break;
		regRes = 0;
		return false;
	case ST_BS:
		for(iter = m_BS.begin();iter != m_BS.end();iter++)
		{
			if(iter->m_dns_name == server_name)
			{
				found = true;
				break;
			}
		}
		if(found)
			break;
		regRes = 0;
		return false;
	case ST_RS:
		for(iter = m_RS.begin();iter != m_RS.end();iter++)
		{
			if(iter->m_dns_name == server_name)
			{
				found = true;
				break;
			}
		}
		if(found)
			break;
		regRes = 0;
		return false;
	default:
		regRes = 0;
		return false;
	};
	if(!iter->m_registerAllowed)
	{
		regRes = 4;
		return false;
	}
	else if(iter->m_serial != serial)
	{
		regRes = 0;
		return false;
	}
	else if(!!iter->m_certPEM)
	{
		std::string notBeforeBuf, notAfterBuf;
		VS_Certificate _cert;
		uint32_t cert_len = !iter->m_certPEM? 0: iter->m_certPEM.Length()+1;
		if(VerifyServerCert(server_name,iter->m_certPEM)&&
			_cert.SetCert(iter->m_certPEM, cert_len, store_PEM_BUF)&&
			_cert.GetExpirationTime(notBeforeBuf, notAfterBuf, false))
		{

			if(!isVerified)
			{
				regRes = 4;
				return false;
			}
		}
	}

	time_t nowT(0);
	time_t not_before(0);
	time_t not_after(0);

	not_after = time(0) + 60*60*24*30;
	not_before = time(0) - 60*60*24*30;

	TimetToFileTime(not_before,&notBefore);
	TimetToFileTime(not_after,&notAfter);

	if(notBefore >= notAfter)
	{
		regRes = 0;
		return false;
	}

	std::unique_ptr<char, free_deleter> SM_PrivateKey;
	std::unique_ptr<char, free_deleter> SM_Certificate;
	int	SM_cert_sz(0);
	VS_CertAuthority	ca;
	unsigned long		certSerialNumber(0);
	char				*pem_buf(0);
	uint32_t sz(0);


	VS_SimpleStr	key2(256);
	VS_GenKeyByMD5(key2);
	out_key = key2;
	VS_SimpleStr sum_key = hwkey;
	sum_key+= key2;
	char hash[64] = {0};
	VS_ConvertToMD5(SimpleStrToStringView(sum_key), hash);
	VS_SimpleStr hw_md5 = hash;

	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	if(0>=rkey.GetValue(SM_PrivateKey, VS_REG_BINARY_VT, SRV_PRIVATE_KEY) ||
		0>=(SM_cert_sz = rkey.GetValue(SM_Certificate, VS_REG_BINARY_VT, SRV_CERT_KEY)))
	{
		regRes = -1;
		return false;
	}
	if (!ca.SetCACertificate(SM_Certificate.get(), SM_cert_sz, store_PEM_BUF)||
		!ca.SetCAPrivateKey(SM_PrivateKey.get(), store_PEM_BUF)||
		(!ca.SetCertRequest(cert_request,static_cast<unsigned long>(strlen(cert_request))+1,store_PEM_BUF))||
		(!ca.VerifyRequestSign()))
	{
		dprint3("CA: Verification Request failed.\n");
		regRes = 0;
		return false;
	}
	VS_SimpleStr hw_key_ext_value;
	VS_SimpleStr version_ext_value = "ASN1:GeneralString:";

	if(!version||!*version)
		version_ext_value+="0";
	else
		version_ext_value+= version;

	if(!!hw_md5)
	{
		hw_key_ext_value = "ASN1:GeneralString:";
		hw_key_ext_value += hw_md5;
	}
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	key.GetValue(&certSerialNumber,sizeof(certSerialNumber),VS_REG_INTEGER_VT,"cert_serial_counter");
	certSerialNumber++;
	if((!ca.SetSerialNumber(certSerialNumber))||
		(!ca.SetExpirationTime(std::chrono::system_clock::from_time_t(not_before),
			std::chrono::system_clock::from_time_t(not_after)))||
		(!ca.SetEntry("organizationName","TrueConf LLC"))||
		(!!hw_key_ext_value ? !ca.SetExtension(HWKEY_HASH_CERT_EXTENSIONS,hw_key_ext_value):false) ||
		(!!version_ext_value ? !ca.SetExtension(SERVER_VERSION_EXTENSIONS,version_ext_value):false))
	{
		regRes = 0;
		return false;
	}
	ca.IssueCertificate(pem_buf,sz,store_PEM_BUF);
	if(!sz)
	{
		regRes = -1;
		return false;
	}
	pem_buf = new char[sz+1];
	if(!ca.IssueCertificate(pem_buf,sz,store_PEM_BUF))
	{
		regRes = -1;
		delete [] pem_buf;
		return false;
	}
	pem_buf[sz] = 0;
	out_cert = pem_buf;
	delete pem_buf;

	VS_SimpleStr key_name = SERVERS_ROOT;
	key_name += server_name;

	char ch_not_after[256] = {0};
	notAfter.ToRUSStr(ch_not_after);

	VS_RegistryKey key1(false, key_name.m_str, false, true);
	if (!key1.SetString(ch_not_after, "NotAfter") ||
		!key1.SetValue(out_cert.m_str, out_cert.ByteLength(), VS_REG_BINARY_VT, SRV_CERT_KEY) ||
		!key1.SetString(out_key.m_str, "key"))
	{
		key1.RemoveValue("NotAfter");
		key1.RemoveValue(SRV_CERT_KEY);
		key1.RemoveValue("key");

		out_cert.Empty();
		out_key.Empty();
		regRes = -1;
		return false;
	}
	Refresh();
	key.SetValue(&certSerialNumber,sizeof(certSerialNumber),VS_REG_INTEGER_VT,"cert_serial_counter");
	regRes = 1;
	return true;
}

bool VS_ServerManagerStorage::UpdateServerCertificate(const char *server_name,const char *cert_request, VS_SimpleStr&out_cert,long &regRes)
{
	if(!cert_request)
	{
		regRes = 0;
		dprint3("UpdateCert: empty cert_req for %s.\n",server_name);
		return false;
	}
	VS_FileTime			notBefore, notAfter;
	VS_AutoLock lock(this);
	Refresh();
	std::vector<VS_ServerManager_ServerInfo>::iterator iter;
	bool found(false);
	switch (server_name ? VS_GetServerType(server_name) : ST_UNKNOWN)
	{
	case ST_AS:
		for(iter = m_AS.begin();iter != m_AS.end();iter++)
		{
			if(iter->m_dns_name == server_name)
			{
				found = true;
				break;
			}
		}
		if(found)
			break;
		regRes = 0;
		return false;
	case ST_BS:
		for(iter = m_BS.begin();iter != m_BS.end();iter++)
		{
			if(iter->m_dns_name == server_name)
			{
				found = true;
				break;
			}
		}
		if(found)
			break;
		regRes = 0;
		return false;
	case ST_RS:
		for(iter = m_RS.begin();iter != m_RS.end();iter++)
		{
			if(iter->m_dns_name == server_name)
			{
				found = true;
				break;
			}
		}
		if(found)
			break;
		dprint3("UpdateCert: server %s not found;\n",server_name);
		regRes = 0;
		return false;
	};
	if(!iter->m_registerAllowed)
	{
		regRes = 0;
		return false;

	}
	else if(!!iter->m_certPEM)
	{
		/**
			провреить время действия, если мньше чем 5 дней, то разрешить
		*/


		std::string notBeforeBuf, notAfterBuf;
		VS_Certificate _cert;

		time_t notAfterT;
		time_t nowT(0);

		time_t new_notAfter;
		time_t new_NotBefore;

		uint32_t cert_len = !iter->m_certPEM? 0: iter->m_certPEM.Length()+1;

		if(VerifyServerCert(server_name,iter->m_certPEM)&&
			_cert.SetCert(iter->m_certPEM,cert_len,store_PEM_BUF) &&
			_cert.GetExpirationTime(notBeforeBuf, notAfterBuf, false))
		{

			char year[5] ={0};
			char month[3] = {0};
			char day[3] = {0};

			strncpy(year,notAfterBuf.data(), 4);
			strncpy(month,notAfterBuf.data() + 4, 2);
			strncpy(day,notAfterBuf.data() + 6, 2);

			time(&nowT);
			tm notAtm = { 0, 0, 0, atoi(day), atoi(month) - 1, atoi(year) - 1900};
			notAfterT = mktime(&notAtm);
			if(5<(notAfterT - nowT)/60/60/24)
			{
				regRes = 0; //перезапрос сертификата запрещен
				dprint3("UpdateCert: updateCert for %s denied: cert is valid yet ;\n",server_name);
				return false;
			}

			new_notAfter = time(0) + 60*60*24*30;
			new_NotBefore = time(0) - 60*60*24*30;

			TimetToFileTime(new_NotBefore,&notBefore);
			TimetToFileTime(new_notAfter,&notAfter);


			if(notBefore >= notAfter)
			{
				dprint3("UpdateCert: notBefore >= motAfter; ServerName = %s\n",server_name);
				regRes = 0;
				return false;
			}
			std::unique_ptr<char, free_deleter> SM_PrivateKey;
			std::unique_ptr<char, free_deleter> SM_Certificate;
			int	SM_cert_sz(0);
			VS_CertAuthority	ca;
			unsigned long		certSerialNumber(0);
			VS_SimpleStr		hw_key_ext_value = "ASN1:GeneralString:";
			VS_SimpleStr		version_ext_value = "ASN1:GeneralString:";

			VS_WideStr			organization_name;
			VS_WideStr			country;
			VS_WideStr			contact_person;
			VS_WideStr			contact_email;

			VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
			if(0>=rkey.GetValue(SM_PrivateKey, VS_REG_BINARY_VT, SRV_PRIVATE_KEY) ||
				0>=(SM_cert_sz = rkey.GetValue(SM_Certificate, VS_REG_BINARY_VT, SRV_CERT_KEY)))
			{
				regRes = 0;
				dprint3("UpdateCert: Read private key from registry failed; ServerName = %s\n",server_name);
				return false;
			}
			if (!ca.SetCACertificate(SM_Certificate.get(), SM_cert_sz, store_PEM_BUF)||
				!ca.SetCAPrivateKey(SM_PrivateKey.get(), store_PEM_BUF)||
				(!ca.SetCertRequest(cert_request,static_cast<unsigned long>(strlen(cert_request))+1,store_PEM_BUF))||
				(!ca.VerifyRequestSign()))
			{
				dprint3("UpdateCert: Verification Request for %s failed.\n",server_name);
				regRes = 0;
				return false;
			}

			std::string buf;
			const char* exts[] = { HWKEY_HASH_CERT_EXTENSIONS, SERVER_VERSION_EXTENSIONS };
			for(int i = 0;i<2;i++)
			{
				bool isSucess(true);
				if(_cert.GetExtension(exts[i], buf))
				{
					std::string currExt = "ASN1:GeneralString:";
					currExt += buf;
					if(!ca.SetExtension(exts[i],currExt.c_str()))
						isSucess = false;
				}
				else
					isSucess = false;
				if(!isSucess)
				{
					dprint3("UpdateCert: ext copy for %s error;\n",server_name);
					regRes = 0; return false;
				}
			}
			buf.clear();
			const char* entries[] = { "organizationName", "countryName", "surname", "emailAddress" };
			for(int i =0; i < 4; i++)
				if(_cert.GetSubjectEntry(entries[i], buf))
					ca.SetEntry(entries[i],buf.c_str());

			VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
			key.GetValue(&certSerialNumber,sizeof(certSerialNumber),VS_REG_INTEGER_VT,"cert_serial_counter");
			certSerialNumber++;
			unsigned long cond_res(0);

			if (!ca.SetSerialNumber(certSerialNumber) ||
				!ca.SetExpirationTime(std::chrono::system_clock::from_time_t(new_NotBefore),
					std::chrono::system_clock::from_time_t(new_notAfter)))
			{
				dprint3("UpdateCert: SetSerial or SetExpTime failed for %s\n",server_name);
				regRes = 0;
				return false;
			}
			uint32_t count(0);
			ca.IssueCertificate(nullptr, count, store_PEM_BUF);
			if (!count)
			{
				dprint3("UpdateCert: IssueCert failed for %s\n", server_name);
				regRes = 0;
				return false;
			}
			buf.resize(count + 1);
			if(!ca.IssueCertificate(buf.data(), count, store_PEM_BUF))
			{
				dprint3("UpdateCert: IssueCert failed for %s\n", server_name);
				regRes = 0;
				return false;
			}
			out_cert = buf.data();
			VS_SimpleStr key_name = SERVERS_ROOT;
			key_name += server_name;
			char ch_not_after[256] = {0};
			notAfter.ToRUSStr(ch_not_after);

			VS_RegistryKey key1(false, key_name.m_str, false, true);
			if (!key1.SetValue(out_cert.m_str, out_cert.ByteLength(), VS_REG_BINARY_VT, SRV_CERT_KEY))
			{
				dprint3("UpdateCert: Write cert to registry failed for %s\n",server_name);
				out_cert.Empty();
				regRes = 0;
				return false;
			}
			if (!key1.SetString(ch_not_after, "NotAfter"))
			{
				dprint3("UpdateCert: Write notAfter to registry failed for %s\n",server_name);
			}
			Refresh();
			key.SetValue(&certSerialNumber,sizeof(certSerialNumber),VS_REG_INTEGER_VT,"cert_serial_counter");
			regRes = 1;
			return true;
		}
		dprint3("UpdateCert: Registration needed for %s;\n",server_name);
		regRes = 0;
		return false;
	}
	else
	{
		dprint3("UpdateCert: Registration needed for %s;\n",server_name);
		regRes = 0;
		return false;
	}
}

void VS_ServerManagerStorage::CheckChangeRegistry()
{
	VS_AutoLock lock(this);

	VS_RegistryKey key(false, "Servers");
	auto last_write = key.GetLastWriteTime();
	if (last_write != m_last_write)
	{
		this->Refresh();
		m_last_write = last_write;
	}
}

void VS_ServerManagerStorage::OnCreate_CleanUp()
{
	VS_AutoLock lock(this);

	// BS
	for(unsigned int i=0; i < m_BS.size(); i++)
	{
		VS_SimpleStr key_name = SERVERS_ROOT;
		key_name += m_BS[i].m_dns_name;

		VS_RegistryKey key(false, key_name.m_str, false, true);

		long status = 0;		// offline
		if (!key.SetValue(&status, sizeof(long), VS_REG_INTEGER_VT, SERVER_STATUS))
			return ;

		m_BS[0].m_status = 0;
	}

	// RS
	for(unsigned int i=0; i < m_RS.size(); i++)
	{
		VS_SimpleStr key_name = SERVERS_ROOT;
		key_name += m_RS[i].m_dns_name;

		VS_RegistryKey key(false, key_name.m_str, false, true);

		long status = 0;		// offline
		if (!key.SetValue(&status, sizeof(long), VS_REG_INTEGER_VT, SERVER_STATUS))
			return ;

		m_RS[0].m_status = 0;
	}

	// AS
	for(unsigned int i=0; i < m_AS.size(); i++)
	{
		VS_SimpleStr key_name = SERVERS_ROOT;
		key_name += m_AS[i].m_dns_name;

		VS_RegistryKey key(false, key_name.m_str, false, true);

		long status = 0;		// offline
		if (!key.SetValue(&status, sizeof(long), VS_REG_INTEGER_VT, SERVER_STATUS))
			return ;

		m_AS[0].m_status = 0;
	}
}

bool VS_ServerManagerStorage::IsRegisteredAS(const char* sid, VS_SimpleStr *domain)
{
	VS_AutoLock lock(this);

	if ( !sid || !*sid )
		return false;

	for(unsigned int i=0; i < m_AS.size(); i++) {
		if ( m_AS[i].m_dns_name == sid ) {
			if (domain)
				*domain = m_AS[i].m_domain;
			return true;
		}
	}

	return false;
}

void VS_ServerManagerStorage::GetServerStat(const char* sid, VS_AppServerStats& stats)
{
	if ( !sid || !*sid ) { return ; }

	VS_SimpleStr key_name = SERVERS_ROOT;
	key_name += sid;

	VS_RegistryKey key(false, key_name.m_str, false, true);
	key.GetValue(stats.m_Version, sizeof(stats.m_Version) - 1, VS_REG_STRING_VT, SERVER_VERSION);
	key.GetValue(&stats.m_CPULoad, sizeof(stats.m_CPULoad), VS_REG_INTEGER_VT, SERVER_CPU_LOAD);
	key.GetValue(&stats.m_Transport_NumEndpoints, sizeof(stats.m_Transport_NumEndpoints), VS_REG_INTEGER_VT, SERVER_NUM_ENDPOINTS);
	key.GetValue(&stats.m_Transport_Bitrate_In, sizeof(stats.m_Transport_Bitrate_In), VS_REG_INTEGER_VT, SERVER_TRANSPORT_BITRATE_IN);
	key.GetValue(&stats.m_Transport_Bitrate_Out, sizeof(stats.m_Transport_Bitrate_Out), VS_REG_INTEGER_VT, SERVER_TRANSPORT_BITRATE_OUT);
	key.GetValue(&stats.m_Streams_NumStreams, sizeof(stats.m_Streams_NumStreams), VS_REG_INTEGER_VT, SERVER_NUM_STREAMS);
	key.GetValue(&stats.m_Streams_Bitrate_In, sizeof(stats.m_Streams_Bitrate_In), VS_REG_INTEGER_VT, SERVER_STREAMS_BITRATE_IN);
	key.GetValue(&stats.m_Streams_Bitrate_Out, sizeof(stats.m_Streams_Bitrate_Out), VS_REG_INTEGER_VT, SERVER_STREAMS_BITRATE_OUT);
	key.GetValue(&stats.m_OnlineUsers, sizeof(stats.m_OnlineUsers), VS_REG_INTEGER_VT, SERVER_ONLINE_USERS);
	key.GetValue(&stats.m_Confs, sizeof(stats.m_Confs), VS_REG_INTEGER_VT, SERVER_CONFS);
	key.GetValue(&stats.m_Participants, sizeof(stats.m_Participants), VS_REG_INTEGER_VT, SERVER_PARTICIPANTS);
}

void VS_ServerManagerStorage::SetServerStat(const char* sid, VS_AppServerStats* stats)
{
	if ( !sid || !*sid || !stats )
		return ;

	VS_SimpleStr key_name = SERVERS_ROOT;
	key_name += sid;

	VS_RegistryKey key(false, key_name.m_str, false, true);

	bool ret = true;
	if (stats->m_Version)
		key.SetString(stats->m_Version, SERVER_VERSION);
	if ( stats->m_CPULoad != -1 )					key.SetValue(&stats->m_CPULoad, sizeof(int), VS_REG_INTEGER_VT, SERVER_CPU_LOAD);
	if ( stats->m_Transport_NumEndpoints != -1 )	key.SetValue(&stats->m_Transport_NumEndpoints, sizeof(int), VS_REG_INTEGER_VT, SERVER_NUM_ENDPOINTS);
	if ( stats->m_Transport_Bitrate_In != -1 )		key.SetValue(&stats->m_Transport_Bitrate_In, sizeof(int), VS_REG_INTEGER_VT, SERVER_TRANSPORT_BITRATE_IN);
	if ( stats->m_Transport_Bitrate_Out != -1 )		key.SetValue(&stats->m_Transport_Bitrate_Out, sizeof(int), VS_REG_INTEGER_VT, SERVER_TRANSPORT_BITRATE_OUT);
	if ( stats->m_Streams_NumStreams != -1 )		key.SetValue(&stats->m_Streams_NumStreams, sizeof(int), VS_REG_INTEGER_VT, SERVER_NUM_STREAMS);
	if ( stats->m_Streams_Bitrate_In != -1 )		key.SetValue(&stats->m_Streams_Bitrate_In, sizeof(int), VS_REG_INTEGER_VT, SERVER_STREAMS_BITRATE_IN);
	if ( stats->m_Streams_Bitrate_Out != -1 )		key.SetValue(&stats->m_Streams_Bitrate_Out, sizeof(int), VS_REG_INTEGER_VT, SERVER_STREAMS_BITRATE_OUT);
	if ( stats->m_OnlineUsers != -1 )				key.SetValue(&stats->m_OnlineUsers, sizeof(int), VS_REG_INTEGER_VT, SERVER_ONLINE_USERS);
	if ( stats->m_Confs != -1 )						key.SetValue(&stats->m_Confs, sizeof(int), VS_REG_INTEGER_VT, SERVER_CONFS);
	if ( stats->m_Participants != -1 )				key.SetValue(&stats->m_Participants, sizeof(int), VS_REG_INTEGER_VT, SERVER_PARTICIPANTS);

	char time[128];		memset(time, 0, 128);
	VS_FileTime t;		t.Now();	t.ToRUSStr(time);
	key.SetString(time, SERVER_LAST_ONLINE_TIME);
}

void VS_ServerManagerStorage::SetServerStat_StartTime(const char* sid)
{
	if ( !sid )
		return ;

	char time[128];		memset(time, 0, 128);
	VS_FileTime t;		t.Now();	t.ToRUSStr(time);

	VS_SimpleStr key_name = SERVERS_ROOT;
	key_name += sid;

	VS_RegistryKey key(false, key_name.m_str, false, true);

	key.SetString(time, SERVER_START_TIME);
}

void VS_ServerManagerStorage::ResetManageCommandIndex()
{
	VS_AutoLock lock(this);
	m_mgr_cmd_index = 0;
	m_mgr_cmd_it = m_AS.begin();
}

bool VS_ServerManagerStorage::GetNextManageCommand(VS_SimpleStr &sid, long &mgr_cmd, long &mgr_cmd_param)
{
	VS_AutoLock lock(this);
	if ( m_mgr_cmd_index == 0 )
	{
		while ( m_mgr_cmd_it != m_AS.end() )
		{
			if ( m_mgr_cmd_it->mgr_cmd != MSC_NONE )
			{
				sid = m_mgr_cmd_it->m_dns_name;
				mgr_cmd = m_mgr_cmd_it->mgr_cmd;
				mgr_cmd_param = m_mgr_cmd_it->mgr_cmd_param;

				// clean up vector
				m_mgr_cmd_it->mgr_cmd = MSC_NONE;
				m_mgr_cmd_it->mgr_cmd_param = 0;

				// clean up registry
				VS_SimpleStr key_name = SERVERS_ROOT;		key_name += m_mgr_cmd_it->m_dns_name;

				VS_RegistryKey key(false, key_name.m_str, false, true);
				if ( !key.SetValue(&m_mgr_cmd_it->mgr_cmd, sizeof(long), VS_REG_INTEGER_VT, SERVER_MANAGECOMMAND) ||
					 !key.SetValue(&m_mgr_cmd_it->mgr_cmd_param, sizeof(long), VS_REG_INTEGER_VT, SERVER_MANAGECOMMAND_PARAM) )
					return false;

				m_mgr_cmd_it++;
				return true;
			}
			m_mgr_cmd_it++;
		}

		m_mgr_cmd_index = 1;
		m_mgr_cmd_it = m_RS.begin();
	}

	if ( m_mgr_cmd_index == 1 )
	{
		while ( m_mgr_cmd_it != m_RS.end() )
		{
			if ( m_mgr_cmd_it->mgr_cmd != MSC_NONE )
			{
				sid = m_mgr_cmd_it->m_dns_name;
				mgr_cmd = m_mgr_cmd_it->mgr_cmd;
				mgr_cmd_param = m_mgr_cmd_it->mgr_cmd_param;

				// clean up vector
				m_mgr_cmd_it->mgr_cmd = MSC_NONE;
				m_mgr_cmd_it->mgr_cmd_param = 0;

				// clean up registry
				VS_SimpleStr key_name = SERVERS_ROOT;		key_name += m_mgr_cmd_it->m_dns_name;

				VS_RegistryKey key(false, key_name.m_str, false, true);
				if ( !key.SetValue(&m_mgr_cmd_it->mgr_cmd, sizeof(long), VS_REG_INTEGER_VT, SERVER_MANAGECOMMAND) ||
					 !key.SetValue(&m_mgr_cmd_it->mgr_cmd_param, sizeof(long), VS_REG_INTEGER_VT, SERVER_MANAGECOMMAND_PARAM) )
					return false;

				m_mgr_cmd_it++;
				return true;
			}
			m_mgr_cmd_it++;
		}

		m_mgr_cmd_index = 2;
		m_mgr_cmd_it = m_BS.begin();
	}

	if ( m_mgr_cmd_index == 2 )
	{
		while ( m_mgr_cmd_it != m_BS.end() )
		{
			if ( m_mgr_cmd_it->mgr_cmd != MSC_NONE )
			{
				sid = m_mgr_cmd_it->m_dns_name;
				mgr_cmd = m_mgr_cmd_it->mgr_cmd;
				mgr_cmd_param = m_mgr_cmd_it->mgr_cmd_param;

				// clean up vector
				m_mgr_cmd_it->mgr_cmd = MSC_NONE;
				m_mgr_cmd_it->mgr_cmd_param = 0;

				// clean up registry
				VS_SimpleStr key_name = SERVERS_ROOT;		key_name += m_mgr_cmd_it->m_dns_name;

				VS_RegistryKey key(false, key_name.m_str, false, true);
				if ( !key.SetValue(&m_mgr_cmd_it->mgr_cmd, sizeof(long), VS_REG_INTEGER_VT, SERVER_MANAGECOMMAND) ||
					 !key.SetValue(&m_mgr_cmd_it->mgr_cmd_param, sizeof(long), VS_REG_INTEGER_VT, SERVER_MANAGECOMMAND_PARAM) )
					return false;

				m_mgr_cmd_it++;
				return true;
			}
			m_mgr_cmd_it++;
		}

		m_mgr_cmd_index = 3;
	}

	return false;
}

void VS_ServerManagerStorage::SetRedirected(const char* sid, const long flag)
{
	if ( !sid )
		return ;

	VS_SimpleStr key_name = SERVERS_ROOT;	key_name += sid;

	VS_RegistryKey key(false, key_name.m_str, false, true);
	if (!key.SetValue(&flag, sizeof(long), VS_REG_INTEGER_VT, SERVER_REDIRECTED))
		return ;
}

void VS_ServerManagerStorage::GetOverLoadedServers(VST_StrIMap<long> &servers)
{
	VS_AutoLock lock(this);
	for(unsigned int i=0; i < m_AS.size(); i++)
	{
		if ( !m_AS[i].m_status )		// online?
			continue;

		const char* sid = m_AS[i].m_dns_name;
		long max_users = m_AS[i].m_max_users;

		if ( max_users == -1 )
			continue;

		long num = 0;

		VS_SimpleStr key_name = SERVERS_ROOT;
		key_name += sid;

		VS_RegistryKey key(false, key_name.m_str, false, true);
		key.GetValue(&num, sizeof(num), VS_REG_INTEGER_VT, SERVER_ONLINE_USERS);

		if ( (num - max_users) <= 50 )
			continue;

		servers[sid] = num - max_users;
	}
}

void VS_ServerManagerStorage::GetAllRS(VS_Container &cnt)
{
	VS_AutoLock lock(this);
	for(std::map<VS_SimpleStr,std::list<std::string>>::iterator i = m_domains_for_rs.begin();i!=m_domains_for_rs.end();i++)
	{
		cnt.AddValue("RS with domains",i->first);
		for(std::list<std::string>::iterator ii=i->second.begin(); ii!= i->second.end(); ii++)
			cnt.AddValue("domainRS",ii->c_str());
	}

}

void VS_ServerManagerStorage::GetRSByDomain(VS_Container & cnt, const char* domain)
{
	if(nullptr == domain)
		return;
	VS_AutoLock lock(this);
	for (auto &i : m_domains_for_rs) {
		for (auto &ii : i.second)
			if (ii == domain) {
				cnt.AddValue("RS with domains", i.first);
				cnt.AddValue("domainRS", domain); // in 4.3.7 only the first "domainRS" has been used
				return;
			}
	}

}

std::map<VS_SimpleStr, std::list<std::string>> VS_ServerManagerStorage::GetAllBS()
{
	VS_AutoLock	lock(this);
	return m_bs_by_dns;
}

void VS_ServerManagerStorage::GetAllAS(string_view domain, std::vector<VS_ServerManager_ServerInfo> &vec)
{
	VS_AutoLock lock(this);
	this->Refresh();
	for(unsigned int i=0; i < m_AS.size(); i++) {
		if (!!m_AS[i].m_domain && domain == m_AS[i].m_domain.m_str)
			vec.push_back(m_AS[i]);
	}
}

void VS_ServerManagerStorage::GetAllAS(std::vector<VS_ServerManager_ServerInfo> &vec)
{
	VS_AutoLock lock(this);
	this->Refresh();
	vec = m_AS;
}

VS_ServCertInfoInterface::get_info_res
VS_ServerManagerStorage::GetPublicKey(
	const VS_SimpleStr& server_name,
	VS_SimpleStr &pub_key, uint32_t &vcs_ver)
{
	VS_AutoLock lock(this);
	Refresh();
	std::vector<VS_ServerManager_ServerInfo>::iterator iter;
	bool found(false);
	auto res = get_info_res::key_is_absent;
	switch (server_name ? VS_GetServerType(server_name.m_str) : ST_UNKNOWN)
	{
	case ST_AS:
		for(iter = m_AS.begin();iter != m_AS.end();iter++)
		{
			if(iter->m_dns_name == server_name)
			{
				found = true;
				break;
			}
		}
		break;
	case ST_BS:
		for(iter = m_BS.begin();iter != m_BS.end();iter++)
		{
			if(iter->m_dns_name == server_name)
			{
				found = true;
				break;
			}
		}
		break;
	case ST_RS:
		for(iter = m_RS.begin();iter != m_RS.end();iter++)
		{
			if(iter->m_dns_name == server_name)
			{
				found = true;
				break;
			}
		}
		break;
	};
	if(found)
	{
		VS_Certificate cert;
		VS_PKey	pkey;
		unsigned long cert_len = !iter->m_certPEM? 0: iter->m_certPEM.Length()+1;
		if(iter->m_autoVerify!=0)
			res = get_info_res::auto_verify;
		else if(!iter->m_certPEM)
			res = get_info_res::key_is_absent;
		else if(!cert.SetCert(iter->m_certPEM,cert_len,store_PEM_BUF)||
			!cert.GetCertPublicKey(&pkey))
			res = get_info_res::db_error;
		else
		{
			char *buf(0);
			uint32_t sz(0);
			std::string srv_ver_buf;
			if(!cert.GetExtension(SERVER_VERSION_EXTENSIONS,srv_ver_buf))
				vcs_ver = 0;
			else
				vcs_ver = atou_s(srv_ver_buf.c_str());
			pkey.GetPublicKey(store_PEM_BUF,buf,&sz);
			if(sz)
			{
				buf = new char[sz];
				if(pkey.GetPublicKey(store_PEM_BUF,buf,&sz))
				{
					res = get_info_res::ok;
					pub_key = buf;
				}
				else
					res = get_info_res::db_error;
				delete [] buf;
			}
			else
				res = get_info_res::db_error;;
		}
	}
	return res;
}
VS_ServCertInfoInterface::get_info_res
VS_ServerManagerStorage::GetServerCertificate(
	const VS_SimpleStr &server_name, VS_SimpleStr &cert)
{
	VS_AutoLock lock(this);
	Refresh();
	std::vector<VS_ServerManager_ServerInfo>::iterator iter;
	bool found(false);
	auto res = get_info_res::key_is_absent;
	switch (server_name ? VS_GetServerType(server_name.m_str) : ST_UNKNOWN)
	{
	case ST_AS:
		for(iter = m_AS.begin();iter != m_AS.end();iter++)
		{
			if(iter->m_dns_name == server_name)
			{
				found = true;
				break;
			}
		}
		break;
	case ST_BS:
		for(iter = m_RS.begin();iter != m_RS.end();iter++)
		{
			if(iter->m_dns_name == server_name)
			{
				found = true;
				break;
			}
		}
		break;
	case ST_RS:
		for(iter = m_RS.begin();iter != m_RS.end();iter++)
		{
			if(iter->m_dns_name == server_name)
			{
				found = true;
				break;
			}
		}
		break;
	};
	if(found)
	{
		if(!iter->m_certPEM)
			res = get_info_res::key_is_absent;
		else
		{
			cert = iter->m_certPEM;
			res = get_info_res::ok;
		}
	}
	return res;
}
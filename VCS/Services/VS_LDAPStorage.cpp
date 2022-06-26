#include "std-generic/cpplib/scope_exit.h"

#include "VS_LDAPStorage.h"
#include "std-generic/cpplib/utf8.h"
#include "std-generic/clib/strcasecmp.h"
#include "std-generic/clib/wcscasecmp.h"
#include "std-generic/cpplib/ThreadUtils.h"
#include "std-generic/cpplib/ignore.h"
#include "std-generic/compat/memory.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/FilesystemUtils.h"
#include "std/cpplib/curl_deleters.h"
#include "ProtectionLib/Protection.h"
#include "ldap_core/CfgHelper.h"
#include "ldap_core/VS_LDAPFactory.h"
#include "ServerServices/VS_ReadLicense.h"
#include "ServersConfigLib/VS_ServersConfigLib.h"
#include "TrueGateway/CallConfig/VS_Indentifier.h"
#include "SecureLib/NTLMDataHeaders.h"
#include "std/VS_RegistryPasswordEncryption.h"

#ifdef _SVKS_M_BUILD_
#include "../../common/sudis/unit_solutions/sudis.h"
#endif

#include <ctime>
#include <sstream>
#include <algorithm>
#include <set>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/filesystem.hpp>

#include "std/cpplib/base64.h"
#include "JPEGUtils/jpeg_utils.h"

#include "IppLib2/VSVideoProcessingIpp.h"

#ifdef _WIN32

#ifndef __wtypes_h__
#include <wtypes.h>
#endif

#ifndef __WINDEF_
#include <windef.h>
#endif

#ifndef LDAP_UNICODE
#define LDAP_UNICODE 1
#endif

#define SECURITY_WIN32 1

#include <windows.h>
#include <windns.h>
#include <winldap.h>

#ifndef UNICODE
#define UNICODE
#include <security.h>
#undef  UNICODE
#else
#include <security.h>
#endif

#else	// linux

#define LDAP_DEPRECATED 1
#include <ldap.h>

// for DIGEST-MD5 bind
#include "ldap_core/liblutil/lutil_ldap.h"

#endif

#define DEBUG_CURRENT_MODULE VS_DM_RSTOR

////////////////////////////////////////////////////////////////////////////////
// Consts
////////////////////////////////////////////////////////////////////////////////

// Conferences
const char CONFERENCES_KEY[]      = "Conferences";

//Properties
const char APPPROPERTIES_KEY[]      = "AppProperties";

///LDAP

//  server
const char	LDAP_BASEDN_TAG[]	=	"LDAP Base DN";
const char	LDAP_SERVER_TAG[]	=	"LDAP Server";
const char	LDAP_DOMAIN_TAG[]	=	"LDAP Domain";

const char	LDAP_SECURE_TAG[]	=	"LDAP Secure";
const unsigned short	LDAP_SECURE_INIT			=0;

const char	LDAP_PORT_TAG[]		=	"LDAP Port";
const unsigned short	LDAP_PORT_INIT				=389;
const unsigned short	LDAP_SECURE_PORT_INIT	=636;

const char	LDAP_TIMEOUT_TAG[]		=	"LDAP Timeout";
const int		LDAP_TIMEOUT_INIT				=30;

const char	LDAP_AB_REFRESH_TAG[]		=	"LDAP AddressBook Refresh";
const uint64_t	LDAP_AB_REFRESH_INIT		= 30*60;

const char	LDAP_VERSION_TAG[]		= "LDAP Version";
const int	LDAP_VERSION_INIT		= LDAP_VERSION3;



/// auth
const char	LDAP_AUTH_METHOD_TAG[]		= "LDAP Auth Method";
const char	LDAP_AUTH_USER_TAG[]		= "LDAP Auth User";
const char	LDAP_AUTH_PASSWORD_TAG_OLD[]	= "LDAP Auth Password";
const char	LDAP_AUTH_PASSWORD_TAG_NEW[]	= "LDAP Auth Password2";
const char	LDAP_AUTH_DOMAIN_TAG[]		= "LDAP Auth Domain";


/// attributes
const char	LDAP_ATTR_LOGIN_TAG[]			=	"LDAP Login";
const char	LDAP_ATTR_LOGIN_INIT[]		=	"sAMAccountName";

const char	LDAP_ATTR_EMAIL_TAG[]			=	"LDAP Emai";
const char	LDAP_ATTR_EMAIL_INIT[]		=	"mai";

const char	LDAP_ATTR_FIRSTNAME_TAG[]	=	"LDAP First Name";
const char	LDAP_ATTR_FIRSTNAME_INIT[]=	"givenName";

const char	LDAP_ATTR_LASTNAME_TAG[]	=	"LDAP Last Name";
const char	LDAP_ATTR_LASTNAME_INIT[]=	"sn";

const char	LDAP_ATTR_DISPLAYNAME_TAG[]	=	"LDAP Display Name";
const char	LDAP_ATTR_DISPLAYNAME_INIT[]=	"displayName";

const char	LDAP_ATTR_COMPANY_TAG[]			=	"LDAP Company";
const char	LDAP_ATTR_COMPANY_INIT[]		=	"company";

const char	LDAP_ATTR_GROUP_MEMBER_TAG[]	=	"LDAP Group Member";
const char	LDAP_ATTR_GROUP_MEMBER_INIT[]	=	"member";

const char	LDAP_GROUP_TAG[]						=	"LDAP Group";

const char			LDAP_GROUPS_UPDATE_PERIOD_TAG[]		= "LDAP Groups Update Period";
const unsigned long	LDAP_GROUPS_UPDATE_PERIOD_INIT		= 60;		// in minutes

///NTLM
const char    NTLM_ON_TAG[]               = "NTLM Authentication";

const char	LDAP_AUTO_DETECT_TAG[]		=	"SrvSettingsAutoDetect";

static_assert(tc::LDAP_AVATARS_SIZE_INIT * tc::LDAP_AVATARS_SIZE_INIT * 24 >= MAX_AVATAR_SIZE); // W * H * i (24 bit <=> 3 byte for .jpg)

////////////////////////////////////////////////////////////////////////////////

static std::string get_avatar_file_name(const VS_Container& cnt, const char *callId, const std::vector<std::pair<std::string, std::string>> &pictureMimeTypes) noexcept
{
	auto &&avatar_type = cnt.GetStrValueView("avatar_type");
	if (avatar_type.empty())
		return {};

	const auto it = std::find_if(pictureMimeTypes.cbegin(), pictureMimeTypes.cend(), [avatar_type](const std::pair<std::string, std::string>& x) { return boost::iequals(x.second, avatar_type); });
	if (it == pictureMimeTypes.cend())
		return {};
	const string_view extension { it->first };
	if (extension.empty())
		return {};

	auto callId_escaped = VS_SimpleStorage::EscapeCallId(static_cast<VS_RealUserLogin>(callId).GetUser());

	if (callId_escaped.empty())
		return {};

	return std::string(VS_SimpleStorage::AVATARS_DIRECTORY).append(callId_escaped).append(extension.data(), extension.length());
}

static bool resize_avatar(const std::string &fileName, std::vector<uint8_t> &out, unsigned avatarSize, std::int32_t avatarsQuality, bool &resized) noexcept
{
	unsigned int src_width;
	unsigned int src_height;

	std::unique_ptr<std::uint8_t[]> src_buff;

	if (!jpeg::read_RGB24_file(fileName.c_str(), src_buff, src_width, src_height))
		return false;

	if (!src_width || !src_height)
		return false;

	assert(src_buff != nullptr);

	unsigned int dst_width = src_width;
	unsigned int dst_height = src_height;

	if (avatarSize) {

		const double aspect = static_cast<double>(src_width) / static_cast<double>(src_height);

		if ((resized = src_width > src_height && src_width > avatarSize)) {
			dst_width = avatarSize;
			dst_height = static_cast<decltype(dst_height)>(std::round(dst_width / aspect));
		}
		else if((resized = src_height > avatarSize))
		{
			dst_height = avatarSize;
			dst_width = static_cast<decltype(dst_height)>(std::round(dst_height / aspect));
		}
	}

	std::unique_ptr<std::uint8_t[]> dst_buff;

	if (resized)
	{
		dst_width = dst_width ? dst_width : 1;
		dst_height = dst_height ? dst_height : 1;

		const auto dst_buffer_size = dst_width * dst_height * 3;

		dst_buff = vs::make_unique<std::uint8_t[]>(dst_buffer_size);

		VSVideoProcessingIpp p{};
		if (!p.ResampleRGB(src_buff.get(), src_width, src_width * 3, src_height, dst_buff.get(), dst_width, dst_width * 3, dst_height))
			return false;
	}
	else
	{
		dst_buff = std::move(src_buff);
	}

	return jpeg::write_RGB24_mem(dst_buff.get(), out, dst_width, dst_height, avatarsQuality);
}

static constexpr char DIR_RESIZED_IMG[] = "resized/";
static constexpr char IMG_EXTENSION[] = ".jpg";

//////////////////////////////////////////////////////////////////////////

#ifdef _SVKS_M_BUILD_
VS_LDAPStorage::VS_AutoLog_Login::~VS_AutoLog_Login()
{
	dprint4("authentication|%s|%s|%s|%d|%s\n",
		login.c_str(),
		oid.c_str(),
		(result)?"success":"denied",
		GetTickCount() - create_tick,
		error_str.c_str());
}

VS_LDAPStorage::VS_AutoLog_AB::~VS_AutoLog_AB()
{
	dprint4("%s|%s|%s|%d|%s\n",
		(ab==AB_COMMON)? "address book": "groups",
		oid.c_str(),
		(result)?"success":"error",
		GetTickCount() - create_tick,
		error_str.c_str());
}

#endif

#include "ProtectionLib/OptimizeDisable.h"
VS_LDAPStorage::VS_LDAPStorage(boost::asio::io_service& ios, const VS_SimpleStr& broker_id,bool useGroups, const std::weak_ptr<VS_TranscoderLogin> &transLogin)
	: VS_SimpleStorage(transLogin)
	, m_state(STATE_CREATED)
	, m_useGroups(useGroups)
#ifdef _WIN32	// not ported
	, m_sec_pack(nullptr)
#else
	, m_ntlm_auth(vs::MakeShared<tc_ldap::NTLMAuthorizer>(ios))
#endif
	, m_ldapCore(nullptr)
#ifdef _SVKS_M_BUILD_
	, m_UseSudis(false)
#endif
{
SECUREBEGIN_C_LDAP;
	m_group_manager = std::make_shared<VS_GroupManager>(true);
	m_ab_storage = new VS_RegABStorage;
#ifdef _WIN32	// not ported
	*(unsigned __int64*)&m_sec_token_expiry=0;
#endif
	m_ab_storage->SetSink(this, this);

	if (!Init(broker_id))
		VS_DBStorageInterface::error_code = !error_code? VSS_LDAP_INIT_ERROR: error_code;

SECUREEND_C_LDAP;
}

VS_LDAPStorage::~VS_LDAPStorage()
{
SECUREBEGIN_C_LDAP;
#ifdef _WIN32	// not ported
  if(*(unsigned __int64*)&m_sec_token_expiry!=0)
    FreeCredentialsHandle(&m_sec_token);
  if(m_sec_pack)
    FreeContextBuffer(m_sec_pack);
#endif
  if (m_ab_storage)
	  delete m_ab_storage;
SECUREEND_C_LDAP;
}

bool VS_LDAPStorage::Init(const VS_SimpleStr& broker_id)
{
#ifdef _SVKS_M_BUILD_
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	if (!key.IsValid())
		return false;
	char host[1024] = {0};
	unsigned long sz = 1024;
	unsigned long port = 0;
	key.GetValue(host, sz, VS_REG_STRING_VT, "Sudis Host");
	key.GetValue(&port, sizeof(unsigned long), VS_REG_INTEGER_VT, "Sudis Port");
	if (!port)
		port = 80;

	if (host && port) {
		sudis::SetSudisAddress(host, (const unsigned short)port);
	}else{
		printf("sudis: invalid sudis server params (host:%s,port:%d)\n", host, port);
		return false;
	}

	memset(host, 0, sizeof(host));
	sz = 1024;
	port = 0;
	key.GetValue(host, sz, VS_REG_STRING_VT, "CryptoPro Host");
	key.GetValue(&port, sizeof(unsigned long), VS_REG_INTEGER_VT, "CryptoPro Port");
	if (!port)
		port = 8001;

	if (host && port) {
		sudis::SetCryptoProAddress(host, (const unsigned short)port);
	}else{
		printf("sudis: invalid CryptoPro server params (host:%s,port:%d)\n", host, port);
		return false;
	}

	memset(host, 0, sizeof(host));
	sz = 1024;
	port = 0;
	key.GetValue(host, sz, VS_REG_STRING_VT, "Sudis SPSB Host");
	key.GetValue(&port, sizeof(unsigned long), VS_REG_INTEGER_VT, "Sudis SPSB Port");
	if (!port)
		port = 80;

	char uuid[1024] = {0};
	sz = 1024;
	key.GetValue(uuid, sz, VS_REG_STRING_VT, "Sudis SPSB UUID");
	if (host && port && uuid) {
		sudis::SetSPSBAddress(host, (const unsigned short)port, uuid);
	}else{
		printf("sudis: invalid SPSB server params (host:%s,port:%d,uuid:%s)\n", host, port, uuid);
	}

	m_UseSudis = sudis::InitWSA();
	if (!m_UseSudis)
	{
		dprint0("Init sudis or CryptoPro failed\n");
	}
	long disable_sudis(false);
	key.GetValue(&disable_sudis, sizeof(disable_sudis), VS_REG_INTEGER_VT, "Disable Sudis");
	if (disable_sudis)
		m_UseSudis = false;
#endif
  // call base
	if (!VS_SimpleStorage::Init(broker_id))
		return false;

	VS_RegistryKey cfg(false, CONFIGURATION_KEY);
	tc::cfg_params_t params;
	tc::ReadCfg(cfg, params);

	tc::eLDAPServerType server_type;
	if (!VS_LDAPFactory::GetLDAPServerType(params, server_type)) return false;
	m_ldapCore = VS_LDAPFactory::CreateInstance(server_type, params, [this](const std::string &login, const std::multimap<tc::attr_name_t, tc::attr_value_t> &attributes)
	{
		this->ProcessAvatars(login, attributes);
	}, true);

	if (!m_ldapCore) return false;

	m_ldapCore->SetLDAPErrorHandler(boost::bind(&VS_LDAPStorage::ProcessLDAPError, this, _1));
	if (!m_ldapCore->SetGroupManager(m_group_manager)) return false;
	//error_code = VS_LDAPCore::Init();
	//if (error_code)
	//	return false;

	int conn_err = m_ldapCore->Connect();
	if (conn_err!=0)
	{
		dprint0("LDAP connect failed err:%d\n", conn_err);
		return false;
	}

#ifndef _WIN32
	m_ntlm_auth->Init(m_ldapCore->m_ldap_server, std::to_string(m_ldapCore->m_ldap_port));
#endif // !_WIN32

	{
		VS_AutoLock lock(&m_appProps_lock);
		m_appProperties.AddValue("authentication_method","1");
	}

#ifdef _WIN32
	if (m_ldapCore->m_use_ntlm)
  {
    SECURITY_STATUS  code = QuerySecurityPackageInfo( L"NTLM", &m_sec_pack );
    if ( code != SEC_E_OK )
    {
      printf( "LDAPS: NTLM not found, code=%08lx\n", code );
      return false;
    }
   }
#endif

SECUREBEGIN_C_LDAP;

	GetPhysRights();
	m_group_manager->UpdateGroupList();

	// kt: pre-init cache with reg-groups (not expanded, without primaryGroupId)

	//tc::LDAPCore::StartUpdateNestedCache();

#ifdef _SVKS_M_BUILD_
	if (!m_region.get())
	{
		char our_endpoint[256] = {0};
		VS_GetServerName(our_endpoint,256);

		VS_WideStr region;
		if (LDAPFindMyRegion(our_endpoint, region) && region.Length())
			m_region.set(region);
		if (!m_region.get())
		{
			dprint3("using base_dn as ldap_region = %S\n", m_basedn.m_str);
			m_region.set(m_basedn);
		}
	}

	dprint0("using ldap_region: %S\n", m_region.get().m_str);
#endif
	m_ldapCore->UpdateLoginGroupCache();
	m_ldapCore->InitUpdateNestedCacheThread();

SECUREEND_C_LDAP;

	m_state = STATE_RUNNING;
	LogEvent(CONNECT_SERVER_EVENT_TYPE);
	return true;
}

#include "ProtectionLib/OptimizeEnable.h"

void VS_LDAPStorage::CleanUp()
{
	VS_SimpleStorage::CleanUp();
	if (m_ldapCore)
	{
		m_ldapCore.reset();
	}
}

bool VS_LDAPStorage::OnPropertiesChange(const char *pass)
{
	bool res = VS_SimpleStorage::OnPropertiesChange(pass);

	VS_AutoLock lock(&m_appProps_lock);
	m_appProperties.AddValue("authentication_method","1");
	return res;
}

bool VS_LDAPStorage::FindUser(const vs_user_id& id,VS_UserData& user, bool)
{
	VS_StorageUserData ude;
	bool result=FindUser(id, ude);
	if (result)
		user=(VS_UserData)ude;
	return result;
}

bool VS_LDAPStorage::FindUser(const vs_user_id& id,VS_StorageUserData& user)
{
	dprint3("LDAP FindUser '%s'\n", id.m_str);
	if (!id || !m_ldapCore || !m_group_manager)
		return false;
	bool result(false);
	VS_RealUserLogin r(SimpleStrToStringView(id));
	if (r.IsGuest())
	{
		auto pLockedUsers = m_users.lock();
		UserMap::ConstIterator it= pLockedUsers->Find(r);
		if(!!it)	{
			user=*(it->data);
			result=true;
		}
		return result;
	}

	// todo(kt): maybe not from cache, but from tc::LDAPCore?
	boost::shared_ptr<VS_StorageUserData> sp;
	{
		VS_AutoLock lock(&m_ldapCore->m_cache_user_info_lock);
		auto it = m_ldapCore->m_cache_user_info.find((const char*)r);
		if (it != m_ldapCore->m_cache_user_info.end())
			sp = it->second;
	}
	if (!sp)
		return false;
	else{
		result = true;
		user = *sp;
	}
	user.m_realLogin = r;
	user.m_login = user.m_name = r;
	VS_UserData::UserRights temp = VS_UserData::UR_NONE;
	FetchRights(user, temp);
	user.m_rights = static_cast<long>(temp);
	FetchTarifOpt(user);

	if (!(user.m_rights & VS_UserData::UR_COMM_CREATEMULTI))
	{
		user.m_rights |= VS_UserData::UR_COMM_CREATEMULTI;
		user.m_tarif_restrictions = 0;
	}
	VS_UserData tmp_ud;
	if (g_storage && g_storage->FindUser(SimpleStrToStringView(id), tmp_ud))
		user.m_appID = tmp_ud.m_appID;

	if (user.m_groups.empty())	// need to get users groups
	{
		std::map<std::string, VS_RegGroupInfo> reg_groups;
		m_ldapCore->GetRegGroups(reg_groups);

		auto c = m_ldapCore->m_cache_groups_users.load();
		if (c)
		{
			for (const auto& rg : reg_groups)
			{
				auto g = c->find(rg.second.ldap_dn);
				if (g != c->end() && g->second)
				{
					if (g->second->find((const char*)user.m_realLogin) != g->second->end())
						user.m_groups.push_back(rg.first);
				}
			}
		}
	}

	return result;
}

bool VS_LDAPStorage::FindUserByAlias(const std::string& alias, VS_StorageUserData& user)
{
	VS_AutoLock lock(&m_ldapCore->m_cache_user_info_lock);
	for (auto const& p : m_ldapCore->m_cache_user_info)
	{
		if (!p.second)
			continue;
		VS_StorageUserData& ude = *p.second;
		for (auto const& p : ude.m_phones)
		{
			if (alias == p.phone.m_str)
			{
				user = ude;
				return true;
			}
		}
	}
	return false;
}

int VS_LDAPStorage::LDAPDoLogin(const vs_user_id& login_,const VS_SimpleStr& password,const VS_SimpleStr& appID, VS_SimpleStr& autoKey, VS_StorageUserData& user, bool check_pass, VS_Container& prop_cnt, const VS_ClientType &client_type)
{
	bool IsTerminalLoginAllowed = false;
	bool IsTerminalPro = false;
	if (client_type == CT_TERMINAL)
	{
		CheckLic_TerminalPro(IsTerminalLoginAllowed, IsTerminalPro);
		if (!IsTerminalLoginAllowed)
			return INVALID_CLIENT_TYPE;
	}

	int error=-1;

	tc::ldap_user_info found;
	std::map<std::string, VS_RegGroupInfo> user_reg_groups;
	std::string user_at_domain;
	auto res = m_ldapCore->LoginUser_CheckLogin(login_.m_str, found, user_reg_groups, user_at_domain);
	dstream3 << "LoginUser_CheckLogin(" << login_.m_str << ")=" << (int)res;
	if (res == tc::LDAPCore::CheckLoginResult::OK)
		;
	else if (res == tc::LDAPCore::CheckLoginResult::NOT_FOUND_OR_LDAPDISABLED_OR_AMBIGUOUS)
		return ACCESS_DENIED;
	else if (res == tc::LDAPCore::CheckLoginResult::NOT_IN_LOGIN_GROUP)
		return USER_DISABLED;
	else if (res == tc::LDAPCore::CheckLoginResult::LDAP_ERROR)
		return RETRY_LOGIN;
	else
		return ACCESS_DENIED;

	VS_RealUserLogin r(user_at_domain);

		std::string reg_user_escaped = VS_GetUserPartEscaped(r.GetUser().c_str());

		if (!password && autoKey)		// try autologin with registry
		{
			VS_SimpleStr root = USERS_KEY;	root += "\\";	root += reg_user_escaped.c_str();	root += "\\AutoLogins";
			char buff[1024]={0};
			VS_RegistryKey logins(false, root.m_str);
			if (logins.IsValid() &&
				logins.GetValue(buff, sizeof(buff)-1, VS_REG_STRING_VT, appID) &&
				strcasecmp(buff, autoKey.m_str)==0)
			{
				check_pass = false;
			}
		}

		// try digest authentication scheme
		if (check_pass && password)	{
			VS_SimpleStr root = USERS_KEY;	root += "\\";	root += reg_user_escaped.c_str();
			char buff[1024]={0};
			VS_RegistryKey usr(false, root.m_str);
			VS_TempUserData data;
			if (usr.IsValid() && usr.GetValue(buff, sizeof(buff)-1, VS_REG_STRING_VT, "HA1 Password"))
				data.m_HA1_password = buff;
			memset(buff, 0, sizeof(buff));
			if (usr.IsValid()) {
				if (usr.GetValue(buff, sizeof(buff) - 1, VS_REG_STRING_VT, H323_PASSWORD_TAG_NEW) > 0)
					data.m_h323_password = sec::DecryptRegistryPassword(usr.GetName(), buff).c_str();
				else if (usr.GetValue(buff, sizeof(buff) - 1, VS_REG_STRING_VT, H323_PASSWORD_TAG_OLD) > 0)
					data.m_h323_password = buff;
			}
				
			check_pass = !TryTerminalLogin(data, r.GetUser().c_str(), password) &&
				!TryTerminalLogin(data, login_.m_str, password);
		}


		if(check_pass)
		{
			if (client_type == VS_ClientType::CT_WEB_CLIENT && !m_license_checker(LE_WEBINARS))
				return ACCESS_DENIED;
			if (password.IsEmpty())
			{
				dprint3(" empty password\n");
				return ACCESS_DENIED;
			}
			if (!m_ldapCore->LoginUser_CheckPassword(user_at_domain, password.m_str, found.dn))
				return ACCESS_DENIED;
		}

		error = FetchUser(found, user) ? 0 : ACCESS_DENIED;
		user.m_client_type = client_type;
		user.m_appID = appID;

		// todo(kt): if cache not ready yet, then fill m_groups with info from GetUserRightsImp()
		for(auto const& g: user_reg_groups)
			user.m_groups.emplace_back(g.first);



#ifdef _SVKS_M_BUILD_
		auto FetchProperty = [&found_users, &prop_cnt](const tc::attr_name_t& a){
			auto it = found_users[0].custom_attrs.find(a);
			if (it != found_users[0].custom_attrs.end())
			{
				VS_WideStr wstr(a.c_str());
				std::string buff;
				if (wstr.ToUTF8(buff))
					prop_cnt.AddValue(buff, it->second.c_str());
			}
		};
		FetchProperty(m_a_ALLOWED_BY_SERVER_MAX_BW);
		FetchProperty(m_a_ALLOWED_BY_SERVER_MAX_FPS);
		FetchProperty(m_a_ALLOWED_BY_SERVER_MAX_WXH);
#endif

	if (error==USER_LOGGEDIN_OK)
	{
		user.m_rights = GetUserRightsImp(user_reg_groups, true);
		if (!(user.m_rights & VS_UserData::UR_LOGIN))
			return USER_DISABLED;
		if (client_type == CT_TERMINAL && IsTerminalPro)
			user.m_rights |= VS_UserData::UR_COMM_PROACCOUNT;
		FetchTarifOpt(user);

		VS_RealUserLogin r_tmp(SimpleStrToStringView(user.m_name));
		VS_SimpleStr alias_full_or_short = (r_tmp == user.m_name) ? r_tmp.GetUser().c_str() : (const char*)r;
		user.m_aliases.Assign(alias_full_or_short,0);
		for (const auto& x : found.custom_attrs)
			if (!strcasecmp(x.first.c_str(), m_ldapCore->m_a_email.c_str()) && !x.second.empty())
				user.m_aliases.Assign(x.second.c_str(), 0);
		std::set<std::string> postfix;
		GetServerIPAsAliasPostfix(postfix);				// all postfix (@ip, @sip_ip, @external_sip_ip
		for (const auto &alias_tag : m_ldapCore->m_user_aliases) {
			auto range = found.custom_attrs.equal_range(alias_tag);
			for (auto it = range.first; it != range.second; ++it) {
				const char* alias = it->second.c_str();
				user.m_aliases.Assign(alias, 0);
				for(const auto& p : postfix) {
					std::string a = alias;
					a += p;
					user.m_aliases.Assign(a.c_str(), 0);
				}
			}
		}

		AddServerIPAsAlias(user);

		if (password || !autoKey)		// save auto login key
		{
			const unsigned int c_MD5_HASH_SIZE = 32;
			autoKey.Resize(c_MD5_HASH_SIZE + 1);
			VS_GenKeyByMD5(autoKey.m_str);
			SaveAutoLoginKey(reg_user_escaped, appID, autoKey);
		}

		OnUserLoggedInAtEndpoint(&user);
		ReadApplicationSettings(user);
		UpdateCacheOnLogin(user);
	}
	return error;
}

int VS_LDAPStorage::LoginAsUser(const VS_SimpleStr& login,const VS_SimpleStr& password,const VS_SimpleStr& appID, VS_SimpleStr& autoKey, VS_StorageUserData& ude, VS_Container& prop_cnt, const VS_ClientType &client_type)
{
#ifdef _SVKS_M_BUILD_
	VS_AutoLog_Login log;
	log.login = login;
#endif
	if (!m_license_checker(LE_LOGIN))
	{
#ifdef _SVKS_M_BUILD_
		log.error_str = "license limit";
#endif
		return LICENSE_USER_LIMIT;
	}

	bool check_pass(true);
#ifdef _SVKS_M_BUILD_
	sudis::oid sudis_user_oid;
	VS_RealUserLogin r(SimpleStrToStringView(login));
	VS_RealUserLogin wr(SimpleStrToStringView(login));
	if (m_UseSudis)
	{
		check_pass = false;
		std::string user_token;
		bool sudis_res = false;

		if (!password && autoKey) {		// autologin
			sudis_res = sudis::CheckUserList(r.GetUser());
			check_pass = true;

			VS_SimpleStr key_name = USERS_KEY; key_name += "\\"; key_name += r.GetUser();
			VS_RegistryKey key(false, key_name);
			if (key.IsValid())
			{
				if (key.GetValue(wlogin.m_str, VS_REG_WSTRING_VT, "Sudis OID") > 0)
				{
					wlogin.ToUTF8(log.oid);
				}
			}

		}else{
			char str1[2048] = {0};
			char str2[1024] = {0};
			sudis_res = sudis::CheckAccount(r.GetUser(), password, &(str1[0]), &(str2[0]));
			user_token = str1;
			sudis_user_oid = str2;
			log.oid = sudis_user_oid;
		}

		dprint3("sudis login %s result:%d user_token:%s autoKey:%s\n", r.GetUser(), sudis_res, user_token.c_str(), autoKey);
		if (!sudis_res) {
			log.error_str = "sudis denied login at ";
			log.error_str += (!password && autoKey)?"cciUserListV2":"cciUserLoginV1";
			return ACCESS_DENIED;
		}else{
			if (sudis_user_oid.length())
				wlogin.AssignStr(sudis_user_oid.c_str());

			char* ptr_tmp = wlogin.ToStr();
			wr = ptr_tmp;
			free((void*)ptr_tmp);
			char key_name[512];key_name[sizeof(key_name)-1]=0;
			_snprintf(key_name,sizeof(key_name)-1,"%s\\%s", USERS_KEY, wr.GetUser().c_str());

			VS_RegistryKey key(false, key_name, false, true);
			if (key.IsValid())
			{
				key.SetString(user_token.c_str(), "sudis_userTokenID");
			}
		}
	}
#endif

	int res = LDAPDoLogin(login,password,appID,autoKey,ude,check_pass,prop_cnt,client_type);

#ifdef _SVKS_M_BUILD_
	if (res == USER_LOGGEDIN_OK)
	{
		if (!!password && sudis_user_oid.length() && login!=sudis_user_oid.c_str())	// not an autologin
		{
			VS_SimpleStr key_name = USERS_KEY; key_name += "\\"; key_name += r.GetUser();
			VS_RegistryKey key(false, key_name, false, true);
			if (key.IsValid())
			{
				key.SetValue(wlogin, 0, VS_REG_WSTRING_VT, "Sudis OID");
			}
		}

		// bug#23296
		dprint1( "ClientCapabilities of %s\r\n" \
		"[TransportCapabilities]\r\n" \
		"  TCP\r\n" \
		"  UDP Multicast\r\n" \
		"  UDP Unicast\r\n" \
		"[TransportAndIdentification]\r\n" \
		"  SSLv1\r\n" \
		"  SSLv2\r\n" \
		"  SSLv3\r\n" \
		"  TLSv1\r\n" \
		"  SASL\r\n" \
		"  TransportCompressionUsingVoiceActivityDetection\r\n" \
		"[VideoCodecsAndCapabilities]\r\n" \
		"  VP8\r\n" \
		"  Cyclone\r\n" \
		"  VP8 SVC\r\n" \
		"  H.261\r\n" \
		"  H.263\r\n" \
		"  H.264\r\n" \
		//	H.264+
		"[AudioCodecsAndCapabilities]\r\n" \
		"  Speex\r\n" \
		"  g.711\r\n" \
		"  g.722\r\n" \
		"  g.722.1\r\n" \
		"  g.723\r\n" \
		"  g.728\r\n" \
		"  g.729\r\n" \
		"  Opus\r\n" \
		"  EchoSuppression\r\n" \
		"  EchoCancellation\r\n" \
		"  NoiseSuppression\r\n" \
		"  AutomaticGainGontrol\r\n" \
		"  VoiceActivityDetection\r\n" \
		"[DirectoryProtocols]\r\n" \
		"  LDAPv2\r\n" \
		"  LDAPv3\r\n",
		wr.GetUser().c_str());
		log.result = true;
	}else{
		log.error_str = "ldap denied login";
	}
#endif

	return res;
}

int VS_LDAPStorage::FindUsers(VS_Container& cnt, int& entries, VS_AddressBook ab, const vs_user_id& owner, const std::string& query, long client_hash, VS_Container& in_cnt)
{
	dstream4 << "LDAP FindUsers: ab=" << ab << ", owner=" << owner.m_str << " q'" << query << "', client_hash=" << client_hash << "\n";

	entries=0;
	VS_RealUserLogin r(SimpleStrToStringView(owner));
	if (r.IsGuest() || !m_ldapCore)
		return SEARCH_FAILED;

	if (ab == AB_PERSON_DETAILS && !query.empty())
	{
		VS_StorageUserData ud;
		VS_RealUserLogin r(query);
		auto user_part = r.GetUser();
		if (r.IsGuest())
		{
			if (g_storage->FindUser(query, ud))
			{
				entries++;
				cnt.AddValue(USERNAME_PARAM, ud.m_name);
				cnt.AddValue(CALLID_PARAM, ud.m_name);
				cnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
				++entries;
			}
			return SEARCH_DONE;
		}

		bool found(false);
		bool ldap_found(false);
		std::vector<tc::ldap_user_info> v;
		if (VS_IsNotTrueConfCallID(query)) {
			VS_UserData ud_tmp;
			found = g_storage->FindUser(query, ud_tmp);
			(*((VS_UserData*)&ud)) = ud_tmp;
		}
		else {
			char buff[4099] = { 0 };
			char buff2[4096] = { 0 };
			snprintf(buff2, sizeof(buff2) / sizeof(buff2[0]), m_ldapCore->m_filter_login.c_str(), user_part.c_str());
			snprintf(buff, sizeof(buff) / sizeof(buff[0]), "(&%s%s)", m_ldapCore->m_filter_disabled.get().c_str(), buff2);		// not Disabled users

			std::vector<tc::attr_name_t> custom_attrs = {
				m_ldapCore->m_a_company,
				m_ldapCore->m_a_firstname,
				m_ldapCore->m_a_lastname
			};
			custom_attrs.insert(custom_attrs.end(), m_ldapCore->m_detailed_user_info.begin(), m_ldapCore->m_detailed_user_info.end());
			ldap_found = m_ldapCore->SearchForUser(buff, v, &custom_attrs) == LDAP_SUCCESS && v.size() == 1 && FetchUser(v[0], ud);
			if (!ldap_found)
			{
				m_ldapCore->FetchForeignUserByLogin(query, v);
				if (v.size() == 1)
				{
					ldap_found = true;
					FetchUser(v[0], ud);
				}
			}
		}
		if (found || ldap_found)
		{
			cnt.AddValue(USERNAME_PARAM, r);
			cnt.AddValue(CALLID_PARAM, r);
			cnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
			cnt.AddValue(FIRSTNAME_PARAM, ud.m_FirstName);
			cnt.AddValue(LASTNAME_PARAM, ud.m_LastName);
			cnt.AddValue(USERCOMPANY_PARAM, ud.m_Company);
			if (ldap_found)
				for (const auto& d : m_ldapCore->m_detailed_user_info)
				{
					auto it = v[0].custom_attrs.find(d);
					if (it != v[0].custom_attrs.end())
						cnt.AddValue(it->first, it->second.c_str());
				}
			++entries;
		}

		dstream3 << "PersonDetails: owner=" << owner.m_str << ", query=" << query << ", entries=" << entries << ", displayName='" << ud.m_displayName << "'\n";
		return SEARCH_DONE;
	} else if (ab == AB_PHONES) {
		return FindUsersPhones(cnt, entries, VS_GetUserPartEscaped(owner.m_str), client_hash);
	} else {
		return VS_SimpleStorage::FindUsers(cnt, entries, ab, owner, query, client_hash, in_cnt);
	}
}

#ifdef _SVKS_M_BUILD_
bool VS_LDAPStorage::GetABForUserImp(const vs_user_id& owner, VS_AbCommonMap& m)
{
	VS_AutoLog_AB log;
	log.ab = AB_COMMON;
	if (!!owner)
		log.oid = owner;

	std::vector<std::wstring> users_dn;
	ldap_error_code_t err = LDAPGetUserAddressBookAttribute(owner,users_dn);
	if (err!=LDAP_SUCCESS || !users_dn.size())
	{
		if (err!=LDAP_SUCCESS) {
			std::stringstream ss;
			ss << "get user " << owner << " addressBook attribute error: " << err;
			log.error_str = ss.str();
		} else {
			log.result = true;
		}
		return true; // todo(kt): error or null size AB_COMMON ?
	}

	std::map<std::wstring, std::wstring> custom_displayNames;	// key=uid, value=customDisplayName

	char buff[1024] = {0};
	std::wstring filter;		// make filter like: (uid=123)(uid=456)(uid=789)
	for(std::vector<std::wstring>::iterator it=users_dn.begin(); it!=users_dn.end(); ++it)
	{
		if (!it->length())
			continue;

		// split to CallId and CustomDisplayName (by delimiter ~#)
		VS_WideStr wstr = it->c_str();
		std::wstring w_call_id;
		VS_WideStr custom_display_name;
		std::vector<std::wstring> arr;
		boost::iter_split(arr, wstr.m_str, boost::first_finder(STRING_SEPARATOR_W.m_str));
		if (arr.size() == 2) {
			w_call_id = arr[0].c_str();
			custom_display_name = arr[1].c_str();
		}else{
			w_call_id = wstr;
			custom_display_name = wstr;
		}

		// get uid=123 from whole dn
		size_t pos1 = w_call_id.find_first_of(L"uid=");
		size_t pos2 = w_call_id.find_first_of(L",");

		if (pos1==std::string::npos ||
			pos2==std::string::npos ||
			pos2 <= pos1)
		{
			// it is not DN, it should be #sip:, so add to abook map
			std::string tmp;
			if (((VS_WideStr)w_call_id.c_str()).ToUTF8(tmp))
				m[tmp] = VS_AbCommonMap_Item(custom_display_name.m_str);
			continue;
		}

		std::wstring uid = w_call_id.substr(pos1, pos2-pos1);

		filter += L"(";
		filter += uid;
		filter += L")";

		// skip "uid=" and save custom displayName
		if (w_call_id!=custom_display_name.m_str)
			custom_displayNames[uid.substr(4,uid.length()-4)] = custom_display_name.m_str;
	}

	if (!filter.empty())		// make OR-filter (|%s)
	{
		filter = std::wstring(L"(|") + filter + std::wstring(L")");
	}


	const wchar_t* attrs[6] = {
		m_a_displayname,
		m_a_ServerName,
		m_a_login,
		L"mvdPosition",
		L"ou",
		0
	};

	std::vector<attrs_t> out;
	err = LDAPSearchImp(m_ldap,m_basedn,LDAP_SCOPE_SUBTREE,filter,&attrs[0],out);
	if (err!=LDAP_SUCCESS) {
		log.result = false;
		std::stringstream ss;
		ss << "get user " << owner << " addressBook content error: " << err;
		log.error_str = ss.str();
	} else {
		log.result = true;
	}

	// prepare result for add to abook map
	for(std::vector<attrs_t>::const_iterator it=out.begin(); it!=out.end(); ++it)
	{
		std::wstring uid;
		std::wstring server_name;
		std::wstring displayName;
		std::wstring mvdPosition;
		std::wstring ou;
		for(attrs_t::const_iterator it2=it->begin(); it2!=it->end(); ++it2)
		{
			if (it2->first == m_a_ServerName.m_str)
				server_name = it2->second;
			else if (it2->first == m_a_login.m_str)
				uid = it2->second;
			else if (it2->first == m_a_displayname.m_str)
				displayName = it2->second;
			else if (it2->first == L"mvdPosition")
				mvdPosition = it2->second;
			else if (it2->first == L"ou")
				ou = it2->second;
		}
		if (!server_name.empty() && !uid.empty())
		{
			server_name = server_name.substr(0, server_name.find(L"#"));	// remove #vcs

			VS_SimpleStr call_id; call_id.Attach(((VS_WideStr)uid.c_str()).ToUTF8());
			call_id += "@";
			VS_SimpleStr tmp; tmp.Attach(((VS_WideStr)server_name.c_str()).ToUTF8());
			call_id += tmp;

			std::map<std::wstring, std::wstring>::const_iterator it_tmp = custom_displayNames.find(uid);
			if (it_tmp!=custom_displayNames.end())
				displayName = it_tmp->second;

			m[call_id.m_str] = VS_AbCommonMap_Item(displayName, mvdPosition, ou);
		}
	}

	return true;
}
#else
bool VS_LDAPStorage::GetABForUserImp(const vs_user_id& owner, VS_AbCommonMap& m)
{
	if (!m_ldapCore || m_ldapCore->m_filter_ab.empty())
		return false;

	std::vector<tc::ldap_user_info> found_users;
	m_ldapCore->SearchForUser(m_ldapCore->m_filter_ab.c_str(), found_users);
	for (const auto& u : found_users)
		m[u.login] = VS_AbCommonMap_Item(u.displayName);
	return true;
}
#endif

#include "ProtectionLib/OptimizeDisable.h"
bool VS_LDAPStorage::GetParticipantLimit (const vs_user_id& user_id, VS_ParticipantDescription::Type type, int& rights, double& limit, double& decLimit)
{
  bool allow=false;
SECUREBEGIN_C_LDAP;
  limit=1.;
  decLimit=0.;
  // set rights

  int urights=GetUserRights(user_id);
  rights = VS_ParticipantDescription::RIGHTS_NORMAL;

	switch(type)
	{
	case VS_ParticipantDescription::PRIVATE_HOST:
	case VS_ParticipantDescription::HPRIVATE_HOST:
    if(urights&VS_UserData::UR_COMM_CALL)
    {
		  rights|= VS_ParticipantDescription::RIGHTS_RCV_LIST;
		  rights|= VS_ParticipantDescription::RIGHTS_RCV_CHAT;
	    limit=1.;
      allow=true;
    }
    else
      error_code=VSS_CONF_ACCESS_DENIED;
		break;

	case VS_ParticipantDescription::PUBLIC_HOST:
    if(urights&VS_UserData::UR_COMM_BROADCAST)
    {
		  rights|= VS_ParticipantDescription::RIGHTS_RCV_LIST;
		  rights|= VS_ParticipantDescription::RIGHTS_RCV_CHAT;
	    limit=1.;
      allow=true;
    }
    else
      error_code=VSS_CONF_ACCESS_DENIED;
		break;

	case VS_ParticipantDescription::PRIVATE_MEMBER:
	case VS_ParticipantDescription::PUBLIC_MEMBER:
	case VS_ParticipantDescription::HPRIVATE_MEMBER:
	case VS_ParticipantDescription::MULTIS_MEMBER:
	case VS_ParticipantDescription::INTERCOM_MEMBER:
	  rights|= VS_ParticipantDescription::RIGHTS_RCV_LIST;
		rights|= VS_ParticipantDescription::RIGHTS_RCV_CHAT;
	  limit=1.;
    allow=true;
		break;

	case VS_ParticipantDescription::HPRIVATE_GUEST:
	  limit=1.;
    allow=true;
		break;

  default:
		limit=1.;
    allow=true;
    break;
  }
SECUREEND_C_LDAP;
	return allow;

}

int VS_LDAPStorage::WriteAvatarToLDAP(const std::string &dn, const std::string &fileName, int avatarSize, const std::string &attr)
{
	assert(!attr.empty());
	assert(!dn.empty());

	std::vector<std::uint8_t> out;
	bool resized;
	if (resize_avatar(fileName, out, avatarSize, m_ldapCore->m_avatars_quality, resized) && out.size() <= MAX_AVATAR_SIZE ? true
		: (out.clear(), resize_avatar(fileName, out, tc::LDAP_AVATARS_SIZE_INIT, m_ldapCore->m_avatars_quality, resized)))
	{
		assert(out.size() <= MAX_AVATAR_SIZE);
		return m_ldapCore->WriteAvatar(dn, attr, out);
	}
	return -1;
}


int VS_LDAPStorage::GetUserRightsImp(const vs_user_id& id)
{
	if (!id || !m_ldapCore || !m_group_manager)
		return false;
	int rights(VS_UserData::UR_NONE);
SECUREBEGIN_C_LDAP;
	LDAPMessage* lmsg=0;
	wchar_t* user_dn=0;

	m_group_manager->UpdateGroupList();

	VS_RealUserLogin r(SimpleStrToStringView(id));

	bool can_login = m_ldapCore->m_login_group.get().empty(); //can login if no group specified

	long grpRights=0;
	int  user_groups=0;

	std::map<std::string, VS_RegGroupInfo> reg_groups;
	m_ldapCore->GetRegGroups(reg_groups);

	auto c = m_ldapCore->m_cache_groups_users.load();
	if (c)
	{
		for (const auto& rg : reg_groups)
		{
			if (rg.second.ldap_dn.empty())
				continue;

			auto g = c->find(rg.second.ldap_dn);
			if (g != c->end() && g->second)
			{
				if (g->second->find((const char*)r) != g->second->end())
				{
					grpRights |= rg.second.rights;
					user_groups++;
				}
			}
		}

		// login group
		if (!m_ldapCore->m_login_group.get().empty())
		{
			auto g = c->find(m_ldapCore->m_login_group.get());
			if (g != c->end() && !!g->second)
			{
				if (g->second->find((const char*)r) != g->second->end())
					can_login = true;
			}
		}
	}
	if(user_groups==0||!m_useGroups)
		grpRights = m_group_manager->m_defRights;

	rights =VS_UserData::UR_APP_COMMUNICATOR;
	if(can_login)
		rights|=VS_UserData::UR_LOGIN;
	rights|= GetPhysRights() & grpRights;
SECUREEND_C_LDAP;
	dprint3("$GetUserRightsImp for %s returned %04x\n",(const char*)id,rights);
	return rights;
}

// Temporary hack to fix applying VMProtect on Windows (64-bit), until VMProtect is integrated properly.
#if defined(ENABLE_VMPROTECT_BUILD) && defined(_MSC_VER)
#pragma optimize("", off)
#endif
int VS_LDAPStorage::GetUserRightsImp(const std::map<std::string, VS_RegGroupInfo>& reg_user_groups, bool IsInsideLoginGroup)
{
	int rights(VS_UserData::UR_NONE);
	if (!m_ldapCore || !m_group_manager) return rights;
SECUREBEGIN_C_LDAP;
	LDAPMessage* lmsg=0;
	wchar_t* user_dn=0;

	m_group_manager->UpdateGroupList();

	std::map<std::string, VS_RegGroupInfo> reg_groups;
	m_ldapCore->GetRegGroups(reg_groups);

	bool can_login = (m_ldapCore->m_login_group.get().empty()) ? true : IsInsideLoginGroup;		//can login if no login group specified
	long grpRights=0;
	int  user_groups=0;

	for (const auto& x : reg_user_groups)
	{
		grpRights |= x.second.rights;
		user_groups++;
	}

	if(user_groups==0||!m_useGroups)
		grpRights = m_group_manager->m_defRights;

	rights =VS_UserData::UR_APP_COMMUNICATOR;
	if(can_login)
		rights|=VS_UserData::UR_LOGIN;
	rights|= GetPhysRights() & grpRights;
SECUREEND_C_LDAP;
	dprint3("$GetUserRightsImp returned %04x\n",rights);
	return rights;
}
#if defined(ENABLE_VMPROTECT_BUILD) && defined(_MSC_VER)
#pragma optimize("", on)
#endif

int VS_LDAPStorage::GetUserRights(const vs_user_id& id)
{
	int rights=0;
SECUREBEGIN_C_LDAP;
	rights=GetUserRightsImp(id);
SECUREEND_C_LDAP;
	dprint3("$GetUserRights for %s returned %04x\n",(const char*)id,rights);
	return rights;
}

// Temporary hack to fix applying VMProtect on Windows (64-bit), until VMProtect is integrated properly.
#if defined(ENABLE_VMPROTECT_BUILD) && defined(_MSC_VER)
#pragma optimize("", off)
#endif
bool VS_LDAPStorage::Authorize(const VS_SimpleStr& src_ep, VS_Container* in_cnt, VS_Container& out_cnt, bool& request, VS_UserData& ud,const VS_ClientType &client_type)
{
	if (!m_ldapCore || !m_ldapCore->m_use_ntlm)
    return false;

  bool result=false;

SECUREBEGIN_C_LDAP;
  dprint3("LDAPS: Auth %s\n", src_ep.m_str);
  request=false;

  void* in_data = nullptr;
  size_t in_data_size;

  if(in_cnt)
    in_data=(void*)in_cnt->GetBinValueRef(DATA_PARAM, in_data_size);


  if(!in_cnt || !in_data) //first step
  {
	  out_cnt.AddValueI32(TYPE_PARAM, LA_NTLM);
	  request=true;

#ifdef _WIN32 // not ported
	  VS_EpCtxtMap::Iterator ep_ctxt=m_sec_ctxt[src_ep];
	  if(!!ep_ctxt)
	  {
		  PCtxtHandle out_ctxt=ep_ctxt;
		  DeleteSecurityContext(out_ctxt);
		  delete out_ctxt;
		  m_sec_ctxt.Erase(ep_ctxt);
	  }
#endif

	  dprint3("step0\n");
	  result = true;
  }
  else
  {
	  int32_t type = -1;
	  in_cnt->GetValue(TYPE_PARAM, type);
	  dprint3("step1 t%d\n", type);
#ifndef _WIN32
	  if (type == LA_NTLM) {
		  int ntlmRes = m_ntlm_auth->DoAuthorize(src_ep.m_str, in_data, in_data_size, out_cnt);
		  if (ntlmRes == LDAP_SASL_BIND_IN_PROGRESS) {
			  result = request = true;
		  }
		  else if (ntlmRes == LDAP_SUCCESS) {
			  using namespace ntlm;

			  request = false;
			  result = false;
			  const AUTHENTICATE_MESSAGE_HEADER& ntlmAuth = *reinterpret_cast<const AUTHENTICATE_MESSAGE_HEADER*>(in_data);
			  assert(ntlmAuth.userName.responseLen % 2 == 0);
			  if (ntlmAuth.userName.responseLen > 0 && ntlmAuth.userName.responseLen % 2 == 0) {	// support only UCS2 for now
				  u16string_view user(reinterpret_cast<char16_t*>(static_cast<char*>(in_data) + ntlmAuth.userName.responseBufferOffset), ntlmAuth.userName.responseLen / 2);

				  VS_Container prop_cnt;
				  VS_SimpleStr autoKey;
				  VS_StorageUserData ude;
				  result = LDAPDoLogin(vs::UTF16toUTF8Convert(user).c_str(), 0, src_ep, autoKey, ude, false, prop_cnt, client_type) == 0;
				  ud = *(VS_UserData*)&ude;
  }
		  }
	   }
#else
	  switch(type)
	  {
	  case LA_NTLM:
		  {
			  NTLMRenewToken();
			  SecBufferDesc out_desc, in_desc;
			  SecBuffer out_buf, in_buf;

			  in_desc.ulVersion = SECBUFFER_VERSION;
			  in_desc.cBuffers  = 1;
			  in_desc.pBuffers  = &in_buf;
			  in_buf.BufferType = SECBUFFER_TOKEN;
			  in_buf.cbBuffer   = in_data_size;
			  in_buf.pvBuffer   = in_data;

			  if(in_buf.pvBuffer!=0)
			  {
				  out_desc.ulVersion = SECBUFFER_VERSION;
				  out_desc.cBuffers = 1;
				  out_desc.pBuffers = &out_buf;
				  out_buf.BufferType = SECBUFFER_TOKEN; // preping a token here
				  out_buf.cbBuffer = m_sec_pack->cbMaxToken;
				  out_buf.pvBuffer = new char[out_buf.cbBuffer];


				  VS_EpCtxtMap::Iterator ep_ctxt=m_sec_ctxt[src_ep];
				  PCtxtHandle out_ctxt=NULL,in_ctxt=NULL;
				  bool new_context=false;
				  if(!!ep_ctxt)
					  out_ctxt=in_ctxt=ep_ctxt;
				  else
				  {
					  new_context=true;
					  out_ctxt=new CtxtHandle;
				  }

				  TimeStamp ts;
				  unsigned long attr;

				  SECURITY_STATUS code=
					  AcceptSecurityContext(&m_sec_token, in_ctxt, &in_desc, 0,
					  SECURITY_NATIVE_DREP, out_ctxt, &out_desc,
					  &attr, &ts);

				  if( code== SEC_I_COMPLETE_AND_CONTINUE || code == SEC_I_COMPLETE_NEEDED )
				  {
					  dprint3(" CTOKEN\n");
					  CompleteAuthToken( out_ctxt, &out_desc );
					  if ( code == SEC_I_COMPLETE_NEEDED )
						  code = SEC_E_OK;
					  else
						  code = SEC_I_CONTINUE_NEEDED;
				  }

				  if(code==SEC_I_CONTINUE_NEEDED)
				  {
					  dprint3(" CONTINUE\n");
					  out_cnt.AddValueI32(TYPE_PARAM, LA_NTLM);
					  out_cnt.AddValue(DATA_PARAM,out_buf.pvBuffer,out_buf.cbBuffer);

					  //save context
					  m_sec_ctxt[src_ep]=out_ctxt;
					  //continue
					  request=true;
					  result=true;
				  }
				  else
				  {
					  if (code==SEC_E_OK)
					  {
						  SecPkgContext_Names user_name;
						  QueryContextAttributes(out_ctxt,SECPKG_ATTR_NAMES,&user_name);
						  dstream3 << " DONE user:" << user_name.sUserName << "\n";
						  wchar_t* login=wcschr(user_name.sUserName, '\\');
						  if(!login || !*login)
							  login = user_name.sUserName;
						  else
							  login += 1;


						  VS_Container prop_cnt;
						  VS_SimpleStr autoKey;
						  if (login && *login) {
							  VS_StorageUserData ude;
							  result = LDAPDoLogin(vs::WideCharToUTF8Convert(login).c_str(), 0, src_ep, autoKey, ude, false, prop_cnt, client_type) == 0;
							  ud = *(VS_UserData*)&ude;
						  }
						  FreeContextBuffer(user_name.sUserName);

					  }
					  else
					  {
						  dprint3(" ERROR %08lx\n", code);
						  result=false;
					  };

					  if(!new_context)
						  DeleteSecurityContext(out_ctxt);
					  delete out_ctxt;
					  m_sec_ctxt.Erase(src_ep);
				  }

				  free(out_buf.pvBuffer);
			  }

			  return result;
		  }

	  default:
		  dprint1("LDAPS:unsupported auth type %d\n",type);
		  //return false;
	  }
#endif
  }// second step
SECUREEND_C_LDAP;
	return result;
}
#if defined(ENABLE_VMPROTECT_BUILD) && defined(_MSC_VER)
#pragma optimize("", on)
#endif

#ifdef _SVKS_M_BUILD_
bool VS_LDAPStorage::AuthByECP(VS_Container& cnt, VS_Container& rCnt, VS_UserData& ud, VS_SimpleStr& autoKey, VS_Container& prop_cnt, VS_ClientType client_type)
{
	if (!m_UseSudis)
		return false;
	sudis::ticketId ticketId;
	const char* ptr = cnt.GetStrValueRef(TICKET_ID_PARAM);
	if (ptr)
		ticketId = ptr;
	if (ticketId.empty())		// step 1: getTicket
	{
		dprint3("AuthByECP: step1 - TicketV3\n");
		char tId[1024] = {0};
		char tBody[4096] = {0};
		if (sudis::CheckAccountBySmartCard_step1(tId, tBody) && strlen(tId) && strlen(tBody))
		{
			rCnt.AddValue(METHOD_PARAM,  AUTH_BY_ECP_METHOD);
			rCnt.AddValue(TICKET_ID_PARAM, tId);
			rCnt.AddValue(TICKET_BODY_PARAM, tBody);
		}
		return false;
	}

	// step 2: try login to Sudis, using cciUserLoginV3
	VS_AutoLog_Login log;
	dprint3("AuthByECP: step2 - cciUserLoginV3\n");
	sudis::oid sudis_user_oid;
	{
		// todo(kt): implement
		// success login result from sudis return attributes, where oid is user_id

		sudis::signedTicketBody signedTicketBody;
		const char* ptr = cnt.GetStrValueRef(SIGNED_TICKET_BODY_PARAM);
		if (ptr)
			signedTicketBody = ptr;

		if (ticketId.length() && signedTicketBody.length())
		{
			dprint3("step2: ticketId:%s, signedTicketBody:%s\n", ticketId.c_str(), signedTicketBody.c_str());
			char tmp[2048] = {0};
			sudis::CheckAccountBySmartCard_step2(ticketId.c_str(), signedTicketBody.c_str(), signedTicketBody.length(), &(tmp[0]));
			sudis_user_oid = tmp;
		}

		// TEST!!!
		//sudis_user_oid = "51bb128df27369bea00001cf";
		//////
	}
	///////
	log.oid = sudis_user_oid;

	if (!sudis_user_oid.length())
	{
		log.error_str = "sudis cciUserLoginV3 has no oid for ticket_id:";
		log.error_str += ticketId;
		return false;
	}

	// step 3: try login to our LDAP
	dprint3("AuthByECP: step3 - LDAP\n");
	VS_WideStr login; login.AssignUTF8(sudis_user_oid.c_str());
	if(!login)
		return false;
	VS_SimpleStr appID(cnt.GetStrValueRef(APPID_PARAM));
	bool res_ldap = LDAPDoLogin(login,0,appID,autoKey,0,ud,false,prop_cnt,client_type) == USER_LOGGEDIN_OK;
	if (res_ldap) {
		log.result = true;
	} else {
		log.error_str = "ldap denied login";
	}
	return res_ldap;
}

void VS_LDAPStorage::LDAPSetMyServerNameForUser(const char* call_id)
{
	// bug#23898
	if (!m_a_ServerName || !call_id || !*call_id)
		return ;
	char our_endpoint[256] = {0};
	VS_GetServerName(our_endpoint,256);

	std::wstring dn;
	if (LDAPGetUserDN(call_id,dn) && dn.length())
	{
		VS_WideStr wstr; wstr.AssignStr( our_endpoint );

		std::wstring currentServerName;
		if (LDAPGetServerNameByDN(dn, currentServerName) && !currentServerName.empty() && currentServerName == wstr.m_str)
		{
			return ;
		}

		wchar_t* new_vals[2] = {wstr,0};
		LDAPMod mod;
		mod.mod_op = LDAP_MOD_REPLACE;
		mod.mod_type = m_a_ServerName;
		mod.mod_vals.modv_strvals = (PWCHAR*) &new_vals;

		LDAPMod* mods[2] = {&mod, 0};

		dprint4("set serverName=%s attribute for %s\n", our_endpoint, call_id);
		if (ldap_modify_s(m_ldap, (PWCHAR) dn.c_str(), mods) != LDAP_SUCCESS)
			ProcessLDAPError();
	}else{
		dprint4("cant find dn of %s\n", call_id);
	}
}
#endif

void VS_LDAPStorage::NTLMRenewToken()
{
#ifdef _WIN32 // not ported
SECUREBEGIN_C_LDAP;
 unsigned __int64 time=0, exp_time=*(unsigned __int64*)&m_sec_token_expiry;
  GetSystemTimeAsFileTime((FILETIME*)&time);
  if( exp_time <= time )
  {
    if(exp_time!=0)
      FreeCredentialsHandle(&m_sec_token);

    SECURITY_STATUS code =
      AcquireCredentialsHandle( NULL, L"NTLM", SECPKG_CRED_INBOUND,
    NULL, NULL, NULL, NULL, &m_sec_token, &m_sec_token_expiry );
    if ( code != SEC_E_OK )
    {
      printf( "LDAPS: NTLM acq failed,code=%08lx\n", code );
    }
  }
SECUREEND_C_LDAP;
#endif
}
#include "ProtectionLib/OptimizeEnable.h"

#ifdef _SVKS_M_BUILD_
ldap_error_code_t VS_LDAPStorage::LDAPGetUserAddressBookAttribute(const vs_user_id& owner, std::vector<std::wstring>& result)
{
	ldap_error_code_t err(LDAP_SUCCESS);
	unsigned long users=0;
	LDAPMessage* lmsg=0;
	unsigned long search,lresult;

	try
	{
		VS_WideStr w_owner;
		w_owner.AssignUTF8(owner);
		wchar_t wbuff[4096] = {0};
		_snwprintf(wbuff,sizeof(wbuff),m_filter_login,w_owner);

		if(ldap_search_ext(m_ldap,m_basedn.m_str,LDAP_SCOPE_SUBTREE,wbuff,
			m_al_AddressBook,false,0,0,m_ldap_timeout.tv_sec,0,&search) != LDAP_SUCCESS)
			throw VSS_LDAP_ERROR;

		lresult=ldap_result(m_ldap,search,LDAP_MSG_ALL,&m_ldap_timeout,&lmsg);
		if(lresult==-1)
			throw VSS_LDAP_ERROR;

		if(lresult==0)
		{
			dprint1("LDAP timeout\n");
			err = LDAP_TIMEOUT;
			ldap_abandon(m_ldap,search);
			throw VSS_LDAP_ERROR;
		};

		LDAPMessage* iter=ldap_first_entry(m_ldap,lmsg);

		users=0;
		while(iter)
		{
			wchar_t** base_list=ldap_get_values(m_ldap,iter,m_a_AddressBook);
			if (base_list)
			{
				for(wchar_t** base=base_list;*base;base++)
				{
					const wchar_t* current_user = *base;
					result.push_back(current_user);
					++users;
				}
				ldap_value_free(base_list);
			}
			iter=ldap_next_entry(m_ldap,iter);
		}
	}
	catch (int error)
	{
		error_code=error;
		users=0;
		if(error==VSS_LDAP_ERROR)
		{
			if (err==LDAP_SUCCESS)
				err = LdapGetLastError();
			ProcessLDAPError();
		}
	};

	if(lmsg)
		ldap_msgfree(lmsg);

	auto ds = dstream4;
	if (ds.enabled())
	{
		ds << "AddressBook of " << owner << " is found with " << users << " entries:\n";
		for (const auto& x : result.end())
			ds << x << '\n';
	}
	else
		dstream3 << "AddressBook of " << owner << " is found with " << users << " entries";
	return err;

}

bool VS_LDAPStorage::LDAPGetUidAndDisplayName(const std::wstring& dn, std::wstring& uid, std::wstring& display_name)
{
	unsigned long users=0;
	LDAPMessage* lmsg=0;
	unsigned long search,lresult;
	bool result(false);

	try
	{
		wchar_t* al_display_name[6];
		al_display_name[0] = m_a_login;
		al_display_name[1] = m_a_displayname;
		al_display_name[2] = m_a_firstname;
		al_display_name[3] = m_a_middlename;
		al_display_name[4] = m_a_lastname;
		al_display_name[5] = 0;

		if(ldap_search_ext(m_ldap,(PWCHAR)dn.c_str(),LDAP_SCOPE_BASE,LDAP_FILTER_ALL,
			al_display_name,false,0,0,m_ldap_timeout.tv_sec,0,&search) != LDAP_SUCCESS)
			throw VSS_LDAP_ERROR;

		lresult=ldap_result(m_ldap,search,LDAP_MSG_ONE,&m_ldap_timeout,&lmsg);
		if(lresult==-1)
			throw VSS_LDAP_ERROR;

		if(lresult==0)
		{
			dprint1("LDAP timeout\n");
			ldap_abandon(m_ldap,search);
			throw VSS_LDAP_ERROR;
		};

		wchar_t**val=ldap_get_values(m_ldap,lmsg,m_a_login);
		if(val)
		{
			VS_WideStr id=val[0];
			uid = id;
			result = true;
			ldap_value_free(val);
		}
		val = 0;

		val=ldap_get_values(m_ldap,lmsg,m_a_displayname);
		if(val)
		{
			VS_WideStr id=val[0];
			display_name = id;
			ldap_value_free(val);
		}else{
			VS_WideStr auto_display_name;
			val=ldap_get_values(m_ldap,lmsg,m_a_firstname);
			if(val)
			{
				VS_WideStr id=val[0];
				auto_display_name = id;
				ldap_value_free(val);
			}
			val = 0;

			val=ldap_get_values(m_ldap,lmsg,m_a_middlename);
			if(val)
			{
				VS_WideStr id=val[0];
				if (auto_display_name.Length()>0)
					auto_display_name += L" ";
				auto_display_name += id;
				ldap_value_free(val);
			}
			val = 0;

			val=ldap_get_values(m_ldap,lmsg,m_a_lastname);
			if(val)
			{
				VS_WideStr id=val[0];
				if (auto_display_name.Length()>0)
					auto_display_name += L" ";
				auto_display_name += id;
				ldap_value_free(val);
			}
			val = 0;

			if (auto_display_name.Length()>0)
				display_name = auto_display_name;
		}
	}
	catch (int error)
	{
		error_code=error;
		result=false;
		if(error==VSS_LDAP_ERROR)
			ProcessLDAPError();
	};

	if(lmsg)
		ldap_msgfree(lmsg);

	return result;

}

bool VS_LDAPStorage::LDAPGetServerNameByDN(const std::wstring& dn, std::wstring& serverName)
{
	const wchar_t* attrs[2] = {
		m_a_ServerName,
		0
	};

	std::vector<attrs_t> out;
	LDAPSearchImp(m_ldap,dn.c_str(),LDAP_SCOPE_BASE,LDAP_FILTER_ALL,&attrs[0],out);

	// prepare result for add to abook map
	for(std::vector<attrs_t>::const_iterator it=out.begin(); it!=out.end(); ++it)
	{
		std::wstring server_name;
		for(attrs_t::const_iterator it2=it->begin(); it2!=it->end(); ++it2)
		{
			if (it2->first == m_a_ServerName.m_str)
			{
				serverName = it2->second;
				serverName = serverName.substr(0, serverName.find(L"#"));	// remove #vcs
				return true;
			}
		}
	}

	return false;
}

bool VS_LDAPStorage::LDAPGetUserDN(const vs_user_id& owner, std::wstring& dn)
{
	unsigned long users=0;
	LDAPMessage* lmsg=0;
	unsigned long search,lresult;
	bool result(false);

	try
	{
		VS_RealUserLogin r(SimpleStrToStringView(owner));
		VS_WideStr w_owner;
		w_owner.AssignUTF8(r.GetUser());
		wchar_t wbuff[4096] = {0};
		_snwprintf(wbuff,sizeof(wbuff),m_filter_login,w_owner);

		if(ldap_search_ext(m_ldap,m_basedn,LDAP_SCOPE_SUBTREE,wbuff,
			0,false,0,0,m_ldap_timeout.tv_sec,0,&search) != LDAP_SUCCESS)
			throw VSS_LDAP_ERROR;

		lresult=ldap_result(m_ldap,search,LDAP_MSG_ONE,&m_ldap_timeout,&lmsg);
		if(lresult==-1)
			throw VSS_LDAP_ERROR;

		if(lresult==0)
		{
			dprint1("LDAP timeout\n");
			ldap_abandon(m_ldap,search);
			throw VSS_LDAP_ERROR;
		};

		unsigned long num=ldap_count_entries(m_ldap,lmsg);
		if(num<1)
		{
			dprint3("user not found\n");
			throw (int)ACCESS_DENIED;
		};

		if(num>1)
			dprint1("more than one user was found!\n");

		dn=ldap_get_dn(m_ldap,lmsg);
		result = true;
	}
	catch (int error)
	{
		error_code=error;
		result=false;
		if(error==VSS_LDAP_ERROR)
			ProcessLDAPError();
	};

	if(lmsg)
		ldap_msgfree(lmsg);

	return result;

}

bool VS_LDAPStorage::LDAPFindMyRegion(const char* our_endpoint, VS_WideStr& my_region_dn)
{
	unsigned long users=0;
	LDAPMessage* lmsg=0;
	unsigned long search,lresult;
	bool result(false);

	VS_WideStr our_endpoint_wstr;	our_endpoint_wstr.AssignUTF8(our_endpoint);

	try
	{
		wchar_t* al_serverName[2];
		al_serverName[0] = m_a_ServerName;
		al_serverName[1] = 0;

		VS_WideStr tmp_base_dn = L"ou=People,";
		tmp_base_dn += m_basedn;

		if(ldap_search_ext(m_ldap,tmp_base_dn,LDAP_SCOPE_ONELEVEL,LDAP_FILTER_ALL,
			al_serverName,false,0,0,m_ldap_timeout.tv_sec,0,&search) != LDAP_SUCCESS)
			throw VSS_LDAP_ERROR;

		lresult=ldap_result(m_ldap,search,LDAP_MSG_ALL,&m_ldap_timeout,&lmsg);
		if(lresult==-1)
			throw VSS_LDAP_ERROR;

		if(lresult==0)
		{
			dprint1("LDAP timeout\n");
			ldap_abandon(m_ldap,search);
			throw VSS_LDAP_ERROR;
		};


		LDAPMessage* iter=ldap_first_entry(m_ldap,lmsg);

		users=0;
		while(iter && !result)
		{
			wchar_t** base_list=ldap_get_values(m_ldap,iter,m_a_ServerName);
			if (base_list)
			{
				for(wchar_t** base=base_list;*base;base++)
				{
					const wchar_t* current_region = *base;
					if (our_endpoint_wstr == current_region)
					{
						wchar_t* dn = ldap_get_dn(m_ldap,iter);
						if (dn&&*dn)
						{
							my_region_dn = dn;
							ldap_memfree(dn);
							result = true;
						}
					}
				}
				ldap_value_free(base_list);
			}
			iter=ldap_next_entry(m_ldap,iter);
		}

	}
	catch (int error)
	{
		error_code=error;
		result=false;
		if(error==VSS_LDAP_ERROR)
			ProcessLDAPError();
	};

	if(lmsg)
		ldap_msgfree(lmsg);

	return result;

}

#endif

/// addressbook sink
bool VS_LDAPStorage::GetDisplayName(const vs_user_id& user_id, std::string& display_name)
{
	if (!user_id)
		return false;
	VS_UserData ud;
	if(!FindUser(user_id,ud))
		return false;
	if(ud.m_displayName.empty())
		return false;
	display_name=ud.m_displayName;
	return true;
}

#include "ProtectionLib/OptimizeDisable.h"
int  VS_LDAPStorage::SearchUsers(VS_Container& cnt, const std::string& query, VS_Container* in_cnt)
{
	int users = 0;
	if (!m_ldapCore) return users;
SECUREBEGIN_C_LDAP;

	VS_StorageUserData param;
	bool do_search = ParseQuery(query, param);

	const char* call_id = (in_cnt) ? in_cnt->GetStrValueRef(CALLID_PARAM) : 0;
	const char* email = (in_cnt) ? in_cnt->GetStrValueRef(EMAIL_PARAM) : 0;
	const char* name = (in_cnt) ? in_cnt->GetStrValueRef(NAME_PARAM) : 0;

	bool do_new_search = false;
	if ((call_id && *call_id) || (email && *email) || (name && *name))
		do_new_search = true;

	std::string f;
	char buff[2048] = { 0 };

	std::map<VS_SimpleStr, VS_StorageUserData> result;

	if (do_search)
	{
		if (!param.m_LastName.empty())
		{
			snprintf(buff, sizeof(buff) / sizeof(buff[0]), "(%s=%s*)", m_ldapCore->m_a_lastname.c_str(), param.m_LastName.c_str());
			f += buff;
		}
		if (!param.m_FirstName.empty())
		{
			snprintf(buff, sizeof(buff) / sizeof(buff[0]), "(%s=%s*)", m_ldapCore->m_a_firstname.c_str(),param.m_FirstName.c_str());
			f += buff;
		}
		if (!!param.m_name)
		{
			snprintf(buff, sizeof(buff) / sizeof(buff[0]), "(%s=*%s*)", m_ldapCore->m_a_login.c_str(), param.m_name.m_str);
			f += buff;
		}
	}
	else if (do_new_search) {

		VS_SimpleStorage::SearchUsersByAlias(email, name, result);

		if (call_id && *call_id)
		{
			std::string term;
			std::stringstream ss;
			ss << call_id;
			ss >> term;
			std::transform(term.begin(), term.end(), term.begin(), ::tolower);
			VS_SimpleStr tmp_filter = "(%s=*%s*)";
			if (term.find("@") != std::string::npos)
			{
				std::string user = term.substr(0, term.find("@"));
				std::string server = term.substr(term.find("@") + 1, term.length() - term.find("@") - 1);

				string_view our_sid = VS_RemoveServerType(g_tr_endpoint_name);
				if (our_sid.find(server) != string_view::npos)
				{
					term = user;
					tmp_filter = "(%s=*%s)";
				}
			}

			auto filter_template = (!m_ldapCore->m_ldap_attr_FullID.empty()) ? m_ldapCore->m_ldap_attr_FullID.c_str() : m_ldapCore->m_a_login.c_str();

			snprintf(buff, sizeof(buff) / sizeof(buff[0]), tmp_filter.m_str, filter_template, term.c_str());
			f += buff;
		}
		if (email && *email)
		{
			std::string term;
			std::stringstream ss;
			ss << email;
			ss >> term;
			std::transform(term.begin(), term.end(), term.begin(), ::tolower);

			snprintf(buff, sizeof(buff) / sizeof(buff[0]), "(%s=*%s*)", m_ldapCore->m_a_email.c_str(), term.c_str());
			f += buff;

			if (!m_ldapCore->m_ldap_attr_FullID.empty())		// show only users with this attribute not empty
			{
				snprintf(buff, sizeof(buff) / sizeof(buff[0]), "(%s=*)", m_ldapCore->m_ldap_attr_FullID.c_str());
				f += buff;
			}
		}
		if (name && *name)
		{
			std::stringstream ss;
			ss << name;

			while (!ss.eof())
			{
				std::string term;
				ss >> term;

				if (term.find('@') != std::wstring::npos)		// call_id
				{
					std::string user = term.substr(0, term.find("@"));
					std::string server = term.substr(term.find("@") + 1, term.length() - term.find("@") - 1);

					string_view our_sid = VS_RemoveServerType(g_tr_endpoint_name);

					if (our_sid.find(server) != string_view::npos)
						term = user;
				}

				if (term.length())
					term += "*";

				snprintf(buff, sizeof(buff) / sizeof(buff[0]), "(|(%s=*%s)(%s=*%s)(%s=*%s)(%s=*%s)(%s=*%s))",
					m_ldapCore->m_a_login.c_str(), term.c_str(),
					m_ldapCore->m_a_email.c_str(), term.c_str(),
					m_ldapCore->m_a_displayname.c_str(), term.c_str(),
					m_ldapCore->m_a_firstname.c_str(), term.c_str(),
					m_ldapCore->m_a_lastname.c_str(), term.c_str());
				f += buff;
			}

			if (!m_ldapCore->m_ldap_attr_FullID.empty())		// show only users with this attribute not empty
			{
				snprintf(buff, sizeof(buff) / sizeof(buff[0]), "(%s=*)", m_ldapCore->m_ldap_attr_FullID.c_str());
				f += buff;
			}
		}
	}

	if (do_search || do_new_search)
	{
		snprintf(buff, sizeof(buff) / sizeof(buff[0]), "(&%s%s%s)", m_ldapCore->m_filter_disabled.get().c_str(), m_ldapCore->LDAP_FILTER_IS_USER, f.c_str());
		dstream3 << "LDAP: SearchUsers: filter=" << buff << ", call_id=" << call_id << ", email=" << email << ", name=" << name << "\n";
		std::vector<tc::ldap_user_info> found_users;
		if (m_ldapCore->m_filter_search_by_login_group && !m_ldapCore->m_login_group.get().empty()){
			std::string filter = buff;
			m_ldapCore->GetGroupUsers(m_ldapCore->m_login_group.get().c_str(), found_users, &m_ldapCore->m_custom_attrs_user_info, nullptr, &filter);
		}
		else
			m_ldapCore->SearchForUser(buff, found_users);
		for (const auto& u : found_users)
		{
			auto login = vs::UTF8ToLower(u.login);
			if (!login.empty()) {
				auto& res = result[login.c_str()];
				res.m_name = VS_RealUserLogin(login);
				res.m_displayName = u.displayName;
			}
		}
	}

	users = result.size();
	for (std::map<VS_SimpleStr, VS_StorageUserData>::iterator it = result.begin(); it != result.end(); ++it)
	{
		cnt.AddValue(USERNAME_PARAM, it->second.m_name);
		cnt.AddValue(CALLID_PARAM, it->second.m_name);
		cnt.AddValue(DISPLAYNAME_PARAM, it->second.m_displayName);
		VS_CallIDInfo ci;
		m_presenceService->Resolve(it->second.m_name, ci, true, 0, false);
		cnt.AddValueI32(USERPRESSTATUS_PARAM, ci.m_status);
	}

	cnt.AddValue(REQUEST_CALL_ID_PARAM, call_id);
	cnt.AddValue(REQUEST_EMAIL_PARAM, email);
	cnt.AddValue(REQUEST_NAME_PARAM, name);
SECUREEND_C_LDAP;
	return users;
}

void VS_LDAPStorage::SetUserStatus(const VS_SimpleStr& call_id,int status, const VS_ExtendedStatusStorage &extStatus, bool set_server,const VS_SimpleStr& server)
{
	VS_SimpleStorage::SetUserStatus(call_id, status, extStatus, set_server, server);
	if (!call_id || !m_ldapCore)
		return ;
#ifdef _SVKS_M_BUILD_
	VS_RealUserLogin r(SimpleStrToStringView(call_id));
	if (m_UseSudis)
	{
		std::string str_call_id;
		if (!!r)
			str_call_id = r;

		std::string str_status;
		if (status < USER_AVAIL) {
			str_status = "offline";
		} else if (status == USER_AVAIL) {
			str_status = "online";
		} else if (status > USER_AVAIL) {
			str_status = "in_conference";
		}

		std::string str_ip;
		VS_UserData ud;
		if (g_storage->FindUser(SimpleStrToStringView(call_id), ud))
		{
			VS_SimpleStr ip;
			g_dbStorage->GetEndpointProperty(ud.m_appID, "Local Ip", ip);
			if (!!ip)
			{
				char* p = strstr(ip.m_str,":");
				if (p)
					*p = 0;
				str_ip = ip;
			}
		}

		sudis::LogEvent(str_call_id.c_str(), str_status.c_str(), str_ip.c_str());
	}
	LDAPSetMyServerNameForUser(r.GetUser());
#endif

SECUREBEGIN_C_LDAP;
	m_ldapCore->SetUserStatus(call_id, status);
SECUREEND_C_LDAP;
}

/// DBStorage address books
int VS_LDAPStorage::FindUserPicture(VS_Container& cnt, int& entries, VS_AddressBook ab, const std::string& query, long client_hash)
{
	bool found_avatars = false;
	bool found_cache_resized = false;
	bool found_cache = false;
	VS_SCOPE_EXIT{ dstream4 << "LDAPStorage::FindUserPicture(" << query << ")"
		", avatars=" << found_avatars <<
		", avatars_cache_resized=" << found_cache_resized <<
		", avatars_cache=" << found_cache <<
		", m_use_avatars=" << m_ldapCore->m_use_avatars;
	};
	// search in avatars directory
	int simple_storage_ret = VS_SimpleStorage::FindUserPicture(cnt, entries, ab, query, client_hash);
	if (simple_storage_ret == SEARCH_FAILED || !m_ldapCore->m_use_avatars)
	{
		if (entries > 0)
			found_avatars = true;
		return simple_storage_ret;
	}

	// search is done successfully
	std::int32_t server_hash;
	cnt.GetValue(HASH_PARAM, server_hash);
	if (server_hash != 1)
	{
		if (entries > 0)
			found_avatars = true;
		return SEARCH_DONE;
	}

	entries = 0;
	cnt.Clear();

	if(VS_SimpleStorage::FindUserPictureImpl((m_ldapCore->m_ldap_avatars_path + DIR_RESIZED_IMG).c_str(), entries, query, client_hash, cnt) == SEARCH_DONE
		&& entries == -1)
	{
		entries = 0;
		cnt.Clear();

		auto ret = VS_SimpleStorage::FindUserPictureImpl(m_ldapCore->m_ldap_avatars_path.c_str(), entries, query, client_hash, cnt);
		if (entries > 0)
			found_cache = true;
		return ret;
	}

	if (entries > 0)
		found_cache_resized = true;
	return SEARCH_DONE;
}

int VS_LDAPStorage::SetUserPicture(VS_Container& cnt, VS_AddressBook ab, const char* callId, long& hash)
{
	bool on_disk = false;
	bool in_ldap = false;
	VS_SCOPE_EXIT{ dstream4 << "VS_LDAPStorage::SetUserPicture(" << callId << ")"
		", on_disk=" << on_disk <<
		", in_ldap=" << in_ldap;
	};
	const auto simple_storage_ret = VS_SimpleStorage::SetUserPicture(cnt, ab, callId, hash);
	if (simple_storage_ret == 0)
		on_disk = true;
	if (simple_storage_ret != 0 || !m_ldapCore->m_avatar_propagating_allowed || !m_ldapCore->m_use_avatars)
		return simple_storage_ret;

	assert(callId != nullptr);

	tc::ldap_user_info found;
	const auto res_find = m_ldapCore->LoginUser_CheckLogin(callId, found, vs::ignore<std::map<std::string, VS_RegGroupInfo>>{}, vs::ignore<std::string>{});
	if(res_find != tc::LDAPCore::CheckLoginResult::OK)
	{
		dstream1 << "can't find dn by callid(" << callId << ")";
		return static_cast<int>(res_find);
	}

	const auto& dn = found.dn;

	std::string file_name = get_avatar_file_name(cnt, callId, m_picture_mime_types);
	if (!file_name.empty())
	{
		if (!m_ldapCore->m_a_avatars.empty())
		{
			in_ldap = WriteAvatarToLDAP(dn, file_name, m_ldapCore->m_avatars_size, m_ldapCore->m_a_avatars) == 0;
			return in_ldap;
		}
	}
	return -1;
}

int VS_LDAPStorage::DeleteUserPicture(VS_AddressBook ab, const char* callId, long& hash)
{
	bool in_avatars = false;
	bool in_cache = false;
	bool in_ldap = false;
	VS_SCOPE_EXIT{ dstream4 << "VS_LDAPStorage::DeleteUserPicture(" << callId << ")"
		", in_avatars=" << in_avatars <<
		", in_cache=" << in_cache <<
		", in_ldap=" << in_ldap;
	};
	int simple_storage_ret = VS_SimpleStorage::DeleteUserPicture(ab, callId, hash);
	if (simple_storage_ret == 0)
		in_avatars = true;
	if (simple_storage_ret != 0 || !m_ldapCore->m_use_avatars)
		return simple_storage_ret;

	boost::filesystem::create_directories(m_ldapCore->m_ldap_avatars_path, vs::ignore<boost::system::error_code>{});

	if(!RemoveAvatarOnDisk(VS_RealUserLogin(callId).GetUser()))
	{
		return VSS_FILE_WRITE_ERROR;
	}

	in_cache = true;

	if (!m_ldapCore->m_avatar_propagating_allowed)
		return 0; //success;

	tc::ldap_user_info found;
	const auto res_find = m_ldapCore->LoginUser_CheckLogin(callId, found, vs::ignore<std::map<std::string, VS_RegGroupInfo>>{}, vs::ignore<std::string>{});
	if (res_find != tc::LDAPCore::CheckLoginResult::OK)
	{
		dstream1 << "can't find dn by callid(" << callId << ")";
		return static_cast<int>(res_find);
	}

	const auto& dn = found.dn;
	in_ldap = m_ldapCore->DeleteAvatar(dn) == 0;

	return 0; //success;
}

#include "ProtectionLib/OptimizeEnable.h"

bool  VS_LDAPStorage::FetchRights(const VS_StorageUserData& ud, VS_UserData::UserRights& rights)
{
	if (!m_group_manager) return false;
	VS_RealUserLogin r(SimpleStrToStringView(ud.m_name));
	if (r.IsGuest())
	{
		VS_UserData::UserRights grpRights = m_group_manager->GuestRights();
		int r = VS_UserData::UR_APP_COMMUNICATOR | VS_UserData::UR_LOGIN;
		r |= GetPhysRights() & grpRights;
		rights = (VS_UserData::UserRights) r;
		return true;
	}

	rights = (VS_UserData::UserRights) GetUserRights(ud.m_name);

	VS_UserData ud_current;
	if (g_storage && g_storage->FindUser(SimpleStrToStringView(ud.m_name), ud_current))
		if (ud_current.m_rights & VS_UserData::UR_COMM_PROACCOUNT)
			rights = (VS_UserData::UserRights)(rights | ud.UR_COMM_PROACCOUNT);

	return true;
}
VS_UserData::UserRights VS_LDAPStorage::GetPhysRights()
{
	if (!m_group_manager) return VS_UserData::UserRights::UR_NONE;

	VS_UserData	ud;
	VS_License lic_sum = p_licWrap->GetLicSum();
	long r= ud.UR_APP_COMMUNICATOR|
			ud.UR_COMM_CALL|
			ud.UR_COMM_BROADCAST|
			ud.UR_COMM_MOBILEPRO|
			ud.UR_COMM_EDITGROUP|
			ud.UR_COMM_CHAT;
	if(VS_CheckLicense_MultiConferences())
	{
		r |= ud.UR_COMM_MULTI |
			 ud.UR_COMM_CREATEMULTI |
			 ud.UR_COMM_PASSWORDMULTI;
	}
	if(m_license_checker(LE_FILETRANSFER))
		r |= ud.UR_COMM_FILETRANSFER;
	if(m_license_checker(LE_WHITEBOARD))
		r |= ud.UR_COMM_WHITEBOARD;
	if(m_license_checker(LE_SLIDESHOW))
		r |= ud.UR_COMM_SLIDESHOW;
	if(m_license_checker(LE_DSHARING))
		r |= (ud.UR_COMM_DSHARING | ud.UR_COMM_SHARE_CONTROL);
	if(m_license_checker(LE_VIDEORECORDING))
		r |= ud.UR_COMM_RECORDING;
	if(m_license_checker(LE_HDVIDEO))
		r |= ud.UR_COMM_HDVIDEO;
	if(m_license_checker(LE_MULTIGATEWAY))
	{
		VS_RegistryKey key(false, CONFIGURATION_KEY);
		char buff[512] = { 0 };
		if (key.GetValue(&buff, 512, VS_REG_STRING_VT, VOIP_GATEWAY_TAG) && buff[0])
			r |= ud.UR_COMM_DIALER;//|ud.UR_COMM_EDITDIAL; removed by SMK: in 3.4.0 disabled
	}

	if( m_ab_storage->CanEdit() )
    r|=ud.UR_COMM_EDITAB|ud.UR_COMM_UPDATEAB|ud.UR_COMM_SEARCHEXISTS;

	return m_group_manager->m_physRights = (VS_UserData::UserRights)r;
}

void VS_LDAPStorage::Timer(unsigned long ticks, VS_TransportRouterServiceHelper* caller)
{
	VS_SimpleStorage::Timer(ticks, caller);

// todo(kt): re-write with new GetGroupUsers();
	auto v = std::move(*m_log_events.lock());
	for(auto const& c: v)
		caller->PostRequest(caller->OurEndpoint(), 0, c, 0, LOG_SRV);

	if (m_state==STATE_RECONNECT)
		TryReConnect();			// kt: this can slow down CheckLicSrv for a while here
#ifndef _SVKS_M_BUILD_
	if (!caller || !m_ldapCore)
		return;
	auto now = std::chrono::system_clock::now();
	VS_AutoLock lock(this);
	if (m_ldapCore->m_ab_cache_timeout.count()>0 && (m_lastupdate == std::chrono::system_clock::time_point() || (now - m_lastupdate > m_ldapCore->m_ab_cache_timeout)))
	{
		VS_Container cnt;
		cnt.AddValue(METHOD_PARAM, ONUSERCHANGE_METHOD);
		cnt.AddValueI32(TYPE_PARAM, 4);
		cnt.AddValue(SESSION_PARAM, m_session_id.m_str);
		caller->PostRequest(caller->OurEndpoint(), 0, cnt, 0, AUTH_SRV);
		m_lastupdate = now;
	}
#endif
}

bool VS_LDAPStorage::GetMissedCallMailTemplate(const std::chrono::system_clock::time_point /*missed_call_time*/, const char* fromId, std::string& inOutFromDn, const char* toId, std::string& inOutToDn, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ)
{
	GetMissedCallMailTemplateBase(subj_templ, body_templ);

	VS_StorageUserData ude1;
	FindUser(fromId, ude1);
	from_email = ude1.m_email;
    if(inOutFromDn.empty()) inOutFromDn = ude1.m_displayName;
	if (!from_email)
		from_email = fromId;

	VS_StorageUserData ude2;
	FindUser(toId, ude2);
	to_email = ude2.m_email;
    if (inOutToDn.empty()) inOutToDn = ude2.m_displayName;
	if (!to_email)
		to_email = toId;

	return true;
}

bool VS_LDAPStorage::GetInviteCallMailTemplate(const std::chrono::system_clock::time_point /*missed_call_time*/, const char *fromId, std::string& inOutFromDn, const char *toId, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ)
{
	GetInviteCallMailTemplateBase(subj_templ, body_templ);

	VS_StorageUserData ude1;
	FindUser(fromId, ude1);
	from_email = ude1.m_email;
    if (inOutFromDn.empty()) inOutFromDn = ude1.m_displayName;
	if (!from_email)
		from_email = fromId;

	// Don't change, because no such user
	to_email = toId;

	return true;
}

bool VS_LDAPStorage::GetMultiInviteMailTemplate(const char* fromId, std::string& inOutFromDn, const char* toId, std::string& inOutToDn, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ)
{
	GetMultiInviteMailTemplateBase(subj_templ, body_templ);

	VS_StorageUserData ude1;
	FindUser(fromId, ude1);
	from_email = ude1.m_email;
    if (inOutFromDn.empty()) inOutFromDn = ude1.m_displayName;
	if (!from_email)
		from_email = fromId;

	VS_StorageUserData ude2;
	FindUser(toId, ude2);
	to_email = ude2.m_email;
    if (inOutToDn.empty()) inOutToDn = ude2.m_displayName;
	if (!to_email)
		to_email = toId;

	return true;
}

bool VS_LDAPStorage::GetMissedNamedConfMailTemplate(const char* fromId, std::string &inOutFromDn, const char* toId, std::string& inOutToDn, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ)
{
	GetMissedNamedConfMailTemplateBase(subj_templ, body_templ);

	VS_StorageUserData ude1;
	FindUser(fromId, ude1);
	from_email = ude1.m_email;
    if(inOutFromDn.empty()) inOutFromDn = ude1.m_displayName;
	if (!from_email)
		from_email = fromId;

	VS_StorageUserData ude2;
	FindUser(toId, ude2);
	to_email = ude2.m_email;
    if (inOutToDn.empty()) inOutToDn = ude2.m_displayName;
	if (!to_email)
		to_email = toId;

	return true;
}

void VS_LDAPStorage::UpdateUsersGroups(const std::function<bool(void)>& is_stopping)
{
#ifndef _SVKS_M_BUILD_
	if (!m_ldapCore) return;
	auto tick1 = m_ldapCore->GetLastUpdateNestedCacheTime();
	m_ldapCore->StartUpdateNestedCache();
	decltype(tick1) tick2;
	auto start = std::chrono::system_clock::now();
	do {
		tick2 = m_ldapCore->GetLastUpdateNestedCacheTime();
		vs::SleepFor(std::chrono::milliseconds(10));
	} while ((tick2 == tick1) && !is_stopping() && !(std::chrono::system_clock::now() - start > std::chrono::minutes(5)));

	VS_SimpleStorage::UpdateUsersPhonesHash_Global();

	// update ApplicationSettings
	vs_user_id* users = 0;
	auto NumOfUsers = g_storage->GetUsers(users);
	VS_SCOPE_EXIT{ delete[] users; };
	for (decltype(NumOfUsers) i = 0; i < NumOfUsers; i++)
	{
		const char* to_user = users[i];
		if (!to_user || VS_IsNotTrueConfCallID(to_user))
			continue;
		VS_StorageUserData ude;
		if (FindUser(to_user, ude))
		{
			ReadApplicationSettings(ude);
			UpdateCacheOnLogin(ude);
		}
	}
#endif
}

bool VS_LDAPStorage::IsLDAP() const
{
	return true;
}

#ifdef _SVKS_M_BUILD_
VS_Search_Result VS_LDAPStorage::FindUsers_CustomGroups(VS_Container& cnt, int& entries, const vs_user_id& owner)
{
	if (!owner)
		return SEARCH_FAILED;
	ldap_error_code_t err(LDAP_SUCCESS);
	VS_AutoLog_AB log;
	log.ab = AB_GROUPS;
	log.oid = VS_RealUserLogin(SimpleStrToStringView(owner).GetUser();
	std::wstring owner_dn;
	if (!LDAPGetUserDN(owner,owner_dn) || owner_dn.empty())
	{
		log.error_str = "user dn not found";
		return SEARCH_FAILED;
	}

	wchar_t* al[] = {m_a_AddressBook, L"description", 0};		// todo(kt): al_CustomGroups_members
	VS_WideStr dn;
	wchar_t buff[4096] = {0};
	std::wstring filter;

	filter = /*m_filter_group*/ L"(objectClass=mvd-v3-ab-group)";		// todo(kt): filter for CustomGroups
	dn = owner_dn.c_str();

	TGroupUsers v;

	wchar_t debug_buff[4096]={0};
	std::wstring debug_print;

	LDAPMessage* lmsg=0;
	try
	{
		unsigned long search,lresult;
		if(ldap_search_ext(m_ldap,dn,LDAP_SCOPE_SUBTREE,(PWCHAR)filter.c_str(),
			al,false,0,0,m_ldap_timeout.tv_sec,0,&search) != LDAP_SUCCESS)
			throw VSS_LDAP_ERROR;

		lresult=ldap_result(m_ldap,search,LDAP_MSG_ALL,&m_ldap_timeout,&lmsg);
		if(lresult==-1)
			throw VSS_LDAP_ERROR;

		if(lresult==0)
		{
			dprint1("LDAP timeout\n");
			err = LDAP_TIMEOUT;
			ldap_abandon(m_ldap,search);
			throw VSS_LDAP_ERROR;
		};

		if (ldap_result2error(m_ldap,lmsg,0) != LDAP_SUCCESS)
			throw VSS_LDAP_ERROR;

		//		_snwprintf(debug_buff, 4096, L"\nGetCustomGroupUsers(%s)\nfilter=%s\nBegin\n", group, filter.c_str());	debug_print+=debug_buff;

		for(LDAPMessage* iter=ldap_first_entry(m_ldap,lmsg);iter;iter=ldap_next_entry(m_ldap,iter))
		{
			wchar_t* tmp=ldap_get_dn(m_ldap,iter);
			if(!tmp || !*tmp)
				continue;
			std::wstring group = tmp;
			ldap_memfree(tmp);

			size_t pos1 = group.find_first_of(L"=");
			size_t pos2 = group.find_first_of(L",");
			if (!pos1 || !pos2 || pos1>=pos2)
				continue;
			++pos1;

			std::wstring gid_str = group.substr(pos1, pos2-pos1);
			cnt.AddValue(GID_PARAM, gid_str.c_str());
			//			debug_print+=group; debug_print+=L"\n";
			wchar_t** desc=ldap_get_values(m_ldap,iter,L"description");
			if (desc && desc[0])
				cnt.AddValue(GNAME_PARAM, (wchar_t*)desc[0]);
			cnt.AddValue(GTYPE_PARAM, (long) 1);		// CustomGroups

			wchar_t** members=ldap_get_values(m_ldap,iter,/*m_a_groupmember*/ m_a_AddressBook);		// todo(kt): attribute name from CustomGroups
			if(members)
			{
				for(int i=0;members[i];i++)
				{
					std::wstring uid;
					std::wstring tmp;
					std::wstring server_name;
					if (LDAPGetUidAndDisplayName(members[i],uid,tmp) && !uid.empty() &&
						LDAPGetServerNameByDN(members[i],server_name))
					{
						std::wstring trueconf_id = uid + L"@" + server_name;
						cnt.AddValue(CALLID_PARAM, trueconf_id.c_str());
					}else{
						cnt.AddValue(CALLID_PARAM, members[i]);		// if it is #sip - add as is
					}
					//					_snwprintf(debug_buff, 4096, L"\tmember[%d]=%s;\n", i, members[i]);	debug_print+=debug_buff;
					//VS_StorageUserData ud;
					//if (FetchUserByDN(members[i], ud)) {
					//	VS_RealUserLogin r(ud.m_name.m_str);
					//	std::string user_id = (const char*)r;
					//	//v.insert(std::make_pair<std::string, VS_StorageUserData>(user_id, ud));
					//	cnt.AddValue(CALLID_PARAM, user_id.c_str);
					//}
				}
			}else{
				//VS_StorageUserData ud;
				//if (FetchUser(iter,ud)) {
				//	VS_RealUserLogin r(ud.m_name.m_str);
				//	std::string user_id = (const char*)r;
				//	v.insert(std::make_pair<std::string, VS_StorageUserData>(user_id, ud));
				//}
			}

			if (members)
				ldap_value_free(members);
		}

		//_snwprintf(debug_buff, 4096, L"End. Total users %d\n", v.size());	debug_print+=debug_buff;
		//dprint3("%S", debug_print.c_str());

		ldap_msgfree(lmsg);
		lmsg=0;
	}
	catch (int error)
	{
		if(lmsg)
			ldap_msgfree(lmsg);
		lmsg=0;
		if(error==VSS_LDAP_ERROR)
		{
			if (err==LDAP_SUCCESS)
				err = LdapGetLastError();
			ProcessLDAPError();
		}
		if (err!=LDAP_SUCCESS)
		{
			std::stringstream ss;
			ss << "get groups ldap error: " << err;
			log.error_str = ss.str();
		}
		//return false;
		return SEARCH_FAILED;
	};

	if(lmsg)
		ldap_msgfree(lmsg);

	cnt.AddValue(HASH_PARAM, (long)GetTickCount());
	log.result = true;
	return SEARCH_DONE;
}
#endif

void VS_LDAPStorage::UpdateCacheOnLogin(const VS_StorageUserData &ude)
{
	if (!m_ldapCore) return;
	VS_AutoLock lock(&m_ldapCore->m_cache_user_info_lock);
	auto udePtr = boost::make_shared<VS_StorageUserData>();
	*udePtr = ude;
	m_ldapCore->m_cache_user_info[(const char*)udePtr->m_realLogin] = udePtr;
}

int VS_LDAPStorage::FindUsersPhones(VS_Container& cnt, int& entries, const vs_userpart_escaped& owner, long client_hash)
{
	long hash = VS_SimpleStorage::GetPhoneBookHash(owner);
	if (VS_CompareHash(hash, client_hash))
		return SEARCH_NOT_MODIFIED;

	VS_AbCommonMap m;
	long server_hash(0);
	m_ab_storage->GetABForUser(owner.c_str(), m, server_hash);			// filter phones by ab_common

	if (!m_ldapCore) return SEARCH_FAILED;

	std::map<std::string, boost::shared_ptr<VS_StorageUserData>> user_info;
	{
		VS_AutoLock lock(&m_ldapCore->m_cache_user_info_lock);
		for (const auto& x : m)
		{
			auto it = m_ldapCore->m_cache_user_info.find(x.first);
			if (it != m_ldapCore->m_cache_user_info.end())
				user_info[it->first] = it->second;
		}
	}

// non-Editable phones
	for (const auto& u : user_info)
		for (const auto& p : u.second->m_phones)
		{
			if (!p.phone)
				continue;
			entries++;
			cnt.AddValue(ID_PARAM, p.id);
			cnt.AddValue(CALLID_PARAM, p.call_id);
			cnt.AddValue(USERPHONE_PARAM, p.phone);
			cnt.AddValueI32(TYPE_PARAM, p.type);
			cnt.AddValue(EDITABLE_PARAM, p.editable);
		}

// Editable phones
	std::vector<VS_UserPhoneItem> v;
	VS_SimpleStorage::FindUsersPhones(owner, v);
	for(std::vector<VS_UserPhoneItem>::iterator it2=v.begin(); it2!=v.end(); ++it2)
	{
		if (!it2->phone)
			continue;
		entries++;
		cnt.AddValue(ID_PARAM, it2->id);
		cnt.AddValue(CALLID_PARAM, it2->call_id);
		cnt.AddValue(USERPHONE_PARAM, it2->phone);
		cnt.AddValueI32(TYPE_PARAM, it2->type);
		cnt.AddValue(EDITABLE_PARAM, it2->editable);
	}

	cnt.AddValueI32(HASH_PARAM, VS_SimpleStorage::GetPhoneBookHash(owner));
	return SEARCH_DONE;
}

bool VS_LDAPStorage::GetRegGroupUsers(const std::string& gid, std::shared_ptr<VS_AbCommonMap>& users)
{
	if (!m_ldapCore || !m_group_manager) return false;
	std::map<std::string, VS_RegGroupInfo> reg_groups;
	m_ldapCore->GetRegGroups(reg_groups);

	auto it = reg_groups.find(gid);
	if (it == reg_groups.end() || it->second.ldap_dn.empty())
		return false;

	auto c = m_ldapCore->m_cache_groups_users.load();
	if (!c)
		return false;
	auto g = c->find(it->second.ldap_dn);
	if (g == c->end() || !g->second)
		return false;
	users = g->second;
	return true;
}

bool VS_LDAPStorage::IsCacheReady() const
{
#ifdef _SVKS_M_BUILD_
	return /*m_all_users_cache_tick!=0*/true;
#else
	if (m_ldapCore) return m_ldapCore->IsCacheReady();
	else return false;
#endif
}

#ifdef _SVKS_M_BUILD_
int VS_LDAPStorage::AddToAddressBook(VS_AddressBook ab,const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt, VS_SimpleStr& add_call_id, std::string& add_display_name, VS_TransportRouterService* srv)
{
	if (ab==AB_COMMON) {
		const char* call_id2_=cnt.GetStrValueRef(CALLID_PARAM);
		if(!call_id2_||!*call_id2_)
			call_id2_=cnt.GetStrValueRef(NAME_PARAM);
		if(!call_id2_||!*call_id2_)
			return VSS_USER_NOT_VALID;
		VS_SimpleStr call_id2 = VS_RemoveTranscoderID(call_id2_).c_str();
		const char* u_dn=cnt.GetStrValueRef(DISPLAYNAME_PARAM);
		VS_WideStr custom_display_name;
		if(u_dn)
			custom_display_name.AssignUTF8(u_dn);

		std::wstring dn;
		if (!LDAPGetUserDN(user_id1,dn) || !dn.length())
			return VSS_USER_NOT_FOUND;

		VS_WideStr dn2;
		std::wstring tmp_dn2;
		if (!LDAPGetUserDN(VS_RealUserLogin(call_id2).GetUser(), tmp_dn2) || !tmp_dn2.length()) {
			dn2.AssignUTF8(call_id2);
		}else{
			dn2 = tmp_dn2.c_str();
		}
		if (!dn2.Length())
			return VSS_USER_NOT_FOUND;
		if (!!custom_display_name && (custom_display_name!=(VS_WideStr)call_id2)) {
			std::wstring uid_wstr;
			std::wstring display_name;
			LDAPGetUidAndDisplayName(dn2.m_str, uid_wstr, display_name);
			if (custom_display_name!=display_name.c_str())
			{
				dn2 += STRING_SEPARATOR_W.m_str;
				dn2 += custom_display_name;
			}
		}

		const wchar_t* new_ab[2] = {dn2,0};

		LDAPMod mod;
		mod.mod_op = LDAP_MOD_ADD;
		mod.mod_type = m_a_AddressBook;
		mod.mod_vals.modv_strvals = (PWCHAR*) &new_ab;

		LDAPMod* mods[2] = {&mod, 0};

		if (ldap_modify_s(m_ldap, (PWCHAR)dn.c_str(), mods) != LDAP_SUCCESS)
			return VSS_LDAP_ERROR;

		std::wstring server_name_wstr;
		std::wstring uid_wstr;
		std::wstring display_name;
		if (LDAPGetServerNameByDN(dn2.m_str, server_name_wstr) && server_name_wstr.length() &&
			LDAPGetUidAndDisplayName(dn2.m_str, uid_wstr, display_name) && uid_wstr.length())
		{
			const char* uid = ((VS_WideStr)uid_wstr.c_str()).ToStr();
			const char* server_name = ((VS_WideStr)server_name_wstr.c_str()).ToStr();
			if (uid&&*uid && server_name&&*server_name)
			{
				VS_SimpleStr call_id;
				call_id = uid;
				call_id += "@";
				call_id += server_name;
				add_call_id = call_id;
				add_display_name = display_name.c_str();
			}
			free((void*)uid);
			free((void*)server_name);
		}

		std::map<std::string, VS_AbCommonMap_Item, ci_less> tmp;
		m_ab_storage->GetABForUser(VS_RealUserLogin(user_id1).GetUser(), tmp, hash);		// get hash of current ldap ab_common
		return 0;
	}else{
		return VS_SimpleStorage::AddToAddressBook(ab, user_id1, cnt, hash, rCnt, add_call_id, add_display_name, srv);
	}
	return VSS_DB_ERROR;
}
int VS_LDAPStorage::RemoveFromAddressBook(VS_AddressBook ab,const vs_user_id& user_id1,const vs_user_id& user_id2, VS_Container &cnt, long& hash, VS_Container &rCnt)
{
	if (ab==AB_COMMON) {
		const char* call_id2_=cnt.GetStrValueRef(CALLID_PARAM);
		if(!call_id2_||!*call_id2_)
			call_id2_=cnt.GetStrValueRef(NAME_PARAM);
		if(!call_id2_||!*call_id2_)
			return VSS_USER_NOT_VALID;
		VS_SimpleStr call_id2 = VS_RemoveTranscoderID(call_id2_).c_str();

		std::wstring dn;
		if (!LDAPGetUserDN(user_id1,dn) || !dn.length())
			return VSS_USER_NOT_FOUND;

		std::vector<std::wstring> v_dn2;
		std::wstring dn2;
		if (!LDAPGetUserDN(VS_RealUserLogin(call_id2).GetUser(), dn2) || !dn2.length())
		{
			VS_WideStr wstr; wstr.AssignUTF8(call_id2);
			dn2 = wstr;
		}
		if (dn2.length())
			v_dn2.push_back(dn2);

		// custom display name
		VS_AbCommonMap m;
		GetABForUserImp(VS_RealUserLogin(user_id1).GetUser(), m);
		VS_AbCommonMap::iterator it = m.find(call_id2.m_str);
		if (it != m.end())
		{
			if (((VS_WideStr)it->first.c_str()) != it->second.displayName.c_str())
			{
				dn2 += STRING_SEPARATOR_W.m_str;
				dn2 += it->second.displayName.c_str();
				v_dn2.push_back(dn2);
			}
		}

		if (v_dn2.empty())
			return VSS_USER_NOT_FOUND;

		bool ok(false);
		for(std::vector<std::wstring>::const_iterator it=v_dn2.begin(); it!=v_dn2.end(); ++it)
		{
			const wchar_t* new_ab[2] = {it->c_str(),0};

			LDAPMod mod;
			mod.mod_op = LDAP_MOD_DELETE;
			mod.mod_type = m_a_AddressBook;
			mod.mod_vals.modv_strvals = (PWCHAR*) &new_ab;

			LDAPMod* mods[2] = {&mod, 0};

			if (ldap_modify_s(m_ldap, (PWCHAR)dn.c_str(), mods) == LDAP_SUCCESS)
				ok = true;
		}

		if (!ok)
			return VSS_LDAP_ERROR;

		std::map<std::string, VS_AbCommonMap_Item, ci_less> tmp;
		m_ab_storage->GetABForUser(user_id1, tmp, hash);		// get hash of current ldap ab_common
		return 0;
	}else{
		return VS_SimpleStorage::RemoveFromAddressBook(ab, user_id1, user_id2, cnt, hash, rCnt);
	}
	return VSS_DB_ERROR;
}
int	VS_LDAPStorage::UpdateAddressBook(VS_AddressBook ab, const vs_user_id& user_id1, const char* call_id2, VS_Container& cnt, long& hash, VS_Container& rCnt)
{
	if (ab==AB_COMMON) {
		if (!user_id1 || !call_id2 || !*call_id2)
			return VSS_USER_NOT_VALID;

		VS_Container tmp_cnt;
		RemoveFromAddressBook(ab, user_id1, call_id2, cnt, hash, tmp_cnt);

		VS_SimpleStr add_call_id;
		VS_WideStr add_display_name;
		AddToAddressBook(ab, user_id1, cnt, hash, rCnt, add_call_id, add_display_name, 0);

		std::map<std::string, VS_AbCommonMap_Item, ci_less> tmp;
		m_ab_storage->GetABForUser(user_id1, tmp, hash);		// get hash of current ldap ab_common
		return 0;
	}else{
		return VS_SimpleStorage::UpdateAddressBook(ab, user_id1, call_id2, cnt, hash, rCnt);
	}
	return VSS_DB_ERROR;
}
#endif

bool VS_LDAPStorage::LogoutUser(const VS_SimpleStr& login)
{
	dprint3("LDAP: logout user %s\n", login.m_str);
#ifdef _SVKS_M_BUILD_
	if (!m_UseSudis)
		return true;
	if (!login)
		return false;

	char key_name[512];key_name[sizeof(key_name)-1]=0;
	_snprintf(key_name,sizeof(key_name)-1,"%s\\%s", USERS_KEY, login);

	VS_RegistryKey key(false, key_name);
	if (!key.IsValid())
		return false;
	char userTokenId[1024] = {0};
	if (key.GetValue(userTokenId, sizeof(userTokenId), VS_REG_STRING_VT, "sudis_userTokenID")>0)
	{
		dprint3("sudis logout user %s (userTokenId: %s)\n", login, userTokenId);
		sudis::LogoutUser(userTokenId);
	}
#endif
	return true;
}

#ifdef _SVKS_M_BUILD_
bool VS_LDAPStorage::ManageGroups_CreateGroup(const vs_user_id& owner, const VS_WideStr& gname, long& gid, long& hash)
{
	if (!owner || !gname)
		return false;

	std::wstring dn;
	if (!LDAPGetUserDN(owner, dn) && dn.empty())
		return false;

	// check that dn doesn't exist
	long N_retries = 5;		// how many times do rand for group and check if it exist at LDAP
	wchar_t buff[2048] = {0};
	long new_gid(0);
	LDAPMessage* lmsg=0;
	bool exist(false);
	{
		_snwprintf(buff,(sizeof(buff) / sizeof(buff[0])) - 1,L"ou=CustomGroups,%s",dn.c_str());
		try
		{
			exist = ldap_search_s(m_ldap, buff, LDAP_SCOPE_BASE, 0, 0, 0, &lmsg ) == LDAP_SUCCESS;
		}
		catch (int error)
		{
			if(error==VSS_LDAP_ERROR)
				ProcessLDAPError();
		};
		if(lmsg)
			ldap_msgfree(lmsg);
		lmsg = 0;
	}
	if (!exist)
	{
		exist = CreateCustomGroupsFolder(buff);
		if (!exist)
			return false;
	}

	while(exist && N_retries>0) {
		new_gid = LONG_MAX - ((rand()%(RAND_MAX/2)) << 16) - rand();
		_snwprintf(buff,(sizeof(buff) / sizeof(buff[0])) - 1,L"cn=%d,ou=CustomGroups,%s",new_gid,dn.c_str());
		try
		{
			exist = ldap_search_s(m_ldap, buff, LDAP_SCOPE_BASE, 0, 0, 0, &lmsg ) == LDAP_SUCCESS;
		}
		catch (int error)
		{
			if(error==VSS_LDAP_ERROR)
				ProcessLDAPError();
		};
		if(lmsg)
			ldap_msgfree(lmsg);
		lmsg = 0;
		--N_retries;
	}

	if (exist || N_retries <= 0)
		return false;

	// if ok, then dn = try_dn
	dn = buff;

	std::vector<std::pair<std::wstring, std::wstring>> attrs;
	attrs.emplace_back(L"objectClass", L"mvd-v3-ab-group");
	attrs.emplace_back(L"objectClass", L"top");
	attrs.emplace_back(L"cn", _itow(new_gid, buff, 10));
	attrs.emplace_back(L"description", gname.m_str);

	std::vector<std::vector<wchar_t*>*> a(attrs.size());	// deleters
	std::vector<LDAPMod*> v;
	for(unsigned int i=0; i < attrs.size(); ++i)
	{
		a[i] = new std::vector<wchar_t*>;
		a[i]->push_back((wchar_t*)attrs[i].second.c_str());
		a[i]->push_back(0);

		LDAPMod* mod = new LDAPMod;
		mod->mod_op = LDAP_MOD_ADD;
		mod->mod_type = (PWCHAR) attrs[i].first.c_str();
		mod->mod_vals.modv_strvals = (PWCHAR*) &((*(a[i]))[0]);

		v.push_back(mod);
	}
	v.push_back(0);

	bool b = ldap_add_s(m_ldap, (PWCHAR) dn.c_str(), &v[0]) == LDAP_SUCCESS;
	dprint4("user %s add CustomGroup %S with gid %d res:%d\n", owner, gname, new_gid, b);

	for(std::vector<std::vector<wchar_t*>*>::iterator it=a.begin(); it!=a.end(); ++it)
		delete *it;
	a.clear();
	for(std::vector<LDAPMod*>::iterator it=v.begin(); it!=v.end(); ++it)
		delete *it;
	v.clear();

	if (b) {
		gid = new_gid;
		hash = GetABGroupsHash(owner);
	} else {
		ProcessLDAPError();
	}

	return b;
}

bool VS_LDAPStorage::CreateCustomGroupsFolder(const std::wstring& dn)
{
	std::vector<std::pair<std::wstring, std::wstring>> attrs;
	attrs.emplace_back(L"objectClass", L"organizationalUnit");
	attrs.emplace_back(L"objectClass", L"top");
	attrs.emplace_back(L"ou", L"CustomGroups");

	std::vector<std::vector<wchar_t*>*> a(attrs.size());	// deleters
	std::vector<LDAPMod*> v;
	for(unsigned int i=0; i < attrs.size(); ++i)
	{
		a[i] = new std::vector<wchar_t*>;
		a[i]->push_back((wchar_t*)attrs[i].second.c_str());
		a[i]->push_back(0);

		LDAPMod* mod = new LDAPMod;
		mod->mod_op = LDAP_MOD_ADD;
		mod->mod_type = (PWCHAR) attrs[i].first.c_str();
		mod->mod_vals.modv_strvals = (PWCHAR*) &((*(a[i]))[0]);

		v.push_back(mod);
	}
	v.push_back(0);

	bool b = ldap_add_s(m_ldap, (PWCHAR) dn.c_str(), &v[0]) == LDAP_SUCCESS;
	dprint4("CreateCustomGroupsFolder for %s\n", dn);

	for(std::vector<std::vector<wchar_t*>*>::iterator it=a.begin(); it!=a.end(); ++it)
		delete *it;
	a.clear();
	for(std::vector<LDAPMod*>::iterator it=v.begin(); it!=v.end(); ++it)
		delete *it;
	v.clear();
	return b;
}

bool VS_LDAPStorage::ManageGroups_DeleteGroup(const vs_user_id& owner, const long gid, long& hash)
{
	if (!owner || !gid)
		return false;

	std::wstring dn;
	if (!LDAPGetUserDN(owner, dn) && dn.empty())
		return false;

	wchar_t buff[2048] = {0};
	_snwprintf(buff,(sizeof(buff) / sizeof(buff[0])) - 1,L"cn=%d,ou=CustomGroups,%s",gid,dn.c_str());

	bool b = ldap_delete_s(m_ldap, buff) == LDAP_SUCCESS;
	dprint4("user %s delete CustomGroup with gid %d res:%d\n", owner, gid, b);

	if (b) {
		hash = GetABGroupsHash(owner);
	}else {
		ProcessLDAPError();
	}

	return b;
}

bool VS_LDAPStorage::ManageGroups_RenameGroup(const vs_user_id& owner, const long gid, const VS_WideStr& gname, long& hash)
{
	if (!owner || !gid)
		return false;

	std::wstring dn;
	if (!LDAPGetUserDN(owner, dn) && dn.empty())
		return false;

	wchar_t buff[2048] = {0};
	_snwprintf(buff,(sizeof(buff) / sizeof(buff[0])) - 1,L"cn=%d,ou=CustomGroups,%s",gid,dn.c_str());

	wchar_t* new_vals[2] = {gname.m_str,0};
	LDAPMod mod;
	mod.mod_op = LDAP_MOD_REPLACE;
	mod.mod_type = L"description";
	mod.mod_vals.modv_strvals = (PWCHAR*) &new_vals;

	LDAPMod* mods[2] = {&mod, 0};

	bool b = ldap_modify_s(m_ldap, buff, mods) == LDAP_SUCCESS;
	dprint4("user %s rename CustomGroup to %S with gid %d res:%d\n", owner, gname, gid, b);

	if (b) {
		hash = GetABGroupsHash(owner);
	} else {
		ProcessLDAPError();
	}

	return b;
}

bool VS_LDAPStorage::ManageGroups_AddUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash)
{
	if (!owner || !gid)
		return false;

	std::wstring dn;
	if (!LDAPGetUserDN(owner, dn) && dn.empty())
		return false;

	std::wstring dn2;
	if (!LDAPGetUserDN(call_id, dn2) && dn2.empty())			// if it is #sip - add as is
	{
		VS_WideStr wstr;	wstr.AssignUTF8(call_id);
		dn2 = wstr.m_str;
	}


	wchar_t buff[2048] = {0};
	_snwprintf(buff,(sizeof(buff) / sizeof(buff[0])) - 1,L"cn=%d,ou=CustomGroups,%s",gid,dn.c_str());

	//	VS_WideStr wstr;	wstr.AssignUTF8(dn2);
	wchar_t* new_vals[2] = {(wchar_t*)/*wstr.m_str*/dn2.c_str(),0};
	LDAPMod mod;
	mod.mod_op = LDAP_MOD_ADD;
	mod.mod_type = m_a_AddressBook;
	mod.mod_vals.modv_strvals = (PWCHAR*) &new_vals;

	LDAPMod* mods[2] = {&mod, 0};

	bool b = ldap_modify_s(m_ldap, buff, mods) == LDAP_SUCCESS;
	dprint4("user %s add %s to CustomGroup with gid %d res:%d\n", owner, call_id, gid, b);

	if (b) {
		hash = GetABGroupsHash(owner);
	} else {
		ProcessLDAPError();
	}
	return b;
}

bool VS_LDAPStorage::ManageGroups_DeleteUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash)
{
	if (!owner || !gid)
		return false;

	std::wstring dn;
	if (!LDAPGetUserDN(owner, dn) && dn.empty())
		return false;

	std::wstring dn2;
	if (!LDAPGetUserDN(call_id, dn2) && dn2.empty())			// if it is #sip - add as is
	{
		VS_WideStr wstr;	wstr.AssignUTF8(call_id);
		dn2 = wstr.m_str;
	}

	wchar_t buff[2048] = {0};
	_snwprintf(buff,(sizeof(buff) / sizeof(buff[0])) - 1,L"cn=%d,ou=CustomGroups,%s",gid,dn.c_str());

	//	VS_WideStr wstr;	wstr.AssignUTF8(call_id);
	wchar_t* new_vals[2] = {(wchar_t*)/*wstr.m_str*/dn2.c_str(),0};
	LDAPMod mod;
	mod.mod_op = LDAP_MOD_DELETE;
	mod.mod_type = m_a_AddressBook;
	mod.mod_vals.modv_strvals = (PWCHAR*) &new_vals;

	LDAPMod* mods[2] = {&mod, 0};

	bool b = ldap_modify_s(m_ldap, buff, mods) == LDAP_SUCCESS;
	dprint4("user %s delete %s from CustomGroup with gid %d res:%d\n", owner, call_id, gid, b);

	if (b) {
		hash = GetABGroupsHash(owner);
	} else {
		ProcessLDAPError();
	}
	return b;
}

long VS_LDAPStorage::GetABGroupsHash(const vs_user_id& owner)
{
	long hash(0);
	int entries;
	VS_Container cnt, tmp;
	if (FindUsers(cnt, entries, AB_GROUPS, owner, 0, 0, tmp)==SEARCH_DONE)
	{
		hash = *(cnt.GetLongValueRef(HASH_PARAM));
	}
	return hash;
}
#endif


bool VS_LDAPStorage::FetchUser(const tc::ldap_user_info& info, VS_StorageUserData& ude)
{
	if (!m_ldapCore) return false;
	bool ret = m_ldapCore->FetchUser(info, ude);
	if (ret && ude.m_Company.empty())
	{
		std::string buff;
		if (m_srvCert.GetSubjectEntry("organizationName", buff))
			ude.m_Company = buff;
	}
	return ret;
}

void VS_LDAPStorage::ProcessLDAPError(const tc::ldap_error_code_t error)
{
	dstream0 << "LDAP Error Code " << error << " desc:" << ldap_err2string(error) << "\n";
	if (m_state == STATE_RUNNING && tc::LDAPCore::IsErrorReconnect(error))
	{
		dprint0("#LDAP connection error, trying to reconnect\n");
		m_state = STATE_RECONNECT;
		LogEvent(DISCONNECT_SERVER_EVENT_TYPE);
		TryReConnect();
	}
}

void VS_LDAPStorage::TryReConnect()
{
	auto now = std::chrono::steady_clock::now();
	if (now - m_no_ldap_last_try_reconnect_tick <= m_timeout_to_reconnect)
		return;
	//m_timeout_to_reconnect *= 2;
	m_no_ldap_last_try_reconnect_tick = now;
	if (m_no_ldap_tick == decltype(m_no_ldap_tick)())
		m_no_ldap_tick = now;
	auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - m_no_ldap_tick);

	if (m_ldapCore && m_ldapCore->Connect(true) != 0)		// if error
		dstream0 << "#reconnect failed, no ldap for " << diff.count() << " seconds";
	else{
		dstream0 << "#reconnect ok (ldap was unavailable for " << diff.count() << " seconds)";
		m_no_ldap_tick = {};
		m_no_ldap_last_try_reconnect_tick = {};
		m_state = STATE_RUNNING;
		LogEvent(CONNECT_SERVER_EVENT_TYPE);
	}
}

/// absink interface realization

bool VS_LDAPStorage::FindUser_Sink(const vs_user_id& user_id, VS_StorageUserData& ude, bool onlyCached){
	bool res(false);
	if (!m_ldapCore)
		return res;
	res = m_ldapCore->FindUser_Sink(user_id, ude, onlyCached);
	if (res)
		ude.m_rights = GetUserRightsImp(user_id);
	return res;
}

bool VS_LDAPStorage::GetRegGroups(std::map<std::string, VS_RegGroupInfo>& reg_groups){
	if (!m_ldapCore) return false;
	else return m_ldapCore->GetRegGroups(reg_groups);
}
bool VS_LDAPStorage::GetAllUsers(std::shared_ptr<VS_AbCommonMap>& users){
	if (!m_ldapCore) return false;
	else return m_ldapCore->GetAllUsers(users);
}
bool VS_LDAPStorage::IsLDAP_Sink() const{
	if (!m_ldapCore) return false;
	else return m_ldapCore->IsLDAP_Sink();
}

void VS_LDAPStorage::LogEvent(string_view event)
{
	std::string json = "{\"event\":\"";
	json += event;
	json += "\",\"server\":\"";
	json += m_ldapCore->m_ldap_server;
	json += "\",\"port\":";
	json += std::to_string(m_ldapCore->m_ldap_port);
	json += "}";
	VS_Container log_cnt;
	log_cnt.AddValue(OBJECT_TYPE_PARAM, SERVER_OBJECT_TYPE);
	log_cnt.AddValue(OBJECT_NAME_PARAM, "ldap_storage");
	log_cnt.AddValue(EVENT_TYPE_PARAM, event);
	log_cnt.AddValue(PAYLOAD_PARAM, json);
	m_log_events->emplace_back(std::move(log_cnt));
}

void VS_LDAPStorage::ProcessAvatars(const std::string& login, const std::multimap<tc::attr_name_t, tc::attr_value_t>& attributes)
{
	auto res = attributes.find(m_ldapCore->m_a_avatars);
	if (res != attributes.cend())
	{
		CreateAvatarOnDisk(login, res->second, m_ldapCore->m_avatars_size, m_ldapCore->m_avatars_quality);
	}
	else
	{
		RemoveAvatarOnDisk(login);
	}
}


void VS_LDAPStorage::CreateAvatarOnDisk(const std::string& user, const std::string& content, int avatarSize, int avatarQuality)
{
	size_t output_len = 0;

	base64_decode(content.c_str(), content.length(), nullptr, output_len);

	if (output_len == 0)
		return;

	std::unique_ptr<char[]> data = vs::make_unique<char[]>(output_len);
	if (!base64_decode(content.c_str(), content.length(), data.get(), output_len))
		return;

	std::string filename;

	const auto escape = EscapeCallId(user);

	filename.reserve(m_ldapCore->m_ldap_avatars_path.length() + escape.length() + (sizeof(DIR_RESIZED_IMG) - 1) + (sizeof(IMG_EXTENSION) - 1));

	filename.append(m_ldapCore->m_ldap_avatars_path);

	boost::system::error_code ec;
	boost::filesystem::create_directories(filename, ec);
	if (ec) {
		dstream1 << "CreateAvatar: Error creating '" << filename << "': " << ec.message();
		return;
	}

	filename.append(escape).append(IMG_EXTENSION, ::strlen(IMG_EXTENSION));

	std::fstream file(filename, std::ios::in | std::ios::ate | std::ios::binary); //Attempt to open file.
	bool override = !(file.is_open() && !file.eof());
	if (!override)
	{
		const std::int64_t file_len = file.tellg();
		if (file_len == output_len) {
			file.seekg(0, std::ios::beg);
			char buff[1024];
			std::int64_t offset = 0;
			do
			{
				file.read(buff, sizeof(buff));

				const auto count = static_cast<unsigned int>(file.gcount());
				static_assert(std::numeric_limits<decltype(count)>::max() >= sizeof(buff));

				if (count && ::memcmp(buff, data.get() + offset, count) != 0)
				{
					override = true;
					break;
				}
				offset += count;

			} while (!file.eof());
		}
		else
		{
			override = true;
		}
	}

	if (override)
	{
		std::string new_filename;

		auto p_file = vs::GetTempFile(filename, "wb", new_filename, ec);

		if (p_file)
		{
			::fwrite(data.get(), sizeof(data[0]), output_len, p_file);
			::fclose(p_file);

			file.close();

			vs::RenameFile(new_filename, filename, ec);

			if (ec)
			{
				dstream1 << "can't rename from: " << new_filename << " to: " << filename << " error msg: " << ec.message();
			}
		}
		else
		{
			dstream1 << "can't open the file: " << new_filename << " error msg: " << ec.message();
		}
	}

	//create resized img
	const auto delim_pos = filename.find_last_of('/');
	filename.insert(delim_pos + 1, DIR_RESIZED_IMG, ::strlen(DIR_RESIZED_IMG));

	const std::size_t len_name_dir = (delim_pos + 1) + ::strlen(DIR_RESIZED_IMG);

	boost::filesystem::create_directories({filename.c_str(), filename.c_str() + len_name_dir }, ec);
	if (ec) {
		dstream1 << "CreateAvatar: Error creating '" << filename << "': " << ec.message();
		return;
	}

	if (!boost::filesystem::exists(filename, ec)) //if file not exist in the cache/resized/filename
	{
		std::vector<std::uint8_t> out;
		bool resized;

		filename.erase(delim_pos + 1, ::strlen(DIR_RESIZED_IMG));

		if (resize_avatar(filename, out, avatarSize, avatarQuality, resized) && resized)
		{
			if (out.size() <= MAX_AVATAR_SIZE ? true
				: (out.clear(), resize_avatar(filename, out, tc::LDAP_AVATARS_SIZE_INIT, avatarQuality, resized)))
			{
				assert(resized);
				assert(out.size() <= MAX_AVATAR_SIZE);

				filename.insert(delim_pos + 1, DIR_RESIZED_IMG, ::strlen(DIR_RESIZED_IMG));

				std::string temp_filename;

				auto p_file = vs::GetTempFile(filename, "wb", temp_filename, ec);

				if (p_file)
				{
					::fwrite(out.data(), sizeof(decltype(out)::value_type), out.size(), p_file);
					::fclose(p_file);

					vs::RenameFile(temp_filename, filename, ec);

					if (ec)
					{
						dstream1 << "can't rename from: " << temp_filename << " to: " << filename << " error msg: " << ec.message();
					}else{
						dstream3 << "LDAPStorage::CreateAvatarOnDisk: user=" << user
							<< ", ava_size=" << avatarSize << ", filename=" << filename;
					}
				}
				else
				{
					dstream1 << "can't open the file: " << temp_filename << " error msg: " << ec.message();
				}
			}
		}
	}
}


bool VS_LDAPStorage::RemoveAvatarOnDisk(const std::string &user)
{
	std::string filename;

	const auto escape = EscapeCallId(user);

	filename.reserve(m_ldapCore->m_ldap_avatars_path.length() + escape.length() + (sizeof(DIR_RESIZED_IMG) - 1) + (sizeof(IMG_EXTENSION) - 1));

	filename.append(m_ldapCore->m_ldap_avatars_path).append(escape).append(IMG_EXTENSION, ::strlen(IMG_EXTENSION));

	boost::system::error_code ec;
	if (vs::RemoveFile(filename, ec))
		dstream3 << "LDAPStorage::RemoveAvatarOnDisk: user=" << user << ", filename=" << filename;

	bool result = true;
	if (ec)
	{
		dstream1 << "can't remove the file: " << filename << " error msg: " << ec.message();
		result = false;
	}

	//remove resized img
	filename.insert(filename.find_last_of('/') + 1, DIR_RESIZED_IMG, ::strlen(DIR_RESIZED_IMG));

	if (vs::RemoveFile(filename, ec))
		dstream3 << "LDAPStorage::RemoveAvatarOnDisk: user=" << user << ", filename=" << filename;

	return result;
}
#include "VS_LDAPCore.h"
#ifndef _WIN32
#include "ldap_core/liblutil/lutil_ldap.h"
#endif
#include "CfgHelper.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/synchronized.h"
#include "std/cpplib/md5.h"
#include "std/cpplib/latch.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_WideStr.h"
#include "std/cpplib/FilesystemUtils.h"
#include "std/cpplib/curl_deleters.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/VS_Replace.h"
#include "std/cpplib/base64.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/cpplib/ignore.h"
#include "std-generic/compat/memory.h"
#include "net/DNSUtils/VS_DNS.h"

#include <curl/curl.h>

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>


#include <sstream>
#include <fstream>

#define LDAP_MATCHING_RULE_IN_CHAIN "1.2.840.113556.1.4.1941"
#define LDAP_OID_SERVER_SORT_CONTROL "1.2.840.113556.1.4.473"

#define DEBUG_CURRENT_MODULE VS_DM_RSTOR

#define GET_STRING_PARAM_OR_DEFAULT(var,tag,init) { std::map<std::string,std::string>::iterator it = m.find(tag); \
	if (it!=m.end())		\
		var = it->second;	\
	else					\
		var = init;			\
}

#define GET_STRING_PARAM_OR_DEFAULT_LOCKED(var,tag,init) { std::map<std::string,std::string>::iterator it = m.find(tag); \
	if (it!=m.end())		\
		var.set(it->second);	\
				else					\
		var.set(init);			\
}

#define GET_LONG_PARAM_OR_DEFAULT(var,tag,init, cast) { std::map<std::string,std::string>::iterator it = m.find(tag); \
	if (it!=m.end())		\
		var = (cast)::atoll(it->second.c_str()); \
	else					\
		var = (cast)init;			\
}

#define GET_BOOL_PARAM_OR_DEFAULT(var,tag,init) { std::map<std::string,std::string>::iterator it = m.find(tag); \
	if (it!=m.end())		\
		var = ::atoi(it->second.c_str()) != 0;	\
	else					\
		var = init;			\
}
#define GET_MULTYSTRING_PARAM(var, tag,init) { std::map<std::string, std::string>::iterator it = m.find(tag); \
	if (it != m.end()){\
		auto p = it->second.c_str(); \
		while (*p){	var.push_back(p); 	p += (strlen(p) + 1); }\
	}\
	else var = init;\
}

namespace tc
{

	static std::string GetDomainFromDistinguishedName(string_view dn) noexcept
	{
		auto str = vs::UTF8ToLower(dn);
		auto pos = str.find("dc=");
		if (pos == string_view::npos)
			return {};
		str.erase(0, pos);
		VS_ReplaceAll(str, "dc=", "");
		VS_ReplaceAll(str, ",", ".");
		return str;
	}

	unsigned long cfg_params_t::CaclHash() const
	{
		MD5 md5;
		for (const auto& x : *this)
		{
			md5.Update(x.first);
			md5.Update(x.second);
		}
		md5.Final();
		unsigned char digest[16];
		md5.GetBytes(digest);
		const auto p = reinterpret_cast<const uint32_t*>(digest);
		unsigned long hash = p[0] ^ p[1] ^ p[2] ^ p[3];
		return hash;
	}
	////////////////////////////////////////////////////////////////////////////////
	LDAPCore::LDAPCore(ProcessAvatarsHandler avatarsHandler)
		: m_server_type(LDAP_SERVER_TYPE_AD)
		, m_use_avatars(false)
		, m_avatar_propagating_allowed(false)
		, m_avatarsHandler(std::move(avatarsHandler))
	{
		m_trustedDomains_updated = false;
		memset(LDAP_FILTER_IS_USER, 0, sizeof(LDAP_FILTER_IS_USER));
		m_params_hash.set(0);
		m_params_changed.set(false);
	}

	LDAPCore::~LDAPCore()
	{
		DeInitUpdateNestedCacheThread();
		m_ldap.store(std::make_shared<tc::LDAPCore::CtxUnbinder>(nullptr));
	}

	bool LDAPCore::Init(const cfg_params_t& params, bool do_not_run_cache_updater)
	{
		assert(!m_nested_thread.joinable());

		unsigned long new_hash = params.CaclHash();
		if (new_hash && (new_hash == m_params_hash.get()))
			return true;
		m_ldap.store(std::make_shared<tc::LDAPCore::CtxUnbinder>(nullptr));
		const unsigned long buff_len = 1024;
		char buff[buff_len] = { 0 };

		std::map<std::string, std::string> m(params.begin(), params.end());
		GET_LONG_PARAM_OR_DEFAULT(m_server_type, LDAP_SERVER_TYPE_TAG, LDAP_SERVER_TYPE_INIT, eLDAPServerType);
		GET_STRING_PARAM_OR_DEFAULT(m_basedn, LDAP_BASEDN_TAG, "");
		GET_STRING_PARAM_OR_DEFAULT(m_our_domain, LDAP_OUR_DOMAIN_TAG, "");
		GET_STRING_PARAM_OR_DEFAULT(m_ldap_server, LDAP_SERVER_TAG, "");
		GET_STRING_PARAM_OR_DEFAULT(m_domain, LDAP_DOMAIN_TAG, "");
		GET_LONG_PARAM_OR_DEFAULT(m_auth_method, LDAP_AUTH_METHOD_TAG, VS_LDAP_AUTH_DEFAULT, AuthMethods);
		GET_STRING_PARAM_OR_DEFAULT(m_auth_user, LDAP_AUTH_USER_TAG, "");
		GET_STRING_PARAM_OR_DEFAULT(m_auth_domain, LDAP_AUTH_DOMAIN_TAG, "");
		GET_STRING_PARAM_OR_DEFAULT(m_auth_password, LDAP_AUTH_PASSWORD_TAG_NEW, "");
		if (m_auth_password.empty())
			GET_STRING_PARAM_OR_DEFAULT(m_auth_password, LDAP_AUTH_PASSWORD_TAG_OLD, "");
		GET_LONG_PARAM_OR_DEFAULT(m_ldap_port, LDAP_PORT_TAG, LDAP_PORT_INIT, unsigned short);
		GET_LONG_PARAM_OR_DEFAULT(m_max_results, LDAP_MAX_RESULTS, LDAP_MAX_RESULTS_INIT, uint32_t);
		GET_LONG_PARAM_OR_DEFAULT(m_ldap_secure, LDAP_SECURE_TAG, LDAP_SECURE_INIT, int);
		GET_BOOL_PARAM_OR_DEFAULT(m_ldap_auto_verify_server_cert, LDAP_AUTOVERIFY_SERVERCERT_TAG, false);
		int32_t tmp;
		GET_LONG_PARAM_OR_DEFAULT(tmp, LDAP_TIMEOUT_TAG, LDAP_TIMEOUT_INIT, decltype(tmp));
		m_ldap_timeout = std::chrono::seconds(tmp);

		GET_LONG_PARAM_OR_DEFAULT(m_ldap_version, LDAP_VERSION_TAG, LDAP_VERSION_INIT, long);
		GET_BOOL_PARAM_OR_DEFAULT(m_use_ntlm, NTLM_ON_TAG, false);
		GET_BOOL_PARAM_OR_DEFAULT(m_ldap_autodetect, LDAP_AUTO_DETECT_TAG, LDAP_AUTO_DETECT_INIT);
		GET_STRING_PARAM_OR_DEFAULT_LOCKED(m_login_group, LDAP_GROUP_TAG, "");
		unsigned long t_seconds(0);
		GET_LONG_PARAM_OR_DEFAULT(t_seconds, LDAP_AB_REFRESH_TAG, LDAP_AB_REFRESH_INIT, unsigned long);
		m_ab_cache_timeout = std::chrono::seconds(t_seconds);
		GET_STRING_PARAM_OR_DEFAULT(m_ldap_avatars_path, LDAP_AVATARS_PATH_TAG, LDAP_AVATARS_PATH_INIT);
		GET_LONG_PARAM_OR_DEFAULT(m_use_avatars, LDAP_USE_AVATARS_TAG, LDAP_USE_AVATARS_INIT, bool);
		GET_LONG_PARAM_OR_DEFAULT(m_avatar_propagating_allowed, LDAP_ALLOW_AVATAR_PROPAGATING_TAG, LDAP_ALLOW_AVATAR_PROPAGATING_INIT, bool);
		GET_LONG_PARAM_OR_DEFAULT(m_avatars_size, LDAP_AVATARS_SIZE_TAG, LDAP_AVATARS_SIZE_INIT, std::int32_t);
		GET_LONG_PARAM_OR_DEFAULT(m_avatars_quality, LDAP_AVATARS_QUALITY_TAG, LDAP_AVATARS_QUALITY_INIT, std::int32_t);

		std::string user_alias_list;
		GET_STRING_PARAM_OR_DEFAULT(user_alias_list, LDAP_ATTR_USER_ALIAS_LIST_TAG, LDAP_ATTR_USER_ALIAS_LIST_INIT);
		if (!user_alias_list.empty() && (user_alias_list != "string, string,...")) {
			boost::trim_if(user_alias_list, boost::is_any_of(", "));
			boost::split(m_user_aliases, user_alias_list, boost::is_any_of(", "), boost::token_compress_on);
		}

		// attrs
		GET_STRING_PARAM_OR_DEFAULT(m_a_login, LDAP_ATTR_LOGIN_TAG, LDAP_ATTR_LOGIN_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_a_email, LDAP_ATTR_EMAIL_TAG, LDAP_ATTR_EMAIL_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_a_firstname, LDAP_ATTR_FIRSTNAME_TAG, LDAP_ATTR_FIRSTNAME_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_a_middlename, LDAP_ATTR_MIDDLENAME_TAG, LDAP_ATTR_MIDDLENAME_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_a_lastname, LDAP_ATTR_LASTNAME_TAG, LDAP_ATTR_LASTNAME_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_a_company, LDAP_ATTR_COMPANY_TAG, LDAP_ATTR_COMPANY_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_a_displayname, LDAP_ATTR_DISPLAYNAME_TAG, LDAP_ATTR_DISPLAYNAME_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_a_distinguishedName, LDAP_ATTR_DISTINGUISHEDNAME_TAG, LDAP_ATTR_DISTINGUISHEDNAME_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_a_memberOf, LDAP_ATTR_MEMBEROF_TAG, LDAP_ATTR_MEMBEROF_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_a_groupmember, LDAP_ATTR_GROUP_MEMBER_TAG, LDAP_ATTR_GROUP_MEMBER_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_a_GroupDisplayName, LDAP_ATTR_GROUP_DISPLAY_NAME_TAG, LDAP_ATTR_GROUP_DISPLAY_NAME_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_a_primaryGroupToken, LDAP_ATTR_PRIMARY_GROUP_TOKEN_TAG, LDAP_ATTR_PRIMARY_GROUP_TOKEN_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_a_primaryGroupId, LDAP_ATTR_PRIMARY_GROUP_ID_TAG, LDAP_ATTR_PRIMARY_GROUP_ID_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_a_objectSid, LDAP_ATTR_OBJECT_SID_TAG, LDAP_ATTR_OBJECT_SID_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_ldap_attr_Phone_mobile, LDAP_ATTR_PHONE_MOBILE_TAG, LDAP_ATTR_PHONE_MOBILE_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_ldap_attr_Phone_work, LDAP_ATTR_PHONE_WORK_TAG, LDAP_ATTR_PHONE_WORK_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_ldap_attr_Phone_home, LDAP_ATTR_PHONE_HOME_TAG, LDAP_ATTR_PHONE_HOME_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_ldap_attr_UserStatus, LDAP_ATTR_USER_STATUS_TAG, LDAP_ATTR_USER_STATUS_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_ldap_attr_UserID, LDAP_ATTR_USER_ID_TAG, LDAP_ATTR_USER_ID_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_ldap_attr_FullID, LDAP_ATTR_FULL_ID_TAG, LDAP_ATTR_FULL_ID_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_a_avatars, LDAP_ATTR_AVATARS_TAG, LDAP_ATTR_AVATARS_INIT);

		// filters
		GET_STRING_PARAM_OR_DEFAULT(m_filter_login, LDAP_FILTER_LOGIN_TAG, LDAP_FILTER_LOGIN_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_filter_callid, LDAP_FILTER_CALLID_TAG, LDAP_FILTER_CALLID_INIT);
		GET_STRING_PARAM_OR_DEFAULT_LOCKED(m_filter_disabled, LDAP_FILTER_DISABLED_TAG, LDAP_FILTER_DISABLED_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_filter_group, LDAP_FILTER_GROUP_TAG, LDAP_FILTER_GROUP_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_filter_ab, LDAP_FILTER_AB_TAG, LDAP_FILTER_AB_INIT);
		GET_BOOL_PARAM_OR_DEFAULT(m_filter_search_by_login_group, LDAP_FILTER_SEARCH_BY_LOGIN_GROUP_TAG, true);
		GET_STRING_PARAM_OR_DEFAULT(m_filter_person, LDAP_FILTER_PERSON_TAG, LDAP_FILTER_PERSON_INIT);

		unsigned int max_filter_login_size = (sizeof(LDAP_FILTER_IS_USER) / sizeof(LDAP_FILTER_IS_USER[0])) - 2;
		if (m_filter_login.length() >= max_filter_login_size)
		{
			dprint4("Too big Filter Login: size=%zu, max_size=%d\n", m_filter_login.length(), max_filter_login_size);
			return false;
		}
		auto n = std::count(m_filter_login.begin(), m_filter_login.end(), L'%');	// amount of %s should be 1
		if (n != 1)
		{
			dprint4("Filter Login should have only one %% argument\n");
			return false;
		}

		snprintf(LDAP_FILTER_IS_USER, max_filter_login_size, m_filter_login.c_str(), "*");

		GET_BOOL_PARAM_OR_DEFAULT(m_referrals_enabled, LDAP_REFERRAL_TAG, true);
		GET_LONG_PARAM_OR_DEFAULT(m_referrals_hops, LDAP_REFERRAL_HOPS_TAG, 32, long);
		GET_BOOL_PARAM_OR_DEFAULT(m_trust_enabled, LDAP_ENABLE_TRUST_TAG, true);
		GET_STRING_PARAM_OR_DEFAULT(m_a_trustPartner, LDAP_ATTR_TRUST_PARTNER_TAG, LDAP_ATTR_TRUST_PARTNER_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_a_flatName, LDAP_ATTR_FLAT_NAME_TAG, LDAP_ATTR_FLAT_NAME_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_filter_trustedDomain, LDAP_FILTER_TRUSTED_DOMAIN_TAG, LDAP_FILTER_TRUSTED_DOMAIN_INIT);
		GET_STRING_PARAM_OR_DEFAULT(m_filter_foreignSecurityPrincipal, LDAP_FILTER_FOREIGN_SECURITY_PRINCIPAL_TAG, LDAP_FILTER_FOREIGN_SECURITY_PRINCIPAL_INIT);
#ifdef _WIN32
		GET_MULTYSTRING_PARAM(m_skip_referrals, LDAP_REFERRAL_SKIP_TAG, DEFAULT_SKIP_REFERRALS);
#endif

		//int res = Connect();
		//if (res)
		//	return false;

		//if (m_basedn.empty())
		//{
		//	GetServerRootInfo();
		//	if (m_basedn.empty())
		//	{
		//		dprint1("\t\tbase dn not found!\n");
		//		return VSS_LDAP_INIT_ERROR;
		//	}
		//	dprint1("\t\tBuilt base dn:%S\n", m_basedn.c_str());
		//}
		//
		//
		//	if (!wcsstr(m_filter_login, "%s"))
		//	{
		//		dprint1("\t\twrong LDAP Login Filter\n");
		//		return VSS_LDAP_INIT_ERROR;
		//	};
		//
		//	if (!wcsstr(m_filter_callid, "%S"))
		//	{
		//		dprint1("\t\twrong LDAP Call ID Filter\n");
		//		return VSS_LDAP_INIT_ERROR;
		//	};
		//
		//
		//
		//#ifdef _SVKS_M_BUILD_
		//	VS_WideStr tmp2;
		//	cfg.GetValue(tmp2.m_str, VS_REG_WSTRING_VT, LDAP_REGION_TAG);
		//	m_region.set(tmp2);
		//#endif
		//
		//	//if (cfg.GetValue(&temp, sizeof(temp), VS_REG_INTEGER_VT, LDAP_AUTOMANAGEAB_MAXUSERS_TAG) != 0)
		//	//	m_AutoManageAB_MaxUsers = temp;
		//	//else
		//	//	m_AutoManageAB_MaxUsers = LDAP_AUTOMANAGEAB_MAXUSERS_INIT;
		//
		//
		//	m_ldap_attr_Phone_mobile.Empty();
		//	cfg.GetValue(m_ldap_attr_Phone_mobile.m_str, VS_REG_WSTRING_VT, LDAP_ATTR_PHONE_MOBILE_TAG);
		//	if (!m_ldap_attr_Phone_mobile)
		//		m_ldap_attr_Phone_mobile = LDAP_ATTR_PHONE_MOBILE_INIT;
		//
		//	m_ldap_attr_Phone_work.Empty();
		//	cfg.GetValue(m_ldap_attr_Phone_work.m_str, VS_REG_WSTRING_VT, LDAP_ATTR_PHONE_WORK_TAG);
		//	if (!m_ldap_attr_Phone_work)
		//		m_ldap_attr_Phone_work = LDAP_ATTR_PHONE_WORK_INIT;
		//
		//	m_ldap_attr_Phone_home.Empty();
		//	cfg.GetValue(m_ldap_attr_Phone_home.m_str, VS_REG_WSTRING_VT, LDAP_ATTR_PHONE_HOME_TAG);
		//	if (!m_ldap_attr_Phone_home)
		//		m_ldap_attr_Phone_home = LDAP_ATTR_PHONE_HOME_INIT;
		//
#ifdef _SVKS_M_BUILD_
		m_a_ALLOWED_BY_SERVER_MAX_BW = ALLOWED_BY_SERVER_MAX_BW;
		m_a_ALLOWED_BY_SERVER_MAX_FPS = ALLOWED_BY_SERVER_MAX_FPS;
		m_a_ALLOWED_BY_SERVER_MAX_WXH = ALLOWED_BY_SERVER_MAX_WXH;
#endif
		//
		//	m_a_AddressBook.Empty();
		//	cfg.GetValue(m_a_AddressBook.m_str, VS_REG_WSTRING_VT, LDAP_ATTR_ADDRESS_BOOK_TAG);
		//	if (!m_a_AddressBook)
		//		m_a_AddressBook = LDAP_ATTR_ADDRESS_BOOK_INIT;
		//
		//	m_a_ServerName.Empty();
		//	cfg.GetValue(m_a_ServerName.m_str, VS_REG_WSTRING_VT, LDAP_ATTR_SERVER_NAME_TAG);
		//	if (!m_a_ServerName)
		//		m_a_ServerName = LDAP_ATTR_SERVER_NAME_INIT;
		//
		std::string DetailedUserInfo;
		GET_STRING_PARAM_OR_DEFAULT(DetailedUserInfo, LDAP_ATTR_DETAILED_USER_INFO_TAG, LDAP_ATTR_DETAILED_USER_INFO_INIT);

		m_detailed_user_info.clear();
		boost::split(m_detailed_user_info, DetailedUserInfo, boost::is_any_of(", "), boost::token_compress_on);

		GET_LONG_PARAM_OR_DEFAULT(m_sort_control_scheme, LDAP_SORT_CONTROL_SCHEME_TAG, SortControlScheme::INVALID, SortControlScheme);
		if (m_sort_control_scheme == SortControlScheme::INVALID)
		{
			if (m_server_type == eLDAPServerType::LDAP_SERVER_TYPE_389DIR)
				m_sort_control_scheme = SortControlScheme::NO;
			else
				m_sort_control_scheme = SortControlScheme::AUTO;
		}

		//	//unsigned int n = 0;
		//	//m_al_userfull[n++] = m_a_login;
		//	//if (!!m_a_login2)
		//	//	m_al_userfull[n++] = m_a_login2;
		//	//m_al_userfull[n++] = m_a_email;
		//	//m_al_userfull[n++] = m_a_displayname;
		//	//m_al_userfull[n++] = m_a_firstname;
		//	//m_al_userfull[n++] = m_a_lastname;
		//	//m_al_userfull[n++] = m_a_company;
		//	//if (!!m_ldap_attr_FullID)
		//	//	m_al_userfull[n++] = m_ldap_attr_FullID;
		//	//if (!!m_ldap_attr_Phone_mobile)
		//	//	m_al_userfull[n++] = m_ldap_attr_Phone_mobile;
		//	//if (!!m_ldap_attr_Phone_work)
		//	//	m_al_userfull[n++] = m_ldap_attr_Phone_work;
		//	//if (!!m_ldap_attr_Phone_home)
		//	//	m_al_userfull[n++] = m_ldap_attr_Phone_home;
		//	//m_al_userfull[n] = 0;
		//
		//	//if (n + m_detailed_user_info.size() < 100)
		//	//{
		//	//	for (unsigned int i = 0; i < n - 1; ++i)
		//	//		m_al_userfull_detailed[i] = m_al_userfull[i];
		//
		//	//	unsigned int index = n - 1;
		//	//	for (std::vector<std::pair<std::string, std::string>>::iterator it = m_detailed_user_info.begin(); it != m_detailed_user_info.end(); ++it)
		//	//	{
		//	//		m_al_userfull_detailed[index++] = (wchar_t*)it->first.c_str();
		//	//	}
		//
		//	//	m_al_userfull_detailed[index] = 0;
		//	//}
		//
		//	//n = 0;
		//	//m_al_user[n++] = m_a_login;
		//	//if (!!m_a_login2)
		//	//	m_al_user[n++] = m_a_login2;
		//	//m_al_user[n++] = m_a_email;
		//	//m_al_user[n++] = m_a_displayname;
		//	//if (!!m_ldap_attr_FullID)
		//	//	m_al_user[n++] = m_ldap_attr_FullID;
		//	//if (!!m_ldap_attr_Phone_mobile)
		//	//	m_al_user[n++] = m_ldap_attr_Phone_mobile;
		//	//if (!!m_ldap_attr_Phone_work)
		//	//	m_al_user[n++] = m_ldap_attr_Phone_work;
		//	//if (!!m_ldap_attr_Phone_home)
		//	//	m_al_user[n++] = m_ldap_attr_Phone_home;
		//	//m_al_user[n] = 0;
		//
		//	//n = 0;
		//	//m_al_userlogin[n++] = m_a_login;
		//	//if (!!m_a_login2)
		//	//	m_al_userlogin[n++] = m_a_login2;
		//	//m_al_userlogin[n++] = m_a_email;
		//	//m_al_userlogin[n++] = m_a_displayname;
		//	//m_al_userlogin[n++] = LDAP_ATTR_AD_GROUP_ID;
		//
		//	//m_al_userlogin[n++] = m_a_ALLOWED_BY_SERVER_MAX_BW;
		//	//m_al_userlogin[n++] = m_a_ALLOWED_BY_SERVER_MAX_FPS;
		//	//m_al_userlogin[n++] = m_a_ALLOWED_BY_SERVER_MAX_WXH;
		//
		//	//m_al_userlogin[n] = 0;
		//
		//	//n = 0;
		//	//m_al_groupmember[n++] = m_a_groupmember;
		//	//m_al_groupmember[n] = 0;
		//
		//	//n = 0;
		//	//m_al_group_member_user[n++] = m_a_login;
		//	//if (!!m_a_login2)
		//	//	m_al_group_member_user[n++] = m_a_login2;
		//	//m_al_group_member_user[n++] = m_a_email;
		//	//m_al_group_member_user[n++] = m_a_displayname;
		//	//m_al_group_member_user[n++] = m_a_groupmember;
		//	//m_al_group_member_user[n++] = m_a_firstname;
		//	//m_al_group_member_user[n++] = m_a_lastname;
		//	//if (!!m_ldap_attr_FullID)
		//	//	m_al_group_member_user[n++] = m_ldap_attr_FullID;
		//	//if (!!m_ldap_attr_Phone_mobile)
		//	//	m_al_group_member_user[n++] = m_ldap_attr_Phone_mobile;
		//	//if (!!m_ldap_attr_Phone_work)
		//	//	m_al_group_member_user[n++] = m_ldap_attr_Phone_work;
		//	//if (!!m_ldap_attr_Phone_home)
		//	//	m_al_group_member_user[n++] = m_ldap_attr_Phone_home;
		//	//m_al_group_member_user[n] = 0;
		//
		//
		//	//n = 0;
		//	//m_al_AddressBook[n++] = m_a_AddressBook;
		//	//m_al_AddressBook[n] = 0;
		//
		//
		//	//////////////////////////////////////////////////////////////////
		//	/// NTLM Initialization
		//
		//	if (cfg.GetValue(&temp, sizeof(temp), VS_REG_INTEGER_VT, NTLM_ON_TAG) != 0)
		//		m_use_ntlm = temp != 0;
		//	else
		//		m_use_ntlm = false;
		//
		//	//////////////////////////////////////////////////////////////////
		//	/// Done
		//
		////	m_state = STATE_RUNNING;

		std::set<tc::attr_name_t> custom_attrs_set = {
			m_a_distinguishedName,		// todo(kt): what to do here for 389dir
			m_a_primaryGroupId,
			m_a_memberOf,
			m_a_login,
			m_a_displayname,
			m_a_objectSid,
			m_a_email,
			m_a_company,
			m_a_firstname,
			m_a_lastname,
			m_ldap_attr_Phone_mobile,
			m_ldap_attr_Phone_work,
			m_ldap_attr_Phone_home,
			m_a_avatars
#ifdef _SVKS_M_BUILD_
			, m_a_ALLOWED_BY_SERVER_MAX_BW
			, m_a_ALLOWED_BY_SERVER_MAX_FPS
			m_a_ALLOWED_BY_SERVER_MAX_WXH
#endif
		};
		if (!m_user_aliases.empty()) {
			custom_attrs_set.insert(std::begin(m_user_aliases), std::end(m_user_aliases));
		}

		std::move(custom_attrs_set.begin(), custom_attrs_set.end(), std::back_inserter(m_custom_attrs_user_info));

		InitLib();

		m_params_hash.set(new_hash);
		m_params_changed.set(true);

		if (!do_not_run_cache_updater)
		{
			InitUpdateNestedCacheThread();
		}

		return true;		// no error
	}

	int LDAPCore::ConnectBySRV(const std::string& domain) {
		if (domain.empty())
			return VSS_LDAP_ERROR;

		std::string srv_record = "_ldap._tcp.";
		srv_record += domain;

		dstream1 << "start resolve DNS SRV: " << srv_record;

		int result = VSS_LDAP_ERROR;

		auto res = net::dns::make_srv_lookup(srv_record).get();

		if(!res.second) // no error
		{
			for(auto &item : res.first)
			{
				m_ldap_server = std::move(item.host);
				m_ldap_port = (item.port) ? item.port : LDAP_PORT_INIT;

				if (m_ldap_secure && m_ldap_port == LDAP_PORT_INIT)
				{
					m_ldap_port = LDAP_SECURE_PORT_INIT;
				}

				dstream1 << "start connect " << m_ldap_server << ":" << m_ldap_port;
				LDAP* new_ctx = nullptr;
				result = ConnectServer(new_ctx, m_ldap_server.c_str(), m_ldap_port);
				if (result == 0 && new_ctx)	// no error
				{
					m_ldap = std::make_shared<tc::LDAPCore::CtxUnbinder>(new_ctx);
					dstream1 << "connected to " << m_ldap_server << ":" << m_ldap_port;
					break;
				}

				dstream1 << "failed to connect " << m_ldap_server << ":" << m_ldap_port;
			}
		}
		else
		{
			dstream1 << "failed SRV lookup: " << res.second.message();
		}

		return result;
	}

	int LDAPCore::ConnectByHost(const std::string& domain) {
		if (domain.empty())
			return VSS_LDAP_ERROR;

		dstream1 << "start resolve DNS A: " << domain;

		int result = VSS_LDAP_ERROR;

		auto res_a = net::dns::make_a_lookup(domain);
		auto res_aaaa = net::dns::make_aaaa_lookup(domain);

		 for(auto item : { &res_a, &res_aaaa })
		 {
			 auto reply = item->get();
			 if(!reply.second) //no err
			 {
				 dstream1 << "resolved " << reply.first.addrs.size();
				 for(const auto& addr : reply.first.addrs)
				 {
					 m_ldap_server = addr.to_string();
					 m_ldap_port = (m_ldap_secure) ? LDAP_SECURE_PORT_INIT : LDAP_PORT_INIT;
					 dstream1 << "start connect " << m_ldap_server << ":" << m_ldap_port;
					 LDAP* new_ctx = nullptr;
					 result = ConnectServer(new_ctx, m_ldap_server.c_str(), m_ldap_port);
					 if (result == 0 && new_ctx)	// no error
					 {
						 m_ldap = std::make_shared<tc::LDAPCore::CtxUnbinder>(new_ctx);
						 dstream1 << "connected to " << m_ldap_server << ":" << m_ldap_port;
						 break;
					 }
					 dstream1 << "failed to connect " << m_ldap_server << ":" << m_ldap_port;
				 }
			 }
		 }

		return result;
	}

	int LDAPCore::ConnectByIp(const std::string & ipStr)
	{
		if (ipStr.empty())
			return VSS_LDAP_ERROR;

		/*
			1. Make anonymous bind by IP.
			2. Fetch dnsHostName record
			3. Connect by dnsHostName
		 */

		m_ldap_port = (m_ldap_secure) ? LDAP_SECURE_PORT_INIT : LDAP_PORT_INIT;

		// prepare for anonymous bind
		auto authMethod = m_auth_method;
		auto authUser = m_auth_user;
		auto authPwd = m_auth_password;
		m_auth_method = AuthMethods::VS_LDAP_AUTH_SIMPLE;
		m_auth_password = m_auth_user = "";

		dstream1 << "start anonymous bind at : " << ipStr;
		LDAP* new_ctx = nullptr;
		int result = ConnectServer(new_ctx, ipStr.c_str(), m_ldap_port);
		// restore values after anonymous bind
		std::tie(m_auth_method, m_auth_user, m_auth_password) = std::tie(authMethod, authUser, authPwd);

		if (result != 0 || !new_ctx)
			return result;
		m_ldap = std::make_shared<tc::LDAPCore::CtxUnbinder>(new_ctx);
		new_ctx = nullptr;
		dstream1 << "start searching for dnsServerName attribute at : " << ipStr;
		std::string dnsName;
		if(!GetDnsServerName(m_ldap.load()->ctx(), dnsName))
			return VSS_LDAP_ERROR;

		dstream1 << "Found dnsServerName=" << dnsName;
		m_ldap_server = dnsName;

		dstream1 << "start connect " << m_ldap_server << ":" << m_ldap_port;
		result = ConnectServer(new_ctx, m_ldap_server.c_str(), m_ldap_port);
		if (result != 0 || !new_ctx) {	// connect directly by IP if any other attepts are failed
			m_ldap_server = ipStr;
			result = ConnectServer(new_ctx, m_ldap_server.c_str(), m_ldap_port);
		}
		if (result == 0 && new_ctx) {
			dstream1 << "connected to " << m_ldap_server << ":" << m_ldap_port;
			m_ldap = std::make_shared<tc::LDAPCore::CtxUnbinder>(new_ctx);
		}
		else
			dstream1 << "failed to connect " << m_ldap_server << ":" << m_ldap_port;

		return result;
	}

	int LDAPCore::WriteAvatar(const std::string &dn, const std::string &attr, const std::vector<std::uint8_t> &buff)
	{
		static_assert(sizeof(std::uint8_t) == sizeof(char), "!");

		if (dn.empty())
			return VSS_USER_NOT_FOUND;

		auto ldap = m_ldap.load(std::memory_order_acquire);
		assert(ldap != nullptr);

#ifdef _WIN32
		auto dn_holder = vs::UTF8ToWideCharConvert(dn);
		auto dn_ = (PWSTR)dn_holder.c_str();
#else
		auto dn_ = (char *)dn.c_str();
#endif

#ifdef _WIN32
		auto attr_hold = vs::UTF8ToWideCharConvert(attr);
		auto name = (wchar_t*)attr_hold.c_str();
#else
		auto name = (char*)attr.c_str();
#endif

		struct berval value;
		value.bv_len = buff.size();
		value.bv_val = (char *)buff.data();
		struct berval* values[2] = { &value, NULL };

		LDAPMod mod;
		mod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
		mod.mod_vals.modv_bvals = values;
		mod.mod_type = name;
		LDAPMod* mods[2] = { &mod, 0 };

		int ret;
		if ((ret = ldap_modify_ext_s(ldap->ctx(), dn_, mods, nullptr, nullptr)) != LDAP_SUCCESS && ret != LDAP_NO_SUCH_ATTRIBUTE)
			return VSS_LDAP_ERROR;

		return 0;
	}

	int LDAPCore::DeleteAvatar(const std::string &dn)
	{
		if (dn.empty())
			return VSS_USER_NOT_FOUND;

		auto ldap = m_ldap.load(std::memory_order_acquire);
		assert(ldap != nullptr);

#ifdef _WIN32
		auto dn_holder = vs::UTF8ToWideCharConvert(dn);
		auto dn_ = (PWSTR)dn_holder.c_str();
#else
		auto dn_ = dn.c_str();
#endif

		LDAPMod mod;
		mod.mod_op = LDAP_MOD_DELETE;
		mod.mod_vals.modv_strvals = NULL;
		LDAPMod* mods[2] = { &mod, 0 };
		if (!m_a_avatars.empty()) {
#ifdef _WIN32
			auto name = vs::UTF8ToWideCharConvert(m_a_avatars);
			mod.mod_type = (wchar_t*)name.c_str();
#else
			mod.mod_type = (char*)m_a_avatars.c_str();
#endif
			const auto res = ldap_modify_ext_s(ldap->ctx(), dn_, mods, nullptr, nullptr);
			return res == LDAP_SUCCESS || res == LDAP_NO_SUCH_ATTRIBUTE ? 0 : VSS_LDAP_ERROR;
		}
		return 0;
	}


	int LDAPCore::Connect(bool force)
	{
		if (!force)
		{
			if (!m_params_changed.get())
				return (m_params_hash.get()) ? 0 : VSS_LDAP_INIT_ERROR;

			m_params_changed.set(false);
		}

		int result = VSS_LDAP_ERROR;

		//////////////////////////////////////////////////////////////////////////////
		// start auto-detect

		if (m_ldap_server.empty() || m_ldap_autodetect)
		{
			m_ldap_autodetect = true;
#ifdef _WIN32  // get my domains on linux
			if (m_domain.empty())
			{
				char buff[1024] = { 0 };
				DWORD len = sizeof(buff);
				dprint1("\t\tSearching for default domain:\n");
				long result = DnsQueryConfig(DnsConfigPrimaryDomainName_UTF8, false, 0, 0, buff, &len);
				if (!result)
				{
					m_domain = buff;
					dstream1 << m_domain << "\n";
				}
				else
					dprint1("not found\n");

			}
			else
				dstream4 << "\t\tdomain " << m_domain << "\n";
#endif

			result = ConnectBySRV(m_domain);
			if (result != 0)	// any error with DNS SRV
				result = ConnectByHost(m_domain);
			if (result != 0)
				result = ConnectByIp(m_domain);
		}
		else
		{
			// no autodetect
			LDAP* new_ctx = nullptr;
			result = ConnectServer(new_ctx, m_ldap_server.c_str(), m_ldap_port);
			if (new_ctx)
				m_ldap = std::make_shared<tc::LDAPCore::CtxUnbinder>(new_ctx);
		}

		// find Base DN
		if (result == 0)
		{
			decltype(m_basedn) basedn;
			std::set<std::string> supportedControl;
			GetServerRootInfo(m_ldap.load()->ctx(), basedn, supportedControl);
			if (m_basedn.empty())
			{
				if (basedn.empty())
				{
					dprint1("\t\tbase dn not found!\n");
					return VSS_LDAP_INIT_ERROR;
				}
				m_basedn = basedn;

				dstream1 << "\t\tBuilt base dn:" << m_basedn << "\n";

			}
			if (m_sort_control_scheme == SortControlScheme::NO)
				m_is_server_sort_control_supported = false;
			else if (m_sort_control_scheme == SortControlScheme::YES)
				m_is_server_sort_control_supported = true;
			else if (m_sort_control_scheme == SortControlScheme::AUTO)
				m_is_server_sort_control_supported = supportedControl.find(LDAP_OID_SERVER_SORT_CONTROL) != supportedControl.end();

			// reconnect old referrals
			auto r = std::move(*m_ldap_refferal.lock());
			for (auto const& i : r)
				ldap_unbind_ext(i.second, nullptr, nullptr);

			std::vector<tc::attrs_t> out;
			std::vector<tc::attr_name_t> custom_attrs = {
				"name"
			};
			Search("(objectClass=domain)", out, &custom_attrs, nullptr, LDAP_SCOPE_BASE);
			if (out.size() == 1)
			{
				for (const auto& a : out[0])
				{
					if (a.first == "name")
						m_our_flatName = a.second;
				}
			}
		}
		else {
			m_ldap.store(std::make_shared<tc::LDAPCore::CtxUnbinder>(nullptr));
		}

		return result;
	}

	ldap_error_code_t LDAPCore::LDAPSearch(LDAP* ld, const std::string& dn, const long& scope, const std::string& filter, const char** attrs, std::vector<attrs_t>& out, page_cookie_t& cookie, const long page_size, const std::pair<std::string, bool>* sort_attr, const bool changed_ctx)
	{
		// todo(kt): also param "Use Paging" (not only protocol version 3)
		if (m_ldap_version == LDAP_VERSION3) {
			return LDAPSearchPagedImp(ld, dn, scope, filter, attrs, out, cookie, page_size, sort_attr, changed_ctx);
		}
		//else {
		//	return LDAPSearchImp(ld, dn, scope, filter, attrs, out);
		//}
		return LDAP_SUCCESS;
	}

//	tc::ldap_error_code_t LDAPCore::LDAPSearchImp(LDAP* ld, const std::string& dn, const long& scope, const std::string& filter, const char** attrs, std::vector<attrs_t>& out)
//	{
//		LDAPMessage* lmsg = 0;
//		ldap_error_code_t err(LDAP_SUCCESS);
//#ifdef _WIN32
//		try
//		{
//			l_timeval t;
//			t.tv_sec = m_ldap_timeout.count();
//			t.tv_usec = 0;
//			unsigned long search, lresult;
//			if (ldap_search_ext(ld, (PWCHAR)dn.c_str(), scope, (PWCHAR)filter.c_str(),
//				(wchar_t**)attrs, false, 0, 0, t.tv_sec, 0, &search) != LDAP_SUCCESS)
//				throw VSS_LDAP_ERROR;
//
//			lresult = ldap_result(m_ldap, search, LDAP_MSG_ALL, &t, &lmsg);
//			if (lresult == -1)
//				throw VSS_LDAP_ERROR;
//
//			if (lresult == 0)
//			{
//				dprint1("LDAP timeout\n");
//				err = LDAP_TIMEOUT;
//				ldap_abandon(m_ldap, search);
//				throw VSS_LDAP_ERROR;
//			};
//
//			for (LDAPMessage* iter = ldap_first_entry(m_ldap, lmsg); iter; iter = ldap_next_entry(m_ldap, iter))
//			{
//				attrs_t v;
//				if (attrs)
//				{
//					for (int i = 0; attrs[i]; ++i)	// attrs
//					{
//						wchar_t** vals = ldap_get_values(ld, iter, (const PWCHAR)attrs[i]);
//						if (vals)
//						{
//							for (wchar_t** val = vals; *val; ++val)
//							{
////								v.push_back(std::make_pair(attrs[i], *val));
//							}
//							ldap_value_free(vals);
//						}
//					}
//				}
//				else {		// empty requested attrs list - fetch all responce attrs
//					BerElement *ber;
//					for (wchar_t* attribute = ldap_first_attribute(ld, iter, &ber);
//						attribute != NULL;
//						attribute = ldap_next_attribute(ld, iter, ber))
//					{
//						struct berval** ldap_value = ldap_get_values_len(ld, iter, attribute);
//						int num_values = ldap_count_values_len(ldap_value);
//
//						//for (int i = 0; i < num_values; ++i) {
//						//	if (ldap_value[i]->bv_val)
//						//		v.push_back(std::make_pair(attribute, vs::UTF8ToWideCharConvert(ldap_value[i]->bv_val)));
//						//}
//						ldap_value_free_len(ldap_value);
//						ldap_memfree(attribute);
//					}
//					if (ber != NULL) {
//						ber_free(ber, 0);
//					}
//				}
//				if (!v.empty())
//				{
//					out.push_back(v);
//				}
//			}
//		}
//		catch (int /*error*/)
//		{
//			//		error_code = error;
//			//		if (error == VSS_LDAP_ERROR)
//			//		{
//			//			if (err == LDAP_SUCCESS)
//			//				err = LdapGetLastError();
//			//			ProcessLDAPError();
//			//		}
//		};
//
//		if (lmsg)
//			ldap_msgfree(lmsg);
//#endif
//		return err;
//	}

	ldap_error_code_t LDAPCore::SearchForUser(const char* filter, std::vector<ldap_user_info>& found_users, const std::vector<tc::attr_name_t>* custom_attrs, const std::pair<std::string, bool>* sort_attr)
	{
		return SearchForUser(filter, found_users, vs::ignore<page_cookie_t>(), 0, custom_attrs, sort_attr);
	}

	ldap_error_code_t LDAPCore::SearchForUser(const char* filter, std::vector<ldap_user_info>& found_users, page_cookie_t& cookie, const long page_size, const std::vector<tc::attr_name_t>* custom_attrs, const std::pair<std::string, bool>* sort_attr)
	{
		if (!filter || !*filter)
			return LDAP_SUCCESS;

		std::vector<tc::attr_name_t> requestAttribs;
		std::set<tc::attr_name_t> requestAttribsSet;
		for (const auto &a : m_custom_attrs_user_info)
		{
			requestAttribsSet.insert(a);
		}
		if (custom_attrs) {
			for (const auto &a : *custom_attrs)
			{
				requestAttribsSet.insert(a);
			}
		}
		for (const auto &ra : requestAttribsSet)
		{
			requestAttribs.push_back(ra);
		}

		std::vector<attrs_t> out;
		tc::ldap_error_code_t err = Search(filter, out, cookie, page_size, &requestAttribs, sort_attr);

		if (err != LDAP_SUCCESS) {
			return err;
		}

		FetchUsersFromResults(out, found_users, &requestAttribs);
		out.clear();

		for (const auto& u : found_users)
		{
			auto ptr = boost::make_shared<VS_StorageUserData>();
			if (FetchUser(u, *ptr) && !!ptr->m_realLogin)
			{
				VS_AutoLock lock(&m_cache_user_info_lock);
				m_cache_user_info[(const char*)ptr->m_realLogin] = ptr;
			}
		}

		return err;
	}


	ldap_error_code_t LDAPCore::Search(const char* filter, std::vector<attrs_t>& out, const std::vector<tc::attr_name_t>* custom_attrs, const std::pair<std::string, bool>* sort_attr, int8_t scope)
	{
		return Search(filter, out, vs::ignore<page_cookie_t>(), 0, custom_attrs, sort_attr, scope);
	}

	ldap_error_code_t LDAPCore::Search(const char* filter, std::vector<attrs_t>& out, page_cookie_t& cookie, const long page_size, const std::vector<tc::attr_name_t>* custom_attrs, const std::pair<std::string, bool>* sort_attr, int8_t scope)
	{
		if (!filter || !*filter)
			return LDAP_SUCCESS;

		std::vector<const char*> attrs_arr;

		if (custom_attrs) {
			for (const auto& x : *custom_attrs) {
				attrs_arr.push_back(x.c_str());
			}
		}
		attrs_arr.push_back(0);

		tc::ldap_error_code_t err(LDAP_SUCCESS);
		err = LDAPSearch(m_ldap.load()->ctx(), m_basedn, scope, filter, (const char**)attrs_arr.data(), out, cookie, page_size, sort_attr);
		if (err != LDAP_SUCCESS) {
			if (m_ldap_error_handler_functor)
				m_ldap_error_handler_functor(err);
		}

		return err;
	}


	void LDAPCore::FetchUsersFromResults(const std::vector<attrs_t>& results, std::vector<ldap_user_info>& users, const std::vector<tc::attr_name_t>* custom_attrs)
	{
		auto our_domain = GetOurDomain();
		std::map<std::string, ldap_user_info> trust_objsSIDs_map;
		for (const auto& res : results)
		{
			ldap_user_info u;
			std::string firstname;
			std::string lastname;
			std::string trueconf_referral_host;
			for (const auto& result : res)
			{
				if (result.first == m_a_login) {
					u.login = result.second;
				}
				else if (result.first == m_a_distinguishedName) {
					u.dn = result.second;
				}
				else if (result.first == m_a_displayname) {
					u.displayName = result.second;
				}
				else if (result.first == "trueconf_referral_host" &&
					trueconf_referral_host.empty()) {		// take only first value (it would be last referral)
					trueconf_referral_host = result.second;
				}
				else {
					u.custom_attrs.insert(result);

					if (result.first == m_a_firstname) {
						firstname = result.second;
					}
					else if (result.first == m_a_lastname) {
						lastname = result.second;
					}
				}
			}
			if (!u.login.empty()) {
				if (u.displayName.empty()) {
					if (!lastname.empty() || !firstname.empty()) {
						u.displayName = lastname + ' ' + firstname;
						boost::trim(u.displayName);
					}
					else
						u.displayName = u.login;
				}
				if (!trueconf_referral_host.empty())
					u.login = trueconf_referral_host + "\\" + u.login;
				else{
					auto d = GetDomainFromDistinguishedName(u.dn);
					if (our_domain != d)
						u.login = d + "\\" + u.login;
				}
				std::transform(u.login.begin(), u.login.end(), u.login.begin(), ::towlower);
				users.push_back(u);
			}
			else {
				// extract SID from distinguishedName
				auto pos1 = u.dn.find_first_of("=");
				auto pos2 = u.dn.find_first_of(",");
				if (pos1 != std::string::npos &&
					pos2 != std::string::npos &&
					++pos1 < pos2)
				{
					auto sid = u.dn.substr(pos1, pos2 - pos1);
					if (IsObjectSID(sid))
						trust_objsSIDs_map[sid] = u;
				}
			}
		}
		std::vector<ldap_user_info> trust_users;
		std::set<std::string> trust_objsSIDs_set;
		for (const auto& p : trust_objsSIDs_map)
			trust_objsSIDs_set.emplace(p.first);
		FetchForeignUsersBySID(trust_objsSIDs_set, trust_users);
		for (auto&& p : trust_users)
		{
			auto it = p.custom_attrs.find(m_a_objectSid);
			if (it != p.custom_attrs.end())
			{
				auto it2 = trust_objsSIDs_map.find(it->second);
				if (it2 != trust_objsSIDs_map.end())
					p.custom_attrs.insert(it2->second.custom_attrs.begin(), it2->second.custom_attrs.end());
			}
		}
		users.insert(users.end(), trust_users.cbegin(), trust_users.cend());
	}


	void LDAPCore::FetchForeignUsersBySID(const std::set<std::string>& objSIDs, std::vector<ldap_user_info>& u_foreign)
	{
		if (objSIDs.empty())
			return;
		std::string filter;
		filter.reserve(objSIDs.size() * 64);
		filter += "(|";
		for (const auto& objSID : objSIDs)
		{
			filter += '(';
			filter += m_a_objectSid;
			filter += '=';
			filter += objSID;
			filter += ')';
		}
		filter += ')';
		FetchForeignUserByFilter(filter.c_str(), u_foreign);
	}

	void LDAPCore::FetchForeignUserByLogin(const std::string& login_, std::vector<ldap_user_info>& u_foreign)
	{

		std::string user = login_;
		std::string desired_trust;

		auto login = tc::LDAPCore::PreprocessCallID(login_);

		std::vector<std::string> v;
		boost::split(v, login, boost::is_any_of("@"));
		if (v.size() == 2)		// domain + user
		{
			user = v[0];
			desired_trust = v[1];
		}
		char buff[4096] = { 0 };
		snprintf(buff, 4096, "(%s=%s)", m_a_login.c_str(), tc::LDAPCore::EscapeForLDAPFilter(user).c_str());		// not Disabled users
		FetchForeignUserByFilter(buff, u_foreign, desired_trust.c_str());
	}

	void LDAPCore::FetchForeignUserByFilter(const char* filter, std::vector<ldap_user_info>& u_foreign, const char* desired_trust)
	{
		auto ds = dstream4;
		ds << "FetchForeign: " << m_trust_enabled << ", filter=" << filter;
		if (!m_trust_enabled)
			return;
		if (!filter || !*filter)
			return;

		decltype(m_trustedDomains) trustedDomains;
		{
			VS_AutoLock lock(&m_trustedDomains);
			if (desired_trust && *desired_trust) {
				for (const auto& i : m_trustedDomains)
				{
					if (!strcasecmp(i.first.c_str(), desired_trust) ||								// trust2.tst
						(i.second && !strcasecmp(i.second->flatName.c_str(), desired_trust)))		// TRUST2
						trustedDomains.insert(i);
				}
			}
			if (trustedDomains.empty())
				trustedDomains.insert(m_trustedDomains.begin(), m_trustedDomains.end());
		}

		std::vector<wchar_t*> attrs;
		for (const auto& x : m_custom_attrs_user_info) {
			attrs.push_back((wchar_t*)x.c_str());
		}
		attrs.push_back(0);

		auto our_domain = GetOurDomain();
		vs::Synchronized<std::vector<ldap_user_info>> trust_users;
		vs::latch finished(trustedDomains.size());
		for (const auto& d : trustedDomains)
		{
			m_trust_threads->Post([&d, this, &filter, &attrs, &trust_users, &finished, our_domain]() {
				VS_SCOPE_EXIT{ finished.count_down(1); };
				if (!d.second)
					return;

				char buff[1024] = { 0 };
				snprintf(buff, (sizeof(buff) / sizeof(char)) - 1, m_filter_callid.c_str(), "*");
				std::string filter_users = "(&";
				filter_users += filter;
				filter_users += buff;
				filter_users += ")";

				std::vector<attrs_t> out;
				for (const auto& baseDN : d.second->m_baseDN)
				{
					// todo(kt): use normal cookie, page_size, sort_attr
					page_cookie_t fake_cookie;
					auto err = LDAPSearch(d.second->m_ldap, baseDN, LDAP_SCOPE_SUBTREE, filter_users, (const char**)attrs.data(), out, fake_cookie);
					if (err != LDAP_SUCCESS) {
						if (m_ldap_error_handler_functor)
							m_ldap_error_handler_functor(err);
						return;
					}
				}

				for (const auto& results : out)
				{
					ldap_user_info u;
					std::string trueconf_referral_host;
					for (const auto& result : results)
					{
						if (result.first == m_a_login) {
							u.login = result.second;
						}
						else if (result.first == m_a_distinguishedName) {
							u.dn = result.second;
						}
						else if (result.first == m_a_displayname) {
							u.displayName = result.second;
						}
						else if (result.first == "trueconf_referral_host" &&
							trueconf_referral_host.empty()) {		// take only first value (it would be last referral)
							trueconf_referral_host = result.second;
						}
						else /*if (custom_attrs)*/ {
							/*auto x = std::find(custom_attrs->begin(), custom_attrs->end(), result.first);
							if (x != custom_attrs->end())*/
							u.custom_attrs.insert(result);
						}
					}
					if (!u.login.empty())
					{
						if (!trueconf_referral_host.empty())
							u.login = trueconf_referral_host + '\\' + u.login;		// sub1.trust1.loc\user (FQDN)
						else if (!d.first.empty()) {								// trust2.tst\user
							string_view domain(d.first);
							if (!domain.empty() && (our_domain != domain))
								u.login = std::string(domain) + '\\' + u.login;
						}
						u.custom_attrs.emplace(m_a_flatName, d.second->flatName);
						std::transform(u.login.begin(), u.login.end(), u.login.begin(), ::towlower);
						trust_users->push_back(u);
					}
				}
			});
		}
		finished.wait();
		u_foreign = std::move(*trust_users.lock());

		ds << ". Found " << u_foreign.size() << " trust users: ";
		for (const auto& u : u_foreign)
			ds << u.login << ",";

		// remove dups
		std::set<std::string> seen_values;
		u_foreign.erase(std::remove_if(u_foreign.begin(), u_foreign.end(), [&seen_values](const ldap_user_info& i) {
			return !seen_values.insert(i.login).second;
		}), u_foreign.end());

		// choose only from desired_trust
		if (desired_trust && *desired_trust)
		{
			u_foreign.erase(std::remove_if(u_foreign.begin(), u_foreign.end(), [&desired_trust, this](const ldap_user_info& i) {
				for (const auto& a : i.custom_attrs)
					if (a.first == m_a_flatName)
						if (boost::iequals(a.second, desired_trust))
							return false;
				string_view sv(i.login);
				auto pos = sv.find_first_of("\\");
				if (pos == string_view::npos)
					return true;		// delete users without slash (without domain)
				sv.remove_suffix(sv.length() - pos);
				return !boost::iequals(sv, desired_trust);
			}), u_foreign.end());
		}
		ds << "; chosen " << ((u_foreign.size() == 1) ? u_foreign[0].login : std::string("<nobody>")) << " by desired_domain=" << desired_trust;
	}

	std::string LDAPCore::GetOurDomain() const
	{
		if (!m_our_domain.empty())
			return GetDomainFromDistinguishedName(m_our_domain);
		else
			return GetDomainFromDistinguishedName(m_basedn);
	}

	bool LDAPCore::IsObjectSID(const std::string& str) const
	{
		// todo(kt): re-write with more proper check (maybe special winapi)
		return boost::istarts_with(str, string_view("S-"));
	}

	std::string LDAPCore::PreprocessCallID(string_view call_id) const
	{
		std::string str = vs::UTF8ToLower(call_id);;
		if (str.empty())
			return str;
		VS_RealUserLogin r(str);
		if (r.IsOurSID())
			str = r.GetUser();

		// convert domain\user to user@domain
		{
			string_view sv(str);
			auto pos = sv.find_first_of('\\');
			if (pos != string_view::npos)
			{
				auto domain = sv.substr(0, pos);
				auto user = sv.substr(++pos);

				std::string new_login = (std::string)user;

				pos = user.find_first_of('@');
				if (pos == string_view::npos)
				{
					new_login += '@';
					new_login += (std::string)domain;
				}

				str = std::move(new_login);
			}
		}
		return str;
	}

	std::string LDAPCore::EscapeForLDAPFilter(string_view sv)
	{
		std::string out_str;
		auto int_to_hexstr = [](int code)->std::string
		{
			std::stringstream ss;
			ss << std::hex << (int)(code & 0xff);
			if (ss.str().length() == 1)
			{
				std::string str = "0";
				str.append(ss.str());
				return str;
			}
			else
				return ss.str();
		};
		for (int i = 0; i < sv.length(); i++)
		{
			if (sv[i] == '\\') out_str.append("\\5c");
			else if (sv[i] == '*') out_str.append("\\2a");
			else if (sv[i] == '(') out_str.append("\\28");
			else if (sv[i] == ')') out_str.append("\\29");
			else if (sv[i] == '!') out_str.append("\\21");
			else if (sv[i] == '&') out_str.append("\\26");
			else if (sv[i] == ':') out_str.append("\\3a");
			else if (sv[i] == '|') out_str.append("\\7c");
			else if (sv[i] == '~') out_str.append("\\7e");
			else if (sv[i] == (int)0) out_str.append("\\00");
			else if ((sv[i] & 0xff) > 127) out_str.append("\\").append(int_to_hexstr(sv[i] & 0xff));
			else out_str.push_back(sv[i]);
		}

		return out_str;
	}

	void LDAPCore::Disconnect()
	{
		m_ldap.store(std::make_shared<tc::LDAPCore::CtxUnbinder>(nullptr));
		auto lock = m_ldap_refferal.lock();
		for (auto const& p: *lock)
		{
			ldap_unbind_ext(p.second, nullptr, nullptr);
		}
	}

	void LDAPCore::InitUpdateNestedCacheThread()
	{
		dprint4("LDAPCore::InitUpdateNestedCacheThread()\n");
		m_nested_thread = std::thread(&LDAPCore::UpdateNestedCacheThread, this);
	}

	inline void LDAPCore::DeInitUpdateNestedCacheThread()
	{
		dprint4("LDAPCore::DeInitUpdateNestedCacheThread()\n");
		if (m_nested_thread.joinable())
		{
			{
				std::unique_lock<std::mutex> lock(m_nested_thread_mutex);
				m_nested_is_stopped = true;
			}
			m_nested_cv.notify_one();
			m_nested_thread.join();
		}
	}

	void LDAPCore::StartUpdateNestedCache()
	{
		dprint4("LDAPCore::StartUpdateNestedCache()\n");
		std::unique_lock<std::mutex> lock(m_nested_thread_mutex);
		if (!m_nested_is_doing_update)
		{
			m_nested_do_update = true;
			m_nested_cv.notify_one();
		}
	}

	void LDAPCore::UpdateNestedCacheThread()
	{
		vs::SetThreadName("LDAPUpdateCache");

		dprint4("LDAPCore::UpdateNestedCacheThread() start\n");
		while (!m_nested_is_stopped)
		{
			dprint4("LDAPCore::UpdateNestedCacheThread() wait: is_stop=%d, do_update=%d\n", m_nested_is_stopped, m_nested_do_update);
			std::unique_lock<std::mutex> lock(m_nested_thread_mutex);
			if (m_ab_cache_timeout.count()>0)
				m_nested_cv.wait_for(lock, std::chrono::seconds(10), [this]() { return m_nested_do_update || m_nested_is_stopped;  });
			else
				m_nested_cv.wait(lock, [this]() { return m_nested_do_update || m_nested_is_stopped;  });
			m_nested_do_update = false;
			lock.unlock();

			dprint4("LDAPCore::UpdateNestedCacheThread() trigger: is_stop=%d\n", m_nested_is_stopped);
			if (!m_nested_is_stopped)
			{
				//std::set<tc::group_dn> ng_param;
				//{
				//	VS_AutoLock lock(&m_nested_groups_lock);
				//	ng_param = m_nested_reg_groups_param;
				//}
				//UpdateNestedGroupsCacheImp(ng_param);

				lock.lock();
				m_nested_is_doing_update = true;
				lock.unlock();
				if (std::chrono::system_clock::now() - GetLastUpdateNestedCacheTime() > m_ab_cache_timeout)
				{
					UpdateAllCacheImp();
				}
				lock.lock();
				m_nested_is_doing_update = false;
				lock.unlock();
			}
		}
		dprint4("LDAPCore::UpdateNestedCacheThread() finished\n");
	}

	void LDAPCore::UpdateAllCacheImp()
	{
		if (!m_group_manager) return;

		UpdateLoginGroupCache();
		if (m_nested_is_stopped)
		{
			ClearCaches();
			return;
		}

		if (!m_trustedDomains_updated)
		{
			InitTrustedDomains();
			m_trustedDomains_updated = true;
		}

		{	// fetch Base DN of trusted domains
			decltype(m_trustedDomains) trustedDomains_needUpdate;
			{
				VS_AutoLock lock(&m_trustedDomains);
				for (const auto& i : m_trustedDomains)
					if (!i.second || !i.second->m_ldap || i.second->m_baseDN.empty())
						trustedDomains_needUpdate.insert(i);
			}

			for (auto& current : trustedDomains_needUpdate)
			{
				std::shared_ptr<TrustedDomain> new_info;
				if (!current.second->m_ldap)
				{
					LDAP* new_ctx(0);
					if (ConnectServer(new_ctx, current.first.c_str(), m_ldap_port) == 0)		// todo(kt): which port to use? from my server?
					{
						if (!new_info)
							new_info = std::make_shared<TrustedDomain>();
						*new_info = *(current.second);		// copy all info (flatName, baseDN), except context
						new_info->m_ldap = new_ctx;
					}
				}
				if (m_nested_is_stopped)
				{
					ClearCaches();
					return;
				}
				if (new_info && new_info->m_ldap)
				{
					std::string base_dn;
					GetServerRootInfo(new_info->m_ldap, base_dn, vs::ignore<std::set<std::string>>());
					if (!base_dn.empty())
						new_info->m_baseDN.insert(base_dn);
				}
				if (new_info)
				{
					VS_AutoLock lock(&m_trustedDomains);
					m_trustedDomains[current.first] = new_info;
				}
				if (m_nested_is_stopped)
				{
					ClearCaches();
					return;
				}
			}
		}

		std::map<std::string, VS_RegGroupInfo> reg_groups;
		GetRegGroups(reg_groups);
		bool HasScopeAllUsers(false);
		std::set<tc::group_dn> reg_groups_to_update;
		for (const auto& i : reg_groups)
		{
			if (i.second.scope == e_ab_scope_all_users)
				HasScopeAllUsers = true;
			if (!i.second.ldap_dn.empty())
				reg_groups_to_update.insert(i.second.ldap_dn);
		}

		if (!m_login_group.get().empty())					// add login group
			reg_groups_to_update.insert(m_login_group.get());



		//std::set<tc::group_dn> ng_param;
		//{
		//	VS_AutoLock lock(&m_nested_groups_lock);
		//	ng_param = m_nested_reg_groups_param;
		//}
		//UpdateNestedGroupsCacheImp(ng_param);

		//auto tick1 = tc::LDAPCore::GetLastUpdateNestedCacheTime();
		//tc::LDAPCore::StartUpdateNestedCache();
		//decltype(tick1) tick2;
		//auto start = std::chrono::system_clock::now();
		//do {
		//	tick2 = tc::LDAPCore::GetLastUpdateNestedCacheTime();
		//	vs::SleepFor(std::chrono::milliseconds(10));
		//} while ((tick2 == tick1) && !(std::chrono::system_clock::now() - start > std::chrono::minutes(5)));

		//	VS_LDAPCore::ClearAllCache(reg_groups);

		if (HasScopeAllUsers)
		{
			std::vector<tc::ldap_user_info> v;
			if (m_login_group.get().empty() && GetAllUsers(v, &m_custom_attrs_user_info) == LDAP_SUCCESS)
			{
				auto v2 = std::make_shared<VS_AbCommonMap>();
				for (const auto& u : v)
				{
					if (m_use_avatars && m_avatarsHandler)
					{
						m_avatarsHandler(u.login, u.custom_attrs);
					}

					VS_RealUserLogin r(vs::UTF8ToLower(u.login));
					if (!r)
						continue;
					(*v2)[(const char*)r] = VS_AbCommonMap_Item(u.displayName);

					auto ude = boost::make_shared<VS_StorageUserData>();
					if (FetchUser(u, *ude) && !!ude->m_realLogin)
					{
						VS_AutoLock lock(&m_cache_user_info_lock);
						m_cache_user_info[(const char*)ude->m_realLogin] = ude;
					}
				}
				m_cache_all_users.store(std::move(v2));
			}
			if (m_nested_is_stopped)
			{
				ClearCaches();
				return;
			}
		}

		auto m = std::make_shared<std::map<std::string, std::shared_ptr<VS_AbCommonMap>>>(); // [key=g_dn,value=map<users>]
		for (const auto& g : reg_groups_to_update)
		{
			auto group_users = std::make_shared<VS_AbCommonMap>();
			std::vector<tc::ldap_user_info> v;
			if (GetGroupUsers(g.c_str(), v, &m_custom_attrs_user_info) == LDAP_SUCCESS)
			{
				for (const auto& u : v)
				{
					if(m_use_avatars && m_avatarsHandler)
					{
						m_avatarsHandler(u.login, u.custom_attrs);
					}

					VS_RealUserLogin r(vs::UTF8ToLower(u.login));
					if (!r)
						continue;
					(*group_users)[(const char*)r] = u.displayName;

					auto ude = boost::make_shared<VS_StorageUserData>();
					if (FetchUser(u, *ude) && !!ude->m_realLogin)
					{
						VS_AutoLock lock(&m_cache_user_info_lock);
						m_cache_user_info[(const char*)ude->m_realLogin] = ude;
					}
				}
			}
			(*m)[g] = group_users;

			if (m_nested_is_stopped)
			{
				ClearCaches();
				return;
			}
		}
		m_cache_groups_users.store(std::move(m));

		{
			VS_AutoLock lock(&m_nested_groups_lock);
			m_last_nested_cache_update = std::chrono::system_clock::now();
		}

		{
			VS_AutoLock lock(&m_page_map_lock);
			for (auto it = m_page_map.begin(); it != m_page_map.end(); )
			{
				if (it->first.IsExpired())
					it = m_page_map.erase(it);
				else
					++it;
			}
		}
	}

	void LDAPCore::ClearCaches() {
		if (!m_nested_is_stopped || !m_group_manager) return;

		{
			VS_AutoLock lock(&m_trustedDomains);
			m_trustedDomains.clear();
			m_trustedDomains_updated = false;
		}
		{
			std::lock_guard<std::mutex> lock(m_group_manager->m_reg_groups_lock);
			m_group_manager->m_reg_groups.clear();
			m_group_manager->m_last_write_time.clear();
		}
		{
			VS_AutoLock lock(&m_cache_user_info_lock);
			m_cache_user_info.clear();
		}

		m_cache_all_users.store(nullptr); // set empty users map
		m_cache_groups_users.store(nullptr); //set empty groups map
	}

	void LDAPCore::ExpandGroup(const std::string& login_group, std::map<std::string, std::string>& result)
	{
		if (m_a_memberOf.empty())
			ExpandGroup_noMemberOf(login_group, result);
		else
			ExpandGroup_hasMemberOf(login_group, result);
	}

	void LDAPCore::ExpandGroup_noMemberOf(const std::string& login_group, std::map<std::string, std::string>& result)
	{
		std::vector<attrs_t> out;
		const char* attrs[3] = {
			m_a_groupmember.c_str(),
			m_a_primaryGroupToken.c_str(),
			0
		};
		page_cookie_t fake_cookie;
		tc::ldap_error_code_t err(LDAP_SUCCESS);
		err = LDAPSearch(m_ldap.load()->ctx(), login_group, LDAP_SCOPE_SUBTREE, m_filter_group, (const char**)attrs, out, fake_cookie);
		if (err != LDAP_SUCCESS) {
			if (m_ldap_error_handler_functor)
				m_ldap_error_handler_functor(err);
			return;
		}

		std::string pgt;
		std::set<std::string> try_recursive;
		for (const auto& users : out)
		{
			for (const auto& a : users)
			{
				if (a.first == m_a_groupmember)
				{
					if (result.find(a.second) == result.end())
						try_recursive.insert(a.second);
				}
				else if (a.first == m_a_primaryGroupToken) {
					pgt = a.second;
				}
			}
		}

		if (!out.empty())		// this is a group, not user
			result[login_group] = pgt;

		for (const auto& r : try_recursive)
			ExpandGroup_noMemberOf(r, result);
	}

	void LDAPCore::ExpandGroup_hasMemberOf(const std::string& login_group, std::map<std::string, std::string>& result)
	{
		std::vector<attrs_t> out;
		tc::ldap_error_code_t err(LDAP_SUCCESS);

		page_cookie_t fake_cookie;
		const char* attrs[2] = {
			m_a_distinguishedName.c_str(),
			0
		};
		std::string filter = "(&";
		filter += m_filter_group;
		filter += "(";
		filter += m_a_memberOf;
		filter += "=";
		filter += login_group;
		filter += ")";
		filter += ")";

		err = LDAPSearch(m_ldap.load()->ctx(), m_basedn, LDAP_SCOPE_SUBTREE, filter, attrs, out, fake_cookie);

		if (err != LDAP_SUCCESS) {
			if (m_ldap_error_handler_functor)
				m_ldap_error_handler_functor(err);
			return;
		}
		std::set<std::string> try_recursive;
		std::string pgt;
		// find a primary group token
		if (!m_a_primaryGroupToken.empty())
		{
			std::vector<attrs_t> login_group_out;
			{
				page_cookie_t fake_cookie;
				const char* attrs[3] = {
					m_a_primaryGroupToken.c_str(),
					0
				};
				if (LDAPSearch(m_ldap.load()->ctx(), login_group, LDAP_SCOPE_SUBTREE, m_filter_group, (const char**)attrs, login_group_out, fake_cookie) != LDAP_SUCCESS) {
					if (m_ldap_error_handler_functor)
					{
						m_ldap_error_handler_functor(err);
					}
					return;
				}
			}
			for (const auto &attrs : login_group_out)
			{
				std::string login_group_dn;
				std::string login_group_pgt;
				for (const auto &a : attrs)
				{
					if (a.first == m_a_distinguishedName)
					{
						login_group_dn = a.second;
					}
					else if (a.first == m_a_primaryGroupToken) {
						login_group_pgt = a.second;
					}
				}

				if (boost::iequals(login_group_dn, login_group))
				{
					pgt = login_group_pgt;
				}
			}
		}
		result[login_group] = pgt;

		// find and check sub-groups
		for (const auto& attrs : out)
		{
			for (const auto& a : attrs)
			{
				if (a.first == m_a_distinguishedName)
				{
					if (result.find(a.second) == result.end())
					{
						try_recursive.insert(a.second);
					}
				}
			}
		}

		for (const auto& r : try_recursive)
			ExpandGroup_hasMemberOf(r, result);
	}

	void LDAPCore::UpdateLoginGroupCache(void)
	{
		if (!m_login_group.get().empty() && (std::chrono::system_clock::now() - m_last_login_group_cache_update > m_ab_cache_timeout))
		{
			auto expand = std::make_shared<std::map<std::string, std::string>>();
			ExpandGroup(m_login_group.get(), *expand);
			if (expand->empty())
			{
				dprint0("Cannnot expand login group\n");
			}
			else
			{
				m_login_group_expand.store(std::move(expand));
			}
			m_last_login_group_cache_update = std::chrono::system_clock::now();
		}
	}

	void LDAPCore::InitTrustedDomains()
	{
		auto ds = dstream4;
		ds << "InitTrustedDomains enabled=" << m_trust_enabled;
		if (!m_trust_enabled)
			return;

		std::vector<char*> attrs = {
			(char*)m_a_trustPartner.c_str(),
			(char*)m_a_flatName.c_str(),
			0
		};
		tc::page_cookie_t fake_cookie;
		std::vector<attrs_t> results;
		auto err = LDAPSearch(m_ldap.load()->ctx(), m_basedn, LDAP_SCOPE_SUBTREE, m_filter_trustedDomain, (const char**)attrs.data(), results, fake_cookie/*, page_size, sort_attr*/);
		if (err != LDAP_SUCCESS) {
			if (m_ldap_error_handler_functor)
				m_ldap_error_handler_functor(err);
			return /*err*/;
		}

		decltype(m_trustedDomains) trustedDomains;

		for (const auto& r : results)
		{
			std::string trustPartner;
			std::string flatName;
			for (const auto& a : r)
			{
				if (a.first == m_a_trustPartner) {
					ds << " trustPartner=" << a.second;
					trustPartner = a.second;
				}
				else if (a.first == m_a_flatName) {
					flatName = a.second;
				}
			}
			if (!trustPartner.empty())
			{
				trustedDomains[trustPartner] = std::make_shared<TrustedDomain>();
				trustedDomains[trustPartner]->flatName = flatName;
			}
		}

		VS_AutoLock lock(&m_trustedDomains);
		m_trustedDomains.clear();
		m_trustedDomains.insert(trustedDomains.begin(), trustedDomains.end());
		ds << " m[";
		for (const auto& d : m_trustedDomains)
		{
			ds << d.first << "~~";
			if (d.second)
				for (const auto& dn : d.second->m_baseDN)
					ds << dn << "##";
			ds << ";";
		}
		ds << "]";
		if (!m_trust_threads && !m_trustedDomains.empty())
		{
			auto n_threads = 40;								// max amount of parallel threads for searching in trusts
			if (m_trustedDomains.size() * 5 < n_threads)		// for 5 parallel users login
				n_threads = m_trustedDomains.size() * 5;
			m_trust_threads = std::make_shared<VS_ThreadPool>(n_threads);
		}
	}

	bool LDAPCore::GetRegGroups(std::map<std::string, VS_RegGroupInfo>& reg_groups) {
		if (m_group_manager) return m_group_manager->GetRegGroups(reg_groups);
		else return false;
	}

	bool LDAPCore::FindUser_Sink(const vs_user_id& user_id, VS_StorageUserData& ude, bool onlyCached)
	{
		if (!user_id || !*user_id || !m_group_manager) return false;

		VS_RealUserLogin r(SimpleStrToStringView(user_id));
		boost::shared_ptr<VS_StorageUserData> sp;
		{
			VS_AutoLock lock(&m_cache_user_info_lock);
			auto it = m_cache_user_info.find((const char*)r);
			if (it != m_cache_user_info.end())
				sp = it->second;
		}

		if (!sp && onlyCached) {
			return false;
		}
		else if (!sp) {
			// search user at LDAP
			char buff[2048] = { 0 };
			snprintf(buff, (sizeof(buff) / sizeof(buff[0])) - 2, m_filter_login.c_str(), r.GetUser().c_str());
			std::vector<ldap_user_info> found_users;
			if ((SearchForUser(buff, found_users) != LDAP_SUCCESS) || found_users.size() != 1)
				return false;
			auto ptr = boost::make_shared<VS_StorageUserData>();
			if (!FetchUser(found_users[0], *ptr))
				return false;
			ude = *ptr;
			VS_AutoLock lock(&m_cache_user_info_lock);
			m_cache_user_info[(const char*)r] = ptr;
		}
		else {
			ude = *sp;
		}

		if (ude.m_groups.empty())	// need to get users groups
		{
			std::map<std::string, VS_RegGroupInfo> reg_groups;
			GetRegGroups(reg_groups);

			auto c = m_cache_groups_users.load();
			if (c)
			{
				for (const auto& rg : reg_groups)
				{
					auto g = c->find(rg.second.ldap_dn);
					if (g != c->end() && g->second)
					{
						if (g->second->find((const char*)ude.m_realLogin) != g->second->end())
							ude.m_groups.push_back(rg.first);
					}
				}
			}
		}
		return true;
	}

	bool LDAPCore::GetAllUsers(std::shared_ptr<VS_AbCommonMap>& users)
	{
		if (!m_group_manager) return false;

		if (!m_login_group.get().empty()) {
			auto all_groups = m_cache_groups_users.load();
			if (all_groups)
			{
				auto g = all_groups->find(m_login_group.get());
				if (g != all_groups->end())
					users = g->second;
			}
		}
		else {
			if (auto users_cached = m_cache_all_users.load())
				users = std::move(users_cached);
		}
		return true;
	}

	bool LDAPCore::IsLDAP_Sink() const
	{
		return true;
	}


	bool LDAPCore::isUselessDN(const std::string &dn) const {
		for (const std::string& s : m_skip_referrals) {
			if (boost::icontains(dn, s)) return true;
		}
		return false;
	}

	ldap_error_code_t tc::LDAPCore::GetAllGroups(std::vector<ldap_group_info>& all_groups, const std::pair<std::string, bool>* sort_attr, const std::string* query)
	{
		page_cookie_t fake_cookie;
		return GetAllGroups(all_groups, fake_cookie, 0, sort_attr, query);
	}

	ldap_error_code_t tc::LDAPCore::GetAllGroups(std::vector<ldap_group_info>& all_groups, page_cookie_t& cookie, const long page_size, const std::pair<std::string, bool>* sort_attr, const std::string* query)
	{
		const char* attrs[4] = {
			m_a_GroupDisplayName.c_str(),
			m_a_primaryGroupToken.c_str(),
			m_a_distinguishedName.c_str(),
			0
		};

		std::string filter = "(&";	// "(&%s%s)"
		filter += m_filter_group;
		if (query)
			filter += *query;
		filter += ")";

		std::vector<attrs_t> out;
		tc::ldap_error_code_t err = LDAPSearch(m_ldap.load()->ctx(), m_basedn, LDAP_SCOPE_SUBTREE, filter, &attrs[0], out, cookie, page_size, sort_attr);
		if (err != LDAP_SUCCESS) {
			if (m_ldap_error_handler_functor)
				m_ldap_error_handler_functor(err);
			return err;
		}

		// prepare result for add to abook map
		for (std::vector<attrs_t>::const_iterator it = out.begin(); it != out.end(); ++it)
		{
			ldap_group_info g;
			for (attrs_t::const_iterator it2 = it->begin(); it2 != it->end(); ++it2)
			{
				if (it2->first == m_a_GroupDisplayName)
					g.displayName = it2->second;
				else if (it2->first == m_a_primaryGroupToken)
					g.primaryGroupToken = ::atol(it2->second.c_str());
				else if (it2->first == m_a_distinguishedName)
					g.dn = it2->second;
			}
			if (!g.displayName.empty() && !g.dn.empty())
			{
				all_groups.push_back(g);
			}
		}

		return err;
	}

	ldap_error_code_t LDAPCore::GetAllUsers(std::vector<ldap_user_info>& all_users, const std::vector<tc::attr_name_t>* custom_attrs, const std::pair<std::string, bool>* sort_attr, const std::string* query)
	{
		page_cookie_t fake_cookie;
		return GetAllUsers(all_users, fake_cookie, 0, custom_attrs, sort_attr, query);
	}

	ldap_error_code_t LDAPCore::GetAllUsers(std::vector<ldap_user_info>& all_users, page_cookie_t& cookie, const long page_size, const std::vector<tc::attr_name_t>* custom_attrs, const std::pair<std::string, bool>* sort_attr, const std::string* query)
	{
		std::string filterByLoginGroup;
		if (!m_login_group.get().empty())
		{
			auto expand = m_login_group_expand.load();
			if (expand && !expand->empty())
			{
				for (const auto& m : *expand)
				{
					filterByLoginGroup += "(" + m_a_memberOf + "=" + m.first + ")";
					filterByLoginGroup += "(" + m_a_primaryGroupId + "=" + m.second + ")";
				}
				if (!filterByLoginGroup.empty())
					filterByLoginGroup = "(|" + filterByLoginGroup + ")";
			}
		}

		int callIdFilterLength = m_filter_callid.length() + 2;
		int filterLength = callIdFilterLength + m_filter_disabled.get().length() + filterByLoginGroup.length() + 5;
		if (query) {
			filterLength += query->length();
		}

		char* callIdFilter = new char[callIdFilterLength];
		char* filter = new char[filterLength];

		snprintf(callIdFilter, callIdFilterLength, m_filter_callid.c_str(), "*");
		snprintf(filter, filterLength, "(&%s%s%s%s)", m_filter_disabled.get().c_str(), callIdFilter, filterByLoginGroup.c_str(), (query) ? query->c_str() : "");		// not Disabled users
		ldap_error_code_t err = SearchForUser(filter, all_users, cookie, page_size, custom_attrs, sort_attr);

		delete[] callIdFilter;
		delete[] filter;

		// remove dups
		std::set<std::string> seen_values;
		all_users.erase(std::remove_if(all_users.begin(), all_users.end(), [&seen_values](const ldap_user_info& i) {
			return !seen_values.insert(i.login).second;
		}), all_users.end());

		return err;
	}


	ldap_error_code_t LDAPCore::GetNoGroupUsers(std::vector<ldap_user_info>& group_users, page_cookie_t& cookie, const long page_size, const std::vector<tc::attr_name_t>* custom_attrs, const std::pair<std::string, bool>* sort_attr, const std::string* query)
	{

		std::string memberOfFilterParts;
		std::string notMemberOfFilterParts;
		std::string notPrimaryGroupTokenFilter;
		std::string loginGroupFilter;

		bool hasLoginGroup = !m_login_group.get().empty();

		std::map<std::string, VS_RegGroupInfo> regGroups;
		GetRegGroups(regGroups);

		for (const auto& rg : regGroups) {
			if (rg.second.ldap_dn.empty()) {
				continue;
			}

			int memberOfPartLength = m_a_memberOf.length() + strlen(LDAP_MATCHING_RULE_IN_CHAIN) + rg.second.ldap_dn.length() + 10;
			char* memberOfFilterPart = new char[memberOfPartLength];
			char* notMemberOfFilterPart = new char[memberOfPartLength + 3];
			snprintf(memberOfFilterPart, memberOfPartLength, "%s:%s:=%s", m_a_memberOf.c_str(), LDAP_MATCHING_RULE_IN_CHAIN, rg.second.ldap_dn.c_str());
			snprintf(notMemberOfFilterPart, memberOfPartLength + 3, "(!%s)", memberOfFilterPart);
			int memberOfFullPartLength = memberOfPartLength + m_a_distinguishedName.length() + rg.second.ldap_dn.length() + 10;
			char* memberOfFullPart = new char[memberOfFullPartLength];
			snprintf(memberOfFullPart, memberOfFullPartLength, "(%s)(%s=%s)", memberOfFilterPart, m_a_distinguishedName.c_str(), rg.second.ldap_dn.c_str());

			memberOfFilterParts += memberOfFullPart;
			notMemberOfFilterParts += notMemberOfFilterPart;

			delete[] memberOfFullPart;
			delete[] memberOfFilterPart;
			delete[] notMemberOfFilterPart;
		}

		if (memberOfFilterParts.empty() && notMemberOfFilterParts.empty())		// no reg groups found (may be except @no_group)
		{
			if (m_login_group.get().empty())
				return GetAllUsers(group_users, cookie, page_size, custom_attrs, sort_attr, query);
			else
				return GetGroupUsers(m_login_group.get().c_str(), group_users, cookie, page_size, custom_attrs, sort_attr, query);
		}


		int nestedGroupsFilterLength = m_filter_group.length() + memberOfFilterParts.length() + 10;
		char* nestedGroupsFilter = new char[nestedGroupsFilterLength];
		snprintf(nestedGroupsFilter, nestedGroupsFilterLength, "(&%s(|%s))", m_filter_group.c_str(), memberOfFilterParts.c_str());
		std::vector<char*> nestedGroupsRequestAttrs{
			(char*)m_a_primaryGroupToken.c_str(),
			0
		};

		auto ptr = m_ldap.load();
		std::vector<attrs_t> nestedGroupsData;
		{
			page_cookie_t fake_cookie;
			ldap_error_code_t err = LDAPSearch(ptr->ctx(), m_basedn, LDAP_SCOPE_SUBTREE, nestedGroupsFilter, (const char**)nestedGroupsRequestAttrs.data(), nestedGroupsData, fake_cookie);
			if (err != LDAP_SUCCESS) {
				if (m_ldap_error_handler_functor)
					m_ldap_error_handler_functor(err);
				delete[] nestedGroupsFilter;
				return err;
			}
		}

		delete[] nestedGroupsFilter;


		for (const auto& nestedGroupData : nestedGroupsData) {
			for (const auto& nestedGroupDataPair : nestedGroupData) {
				if (nestedGroupDataPair.second.empty()) {
					continue;
				}

				if (nestedGroupDataPair.first == m_a_primaryGroupToken) {
					notPrimaryGroupTokenFilter += "(!" + m_a_primaryGroupId + "=" + nestedGroupDataPair.second + ")";
				}
			}
		}

		if (hasLoginGroup) {
			std::vector<attrs_t> loginGroupNestedGroupsData;
			int loginGroupNestedGroupsFilterLength = m_filter_group.length() + m_a_memberOf.length() + strlen(LDAP_MATCHING_RULE_IN_CHAIN) +
				m_login_group.get().length() + m_a_distinguishedName.length() + m_login_group.get().length() + 15;

			char* loginGroupNestedGroupsFilter = new char[loginGroupNestedGroupsFilterLength];
			snprintf(loginGroupNestedGroupsFilter, loginGroupNestedGroupsFilterLength, "(&%s(|(%s:%s:=%s)(%s=%s)))",
				m_filter_group.c_str(), m_a_memberOf.c_str(), LDAP_MATCHING_RULE_IN_CHAIN, m_login_group.get().c_str(),
				m_a_distinguishedName.c_str(), m_login_group.get().c_str());

			page_cookie_t fake_cookie;
			ldap_error_code_t err = LDAPSearch(ptr->ctx(), m_basedn, LDAP_SCOPE_SUBTREE, loginGroupNestedGroupsFilter, (const char**)nestedGroupsRequestAttrs.data(), loginGroupNestedGroupsData, fake_cookie);
			if (err != LDAP_SUCCESS) {
				if (m_ldap_error_handler_functor)
					m_ldap_error_handler_functor(err);
				delete[] loginGroupNestedGroupsFilter;
				return err;
			}

			delete[] loginGroupNestedGroupsFilter;

			std::string loginGroupFilterParts;
			for (const auto& loginGroupNestedGroupData : loginGroupNestedGroupsData) {
				auto primaryGroupTokePair = std::find_if(loginGroupNestedGroupData.begin(), loginGroupNestedGroupData.end(),
					[this](const std::pair<attr_name_t, attr_value_t>& p) {
					return p.first == m_a_primaryGroupToken && !p.second.empty();
				});

				if (primaryGroupTokePair == loginGroupNestedGroupData.end()) {
					continue;
				}

				loginGroupFilterParts += "(" + m_a_primaryGroupId + "=" + primaryGroupTokePair->second + ")";
			}

			if (!loginGroupFilterParts.empty()) {
				int loginGroupFilterBuffLength = m_a_memberOf.length() + strlen(LDAP_MATCHING_RULE_IN_CHAIN) +
					m_login_group.get().length() + loginGroupFilterParts.length() + 15;
				char* loginGroupFilterBuff = new char[loginGroupFilterBuffLength];
				snprintf(loginGroupFilterBuff, loginGroupFilterBuffLength, "(|(%s:%s:=%s)%s)",
					m_a_memberOf.c_str(), LDAP_MATCHING_RULE_IN_CHAIN, m_login_group.get().c_str(),
					loginGroupFilterParts.c_str());
				loginGroupFilter = loginGroupFilterBuff;
				delete[] loginGroupFilterBuff;
			}
		}

		int completeFilterLength = strlen(LDAP_FILTER_IS_USER) + m_filter_disabled.get().length() +
			m_filter_foreignSecurityPrincipal.length() + ((query && query->length()) ? query->length() : 0) +
			notMemberOfFilterParts.length() + notPrimaryGroupTokenFilter.length() + loginGroupFilter.length() + 20;
		char* completeFilter = new char[completeFilterLength];
		snprintf(completeFilter, completeFilterLength,
			"(&(&(|(&%s%s)%s)%s)"		// ((is_user AND disabled) OR trust) AND query
			"(&%s%s)"
			"%s)",
			LDAP_FILTER_IS_USER,
			m_filter_disabled.get().c_str(),
			(m_trust_enabled) ? m_filter_foreignSecurityPrincipal.c_str() : "",
			(query && query->length()) ? query->c_str() : "",
			notMemberOfFilterParts.c_str(),
			notPrimaryGroupTokenFilter.c_str(),
			loginGroupFilter.c_str()
		);
		std::vector<char*> attrs_arr2{
			(char*)m_a_login.c_str(),
			(char*)m_a_displayname.c_str(),
			(char*)m_a_distinguishedName.c_str(),
		};
		if (custom_attrs)
			for (const auto& x : *custom_attrs)
				attrs_arr2.push_back((char*)x.c_str());
		attrs_arr2.push_back(0);

		std::vector<attrs_t> out2;
		ldap_error_code_t err = LDAPSearch(ptr->ctx(), m_basedn, LDAP_SCOPE_SUBTREE, completeFilter, (const char**)attrs_arr2.data(), out2, cookie, page_size, sort_attr);
		if (err != LDAP_SUCCESS) {
			if (m_ldap_error_handler_functor)
				m_ldap_error_handler_functor(err);
			delete[] completeFilter;
			return err;
		}

		delete[] completeFilter;

		FetchUsersFromResults(out2, group_users, custom_attrs);
		return err;


	}

	ldap_error_code_t LDAPCore::GetGroupUsers(const char* group_dn, std::vector<ldap_user_info>& group_users, const std::vector<tc::attr_name_t>* custom_attrs, const std::pair<std::string, bool>* sort_attr, const std::string* query)
	{
		page_cookie_t fake_cookie;
		return GetGroupUsers(group_dn, group_users, fake_cookie, 0, custom_attrs, sort_attr, query);
	}


	ldap_error_code_t LDAPCore::GetGroupUsers(const char* group_dn, std::vector<ldap_user_info>& group_users, page_cookie_t& cookie, const long page_size, const std::vector<tc::attr_name_t>* custom_attrs, const std::pair<std::string, bool>* sort_attr, const std::string* query)
	{
		ldap_error_code_t err(LDAP_SUCCESS);
		if (!group_dn || !*group_dn)
			return err;

		std::string q;
		if (query)		// preprocess query: remove known trusted domains, like "sub1.trust1.loc\user"
		{
			q = *query;
			VS_AutoLock lock(&m_trustedDomains);
			for (auto const& t : m_trustedDomains)
				VS_ReplaceAll(q, t.first + "%5C", "");
		}

		auto ptr = m_ldap.load();
		std::set<tc::group_dn> total_memberOf;
		std::vector<tc::group_dn> curr_groups;
		total_memberOf.insert(group_dn);
		curr_groups.emplace_back(group_dn);
		std::string primaryGroupTokens_filter;
		unsigned deep = 0;
		do {
			std::string nestedGroupsFilter;
			nestedGroupsFilter.reserve(curr_groups.size() * 100);
			nestedGroupsFilter += "(&";
			nestedGroupsFilter += m_filter_group;
			{
				if (curr_groups.size() > 1 || deep < 1)
					nestedGroupsFilter += "(|";
				for (const auto& g : curr_groups)
				{
					nestedGroupsFilter += "(";
					nestedGroupsFilter += m_a_memberOf;
					nestedGroupsFilter += "=";
					nestedGroupsFilter += g;
					nestedGroupsFilter += ")";
				}
				if (deep < 1)	// add self group
				{
					nestedGroupsFilter += "(";
					nestedGroupsFilter += m_a_distinguishedName;
					nestedGroupsFilter += "=";
					nestedGroupsFilter += group_dn;
					nestedGroupsFilter += ")";
				}
				if (curr_groups.size() > 1 || deep < 1)
					nestedGroupsFilter += ")";
			}
			nestedGroupsFilter += ")";

			// get primaryGroupToken of nested groups
			std::vector<char*> attrs_arr{
				(char*)m_a_primaryGroupToken.c_str(),
				0
			};
			page_cookie_t fake_cookie;
			std::vector<attrs_t> out;
			err = LDAPSearch(ptr->ctx(), m_basedn, LDAP_SCOPE_SUBTREE, nestedGroupsFilter, (const char**)attrs_arr.data(), out, fake_cookie);
			if (err != LDAP_SUCCESS) {
				if (m_ldap_error_handler_functor)
					m_ldap_error_handler_functor(err);
				return err;
			}

			curr_groups.clear();

			for (const auto& params : out)
			{
				for (const auto& p : params)
				{
					if (p.first == m_a_primaryGroupToken && !p.second.empty())
						primaryGroupTokens_filter += "(" + m_a_primaryGroupId + "=" + p.second + ")";
					else if (p.first == m_a_distinguishedName && !p.second.empty())
						curr_groups.push_back(p.second);
				}
			}
			curr_groups.erase(std::remove_if(curr_groups.begin(), curr_groups.end(), [&total_memberOf](const tc::group_dn& g) {
				return total_memberOf.find(g) != total_memberOf.end();
			}), curr_groups.end());
			for (const auto& g : curr_groups)
				total_memberOf.insert(g);
		} while (curr_groups.size()>0 && ++deep <= 50);

		std::string filterByLoginGroup;
		if (!m_login_group.get().empty() && m_login_group.get() != group_dn)
		{
			auto expand = m_login_group_expand.load();
			if (expand && !expand->empty())
			{
				for (const auto& m : *expand)
				{
					if (!m_a_memberOf.empty())
					{
						filterByLoginGroup += "(" + m_a_memberOf + "=" + m.first + ")";
					}
					if (!m_a_primaryGroupId.empty())
					{
						filterByLoginGroup += "(" + m_a_primaryGroupId + "=" + m.second + ")";
					}
				}
				if (!filterByLoginGroup.empty())
					filterByLoginGroup = "(|" + filterByLoginGroup + ")";
			}
		}

		auto add_match_sub_groups = [&total_memberOf, &primaryGroupTokens_filter](const std::string& attr_to_match, std::string& filter)
		{
			filter += "(|";
			for (const auto& i : total_memberOf)
			{
				filter += "(";
				filter += attr_to_match;
				filter += "=";
				filter += i;
				filter += ")";
			}
			filter += primaryGroupTokens_filter;
			filter += ")";
		};

		auto build_filter_UserOrTrust = [this, &filterByLoginGroup, q, &add_match_sub_groups](bool do_add_match_sub_groups, const std::string& attr_to_match, std::string& filter) {
			filter += "(&";
			{
				if (m_trust_enabled)
					filter += "(|";
				{
					filter += "(&";
					filter += LDAP_FILTER_IS_USER;			// match all users
					filter += m_filter_disabled.get();		// AND not disabled
					filter += filterByLoginGroup;			// AND filterByLoginGroup
					filter += ")";
				}
				if (m_trust_enabled)
				{
					filter += m_filter_foreignSecurityPrincipal;
					filter += ")";
				}

				filter += q;

				// match group
				if (do_add_match_sub_groups)
					add_match_sub_groups(attr_to_match, filter);
			}
			filter += ")";
		};
		std::string filter;
		filter.reserve(1024);
		build_filter_UserOrTrust(true, m_a_memberOf, filter);

		std::vector<char*> attrs_arr2{
			(char*)m_a_login.c_str(),
			(char*)m_a_displayname.c_str(),
			(char*)m_a_distinguishedName.c_str(),
			(char*)m_a_email.c_str(),
		};
		if (custom_attrs)
			for (const auto& x : *custom_attrs)
				attrs_arr2.push_back((char*)x.c_str());
		attrs_arr2.push_back(0);

		std::vector<attrs_t> out2;
		err = LDAPSearch(ptr->ctx(), m_basedn, LDAP_SCOPE_SUBTREE, filter, (const char**)attrs_arr2.data(), out2, cookie, page_size, sort_attr);
		if (err != LDAP_SUCCESS) {
			if (m_ldap_error_handler_functor)
				m_ldap_error_handler_functor(err);
			return err;
		}

		if (!q.empty())
			FetchForeignUserByFilter(q.c_str(), group_users);

		FetchUsersFromResults(out2, group_users, custom_attrs);

		// kt: if insufficient rights, AD will not return us foreignSecurityPrincipals when search by memberOf attr (only by member attr)
		// (also would not get other domain by global catalog)
		// So, get all member values and exclude users, that we already have by memberOf. Fetch them directly.

		// 1. Get member attr from all nested groups
		std::vector<std::string> member;
		{
			std::string filter_member;
			filter_member.reserve(1024);
			add_match_sub_groups(m_a_distinguishedName, filter_member);

			std::vector<char*> attrs_arr{
				(char*)m_a_groupmember.c_str(),
				0
			};
			page_cookie_t fake_cookie;
			std::vector<attrs_t> out_member;
			err = LDAPSearch(ptr->ctx(), m_basedn, LDAP_SCOPE_SUBTREE, filter_member, (const char**)attrs_arr.data(), out_member, fake_cookie);
			if (err != LDAP_SUCCESS) {
				if (m_ldap_error_handler_functor)
					m_ldap_error_handler_functor(err);
				return err;
			}

			for (const auto& params : out_member)
			{
				for (const auto& p : params)
				{
					if (p.first == m_a_groupmember && !p.second.empty())
						member.emplace_back(p.second);
				}
			}
		}

		// 2. Get extra member attr, that were not matched in search by memberOf
		member.erase(std::remove_if(member.begin(), member.end(), [&out2, this](const std::string& str) {
			for (const auto& attrs : out2)
				for (const auto& attr : attrs)
					if (attr.first == m_a_distinguishedName && attr.second == str)
						return true;
			return false;
		}), member.end());
		out2.clear();
		
		// 3. Fetch them separatly (FetchForeign; local at other Domain)
		if (!member.empty())
		{
			std::string filter;
			filter.reserve(1024);
			build_filter_UserOrTrust(false, std::string(), filter);

			for (const auto& i : member)
			{
				page_cookie_t fake_cookie;
				std::vector<attrs_t> out_users;
				auto err_local = LDAPSearch(ptr->ctx(), i, LDAP_SCOPE_SUBTREE, filter, (const char**)attrs_arr2.data(), out_users, fake_cookie);
				if (err_local != LDAP_SUCCESS) {
					if (m_ldap_error_handler_functor)
						m_ldap_error_handler_functor(err_local);
					//return err;
				}
				FetchUsersFromResults(out_users, group_users, custom_attrs);
			}
		}

		// remove dups
		std::set<std::string> seen_values;
		group_users.erase( std::remove_if(group_users.begin(), group_users.end(), [&seen_values](const ldap_user_info& i) {
			return !seen_values.insert(i.login).second;
		}), group_users.end());

		if (!q.empty())	// kt: dirty hack for web_config: filter trusts by login group
		{
			std::shared_ptr<VS_AbCommonMap> users_of_login_group;
			LDAPCore::GetAllUsers(users_of_login_group);
			if (users_of_login_group)
			{
				group_users.erase(std::remove_if(group_users.begin(), group_users.end(), [&users_of_login_group](const ldap_user_info& i) {
					return users_of_login_group->find(VS_RealUserLogin(i.login).GetID()) == users_of_login_group->end();
				}), group_users.end());
			}
		}
		return err;
	}

	ldap_error_code_t LDAPCore::GetGroupUsers_noMemberOf(const char* group_dn, std::vector<ldap_user_info>& group_users, page_cookie_t& cookie, const long page_size, const std::vector<tc::attr_name_t>* custom_attrs, const std::pair<std::string, bool>* sort_attr, const std::string* query)
	{
		auto our_domain = this->GetOurDomain();
		if (!group_dn) return LDAP_INVALID_SYNTAX;

		bool empty_cookie = cookie == page_cookie_t();
		ldap_error_code_t err(LDAP_SUCCESS);
		std::string uids_filter;
		// get only "uid=name" part from distinguishedName
		if (empty_cookie) {
			err = GetGroupUsersUIDs(group_dn, uids_filter);
			if (err != LDAP_SUCCESS) {
				if (m_ldap_error_handler_functor)
					m_ldap_error_handler_functor(err);
				return err;
			}

			if (!uids_filter.empty())
				uids_filter = "(|" + uids_filter + ")";
		}

		// Without this check we are going to find all users on the LDAP server in the case if LDAP group is empty.
		// if cookie is not empty, then filter should be ignored
		if (!uids_filter.empty() || !empty_cookie)
		{
			std::string filter = "(&";	// "(&%s%s%s)"
			filter += LDAP_FILTER_IS_USER;
			if (query)
				filter += *query;
			filter += uids_filter;
			filter += ")";
			err = SearchForUser(filter.c_str(), group_users, cookie, page_size, custom_attrs, sort_attr);
			if (err != LDAP_SUCCESS) {
				if (m_ldap_error_handler_functor)
					m_ldap_error_handler_functor(err);
				return err;
			}
		}

		// todo(kt): this is variant without sort_attr working
		//for (const auto& results : out)
		//{
		//	for (const auto& result : results)
		//	{
		//		if (result.first == m_a_groupmember)
		//		{
		//			memset(wbuff, 0, 4096);
		//			swprintf_s(wbuff, 4096, "(&%s%s)", LDAP_FILTER_IS_USER, (query)? query->c_str(): "");
		//			page_cookie_t fake_cookie;
		//			std::vector<ldap_user_info> found_users;
		//			err = SearchForUser(wbuff, found_users, fake_cookie, page_size, custom_attrs, sort_attr, result.second.c_str());
		//			if (err != LDAP_SUCCESS) {
		//				if (m_ldap_error_handler_functor)
		//					m_ldap_error_handler_functor(err);
		//				return err;
		//			}
		//			if (found_users.size() == 1)
		//				group_users.push_back(found_users[0]);
		//		}
		//	}
		//}
		return err;
	}

	ldap_error_code_t LDAPCore::GetGroupUsersUIDs(const char* group_dn, std::string &uids_filter) {
		if (!group_dn) return LDAP_INVALID_SYNTAX;

		std::vector<attrs_t> out;
		ldap_error_code_t err(LDAP_SUCCESS);
		page_cookie_t fake_cookie;

		std::vector<char*> attrs_arr{
			(char*)m_a_groupmember.c_str(),
			0
		};

		err = LDAPSearch(m_ldap.load()->ctx(), group_dn, LDAP_SCOPE_SUBTREE, m_filter_group, (const char**)attrs_arr.data(), out, fake_cookie);
		if (err != LDAP_SUCCESS) return err;

		for (const auto& results : out)
		{
			for (const auto& result : results)
			{
				if (result.first == m_a_groupmember)
				{
					auto p = result.second.find_first_of(",");
					if (p != std::string::npos) {
						auto first_relative_dn = std::string(result.second).substr(0, p);
						if (first_relative_dn.find("uid=") != std::string::npos) {				// it's user
							uids_filter += "(" + first_relative_dn + ")";						// add it to uids
						}
						else {
							err = GetGroupUsersUIDs(result.second.c_str(), uids_filter);
							if (err != LDAP_SUCCESS) return err;
						}

					}
				}
			}
		}

		return err;
	}

	std::chrono::system_clock::time_point LDAPCore::GetLastUpdateNestedCacheTime()
	{
		VS_AutoLock lock(&m_nested_groups_lock);
		return m_last_nested_cache_update;
	}

	ldap_error_code_t LDAPCore::GetAllAttributes(const char* dn, const std::vector<tc::attr_name_t>* custom_attrs, std::map<attr_name_t, std::vector<attr_value_t>>& attrs)
	{
		return GetAllAttributes(dn, LDAP_FILTER_ALL, custom_attrs, attrs);
	}

	ldap_error_code_t LDAPCore::GetAllAttributes(const char* dn, const char* filter, const std::vector<tc::attr_name_t>* custom_attrs, std::map<attr_name_t, std::vector<attr_value_t>>& attrs)
	{
		ldap_error_code_t err(LDAP_SUCCESS);
		if (!dn || !*dn)
			return err;

		std::vector<char*> attrs_arr;

		if (custom_attrs)
			for (const auto& x : *custom_attrs)
				attrs_arr.push_back((char*)x.c_str());
		attrs_arr.push_back(0);

		page_cookie_t fake_cookie;
		std::vector<attrs_t> out;
		err = LDAPSearch(m_ldap.load()->ctx(), dn, LDAP_SCOPE_BASE, filter, (const char**)attrs_arr.data(), out, fake_cookie);
		if (err != LDAP_SUCCESS) {
			if (m_ldap_error_handler_functor)
				m_ldap_error_handler_functor(err);
			return err;
		}

		//const unsigned long buff_len = 1024;
		//char buff[buff_len] = { 0 };

		//	unsigned long primaryGroupId(0);
		//	std::set<std::string> memberOf;
		if (out.size() == 1)
		{
			for (attrs_t::iterator it = out[0].begin(); it != out[0].end(); ++it)
			{
				attrs[it->first].push_back(it->second);

				//// special processing of some attrs
				//if (it->first == m_a_memberOf)
				//{
				//	memberOf.insert(it->second);
				//} else if (it->first == m_a_primaryGroupId) {
				//	primaryGroupId = atol(WSTR_TO_STR(it->second.c_str()));
				//}
			}
		}
		//std::set<std::string> parent_groups;
		//for(std::set<std::string>::iterator it=memberOf.begin(); it!=memberOf.end(); ++it)
		//{
		//	std::string parent_group = GetParentGroup(it->c_str(),primaryGroupId);
		//	if (parent_group.length())
		//		parent_groups.insert(parent_group);
		//}
		//if (parent_groups.size())
		//	std::copy(parent_groups.begin(), parent_groups.end(), std::back_inserter(attrs[LDAP_TRUECONF_MEMBEROF]));
		return err;
	}

	bool LDAPCore::IsErrorReconnect(const ldap_error_code_t err)
	{
		return (err == LDAP_SERVER_DOWN || err == LDAP_UNAVAILABLE || err == LDAP_LOCAL_ERROR || err == LDAP_TIMEOUT) ? true : false;
	}

	bool LDAPCore::FetchUser(const tc::ldap_user_info& info, VS_StorageUserData& ude)
	{
		if (info.login.empty())
			return false;
		VS_RealUserLogin r(vs::UTF8ToLower(info.login));
		ude.m_realLogin = r;
		ude.m_login = ude.m_name = r;

		ude.m_displayName = info.displayName;
		//	ude.m_email = ??		// ToUTF8()
		//	ude.m_FirstName = ??
		//	ude.m_LastName = ??
		ude.m_type = 0;		// todo(kt): always zero here?

		int user_phone_idx(0);
		auto get_phone = [&](const std::pair<std::string, std::string>& iterator, const std::string& attr_name, VS_UserPhoneType type) {
			if (!attr_name.empty() && iterator.first == attr_name)
			{
				char buff[1024] = { 0 };
				memset(buff, 0, 1024);
				VS_UserPhoneItem item;
				snprintf(buff, (sizeof(buff) / sizeof(buff[0])) - 1, "Users\\%s\\UsersPhones\\%d", VS_RealUserLogin(SimpleStrToStringView(ude.m_name)).GetUser().c_str(), ++user_phone_idx);
				item.id = buff;
				item.phone = iterator.second.c_str();
				item.type = type;
				item.call_id = ude.m_name;
				ude.m_phones.push_back(item);
			}
		};

		for (const auto& x : info.custom_attrs)
		{
			if (!m_ldap_attr_FullID.empty() && x.first == m_ldap_attr_FullID)
				ude.m_name = VS_RealUserLogin(vs::UTF8ToLower(x.second));
			get_phone(x, m_ldap_attr_Phone_mobile, USERPHONETYPE_MOBILE);
			get_phone(x, m_ldap_attr_Phone_work, USERPHONETYPE_WORK);
			get_phone(x, m_ldap_attr_Phone_home, USERPHONETYPE_HOME);
			if (x.first == m_a_email)		ude.m_email = x.second.c_str();
			if (x.first == m_a_firstname)	ude.m_FirstName = x.second;
			if (x.first == m_a_lastname)	ude.m_LastName = x.second;
			if (x.first == m_a_company)		ude.m_Company = x.second;
		}
		if (ude.m_Company.empty())
		{
			// todo(kt): imp m_srvCert at PHP_WIN32
			//char buff[1024] = { 0 };
			//unsigned long buffSize = 0;
			//m_srvCert.GetSubjectEntry("organizationName", buff, buffSize);
			//if (buffSize > 0 && buffSize < 1024)
			//{
			//	if (m_srvCert.GetSubjectEntry("organizationName", buff, buffSize))
			//		ude.m_Company.AssignUTF8(buff);
			//}
		}
		return true;
	}

	bool LDAPCore::IsCacheReady() const
	{
		if (m_params_hash.get() == 0)
			return false;
		if (m_ab_cache_timeout.count() > 0)
			return m_last_nested_cache_update != std::chrono::system_clock::time_point();
		else
			return true;
	}

	ldap_error_code_t LDAPCore::GetAllGroupsOfUser_noMemberOf(const tc::ldap_user_info& user, const unsigned long user_primary_group_id, const std::string& objectSid, std::set<tc::group_dn>& ret)
	{
		tc::ldap_error_code_t err(LDAP_SUCCESS);
		if (user.dn.empty())
			return err;
		char wbuff[4096] = { 0 };
		snprintf(wbuff, sizeof(wbuff), "(&%s(%s=%s))", m_filter_group.c_str(), m_a_groupmember.c_str(), user.dn.c_str());
		tc::page_cookie_t fake_cookie;
		std::vector<attrs_t> results;
		err = LDAPSearch(m_ldap.load()->ctx(), m_basedn, LDAP_SCOPE_SUBTREE, wbuff, (const char**)0, results, fake_cookie/*, page_size, sort_attr*/);
		if (err != LDAP_SUCCESS) {
			if (m_ldap_error_handler_functor)
				m_ldap_error_handler_functor(err);
			return err;
		}

		for (const auto& g : results)
		{
			for (const auto& a : g)
			{
				if (a.first == m_a_distinguishedName)
					ret.insert(a.second);
			}
		}
		return err;
	}

	bool LDAPCore::SetGroupManager(const std::shared_ptr<VS_GroupManager> &gr_manager) {
		if (!gr_manager) return false;

		m_group_manager = gr_manager;
		return true;
	}

	bool LDAPCore::GetRegGroupUsers(const std::string& gid, std::shared_ptr<VS_AbCommonMap>& users)
	{
		std::map<std::string, VS_RegGroupInfo> reg_groups;
		GetRegGroups(reg_groups);
		auto it = reg_groups.find(gid);
		if (it != reg_groups.end())
		{
			if (!it->second.ldap_dn.empty())
			{
				auto c = m_cache_groups_users.load();
				if (!c)
					return false;
				auto g = c->find(it->second.ldap_dn);
				if (g == c->end() || !g->second)
					return false;
				users = g->second;
			}
		}
		return true;
	}

	void LDAPCore::SetLDAPErrorHandler(const boost::function<void(const tc::ldap_error_code_t)>& functor)
	{
		m_ldap_error_handler_functor = functor;
	}

	void LDAPCore::GetServerRootInfo(LDAP* ctx, std::string& base_dn, std::set<std::string>& supportedControl)
	{
		// todo(kt): rewrite as LDAPSearchImp
		static const char* a_contexts = "namingContexts";
		static const char* a_def_context = "defaultNamingContext";
		static const char* a_supporetedControl = "supportedControl";
		static const char* al_config[4] = { a_contexts, a_def_context, a_supporetedControl, 0 };

		page_cookie_t fake_cookie;
		std::vector<attrs_t> out;
		tc::ldap_error_code_t err = LDAPSearch(ctx, "", LDAP_SCOPE_BASE, LDAP_FILTER_ALL, &al_config[0], out, fake_cookie);
		if (err != LDAP_SUCCESS) {
			if (m_ldap_error_handler_functor)
				m_ldap_error_handler_functor(err);
			//		return err;
			return;
		}

		// prepare result for add to abook map
		for (std::vector<attrs_t>::const_iterator it = out.begin(); it != out.end(); ++it)
		{
			ldap_group_info g;
			for (attrs_t::const_iterator it2 = it->begin(); it2 != it->end(); ++it2)
			{
				if (it2->first == a_def_context)
					base_dn = it2->second;
				else if (it2->first == a_contexts) {
					if (base_dn.empty())
					{
						if ((strcasecmp(it2->second.c_str(), "CN=Configuration") == 0) &&
							(strcasecmp(it2->second.c_str(), "O=Netscape") != 0))
						{
							base_dn = it2->second;
						}
					}
				}
				else if (it2->first == a_supporetedControl) {
					supportedControl.emplace(it2->second);
				}

			}
		}
		/*
		LDAPMessage* lmsg = 0;
		wchar_t** base_list = 0;

		static wchar_t* a_contexts = "namingContexts";
		static wchar_t* a_def_context = "defaultNamingContext";
		static wchar_t* al_config[3] = { a_contexts, a_def_context, 0 };

		try
		{
		unsigned long search, lresult;

		{

		if (ldap_search_ext(m_ldap, NULL, LDAP_SCOPE_BASE, LDAP_FILTER_ALL,
		al_config, false, 0, 0, m_ldap_timeout.tv_sec, 0, &search) != LDAP_SUCCESS)
		throw VSS_LDAP_ERROR;

		lresult = ldap_result(m_ldap, search, LDAP_MSG_ONE, &m_ldap_timeout, &lmsg);
		if (lresult == -1)
		throw VSS_LDAP_ERROR;

		if (lresult == 0)
		{
		dprint1("LDAP timeout\n");
		ldap_abandon(m_ldap, search);
		throw VSS_LDAP_ERROR;
		};

		base_list = ldap_get_values(m_ldap, lmsg, a_def_context);
		if (base_list && base_list[0])
		{
		m_basedn = base_list[0];
		}
		else
		{
		if (base_list)
		ldap_value_free(base_list);

		base_list = ldap_get_values(m_ldap, lmsg, a_contexts);
		if (base_list)
		for (wchar_t** base = base_list; *base; base++)
		{
		if (wcsstr(*base, "CN=Configuration"))
		continue;
		if (_wcsicmp(*base, "O=Netscape") == 0)
		continue;
		m_basedn = *base;
		break;
		}
		}
		};

		}
		catch (int error)
		{
		//		error_code=error;
		if (error == VSS_LDAP_ERROR)
		ProcessLDAPError();
		};

		if (lmsg)
		ldap_msgfree(lmsg);
		if (base_list)
		ldap_value_free(base_list);
		return;
		*/
	}

	bool tc::LDAPCore::GetDnsServerName(LDAP * ctx, std::string & foundName)
	{
		if(!ctx)
			return false;

		const std::string a_dns = "dnsHostName";
		const char* al_config[2] = { a_dns.c_str(), 0 };

		std::vector<attrs_t> out;
		tc::ldap_error_code_t err = LDAPSearch(ctx, "", LDAP_SCOPE_BASE, LDAP_FILTER_ALL, &al_config[0], out, vs::ignore<page_cookie_t>(), -1);
		if (err != LDAP_SUCCESS) {
			if (m_ldap_error_handler_functor)
				m_ldap_error_handler_functor(err);
			return false;
		}

		for (const auto& atrs : out)
		{
			auto it = std::find_if(atrs.begin(), atrs.end(), [&a_dns](const std::pair<attr_name_t, attr_value_t>& atr) {
				return atr.first == a_dns;
			});
			if (it != atrs.end()) {
				foundName = it->second;
				return !foundName.empty();
			}
		}

		return false;
	}

void LDAPCore::SetUserStatus(const VS_SimpleStr& call_id, int status)
{
	if (m_ldap_attr_UserStatus.empty() && m_ldap_attr_UserID.empty())
		return ;

	dprint3("LDAP: SetUserStatus for %s: %d\n", call_id.m_str, status);


	char filter[256];
	VS_RealUserLogin r(SimpleStrToStringView(call_id));
	snprintf(filter, sizeof(filter), (char*)m_filter_callid.c_str(), r.GetUser().c_str());

	std::vector<tc::ldap_user_info> found_users;
	SearchForUser(filter, found_users);
	if (found_users.size() != 1)
		return;
	auto it = found_users[0].custom_attrs.find(m_a_distinguishedName);
	if (it == found_users[0].custom_attrs.end())
		return;
#ifdef _WIN32
	auto dn_holder = vs::UTF8ToWideCharConvert(it->second);
	auto dn = (PWSTR)dn_holder.c_str();
#else
	auto dn = it->second.c_str();
#endif

	auto ptr = m_ldap.load();
	if (!m_ldap_attr_UserStatus.empty())
	{
#ifdef _WIN32
		auto name = (wchar_t*)m_ldap_attr_UserStatus.c_str();
		wchar_t tmp[32] = { 0 };		_itow(status, tmp, 10);
		wchar_t* new_status[2] = { tmp, 0 };
		auto val = (PWCHAR*)&new_status;
#else
		auto name = (char*)m_ldap_attr_UserStatus.c_str();
		auto tmp = std::to_string(status);
		char* new_status[2] = { (char*)tmp.c_str(),0 };
		auto val = (char**)&new_status;
#endif

		LDAPMod mod;
		mod.mod_op = LDAP_MOD_REPLACE;
		mod.mod_type = name;
		mod.mod_vals.modv_strvals = val;

		LDAPMod* mods[2] = { &mod, 0 };

		if (ldap_modify_ext_s(ptr->ctx(), dn, mods, nullptr, nullptr) != LDAP_SUCCESS)
			dstream4 << "ldap_modify_ext_s(" << dn << ") failed";
	}

	if (!m_ldap_attr_UserID.empty())
	{
#ifdef _WIN32
		auto name = (wchar_t*)m_ldap_attr_UserID.c_str();
		auto wstr = vs::UTF8ToWideCharConvert((const char*)r);
		wchar_t* new_vals[2] = { (wchar_t*)wstr.c_str(),0 };
		auto val = (PWCHAR*)&new_vals;
#else
		auto name = (char*)m_ldap_attr_UserID.c_str();
		std::string str = (const char*)r;
		char* new_vals[2] = { (char*)str.c_str(),0 };
		auto val = (char**)&new_vals;
#endif

		LDAPMod mod;
		mod.mod_op = LDAP_MOD_REPLACE;
		mod.mod_type = name;
		mod.mod_vals.modv_strvals = val;

		LDAPMod* mods[2] = { &mod, 0 };

		if (ldap_modify_ext_s(ptr->ctx(), dn, mods, nullptr, nullptr) != LDAP_SUCCESS)
			dstream4 << "ldap_modify_ext_s(" << dn << ") failed";
	}
}

LDAPCore::CheckLoginResult LDAPCore::LoginUser_CheckLogin(string_view login_, tc::ldap_user_info& found, std::map<std::string, VS_RegGroupInfo>& user_reg_groups, std::string& user_at_domain)
{
	auto login = PreprocessCallID(login_);
	if (login.empty())
		return CheckLoginResult::UNDEFINED;
	VS_RealUserLogin r(login);

	user_at_domain = login;
	std::string desired_domain;
	bool desired_domain_is_ours(true);
	{
		string_view sv(login);
		auto pos = sv.find_first_of('@');
		if (pos != string_view::npos)
		{
			auto user_part(sv);
			user_part.remove_suffix(user_part.length() - pos);
			sv.remove_prefix(++pos);
			desired_domain = (std::string)sv;

			auto our_domain = GetOurDomain();
			if (!desired_domain.empty() &&
				((!our_domain.empty() && boost::iequals(desired_domain, our_domain)) ||
				((!m_our_flatName.empty() && boost::iequals(desired_domain, m_our_flatName)))))
				login = (std::string)user_part;
			else
				desired_domain_is_ours = false;
		}
	}

	uint32_t primaryGroupId(0);

	dstream3 << "LDAP LoginUser '" << login << "'\n";

	char buff[4099] = { 0 };
	char buff2[4096] = { 0 };
	string_view login_without_domain(login);
	{
		auto pos = login_without_domain.find_first_of('@');
		if (pos != string_view::npos)
			login_without_domain.remove_suffix(login_without_domain.length() - pos);
	}
	snprintf(buff2, sizeof(buff2) / sizeof(buff2[0]), m_filter_login.c_str(), EscapeForLDAPFilter(login_without_domain).c_str());
	snprintf(buff, sizeof(buff) / sizeof(buff[0]), "(&%s%s)", m_filter_disabled.get().c_str(), buff2);		// not Disabled users

#ifdef _SVKS_M_BUILD_
	std::string dn_to_search;
	dn_to_search = m_region.get();
#endif

	std::vector<tc::ldap_user_info> found_users;
	tc::ldap_error_code_t err = SearchForUser(buff, found_users);
	if (err != LDAP_SUCCESS)
		return (tc::LDAPCore::IsErrorReconnect(err)) ? CheckLoginResult::LDAP_ERROR : CheckLoginResult::NOT_FOUND_OR_LDAPDISABLED_OR_AMBIGUOUS;
	if (!desired_domain.empty())
	{
		found_users.erase(std::remove_if(found_users.begin(), found_users.end(), [&desired_domain, desired_domain_is_ours](const tc::ldap_user_info& u) {
			string_view sv(u.login);
			auto pos = sv.find_first_of('\\');
			if (pos == string_view::npos)
				return !desired_domain_is_ours;
			sv.remove_suffix(sv.length() - pos);
			return !boost::iequals(desired_domain, sv);
		}), found_users.end());
	}
	if (found_users.size() < 1 && !m_trustedDomains.empty())		// try trustedDomains
	{
		auto ds = dstream4;
		ds << "local user not found, search trusts ";
		std::vector<tc::ldap_user_info> trust_users;
		FetchForeignUserByLogin(login, trust_users);
		ds << "found=" << trust_users.size();
		if (trust_users.size() == 1)		// only one user found (not less, not more)
		{
			const tc::ldap_user_info& u_foreign = trust_users[0];
			ds << " login=" << u_foreign.login;
			r = VS_RealUserLogin(vs::UTF8ToLower(u_foreign.login));

			// get DN of remote user at our AD
			auto objSID = u_foreign.custom_attrs.find(m_a_objectSid);
			if (objSID != u_foreign.custom_attrs.end())
			{
				snprintf(buff, sizeof(buff) / sizeof(buff[0]), "(%s=%s)", m_a_objectSid.c_str(), objSID->second.c_str());
				std::vector<tc::ldap_user_info> our_link;
				tc::ldap_error_code_t err = SearchForUser(buff, our_link);
				if (err == LDAP_SUCCESS &&
					our_link.size() == 1 &&
					!our_link[0].dn.empty())
				{
					// at our domain foreign user do not have login, displayName, so copy them from remote domain
					// (but we have our memberOf)
					our_link[0].login = u_foreign.login;
					our_link[0].displayName = u_foreign.displayName;
					found_users = std::move(our_link);
				}
				else {
					found_users.push_back(u_foreign);		// no ObjSID at our AD, but found user at trust AD
				}
			}
		}
	}

	if (found_users.size() != 1)
	{
		if (found_users.empty())
			dprint3("user not found\n");
		else
			dprint1("more than one user was found!\n");
		return CheckLoginResult::NOT_FOUND_OR_LDAPDISABLED_OR_AMBIGUOUS;
	}

	for (const auto& x : found_users[0].custom_attrs)
	{
		if (!m_a_primaryGroupId.empty() && !strcasecmp(x.first.c_str(), m_a_primaryGroupId.c_str()))
			primaryGroupId = std::atol(x.second.c_str());
	}

	// check user is in login group

	std::string objSid;
	auto it = found_users[0].custom_attrs.find(m_a_objectSid);
	if (it != found_users[0].custom_attrs.end())
		objSid = it->second;

	std::set<tc::group_dn> ldap_user_groups;
	err = GetAllGroupsOfUser(found_users[0], primaryGroupId, objSid, ldap_user_groups);
	{
		auto ds = dstream4;
		ds << "GetAllGroupsOfUser(" << found_users[0].login << ")=" << std::to_string(ldap_user_groups.size()) << ": ";
		for (const auto& g : ldap_user_groups)
			ds << g << "~~";
	}
	if (err != LDAP_SUCCESS)
		return CheckLoginResult::LDAP_ERROR;

	bool IsInsideLoginGroup(false);
	if (!m_login_group.get().empty())
	{
		auto expand = m_login_group_expand.load();
		if (expand)
		{
			for (const auto& g : *expand)
			{
				if (ldap_user_groups.find(g.first) != ldap_user_groups.end())
				{
					IsInsideLoginGroup = true;
					break;
				}
			}
		}
	}
	else {
		IsInsideLoginGroup = true;
	}
	if (!IsInsideLoginGroup)
		return CheckLoginResult::NOT_IN_LOGIN_GROUP;

	std::map<std::string, VS_RegGroupInfo> reg_groups;
	GetRegGroups(reg_groups);
	for (const auto& rg : reg_groups)
	{
		auto g = ldap_user_groups.find(rg.second.ldap_dn);
		if (g != ldap_user_groups.end())
			user_reg_groups.insert(rg);
	}

	found = found_users[0];
	return CheckLoginResult::OK;
}

bool LDAPCore::LoginUser_CheckPassword(string_view login, string_view password, string_view user_dn_)
{
	if (password.empty())
	{
		dprint3(" empty password\n");
		return false;
	}

	auto auth_ctx = NewAuthLDAPConn();
	VS_SCOPE_EXIT{ if (auth_ctx) ldap_unbind_ext(auth_ctx, nullptr, nullptr); };
	if (!auth_ctx)
	{
		dstream3 << "can not create auth_ctx, server=" << m_ldap_server << ", port=" << m_ldap_port << ", is_sec=" << m_ldap_secure;
		return false;
	}

	int ldresult(0);

#ifndef _SVKS_M_BUILD_
#ifdef _WIN32
	auto user = vs::UTF8ToWideCharConvert(login);
	auto pass = vs::UTF8ToWideCharConvert(password);
	auto user_dn = vs::UTF8ToWideCharConvert(user_dn_);

	SEC_WINNT_AUTH_IDENTITY_W id;
	id.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
	id.Domain = (unsigned short*)"";		// you must pass empty string, not nullptr (because then default domain/workgroup will be used)
	id.DomainLength = 0;
	id.User = (unsigned short*)(user.c_str());
	id.UserLength = user.length();
	id.Password = (unsigned short*)(pass.c_str());
	id.PasswordLength = pass.length();

	switch (m_auth_method)
	{
	case tc::AuthMethods::VS_LDAP_AUTH_SIMPLE:
		dstream3 << "CheckPass: binding to LDAP with simple auth, user=" << user_dn_ << ", domain=" << (wchar_t*)(id.Domain) << ", pass_len=" << pass.length();
		if (user_dn.empty())
		{
			dstream3 << "anonymous bind is forbidden";
			return false;
		}
		ldresult = ldap_bind_s(auth_ctx, (PWSTR)user_dn.c_str(), (PWCHAR)(wchar_t *)pass.c_str(), LDAP_AUTH_SIMPLE);
		break;
	case tc::AuthMethods::VS_LDAP_AUTH_NTLM:
	case tc::AuthMethods::VS_LDAP_AUTH_NTLM_CURRENTUSER:
	case tc::AuthMethods::VS_LDAP_AUTH_GSS:
	{
		ldresult = ldap_bind_s(auth_ctx, NULL, (wchar_t*)&id, (m_auth_method == tc::AuthMethods::VS_LDAP_AUTH_GSS) ? LDAP_AUTH_NEGOTIATE : LDAP_AUTH_NTLM);
		dstream3 << "CheckPass: result=" << ldresult << ", auth_method=" << m_auth_method << ", user=" << (wchar_t*)(id.User) << ", domain=" << (wchar_t*)(id.Domain) << ", pass_len=" << id.PasswordLength;
	}
	break;
	case tc::AuthMethods::VS_LDAP_AUTH_DIGEST_MD5:
		break;
	default:
		dprint3("CheckPass: access denied: invalid auth_method\n");
		return false;
		break;
	}


#else	// OpenLDAP bind
	std::string pwd(password);
	std::string user_at_domain(login);
	auto defaults = lutil_sasl_defaults(auth_ctx,
		"DIGEST-MD5",
		nullptr /*(char*)m_auth_domain.c_str()*//*sasl_realm*/,
		(char*)user_at_domain.c_str()/*sasl_authc_id*/,
		(char*)pwd.c_str()/*passwd.bv_val*/,
		(char*)user_at_domain.c_str()/*sasl_authz_id*/);
	VS_SCOPE_EXIT{ lutil_sasl_freedefs(defaults); };
	struct berval pass_ber = { pwd.length(), (char*)pwd.c_str() };

	auto ds = dstream3;
	ds << "m_authldap, ldap_bind  " << user_at_domain << ", pass_len=" << pwd.length() << ", ";
	switch (m_auth_method)
	{
	case tc::AuthMethods::VS_LDAP_AUTH_SIMPLE:
		ds << "simple auth";
		ldresult = ldap_sasl_bind_s(auth_ctx, user_at_domain.c_str(), LDAP_SASL_SIMPLE, &pass_ber, NULL, NULL, NULL);
		break;
	case tc::AuthMethods::VS_LDAP_AUTH_NTLM_CURRENTUSER:
		ds << "NT current user; NOT IMPLEMENTED";
		//ldresult = ldap_bind_s(ctx, NULL, NULL, LDAP_AUTH_NTLM);
		break;
	case tc::AuthMethods::VS_LDAP_AUTH_NTLM:
		ds << "NT auth";
		ldresult = ldap_sasl_interactive_bind_s(auth_ctx, nullptr, "NTLM", "GSS-SPNEGO",
			nullptr/*sctrlsp*/, nullptr, 0/*sasl_flags*/, lutil_sasl_interact, defaults);
		break;
	case tc::AuthMethods::VS_LDAP_AUTH_GSS:
		ds << "GSS auth; NOT IMPLEMENTED";
		//ldresult = ldap_bind_s(ctx, NULL, (wchar_t*)&id, LDAP_AUTH_NEGOTIATE);
		break;
	case tc::AuthMethods::VS_LDAP_AUTH_DIGEST_MD5:
		ds << "DIGEST-MD5 auth";
		ldresult = ldap_sasl_interactive_bind_s(auth_ctx, nullptr, "DIGEST-MD5", nullptr,
			nullptr/*sctrlsp*/, nullptr, 0/*sasl_flags*/, lutil_sasl_interact, defaults);
		break;
	default:
		dprint3("CheckPass: access denied: invalid auth_method\n");
		return false;
		break;
	}
	ds << " ldap_res:" << std::to_string(ldresult);
#endif	// OpenLDAP
#endif	// _SVKS_M_BUILD_
	if (ldresult != LDAP_SUCCESS)
	{
		dprint3(" access denied (ldap_err:%d)\n", ldresult);
		return false;
	}
	return true;
}

LDAP* LDAPCore::NewAuthLDAPConn()
{
	LDAP* ctx = nullptr;
#ifdef _WIN32	// not ported
	ctx = ldap_sslinit((PWSTR)vs::UTF8ToWideCharConvert(m_ldap_server).c_str(), m_ldap_port, m_ldap_secure);
#else
	std::string conn_str = "ldap";
	if (m_ldap_secure)
		conn_str += "s";
	conn_str += "://";
	conn_str += m_ldap_server;
	conn_str += ":";
	conn_str += std::to_string(m_ldap_port);
	conn_str += "/";

	ldap_initialize(&ctx, conn_str.c_str());
#endif

	if (!ctx)
		return nullptr;
	unsigned long ldresult = ldap_set_option(ctx, LDAP_OPT_PROTOCOL_VERSION, (void*)&m_ldap_version);
	if (ldresult != LDAP_SUCCESS)
		dprint1("can't set auth LDAP connection version to 3\n");
	return ctx;
}

}	// namespace tc

//#undef UNICODE

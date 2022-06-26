#pragma once

#ifdef _WIN32

#ifndef __wtypes_h__
#include <wtypes.h>
#endif

#ifndef __WINDEF_
#include <windef.h>
#endif

#include <winldap.h>

#else // linux
#include <ldap.h>
#endif

#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <string>
#include <stdint.h>

namespace tc
{
#define COOKIE_LIFETIME_MINUTES 5
#define LDAP_DEFAULT_PAGE_SIZE 0

	typedef std::string attr_name_t;
	typedef std::string attr_value_t;
	typedef std::vector< std::pair<attr_name_t, attr_value_t> > attrs_t;
	typedef long ldap_error_code_t;

	// nested groups cache
	typedef std::string group_dn;
	typedef unsigned long primary_group_token;

	class cfg_params_t : public std::map<std::string, std::string>
	{
	public:
		unsigned long CaclHash() const;
	};


	class ldap_group_info
	{
	public:
		ldap_group_info() : primaryGroupToken(0)
		{}
		std::string dn;
		std::string displayName;
		unsigned long primaryGroupToken;
	};

	class ldap_user_info
	{
	public:
		std::string dn;
		std::string login;
		std::string displayName;
		std::multimap<attr_name_t, attr_value_t> custom_attrs;
	};

	class page_cookie_t
	{
		long m_cookie;
		std::chrono::system_clock::time_point m_create_time;
	public:
		page_cookie_t() : m_cookie(0), m_create_time(std::chrono::system_clock::now()) {}
		explicit page_cookie_t(long val) : m_cookie(val), m_create_time(std::chrono::system_clock::now()) {}
		operator long()
		{
			return m_cookie;
		}
		void operator=(long val)
		{
			m_cookie = val;
		}
		page_cookie_t& operator++()
		{
			++m_cookie;
			if (m_cookie == 0)
				++m_cookie;
			return *this;
		}
		bool IsExpired() const
		{
			return (std::chrono::system_clock::now() - m_create_time) > std::chrono::minutes(COOKIE_LIFETIME_MINUTES);
		}

		friend bool operator<(const page_cookie_t& l, const page_cookie_t& r)
		{
			return l.m_cookie < r.m_cookie;
		}
	};

	struct referral_t
	{
		std::string		host;
		unsigned short	port;
		std::string	baseDN;
		bool			IsSecure;
		referral_t() : port(0), IsSecure(false)
		{}
		bool operator ==(const referral_t& right)
		{
			return (host == right.host &&
				port == right.port &&
				baseDN == right.baseDN &&
				IsSecure == right.IsSecure) ? true : false;
		}
	};

	/////////////////////////////////////////////////////
	const char	LDAP_BASEDN_TAG[] = "LDAP Base DN";
	const char	LDAP_OUR_DOMAIN_TAG[] = "LDAP Our Domain";
	const char	LDAP_SERVER_TAG[] = "LDAP Server";
	const char	LDAP_DOMAIN_TAG[] = "LDAP Domain";

	const char	LDAP_SECURE_TAG[] = "LDAP Secure";
	const unsigned short	LDAP_SECURE_INIT = 0;
	const char	LDAP_AUTOVERIFY_SERVERCERT_TAG[] = "LDAP AutoVerify ServerCert";

	const char	LDAP_PORT_TAG[] = "LDAP Port";
	const unsigned short	LDAP_PORT_INIT = 389;
	const unsigned short	LDAP_SECURE_PORT_INIT = 636;

	static const char LDAP_MAX_RESULTS[] = "LDAP MaxResults";
	static const uint32_t LDAP_MAX_RESULTS_INIT = 50'000;

	enum eLDAPServerType
	{
		LDAP_SERVER_TYPE_AD = 0,
		LDAP_SERVER_TYPE_OPENLDAP = 1,
		LDAP_SERVER_TYPE_389DIR = 2,
		LDAP_SERVER_TYPE_CUSTOM = 0xff
	};
	const char	LDAP_SERVER_TYPE_TAG[] = "LDAP Server Type";
	const eLDAPServerType	LDAP_SERVER_TYPE_INIT = LDAP_SERVER_TYPE_AD;

	const char	LDAP_TIMEOUT_TAG[] = "LDAP Timeout";
	const int		LDAP_TIMEOUT_INIT = 30;

	const char	LDAP_AB_REFRESH_TAG[] = "LDAP AddressBook Refresh";
	const uint64_t	LDAP_AB_REFRESH_INIT = 15 * 60;

	const char	LDAP_VERSION_TAG[] = "LDAP Version";
	const int	LDAP_VERSION_INIT = LDAP_VERSION3;

	const char	LDAP_USER_STATUS_TAG[] = "LDAP User Status";

	const char	LDAP_AVATARS_PATH_TAG[] = "LDAP Avatars Path";

	const char	LDAP_AVATARS_PATH_INIT[] = "avatars/cache/";

	const char	LDAP_USE_AVATARS_TAG[] = "LDAP Use Avatars";
	const std::int32_t LDAP_USE_AVATARS_INIT = 1;

	const char	LDAP_ALLOW_AVATAR_PROPAGATING_TAG[] = "LDAP Allow Avatar Propagating";
	const std::int32_t LDAP_ALLOW_AVATAR_PROPAGATING_INIT = 1;

	const char	LDAP_AVATARS_SIZE_TAG[] = "LDAP Avatars Size";
	const std::int32_t LDAP_AVATARS_SIZE_INIT = 160;

	const char	LDAP_AVATARS_QUALITY_TAG[] = "LDAP Avatars Quality";
	const std::int32_t	LDAP_AVATARS_QUALITY_INIT = 100;

	/// auth
	const char	LDAP_AUTH_METHOD_TAG[] = "LDAP Auth Method";
	const char	LDAP_AUTH_USER_TAG[] = "LDAP Auth User";
	const char	LDAP_AUTH_PASSWORD_TAG_OLD[] = "LDAP Auth Password";
	const char	LDAP_AUTH_PASSWORD_TAG_NEW[] = "LDAP Auth Password2";
	const char	LDAP_AUTH_DOMAIN_TAG[] = "LDAP Auth Domain";


	/// attributes
	const char	LDAP_ATTR_LOGIN_TAG[] = "LDAP Login";
	const char	LDAP_ATTR_LOGIN_INIT[] = "sAMAccountName";

	const char	LDAP_ATTR_EMAIL_TAG[] = "LDAP Email";
	const char	LDAP_ATTR_EMAIL_INIT[] = "mail";

	const char	LDAP_ATTR_FIRSTNAME_TAG[] = "LDAP First Name";
	const char	LDAP_ATTR_FIRSTNAME_INIT[] = "givenName";

	const char	LDAP_ATTR_MIDDLENAME_TAG[] = "LDAP Middle Name";
	const char	LDAP_ATTR_MIDDLENAME_INIT[] = "patronymic";

	const char	LDAP_ATTR_LASTNAME_TAG[] = "LDAP Last Name";
	const char	LDAP_ATTR_LASTNAME_INIT[] = "sn";

	const char	LDAP_ATTR_DISPLAYNAME_TAG[] = "LDAP Display Name";
	const char	LDAP_ATTR_DISPLAYNAME_INIT[] = "displayName";

	const char	LDAP_ATTR_DISTINGUISHEDNAME_TAG[] = "LDAP Distinguished Name";
	const char	LDAP_ATTR_DISTINGUISHEDNAME_INIT[] = "distinguishedName";

	const char	LDAP_ATTR_COMPANY_TAG[] = "LDAP Company";
	const char	LDAP_ATTR_COMPANY_INIT[] = "company";

	const char	LDAP_ATTR_GROUP_MEMBER_TAG[] = "LDAP Group Member";
	const char	LDAP_ATTR_GROUP_MEMBER_INIT[] = "member";

	const char	LDAP_ATTR_MEMBEROF_TAG[] = "LDAP memberOf";
	const char	LDAP_ATTR_MEMBEROF_INIT[] = "memberOf";
//	const char	LDAP_TRUECONF_MEMBEROF[] = "trueconf_memberOf";

	const char	LDAP_ATTR_PHONE_MOBILE_TAG[] = "LDAP Mobile Phone";
	const char	LDAP_ATTR_PHONE_MOBILE_INIT[] = "mobile";

	const char	LDAP_ATTR_PHONE_WORK_TAG[] = "LDAP Work Phone";
	const char	LDAP_ATTR_PHONE_WORK_INIT[] = "telephoneNumber";

	const char	LDAP_ATTR_PHONE_HOME_TAG[] = "LDAP Home Phone";
	const char	LDAP_ATTR_PHONE_HOME_INIT[] = "homePhone";

	const char	LDAP_ATTR_ADDRESS_BOOK_TAG[] = "LDAP AddressBook Attribute";
	const char	LDAP_ATTR_ADDRESS_BOOK_INIT[] = "addressBook";

	const char	LDAP_ATTR_SERVER_NAME_TAG[] = "LDAP ServerName Attribute";
	const char	LDAP_ATTR_SERVER_NAME_INIT[] = "serverName";

	const char	LDAP_ATTR_AVATARS_TAG[] = "LDAP Avatars Attr";
	const char	LDAP_ATTR_AVATARS_INIT[] = "thumbnailPhoto";

	const char	LDAP_ATTR_USER_ALIAS_LIST_TAG[] = "LDAP User Alias List";
	const char	LDAP_ATTR_USER_ALIAS_LIST_INIT[] = "";

	const char	LDAP_ATTR_DETAILED_USER_INFO_TAG[] = "LDAP DetailedUserInfo Attribute";
#ifdef _SVKS_M_BUILD_
	const char	LDAP_ATTR_DETAILED_USER_INFO_INIT[] = "patronymic,mvdDepartment,mvdCode,mvdHQCode,mvdPosition,ou";
#else
	const char	LDAP_ATTR_DETAILED_USER_INFO_INIT[] = "";
#endif

	///LDAP ad attrs
	const char	LDAP_ATTR_PRIMARY_GROUP_ID_TAG[] = "LDAP Attr primaryGroupId";
	const char	LDAP_ATTR_PRIMARY_GROUP_ID_INIT[] = "primaryGroupId";

	const char	LDAP_ATTR_PRIMARY_GROUP_TOKEN_TAG[] = "LDAP Attr primaryGroupToken";
	const char	LDAP_ATTR_PRIMARY_GROUP_TOKEN_INIT[] = "primaryGroupToken";

	const char	LDAP_ATTR_OBJECT_SID_TAG[] = "LDAP Attr objectSid";
	const char	LDAP_ATTR_OBJECT_SID_INIT[] = "objectSid";

	const char	LDAP_ATTR_GROUP_DISPLAY_NAME_TAG[] = "LDAP Group DisplayName";
	const char	LDAP_ATTR_GROUP_DISPLAY_NAME_INIT[] = "cn";

	/// static attribute lists
	//	char* tc::LDAPCore::m_al_dn[2] = { "dn", 0 };
	//char* VS_LDAPStorage::m_al_ad_primarygroup[2]={LDAP_ATTR_AD_GROUP_ID,0};

	//  search
	const char	LDAP_FILTER_ALL[] = "(objectClass=*)";

	const char	LDAP_FILTER_LOGIN_TAG[] = "LDAP Filter Login";
	const char	LDAP_FILTER_LOGIN_INIT[] = "(&(sAMAccountType=805306368)(sAMAccountName=%s))";

	const char	LDAP_FILTER_CALLID_TAG[] = "LDAP Filter CallID";
	const char	LDAP_FILTER_CALLID_INIT[] = "(&(sAMAccountType=805306368)(sAMAccountName=%S))";

	const char	LDAP_FILTER_AB_TAG[] = "LDAP Filter AddressBook";
	const char	LDAP_FILTER_AB_INIT[] = "";

	const char	LDAP_FILTER_GROUP_TAG[] = "LDAP Filter Group";
	const char	LDAP_FILTER_GROUP_INIT[] = "(objectClass=group)";

	const char	LDAP_FILTER_DISABLED_TAG[] = "LDAP Filter Disabled";
	const char	LDAP_FILTER_DISABLED_INIT[] = "(!(userAccountControl:1.2.840.113556.1.4.803:=2))";

	const char	LDAP_FILTER_PERSON_TAG[] = "LDAP Filter Person";
	const char	LDAP_FILTER_PERSON_INIT[] = "(objectClass=person)";

	const char	LDAP_GROUP_TAG[] = "LDAP Group";
#ifdef _SVKS_M_BUILD_
	const char	LDAP_REGION_TAG[] = "LDAP Region";
#endif

	const char			LDAP_GROUPS_UPDATE_PERIOD_TAG[] = "LDAP Groups Update Period";
	const unsigned long	LDAP_GROUPS_UPDATE_PERIOD_INIT = 60;		// in minutes

	const char		LDAP_FILTER_SEARCH_BY_LOGIN_GROUP_TAG[] = "LDAP FilterClientSearchByLoginGroup";

	///NTLM
	const char    NTLM_ON_TAG[] = "NTLM Authentication";

	const char	LDAP_AUTO_DETECT_TAG[] = "SrvSettingsAutoDetect";
	const bool	LDAP_AUTO_DETECT_INIT = true;

	const char	LDAP_ATTR_USER_STATUS_TAG[] = "LDAP User Status Attr";
	const char	LDAP_ATTR_USER_STATUS_INIT[] = "";

	const char	LDAP_ATTR_USER_ID_TAG[] = "LDAP User ID Attr";
	const char	LDAP_ATTR_USER_ID_INIT[] = "";

	const char	LDAP_ATTR_FULL_ID_TAG[] = "LDAP Full ID Attr";
	const char	LDAP_ATTR_FULL_ID_INIT[] = "";

	const char	LDAP_ENABLE_TRUST_TAG[] = "LDAP Trust Enabled";

	const char	LDAP_FILTER_TRUSTED_DOMAIN_TAG[] = "LDAP TrustedDomain Filter";
	const char	LDAP_FILTER_TRUSTED_DOMAIN_INIT[] = "(objectClass=trustedDomain)";

	const char	LDAP_ATTR_TRUST_PARTNER_TAG[] = "LDAP TrustPartner Attr";
	const char	LDAP_ATTR_TRUST_PARTNER_INIT[] = "trustPartner";

	const char	LDAP_ATTR_FLAT_NAME_TAG[] = "LDAP FlatName Attr";
	const char	LDAP_ATTR_FLAT_NAME_INIT[] = "flatName";

	const char	LDAP_FILTER_FOREIGN_SECURITY_PRINCIPAL_TAG[] = "LDAP ForeignSecurityPrincipal Filter";
	const char	LDAP_FILTER_FOREIGN_SECURITY_PRINCIPAL_INIT[] = "(objectClass=foreignSecurityPrincipal)";

	const char		LDAP_AUTOMANAGEAB_MAXUSERS_TAG[] = "LDAP AutoManageAB MaxUsers";
	const unsigned long	LDAP_AUTOMANAGEAB_MAXUSERS_INIT = 1000;

	const char		LDAP_REFERRAL_TAG[]			= "LDAP Referral Enabled";
	const char		LDAP_REFERRAL_HOPS_TAG[]	= "LDAP Referral Hops";
	const char		LDAP_REFERRAL_SKIP_TAG[]	= "LDAP Skip BaseDN";
	const std::vector<std::string> DEFAULT_SKIP_REFERRALS = { "DC=ForestDnsZones",
																"DC=DomainDnsZones",
																"CN=Configuration",
																"CN=Schema" };

	enum AuthMethods
	{
		VS_LDAP_UNKNOWN = -1,
		VS_LDAP_AUTH_SIMPLE = 1,
		VS_LDAP_AUTH_NTLM,
		VS_LDAP_AUTH_NTLM_CURRENTUSER,
		VS_LDAP_AUTH_GSS,
		VS_LDAP_AUTH_DIGEST_MD5,
		VS_LDAP_AUTH_DEFAULT = VS_LDAP_AUTH_NTLM_CURRENTUSER,
	};

	const char LDAP_SORT_CONTROL_SCHEME_TAG[] = "LDAP SortControlScheme";
	enum class SortControlScheme : int8_t
	{
		INVALID = -1,
		NO      = 0,
		YES     = 1,
		AUTO    = 2,
	};

}  // namespace tc
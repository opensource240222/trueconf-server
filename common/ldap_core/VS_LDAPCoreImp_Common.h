#pragma once

#include "VS_LDAPConst.h"
#include "std/cpplib/VS_Lock.h"
#include "std/cpplib/VS_ThreadPool.h"
#include "std-generic/cpplib/synchronized.h"
#include "std-generic/cpplib/string_view.h"

namespace tc {

std::string ConvectSIDtoString(string_view in_bytes);

class VS_LDAPCoreImp_Common
{
protected:
	VS_LDAPCoreImp_Common()
		: m_referrals_enabled(true)
		, m_referrals_hops(0)
		, m_ldap_port(0)
		, m_ldap_secure(false)
		, m_ldap_version(0)
		, m_auth_method(VS_LDAP_AUTH_DEFAULT)
	{}

	std::string		m_auth_user;
	std::string		m_auth_domain;
	std::string		m_auth_password;

	std::chrono::seconds m_ldap_timeout;
	bool m_referrals_enabled;
	int8_t m_referrals_hops;
	bool m_is_server_sort_control_supported = true;
	bool m_ldap_auto_verify_server_cert = false;

	virtual bool isUselessDN(const std::string &dn) const = 0;

	vs::Synchronized<std::map<std::pair<std::string, unsigned long>, LDAP*>>	m_ldap_refferal;	// [key=host:port, val=ldap-struct]

public:
	virtual void InitLib() = 0;

	std::string		m_ldap_server;
	unsigned short	m_ldap_port;
	uint32_t		m_max_results;
	int				m_ldap_secure;
	uint32_t		m_ldap_version;
	AuthMethods		m_auth_method;

	std::string		m_a_distinguishedName;
	std::string		m_a_objectSid;

};

} // namespace tc
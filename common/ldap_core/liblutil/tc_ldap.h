#pragma once

#if defined(_WIN32)
#	include <Windows.h>
#	include <Winldap.h>
#else
#	define LDAP_DEPRECATED 1
#	include <ldap.h>
#endif
#ifdef _WIN32
#define ldap_unbind_ext(ctx, ignore1, ignore2) ldap_unbind(ctx)
#define ldap_compare_ext_s(ld, dn, attr, bvalue, sctrls, cctrls) ldap_compare_ext_s(ld, dn, attr, nullptr, bvalue, sctrls, cctrls)
#endif

#include <string>
#include "ldap_core/VS_LDAPConst.h"

namespace tc_ldap {
	LDAP* Connect(const std::string& name, short port, bool secure);
	unsigned long Bind(LDAP* ctx, const std::string& user, const std::string& password, const std::string& domain, tc::AuthMethods method);
}
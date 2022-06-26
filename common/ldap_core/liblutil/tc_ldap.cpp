#include "tc_ldap.h"
#include "std-generic/cpplib/utf8.h"
#include "std-generic/cpplib/scope_exit.h"

#include "std/debuglog/VS_Debug.h"

#ifndef _WIN32
#include "ldap_core/liblutil/lutil_ldap.h"
#endif // !_WIN32

#define DEBUG_CURRENT_MODULE VS_DM_RSTOR

LDAP * tc_ldap::Connect(const std::string & name, short port, bool secure)
{
	LDAP * ctx = nullptr;
#ifdef _WIN32
	auto wName = vs::UTF8ToWideCharConvert(name);
	ctx = ldap_sslinitW((wchar_t*)wName.c_str(), port, secure ? 1 : 0);
	if (ctx != nullptr)
	{
		l_timeval conn_timeout;
		conn_timeout.tv_sec = 5;
		conn_timeout.tv_usec = 0;
		(void)ldap_connect(ctx, &conn_timeout);
	}
#else
	std::string connStr = secure ? "ldaps://" : "ldap://";
	connStr += name; connStr += ':';
	connStr += std::to_string(port);
	connStr += '/';
	auto err = ldap_initialize(&ctx, connStr.c_str());
	if (err != LDAP_SUCCESS)
		return nullptr;
#endif
	return ctx;
}

#ifdef _WIN32
unsigned long tc_ldap::Bind(LDAP* ctx, const std::string & user, const std::string & password, const std::string & domain, tc::AuthMethods method) {
	if (!ctx)
		return LDAP_PARAM_ERROR;
	auto w_domain = vs::UTF8ToWideCharConvert(domain);
	auto w_user = vs::UTF8ToWideCharConvert(user);
	auto w_password = vs::UTF8ToWideCharConvert(password);

	SEC_WINNT_AUTH_IDENTITY_W id;
	id.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
	id.Domain = (unsigned short*)((wchar_t *)w_domain.c_str());
	id.DomainLength = w_domain.length();
	id.User = (unsigned short*)((wchar_t *)w_user.c_str());
	id.UserLength = w_user.length();
	id.Password = (unsigned short*)((wchar_t *)w_password.c_str());
	id.PasswordLength = w_password.length();

	unsigned long ldresult = LDAP_NOT_SUPPORTED;
	auto ds = dstream3;
	ds << "ldap_bind user=" << user << ", domain=" << domain << " with ";
	switch (method)
	{
	case tc::AuthMethods::VS_LDAP_AUTH_SIMPLE:
		ds << "simple auth";
		ldresult = ldap_bind_sW(ctx, (PWCHAR)w_user.c_str(), (PWCHAR)w_password.c_str(), LDAP_AUTH_SIMPLE);
		break;
	case tc::AuthMethods::VS_LDAP_AUTH_NTLM_CURRENTUSER:
		ds << "NT current user";
		ldresult = ldap_bind_sW(ctx, NULL, NULL, LDAP_AUTH_NTLM);
		break;
	case tc::AuthMethods::VS_LDAP_AUTH_NTLM:
		ds << "NT auth";
		ldresult = ldap_bind_sW(ctx, NULL, (wchar_t*)&id, LDAP_AUTH_NTLM);
		break;
	case tc::AuthMethods::VS_LDAP_AUTH_GSS:
		ds << "GSS auth";
		ldresult = ldap_bind_sW(ctx, NULL, (wchar_t*)&id, LDAP_AUTH_NEGOTIATE);
		break;
	case tc::AuthMethods::VS_LDAP_AUTH_DIGEST_MD5:
		ds << "DIGEST-MD5 auth; NOT IMPLEMENTED";
		break;
	}
	ds << " ldap_res:" << std::to_string(ldresult);
	return ldresult;
}
#else
unsigned long tc_ldap::Bind(LDAP* ctx, const std::string & user, const std::string & password, const std::string & domain, tc::AuthMethods method)
{
	if (!ctx)
		return LDAP_PARAM_ERROR;

	auto defaults = lutil_sasl_defaults(ctx,
		"DIGEST-MD5",
		(char*)domain.c_str()/*sasl_realm*/,
		(char*)user.c_str()/*sasl_authc_id*/,
		(char*)password.c_str()/*passwd.bv_val*/,
		(char*)user.c_str()/*sasl_authz_id*/);
	VS_SCOPE_EXIT{ lutil_sasl_freedefs(defaults); };
	struct berval pass_ber = { password.length(), (char*)password.c_str() };

	unsigned long ldresult = LDAP_NOT_SUPPORTED;
	timeval conn_timeout;
	conn_timeout.tv_sec = 5;
	conn_timeout.tv_usec = 0;
	ldresult = ldap_set_option(ctx, LDAP_OPT_NETWORK_TIMEOUT, &conn_timeout);
	if (ldresult != LDAP_OPT_SUCCESS)
		return ldresult;
	auto ds = dstream3;
	ds << "ldap_bind user=" << user << ", domain=" << domain << " with ";
	switch (method)
	{
	case tc::AuthMethods::VS_LDAP_AUTH_SIMPLE:
		ds << "simple auth";
		ldresult = ldap_sasl_bind_s(ctx, user.c_str(), LDAP_SASL_SIMPLE, &pass_ber, NULL, NULL, NULL);
		break;
	case tc::AuthMethods::VS_LDAP_AUTH_NTLM_CURRENTUSER:
		ds << "NT current user; NOT IMPLEMENTED";
		//ldresult = ldap_bind_s(ctx, NULL, NULL, LDAP_AUTH_NTLM);
		break;
	case tc::AuthMethods::VS_LDAP_AUTH_NTLM:
		ds << "NT auth";
		ldresult = ldap_sasl_interactive_bind_s(ctx, nullptr, "NTLM", "GSS-SPNEGO",
			nullptr/*sctrlsp*/, nullptr, 0/*sasl_flags*/, lutil_sasl_interact, defaults);
		break;
	case tc::AuthMethods::VS_LDAP_AUTH_GSS:
		ds << "GSS auth; NOT IMPLEMENTED";
		//ldresult = ldap_bind_s(ctx, NULL, (wchar_t*)&id, LDAP_AUTH_NEGOTIATE);
		break;
	case tc::AuthMethods::VS_LDAP_AUTH_DIGEST_MD5:
		ds << "DIGEST-MD5 auth";
		ldresult = ldap_sasl_interactive_bind_s(ctx, nullptr, "DIGEST-MD5", nullptr,
			nullptr/*sctrlsp*/, nullptr, 0/*sasl_flags*/, lutil_sasl_interact, defaults);
		break;
	default:
		dprint3("CheckPass: access denied: invalid auth_method\n");
		return LDAP_NOT_SUPPORTED;
		break;
	}
	ds << " ldap_res:" << std::to_string(ldresult);
	return ldresult;
}
#endif
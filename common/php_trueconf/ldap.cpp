/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2012 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Amitay Isaacs  <amitay@w-o-i.com>                           |
   |          Eric Warnke    <ericw@albany.edu>                           |
   |          Rasmus Lerdorf <rasmus@php.net>                             |
   |          Gerrit Thomson <334647@swin.edu.au>                         |
   |          Jani Taskinen  <sniper@iki.fi>                              |
   |          Stig Venaas    <venaas@uninett.no>                          |
   |          Doug Goldstein <cardoe@cardoe.com>                          |
   | PHP 4.0 updates:  Zeev Suraski <zeev@zend.com>                       |
   +----------------------------------------------------------------------+
 */
/* $Id$ */
#define IS_EXT_MODULE

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Additional headers for NetWare */
#if defined(NETWARE) && (NEW_LIBC)
#include <sys/select.h>
#include <sys/timeval.h>
#endif

#include "common.h"

#include "php.h"
#include "php_ini.h"
extern "C" {
#include "ext/standard/php_string.h"
}
#include "ext/standard/info.h"

#include "php_ldap.h"
#include "vs_zend_parse_parameters.h"

#include "ldap_core/liblutil/tc_ldap.h"
#include "std-generic/attributes.h"
#include "std-generic/cpplib/utf8.h"
#include "std-generic/cpplib/scope_exit.h"

#include "std/debuglog/VS_Debug.h"

#ifdef PHP_WIN32
#include <string.h>
#include "config.w32.h"
#if HAVE_NSLDAP
#include <winsock2.h>
#endif
#ifndef strdup
#define strdup _strdup
#endif
#undef WINDOWS
#undef strcasecmp
#undef strncasecmp
#define WINSOCK 1
#endif

#undef HAVE_LDAP_SASL
#undef HAVE_LDAP_SASL_H
#undef HAVE_LDAP_SASL_SASL_H
#define LDAP_OPT_SERVER_CONTROLS	0x00174069 // magic 1
#define LDAP_OPT_CLIENT_CONTROLS	0x00142569 // magic 2
#define LDAP_AUTH_CURRENT_USER		0x019f3434 // magic 3

#ifdef _WIN32
#include <Winldap.h>
#include <Winber.h>
#include <Rpc.h>
#endif

#ifdef HAVE_LDAP_SASL_H
#include <sasl.h>
#elif defined(HAVE_LDAP_SASL_SASL_H)
#include <sasl/sasl.h>
#endif

#define DEBUG_CURRENT_MODULE VS_DM_PHP

typedef struct {
	LDAP *link;
	zval rebindproc;
} ldap_linkdata;

typedef struct {
	LDAPMessage *data;
	BerElement  *ber;
	zval         res;
} ldap_resultentry;

static int le_link, le_result, le_result_entry;
static ldap_linkdata *global_ld = NULL;

#ifdef COMPILE_DL_LDAP
ZEND_GET_MODULE(ldap)
#endif

static void _close_ldap_link(zend_resource *rsrc TSRMLS_DC) /* {{{ */
{
	ldap_linkdata *ld = (ldap_linkdata *)rsrc->ptr;

	if (ld == global_ld)
	{
		global_ld = NULL;
	}

	ldap_unbind_ext(ld->link, nullptr, nullptr);
#if defined(LDAP_API_FEATURE_X_OPENLDAP) && defined(HAVE_3ARG_SETREBINDPROC)
	if (ld->rebindproc != NULL) {
		zval_dtor(ld->rebindproc);
		FREE_ZVAL(ld->rebindproc);
	}
#endif
	efree(ld);
}
/* }}} */

static void _free_ldap_result(zend_resource *rsrc TSRMLS_DC) /* {{{ */
{
	LDAPMessage *result = (LDAPMessage *)rsrc->ptr;
	ldap_msgfree(result);
}
/* }}} */

static void _free_ldap_result_entry(zend_resource *rsrc TSRMLS_DC) /* {{{ */
{
	ldap_resultentry *entry = (ldap_resultentry *)rsrc->ptr;

	if (entry->ber != NULL) {
		ber_free(entry->ber, 0);
		entry->ber = NULL;
	}
	zend_list_delete(rsrc);
	efree(entry);
}
/* }}} */

// save windows declarations for methods that are supported in openldap
#ifndef _WIN32
#define LDAP_AUTH_SIMPLE                0x80L
#define LDAP_AUTH_OTHERKIND             0x86L
#define LDAP_AUTH_NEGOTIATE             (LDAP_AUTH_OTHERKIND | 0x0400)
#define LDAP_AUTH_NTLM                  (LDAP_AUTH_OTHERKIND | 0x1000)
#define LDAP_AUTH_DIGEST                (LDAP_AUTH_OTHERKIND | 0x4000)
#endif // !_WIN32

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(ldap)
{
	/* Constants to be used with deref-parameter in php_ldap_do_search() */
	REGISTER_LONG_CONSTANT("LDAP_DEREF_NEVER", LDAP_DEREF_NEVER, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_DEREF_SEARCHING", LDAP_DEREF_SEARCHING, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_DEREF_FINDING", LDAP_DEREF_FINDING, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_DEREF_ALWAYS", LDAP_DEREF_ALWAYS, CONST_PERSISTENT | CONST_CS);

	REGISTER_LONG_CONSTANT("LDAP_AUTH_SIMPLE",		LDAP_AUTH_SIMPLE,	CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_AUTH_DIGEST",		LDAP_AUTH_DIGEST,	CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_AUTH_NEGOTIATE", LDAP_AUTH_NEGOTIATE, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_AUTH_NTLM", LDAP_AUTH_NTLM, CONST_PERSISTENT | CONST_CS);
#ifdef _WIN32
	REGISTER_LONG_CONSTANT("LDAP_AUTH_DPA",			LDAP_AUTH_DPA,		CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_AUTH_MSN",			LDAP_AUTH_MSN,		CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_AUTH_SICILY",		LDAP_AUTH_SICILY,	CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_AUTH_SSPI",		LDAP_AUTH_SSPI,		CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_AUTH_CURRENT_USER",LDAP_AUTH_CURRENT_USER,	CONST_PERSISTENT | CONST_CS);
#endif

#if (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10
	/* LDAP options */
	REGISTER_LONG_CONSTANT("LDAP_OPT_DEREF", LDAP_OPT_DEREF, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_OPT_SIZELIMIT", LDAP_OPT_SIZELIMIT, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_OPT_TIMELIMIT", LDAP_OPT_TIMELIMIT, CONST_PERSISTENT | CONST_CS);
#ifdef _WIN32
	REGISTER_LONG_CONSTANT("LDAP_OPT_REFERRAL_HOP_LIMIT", LDAP_OPT_REFERRAL_HOP_LIMIT, CONST_PERSISTENT | CONST_CS);
#else
	REGISTER_LONG_CONSTANT("LDAP_OPT_REFERRAL_HOP_LIMIT", LDAP_OPT_REFHOPLIMIT, CONST_PERSISTENT | CONST_CS);
#endif
#ifdef LDAP_OPT_NETWORK_TIMEOUT
	REGISTER_LONG_CONSTANT("LDAP_OPT_NETWORK_TIMEOUT", LDAP_OPT_NETWORK_TIMEOUT, CONST_PERSISTENT | CONST_CS);
#elif defined (LDAP_X_OPT_CONNECT_TIMEOUT)
	REGISTER_LONG_CONSTANT("LDAP_OPT_NETWORK_TIMEOUT", LDAP_X_OPT_CONNECT_TIMEOUT, CONST_PERSISTENT | CONST_CS);
#endif
	REGISTER_LONG_CONSTANT("LDAP_OPT_PROTOCOL_VERSION", LDAP_OPT_PROTOCOL_VERSION, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_OPT_ERROR_NUMBER", LDAP_OPT_ERROR_NUMBER, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_OPT_REFERRALS", LDAP_OPT_REFERRALS, CONST_PERSISTENT | CONST_CS);
#ifdef LDAP_OPT_RESTART
	REGISTER_LONG_CONSTANT("LDAP_OPT_RESTART", LDAP_OPT_RESTART, CONST_PERSISTENT | CONST_CS);
#endif
#ifdef LDAP_OPT_HOST_NAME
	REGISTER_LONG_CONSTANT("LDAP_OPT_HOST_NAME", LDAP_OPT_HOST_NAME, CONST_PERSISTENT | CONST_CS);
#endif
	REGISTER_LONG_CONSTANT("LDAP_OPT_ERROR_STRING", LDAP_OPT_ERROR_STRING, CONST_PERSISTENT | CONST_CS);
#ifdef LDAP_OPT_MATCHED_DN
	REGISTER_LONG_CONSTANT("LDAP_OPT_MATCHED_DN", LDAP_OPT_MATCHED_DN, CONST_PERSISTENT | CONST_CS);
#endif
	REGISTER_LONG_CONSTANT("LDAP_OPT_SERVER_CONTROLS", LDAP_OPT_SERVER_CONTROLS, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_OPT_CLIENT_CONTROLS", LDAP_OPT_CLIENT_CONTROLS, CONST_PERSISTENT | CONST_CS);
#endif
#ifdef LDAP_OPT_DEBUG_LEVEL
	REGISTER_LONG_CONSTANT("LDAP_OPT_DEBUG_LEVEL", LDAP_OPT_DEBUG_LEVEL, CONST_PERSISTENT | CONST_CS);
#endif

#ifdef HAVE_LDAP_SASL
	REGISTER_LONG_CONSTANT("LDAP_OPT_X_SASL_MECH", LDAP_OPT_X_SASL_MECH, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_OPT_X_SASL_REALM", LDAP_OPT_X_SASL_REALM, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_OPT_X_SASL_AUTHCID", LDAP_OPT_X_SASL_AUTHCID, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("LDAP_OPT_X_SASL_AUTHZID", LDAP_OPT_X_SASL_AUTHZID, CONST_PERSISTENT | CONST_CS);
#endif

#ifdef ORALDAP
	REGISTER_LONG_CONSTANT("GSLC_SSL_NO_AUTH", GSLC_SSL_NO_AUTH, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("GSLC_SSL_ONEWAY_AUTH", GSLC_SSL_ONEWAY_AUTH, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("GSLC_SSL_TWOWAY_AUTH", GSLC_SSL_TWOWAY_AUTH, CONST_PERSISTENT | CONST_CS);
#endif

	le_link = zend_register_list_destructors_ex(_close_ldap_link, NULL, "ldap link", module_number);
	le_result = zend_register_list_destructors_ex(_free_ldap_result, NULL, "ldap result", module_number);
	le_result_entry = zend_register_list_destructors_ex(_free_ldap_result_entry, NULL, "ldap result entry", module_number);

	ldap_module_entry.type = type;

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(ldap)
{
	dprint2("Metamodule RINIT_FUNCTION called\n");
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(ldap)
{
	char tmp[32];

	php_info_print_table_start();
	php_info_print_table_row(2, "LDAP Support", "enabled");

#ifdef LDAP_API_VERSION
	snprintf(tmp, 31, "%d", LDAP_API_VERSION);
	php_info_print_table_row(2, "API Version", tmp);
#endif

#ifdef HAVE_LDAP_SASL
	php_info_print_table_row(2, "SASL Support", "Enabled");
#endif

	php_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ proto resource ldap_connect([string host [, int port [, string wallet [, string wallet_passwd [, int authmode]]]]])
   Connect to an LDAP server */
PHP_FUNCTION(ldap_connect)
{
	char *host = NULL;
	size_t hostlen;
	zend_long port = 0 ;
	zend_bool use_ssl = 0;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|slb", &host, &hostlen, &port, &use_ssl) != SUCCESS) {
		dprint1("ldap_connect called with wrong arguments!\n");
		RETURN_FALSE;
	}
	if (!host) {
		dprint1("ldap_connect: Error, no host!\n");
		RETURN_FALSE;
	}

	LDAP *ldap = tc_ldap::Connect(host, port, use_ssl);
	if (!ldap) {
		dprint1("ldap_connect failed. host=%s, port=%d, use_ssl=%d\n", host, (int32_t)port, use_ssl);
		RETURN_FALSE;
	}
	else
	{
		dprint2("ldap_connect to %s:%d (ssl=%d) success\n", host, (int32_t)port, use_ssl);
		ldap_linkdata *ld;
		ld = (ldap_linkdata*)ecalloc(1, sizeof(ldap_linkdata));
		ld->link = ldap;
		RETURN_RES(zend_register_resource(ld, le_link));
	}
}

/* }}} */

/* {{{ _get_lderrno
 */
static int _get_lderrno(LDAP *ldap)
{
#if !HAVE_NSLDAP
#if LDAP_API_VERSION > 2000 || HAVE_ORALDAP_10
	int lderr;

	/* New versions of OpenLDAP do it this way */
	ldap_get_option(ldap, LDAP_OPT_ERROR_NUMBER, &lderr);
	return lderr;
#else
	return ldap->ld_errno;
#endif
#else
	return ldap_get_lderrno(ldap, NULL, NULL);
#endif
}
/* }}} */

tc::AuthMethods ConvertMethod(int method) {
	switch (method)
	{
	case LDAP_AUTH_SIMPLE: return tc::AuthMethods::VS_LDAP_AUTH_SIMPLE;
	case LDAP_AUTH_NEGOTIATE: return tc::AuthMethods::VS_LDAP_AUTH_GSS;
	case LDAP_AUTH_NTLM: return tc::AuthMethods::VS_LDAP_AUTH_NTLM;
	case LDAP_AUTH_DIGEST: return tc::AuthMethods::VS_LDAP_AUTH_DIGEST_MD5;
	default:
		break;
	}
	return tc::AuthMethods::VS_LDAP_UNKNOWN;
}

/* {{{ proto bool ldap_bind(resource link [, string dn [, string password [, int method ]])
   Bind to LDAP directory */
PHP_FUNCTION(ldap_bind)
{
	zval *link;
	ldap_linkdata *ld;
	int rc = -1;
	zend_long method = -1;

	char* user = 0;
	char* pass = 0;
	char* domain = 0;
	size_t user_len = 0;
	size_t pass_len = 0;
	size_t domain_len = 0;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl|sss", &link, &method,
		&user, &user_len,
		&pass, &pass_len,
		&domain, &domain_len
		) != SUCCESS) {
		dprint1("ldap_bind called with wrong arguments!\n");
		RETURN_FALSE;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_bind can't fetch resourse!\n");
		RETURN_FALSE;
	}
	if (_get_lderrno(ld->link) != LDAP_SUCCESS) {
		dprint1("ldap_bind wont bind - incorrect ctx(context)");
		RETURN_FALSE;
	}
	auto meth = ConvertMethod(method);
	if (meth == tc::AuthMethods::VS_LDAP_UNKNOWN) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "method is not supported");
		RETURN_FALSE;
	}

	uint32_t ldapVersion = 3;
	rc = ldap_set_option(ld->link, LDAP_OPT_PROTOCOL_VERSION, (void*)&ldapVersion);
	rc = tc_ldap::Bind(ld->link, std::string(user, user_len), std::string(pass, pass_len), std::string(domain, domain_len), meth);
	if (rc != LDAP_SUCCESS)
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to bind to server: %s", ldap_err2string(rc));
		RETURN_FALSE;
	} else {
		dprint2("ldap_bind done successfully.\n");
		RETURN_TRUE;
	}
}
/* }}} */

#ifdef HAVE_LDAP_SASL
typedef struct {
	char *mech;
	char *realm;
	char *authcid;
	char *passwd;
	char *authzid;
} php_ldap_bictx;

/* {{{ _php_sasl_setdefs
 */
static php_ldap_bictx *_php_sasl_setdefs(LDAP *ld, char *sasl_mech, char *sasl_realm, char *sasl_authc_id, char *passwd, char *sasl_authz_id)
{
	php_ldap_bictx *ctx;

	ctx = ber_memalloc(sizeof(php_ldap_bictx));
	ctx->mech    = (sasl_mech) ? ber_strdup(sasl_mech) : NULL;
	ctx->realm   = (sasl_realm) ? ber_strdup(sasl_realm) : NULL;
	ctx->authcid = (sasl_authc_id) ? ber_strdup(sasl_authc_id) : NULL;
	ctx->passwd  = (passwd) ? ber_strdup(passwd) : NULL;
	ctx->authzid = (sasl_authz_id) ? ber_strdup(sasl_authz_id) : NULL;

	if (ctx->mech == NULL) {
		ldap_get_option(ld, LDAP_OPT_X_SASL_MECH, &ctx->mech);
	}
	if (ctx->realm == NULL) {
		ldap_get_option(ld, LDAP_OPT_X_SASL_REALM, &ctx->realm);
	}
	if (ctx->authcid == NULL) {
		ldap_get_option(ld, LDAP_OPT_X_SASL_AUTHCID, &ctx->authcid);
	}
	if (ctx->authzid == NULL) {
		ldap_get_option(ld, LDAP_OPT_X_SASL_AUTHZID, &ctx->authzid);
	}

	return ctx;
}
/* }}} */

/* {{{ _php_sasl_freedefs
 */
static void _php_sasl_freedefs(php_ldap_bictx *ctx)
{
	if (ctx->mech) ber_memfree(ctx->mech);
	if (ctx->realm) ber_memfree(ctx->realm);
	if (ctx->authcid) ber_memfree(ctx->authcid);
	if (ctx->passwd) ber_memfree(ctx->passwd);
	if (ctx->authzid) ber_memfree(ctx->authzid);
	ber_memfree(ctx);
}
/* }}} */

/* {{{ _php_sasl_interact
   Internal interact function for SASL */
static int _php_sasl_interact(LDAP *ld, unsigned flags, void *defaults, void *in)
{
	sasl_interact_t *interact = in;
	const char *p;
	php_ldap_bictx *ctx = defaults;

	for (;interact->id != SASL_CB_LIST_END;interact++) {
		p = NULL;
		switch(interact->id) {
			case SASL_CB_GETREALM:
				p = ctx->realm;
				break;
			case SASL_CB_AUTHNAME:
				p = ctx->authcid;
				break;
			case SASL_CB_USER:
				p = ctx->authzid;
				break;
			case SASL_CB_PASS:
				p = ctx->passwd;
				break;
		}
		if (p) {
			interact->result = p;
			interact->len = strlen(interact->result);
		}
	}
	return LDAP_SUCCESS;
}
/* }}} */

/* {{{ proto bool ldap_sasl_bind(resource link [, string binddn [, string password [, string sasl_mech [, string sasl_realm [, string sasl_authc_id [, string sasl_authz_id [, string props]]]]]]])
   Bind to LDAP directory using SASL */
PHP_FUNCTION(ldap_sasl_bind)
{
	zval *link;
	ldap_linkdata *ld;
	char *binddn = NULL;
	char *passwd = NULL;
	char *sasl_mech = NULL;
	char *sasl_realm = NULL;
	char *sasl_authz_id = NULL;
	char *sasl_authc_id = NULL;
	char *props = NULL;
	size_t rc(0), dn_len(0), passwd_len(0), mech_len(0), realm_len(0), authc_id_len(0), authz_id_len(0), props_len(0);
	php_ldap_bictx *ctx;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|sssssss", &link, &binddn, &dn_len, &passwd, &passwd_len, &sasl_mech, &mech_len, &sasl_realm, &realm_len, &sasl_authc_id, &authc_id_len, &sasl_authz_id, &authz_id_len, &props, &props_len) != SUCCESS) {
		RETURN_FALSE;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		RETURN_FALSE;
	}

	ctx = _php_sasl_setdefs(ld->link, sasl_mech, sasl_realm, sasl_authc_id, passwd, sasl_authz_id);

	if (props) {
		ldap_set_option(ld->link, LDAP_OPT_X_SASL_SECPROPS, props);
	}

	rc = ldap_sasl_interactive_bind_s(ld->link, binddn, ctx->mech, NULL, NULL, LDAP_SASL_QUIET, _php_sasl_interact, ctx);
	if (rc != LDAP_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to bind to server: %s", ldap_err2string(rc));
		RETVAL_FALSE;
	} else {
		RETVAL_TRUE;
	}
	_php_sasl_freedefs(ctx);
}
/* }}} */
#endif /* HAVE_LDAP_SASL */

/* {{{ proto bool ldap_unbind(resource link)
   Unbind from LDAP directory */
PHP_FUNCTION(ldap_unbind)
{
	zval *link;
	ldap_linkdata *ld;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &link) != SUCCESS) {
		dprint1("ldap_unbind called with wrong arguments!\n");
		RETURN_FALSE;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_unbind can't fetch LDAP ctx!\n");
		RETURN_FALSE;
	}

	zend_list_delete(Z_RES_P(link));
	dprint2("ldap_unbind done successfully.\n");
	RETURN_TRUE;
}
/* }}} */

/* {{{ php_set_opts
 */
static void php_set_opts(LDAP *ldap, int sizelimit, int timelimit, int deref, int *old_sizelimit, int *old_timelimit, int *old_deref)
{
	/* sizelimit */
	if (sizelimit > -1) {
#if (LDAP_API_VERSION >= 2004) || HAVE_NSLDAP || HAVE_ORALDAP_10
		ldap_get_option(ldap, LDAP_OPT_SIZELIMIT, old_sizelimit);
		ldap_set_option(ldap, LDAP_OPT_SIZELIMIT, &sizelimit);
#else
		*old_sizelimit = ldap->ld_sizelimit;
		ldap->ld_sizelimit = sizelimit;
#endif
	}

	/* timelimit */
	if (timelimit > -1) {
#if (LDAP_API_VERSION >= 2004) || HAVE_NSLDAP || HAVE_ORALDAP_10
		ldap_get_option(ldap, LDAP_OPT_SIZELIMIT, old_timelimit);
		ldap_set_option(ldap, LDAP_OPT_TIMELIMIT, &timelimit);
#else
		*old_timelimit = ldap->ld_timelimit;
		ldap->ld_timelimit = timelimit;
#endif
	}

	/* deref */
	if (deref > -1) {
#if (LDAP_API_VERSION >= 2004) || HAVE_NSLDAP || HAVE_ORALDAP_10
		ldap_get_option(ldap, LDAP_OPT_SIZELIMIT, old_deref);
		ldap_set_option(ldap, LDAP_OPT_DEREF, &deref);
#else
		*old_deref = ldap->ld_deref;
		ldap->ld_deref = deref;
#endif
	}
}
/* }}} */

bool UTF8ToWCharArr(char** arr, size_t len, std::vector<std::wstring> &converted) {
	if (!arr || len == 0)
		return false;

	converted.reserve(len);
	for (size_t i = 0; i < len; ++i)
	{
		if (!arr[i])
			continue;
		converted.emplace_back(vs::UTF8ToWideCharConvert(arr[i]));
	}
	return true;
}

void MakeAttrsArray(const std::vector<std::wstring> &attrs, std::vector<wchar_t *> attrsArray) {
	attrsArray.reserve(attrs.size() + 1);
	for (auto& attr : attrs){
		attrsArray.emplace_back((wchar_t *)attr.c_str());
	}
	attrsArray.emplace_back(nullptr);
}

/* {{{ php_ldap_do_search
 */
static void php_ldap_do_search(INTERNAL_FUNCTION_PARAMETERS, int scope)
{
	zval *link, *base_dn, *filter, *attrs = NULL, *attr;
	zend_long attrsonly = 0;
	zend_long sizelimit = 0, timelimit = 0, deref = 0;
	char *ldap_base_dn = NULL, *ldap_filter = NULL, **ldap_attrs = NULL;
	ldap_linkdata *ld = NULL;
	LDAPMessage *ldap_res;
	int ldap_attrsonly = 0, ldap_sizelimit = -1, ldap_timelimit = -1, ldap_deref = -1;
	int old_ldap_sizelimit = -1, old_ldap_timelimit = -1, old_ldap_deref = -1;
	int num_attribs = 0, ret = 1, i, err, argcount = ZEND_NUM_ARGS();

	if (vs_zend_parse_parameters(argcount TSRMLS_CC, "zzz|allll", &link, &base_dn, &filter, &attrs, &attrsonly,
		&sizelimit, &timelimit, &deref) == FAILURE) {
		dprint1("php_ldap_do_search called with wrong arguments!\n");
		return;
	}

	/* Reverse -> fall through */
	switch (argcount) {
		case 8:
			ldap_deref = deref;
			VS_FALLTHROUGH;
		case 7:
			ldap_timelimit = timelimit;
			VS_FALLTHROUGH;
		case 6:
			ldap_sizelimit = sizelimit;
			VS_FALLTHROUGH;
		case 5:
			ldap_attrsonly = attrsonly;
			VS_FALLTHROUGH;
		case 4:
			num_attribs = zend_hash_num_elements(Z_ARRVAL_P(attrs));
			ldap_attrs = (char**)safe_emalloc((num_attribs+1), sizeof(char *), 0);

			for (i = 0; i<num_attribs; i++) {
				if ((attr = zend_hash_index_find(Z_ARRVAL_P(attrs), i)) == NULL) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Array initialization wrong");
					ret = 0;
					goto cleanup;
				}

				SEPARATE_ZVAL(attr);
				convert_to_string_ex(attr);
				ldap_attrs[i] = Z_STRVAL_P(attr);
			}
			ldap_attrs[num_attribs] = NULL;
	}

	/* parallel search? */
	if (Z_TYPE_P(link) == IS_ARRAY) {
		int i, nlinks, nbases, nfilters, *rcs;
		ldap_linkdata **lds;
		zval *entry, resource;

		nlinks = zend_hash_num_elements(Z_ARRVAL_P(link));
		if (nlinks == 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "No links in link array");
			ret = 0;
			goto cleanup;
		}

		if (Z_TYPE_P(base_dn) == IS_ARRAY) {
			nbases = zend_hash_num_elements(Z_ARRVAL_P(base_dn));
			if (nbases != nlinks) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Base must either be a string, or an array with the same number of elements as the links array");
				ret = 0;
				goto cleanup;
			}
			zend_hash_internal_pointer_reset(Z_ARRVAL_P(base_dn));
		} else {
			nbases = 0; /* this means string, not array */
			/* If anything else than string is passed, ldap_base_dn = NULL */
			if (Z_TYPE_P(base_dn) == IS_STRING) {
				ldap_base_dn = Z_STRVAL_P(base_dn);
			} else {
				ldap_base_dn = NULL;
			}
		}

		if (Z_TYPE_P(filter) == IS_ARRAY) {
			nfilters = zend_hash_num_elements(Z_ARRVAL_P(filter));
			if (nfilters != nlinks) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Filter must either be a string, or an array with the same number of elements as the links array");
				ret = 0;
				goto cleanup;
			}
			zend_hash_internal_pointer_reset(Z_ARRVAL_P(filter));
		} else {
			nfilters = 0; /* this means string, not array */
			convert_to_string_ex(filter);
			ldap_filter = Z_STRVAL_P(filter);
		}

		lds = (ldap_linkdata **)safe_emalloc(nlinks, sizeof(ldap_linkdata), 0);
		rcs = (int *)safe_emalloc(nlinks, sizeof(*rcs), 0);

		zend_hash_internal_pointer_reset(Z_ARRVAL_P(link));
		for (i=0; i<nlinks; i++) {
			entry = zend_hash_get_current_data(Z_ARRVAL_P(link));

			ld = (ldap_linkdata *) zend_fetch_resource_ex(entry, "ldap link", le_link);
			if (ld == NULL) {
				ret = 0;
				goto cleanup_parallel;
			}
			if (nbases != 0) { /* base_dn an array? */
				entry = zend_hash_get_current_data(Z_ARRVAL_P(base_dn));
				zend_hash_move_forward(Z_ARRVAL_P(base_dn));

				/* If anything else than string is passed, ldap_base_dn = NULL */
				if (Z_TYPE_P(entry) == IS_STRING) {
					ldap_base_dn = Z_STRVAL_P(entry);
				} else {
					ldap_base_dn = NULL;
				}
			}
			if (nfilters != 0) { /* filter an array? */
				entry = zend_hash_get_current_data(Z_ARRVAL_P(filter));
				zend_hash_move_forward(Z_ARRVAL_P(filter));
				convert_to_string_ex(entry);
				ldap_filter = Z_STRVAL_P(entry);
			}
//!!!!
			php_set_opts(ld->link, ldap_sizelimit, ldap_timelimit, ldap_deref, &old_ldap_sizelimit, &old_ldap_timelimit, &old_ldap_deref);

			/* Run the actual search */
#ifdef _WIN32
			rcs[i] = ldap_search(ld->link, ldap_base_dn, scope, ldap_filter, ldap_attrs, ldap_attrsonly);
#else
			struct timeval tv;
			tv.tv_sec = ldap_timelimit;
			tv.tv_usec = 0;
			int dummyMsgID;	// we are using LDAP_RES_ANY for now, msgID is not important
			rcs[i] = ldap_search_ext(ld->link, ldap_base_dn, scope, ldap_filter, ldap_attrs, ldap_attrsonly, nullptr, nullptr, &tv, ldap_sizelimit, &dummyMsgID);
#endif
			lds[i] = ld;
			zend_hash_move_forward(Z_ARRVAL_P(link));
		}

		array_init(return_value);

		/* Collect results from the searches */
		for (i=0; i<nlinks; i++) {
			if (rcs[i] != -1) {
				rcs[i] = ldap_result(lds[i]->link, LDAP_RES_ANY, 1 /* LDAP_MSG_ALL */, NULL, &ldap_res);
			}
			if (rcs[i] != -1) {
				ZVAL_RES(&resource, zend_register_resource(ldap_res, le_result));
				add_next_index_zval(return_value, &resource);
			} else {
				add_next_index_bool(return_value, 0);
			}
		}

cleanup_parallel:
		efree(lds);
		efree(rcs);
	} else {
		convert_to_string_ex(filter);
		ldap_filter = Z_STRVAL_P(filter);

		/* If anything else than string is passed, ldap_base_dn = NULL */
		if (Z_TYPE_P(base_dn) == IS_STRING) {
			ldap_base_dn = Z_STRVAL_P(base_dn);
		}
		else
		{
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Illegal Base DN parameter");
			goto cleanup;
		}

		ld = (ldap_linkdata *) zend_fetch_resource_ex(link, "ldap link", le_link);
		if (ld == NULL) {
			ret = 0;
			goto cleanup;
		}

		php_set_opts(ld->link, ldap_sizelimit, ldap_timelimit, ldap_deref, &old_ldap_sizelimit, &old_ldap_timelimit, &old_ldap_deref);

		/* Run the actual search */
#ifdef _WIN32
		auto dn = vs::UTF8ToWideCharConvert(ldap_base_dn);
		auto filter = vs::UTF8ToWideCharConvert(ldap_filter);
		std::vector<std::wstring> attrHolder;
		std::vector<wchar_t*> attrs;

		if(ldap_attrs != nullptr){
			size_t nattrs = 0;
			for (nattrs = 0; ldap_attrs[nattrs] != NULL; nattrs++)
				;
			UTF8ToWCharArr(ldap_attrs, nattrs, attrHolder);
			MakeAttrsArray(attrHolder, attrs);
		}
		attrs.emplace_back(nullptr);
		err = ldap_search_sW(ld->link, (PWSTR)dn.c_str(), scope, (PWSTR)filter.c_str(), attrs.data(), ldap_attrsonly, &ldap_res);
#else
		struct timeval tv;
		tv.tv_sec = ldap_timelimit;
		tv.tv_usec = 0;
		err = ldap_search_ext_s(ld->link, ldap_base_dn, scope, ldap_filter, ldap_attrs, ldap_attrsonly, nullptr, nullptr, &tv, ldap_sizelimit, &ldap_res);
#endif

		if (err != LDAP_SUCCESS
			&& err != LDAP_SIZELIMIT_EXCEEDED
#ifdef LDAP_ADMINLIMIT_EXCEEDED
			&& err != LDAP_ADMINLIMIT_EXCEEDED
#endif
#ifdef LDAP_REFERRAL
			&& err != LDAP_REFERRAL
#endif
		) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Search: %s", ldap_err2string(err));
			ret = 0;
		} else {
			if (err == LDAP_SIZELIMIT_EXCEEDED) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Partial search results returned: Sizelimit exceeded");
			}
#ifdef LDAP_ADMINLIMIT_EXCEEDED
			else if (err == LDAP_ADMINLIMIT_EXCEEDED) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Partial search results returned: Adminlimit exceeded");
			}
#endif
			dprint2("php_ldap_do_search done successfully.\n");
			RETVAL_RES(zend_register_resource(ldap_res, le_result));
		}
	}

cleanup:
	if (ld) {
		/* Restoring previous options */
		php_set_opts(ld->link, old_ldap_sizelimit, old_ldap_timelimit, old_ldap_deref, &ldap_sizelimit, &ldap_timelimit, &ldap_deref);
	}
	if (ldap_attrs != NULL) {
		efree(ldap_attrs);
	}
	if (!ret) {
		RETVAL_BOOL(ret);
	}
}
/* }}} */

/* {{{ proto resource ldap_read(resource|array link, string base_dn, string filter [, array attrs [, int attrsonly [, int sizelimit [, int timelimit [, int deref]]]]])
   Read an entry */
PHP_FUNCTION(ldap_read)
{
	dprint2("do ldap_read\n");
	php_ldap_do_search(INTERNAL_FUNCTION_PARAM_PASSTHRU, LDAP_SCOPE_BASE);
}
/* }}} */

/* {{{ proto resource ldap_list(resource|array link, string base_dn, string filter [, array attrs [, int attrsonly [, int sizelimit [, int timelimit [, int deref]]]]])
   Single-level search */
PHP_FUNCTION(ldap_list)
{
	dprint2("do ldap_list\n");
	php_ldap_do_search(INTERNAL_FUNCTION_PARAM_PASSTHRU, LDAP_SCOPE_ONELEVEL);
}
/* }}} */

/* {{{ proto resource ldap_search(resource|array link, string base_dn, string filter [, array attrs [, int attrsonly [, int sizelimit [, int timelimit [, int deref]]]]])
   Search LDAP tree under base_dn */
PHP_FUNCTION(ldap_search)
{
	dprint2("do ldap_search\n");
	php_ldap_do_search(INTERNAL_FUNCTION_PARAM_PASSTHRU, LDAP_SCOPE_SUBTREE);
}
/* }}} */

/* {{{ proto bool ldap_free_result(resource result)
   Free result memory */
PHP_FUNCTION(ldap_free_result)
{
	zval *result;
	LDAPMessage *ldap_result;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &result) != SUCCESS) {
		dprint1("ldap_free_result called with wrong arguments!\n");
		return;
	}

	if ((ldap_result = (LDAPMessage *)zend_fetch_resource(Z_RES_P(result), "ldap result", le_result)) == NULL) {
		dprint1("ldap_free_result can't fetch LDAP ctx!");
		RETURN_FALSE;
	}

	zend_list_delete(Z_RES_P(result));  /* Delete list entry */
	dprint2("ldap_free_result done successfully.\n");
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int ldap_count_entries(resource link, resource result)
   Count the number of entries in a search result */
PHP_FUNCTION(ldap_count_entries)
{
	zval *link, *result;
	ldap_linkdata *ld;
	LDAPMessage *ldap_result;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &link, &result) != SUCCESS) {
		dprint1("ldap_count_entries called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_count_entries can't fetch LDAP ctx with ldap link!\n");
		RETURN_FALSE;
	}
	if ((ldap_result = (LDAPMessage *)zend_fetch_resource(Z_RES_P(result), "ldap result", le_result)) == NULL) {
		dprint1("ldap_count_entries can't fetch LDAP ctx with ldap result!\n");
		RETURN_FALSE;
	}

	dprint2("ldap_count_entries done successfully.\n");
	RETURN_LONG(ldap_count_entries(ld->link, ldap_result));
}
/* }}} */

/* {{{ proto resource ldap_first_entry(resource link, resource result)
   Return first result id */
PHP_FUNCTION(ldap_first_entry)
{
	zval *link, *result;
	ldap_linkdata *ld;
	ldap_resultentry *resultentry;
	LDAPMessage *ldap_result, *entry;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &link, &result) != SUCCESS) {
		dprint1("ldap_first_entry called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_first_entry can't fetch LDAP ctx with ldap link\n!");
		RETURN_FALSE;
	}
	if ((ldap_result = (LDAPMessage *)zend_fetch_resource(Z_RES_P(result), "ldap result", le_result)) == NULL) {
		dprint1("ldap_first_entry can't fetch LDAP ctx with ldap result!\n");
		RETURN_FALSE;
	}

	if ((entry = ldap_first_entry(ld->link, ldap_result)) == NULL) {
		dprint1("ldap_first_entry failed!\n");
		RETVAL_FALSE;
	} else {
		resultentry = (ldap_resultentry *)emalloc(sizeof(ldap_resultentry));
		RETVAL_RES(zend_register_resource(resultentry, le_result_entry));
		ZVAL_COPY(&resultentry->res, result);
		resultentry->data = entry;
		resultentry->ber = NULL;
		dprint2("ldap_first_entry done successfully.\n");
	}
}
/* }}} */

/* {{{ proto resource ldap_next_entry(resource link, resource result_entry)
   Get next result entry */
PHP_FUNCTION(ldap_next_entry)
{
	zval *link, *result_entry;
	ldap_linkdata *ld;
	ldap_resultentry *resultentry, *resultentry_next;
	LDAPMessage *entry_next;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &link, &result_entry) != SUCCESS) {
		dprint1("ldap_next_entry called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_next_entry can't fetch LDAP ctx with ldap link!\n");
		RETURN_FALSE;
	}
	if ((resultentry = (ldap_resultentry *)zend_fetch_resource(Z_RES_P(result_entry), "ldap result entry", le_result_entry)) == NULL) {
		dprint1("ldap_next_entry can't fetch LDAP ctx with ldap result entry!\n");
		RETURN_FALSE;
	}

	if ((entry_next = ldap_next_entry(ld->link, resultentry->data)) == NULL) {
		dprint1("ldap_next_entry failed!\n");
		RETVAL_FALSE;
	} else {
		resultentry_next = (ldap_resultentry *)emalloc(sizeof(ldap_resultentry));
		RETVAL_RES(zend_register_resource(resultentry_next, le_result_entry));
		ZVAL_COPY(&resultentry_next->res, &resultentry->res);
		resultentry_next->data = entry_next;
		resultentry_next->ber = NULL;
		dprint2("ldap_next_entry done successfully.\n");
	}
}
/* }}} */

/* {{{ proto array ldap_get_entries(resource link, resource result)
   Get all result entries */
PHP_FUNCTION(ldap_get_entries)
{
	zval *link, *result;
	LDAPMessage *ldap_result, *ldap_result_entry;
	zval tmp1, tmp2;
	ldap_linkdata *ld;
	LDAP *ldap;
	int num_entries, num_attrib, num_values, i;
	BerElement *ber;
	char *attribute;
	size_t attr_len;
	struct berval **ldap_value;
	char *dn;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS(), "rr", &link, &result) != SUCCESS) {
		dprint1("ldap_get_entries called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_get_entries can't fetch LDAP ctx with ldap link!\n");
		RETURN_FALSE;
	}
	if ((ldap_result = (LDAPMessage *)zend_fetch_resource(Z_RES_P(result), "ldap result", le_result)) == NULL) {
		dprint1("ldap_get_entries can't fetch LDAP ctx with ldap result!\n");
		RETURN_FALSE;
	}

	ldap = ld->link;
	num_entries = ldap_count_entries(ldap, ldap_result);

	array_init(return_value);
	add_assoc_long(return_value, "count", num_entries);

	if (num_entries == 0) {
		return;
	}

	ldap_result_entry = ldap_first_entry(ldap, ldap_result);
	if (ldap_result_entry == NULL) {
		zval_dtor(return_value);
		RETURN_FALSE;
	}

	num_entries = 0;
	while (ldap_result_entry != NULL) {
		array_init(&tmp1);

		num_attrib = 0;
		attribute = ldap_first_attribute(ldap, ldap_result_entry, &ber);

		while (attribute != NULL) {
			ldap_value = ldap_get_values_len(ldap, ldap_result_entry, attribute);
			num_values = ldap_count_values_len(ldap_value);

			array_init(&tmp2);
			add_assoc_long(&tmp2, "count", num_values);
			for (i = 0; i < num_values; i++) {
				add_index_stringl(&tmp2, i, ldap_value[i]->bv_val, ldap_value[i]->bv_len);
			}
			ldap_value_free_len(ldap_value);

			attr_len = strlen(attribute);
			zend_hash_str_update(Z_ARRVAL(tmp1), php_strtolower(attribute, attr_len), attr_len, &tmp2);
			add_index_string(&tmp1, num_attrib, attribute);

			num_attrib++;
#if (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10 || WINDOWS
			ldap_memfree(attribute);
#endif
			attribute = ldap_next_attribute(ldap, ldap_result_entry, ber);
		}
#if (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10 || WINDOWS
		if (ber != NULL) {
			ber_free(ber, 0);
		}
#endif

		add_assoc_long(&tmp1, "count", num_attrib);
		dn = ldap_get_dn(ldap, ldap_result_entry);
		add_assoc_string(&tmp1, "dn", dn);
#if (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10 || WINDOWS
		ldap_memfree(dn);
#else
		free(dn);
#endif

		zend_hash_index_update(Z_ARRVAL_P(return_value), num_entries, &tmp1);

		num_entries++;
		ldap_result_entry = ldap_next_entry(ldap, ldap_result_entry);
	}

	add_assoc_long(return_value, "count", num_entries);
	dprint2("ldap_get_entries done successfully. count=%d\n", num_entries);

}
/* }}} */

/* {{{ proto string ldap_first_attribute(resource link, resource result_entry)
   Return first attribute */
PHP_FUNCTION(ldap_first_attribute)
{
	zval *link, *result_entry;
	ldap_linkdata *ld;
	ldap_resultentry *resultentry;
	char *attribute;
	zend_long dummy_ber = 0;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr|l", &link, &result_entry, &dummy_ber) != SUCCESS) {
		dprint1("ldap_first_attribute called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_first_attribute can't fetch LDAP ctx with ldap link!\n");
		RETURN_FALSE;
	}
	if ((resultentry = (ldap_resultentry *)zend_fetch_resource(Z_RES_P(result_entry), "ldap result entry", le_result_entry)) == NULL) {
		dprint1("ldap_first_attribute can't fetch le_result_entry!\n");
		RETURN_FALSE;
	}

	if ((attribute = ldap_first_attribute(ld->link, resultentry->data, &resultentry->ber)) == NULL) {
		dprint1("ldap_first_attribute failed!\n");
		RETURN_FALSE;
	} else {
		dprint2("ldap_first_attribute done successfully.\n");
		RETVAL_STRING(attribute);
#if (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10 || WINDOWS
		ldap_memfree(attribute);
#endif
	}
}
/* }}} */

/* {{{ proto string ldap_next_attribute(resource link, resource result_entry)
   Get the next attribute in result */
PHP_FUNCTION(ldap_next_attribute)
{
	zval *link, *result_entry;
	ldap_linkdata *ld;
	ldap_resultentry *resultentry;
	char *attribute;
	zend_long dummy_ber = 0;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr|l", &link, &result_entry, &dummy_ber) != SUCCESS) {
		dprint1("ldap_next_attribute called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_next_attribute can't fetch LDAP ctx with ldap link!\n");
		RETURN_FALSE;
	}
	if ((resultentry = (ldap_resultentry *)zend_fetch_resource(Z_RES_P(result_entry), "ldap result entry", le_result_entry)) == NULL) {
		dprint1("ldap_next_attribute can't fetch ldap result entry!\n");
		RETURN_FALSE;
	}

	if (resultentry->ber == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "called before calling ldap_first_attribute() or no attributes found in result entry");
		RETURN_FALSE;
	}

	if ((attribute = ldap_next_attribute(ld->link, resultentry->data, resultentry->ber)) == NULL) {
#if (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10 || WINDOWS
		if (resultentry->ber != NULL) {
			ber_free(resultentry->ber, 0);
			resultentry->ber = NULL;
		}
#endif
		dprint1("ldap_next_attribute failed!\n");
		RETURN_FALSE;
	} else {
		dprint2("ldap_next_attribute done successfully.");
		RETVAL_STRING(attribute);
#if (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10 || WINDOWS
		ldap_memfree(attribute);
#endif
	}
}
/* }}} */

/* {{{ proto array ldap_get_attributes(resource link, resource result_entry)
   Get attributes from a search result entry */
PHP_FUNCTION(ldap_get_attributes)
{
	zval *link, *result_entry;
	zval tmp;
	ldap_linkdata *ld;
	ldap_resultentry *resultentry;
	char *attribute;
	struct berval **ldap_value;
	int i, num_values, num_attrib;
	BerElement *ber;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS(), "rr", &link, &result_entry) != SUCCESS) {
		dprint1("ldap_get_attributes called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_get_attributes can't fetch LDAP ctx with ldap link!\n");
		RETURN_FALSE;
	}

	if ((resultentry = (ldap_resultentry *)zend_fetch_resource(Z_RES_P(result_entry), "ldap result entry", le_result_entry)) == NULL) {
		dprint1("ldap_get_attributes can't fetch ldap result entry!\n");
		RETURN_FALSE;
	}

	array_init(return_value);
	num_attrib = 0;

	attribute = ldap_first_attribute(ld->link, resultentry->data, &ber);
	while (attribute != NULL) {
		ldap_value = ldap_get_values_len(ld->link, resultentry->data, attribute);
		num_values = ldap_count_values_len(ldap_value);

		array_init(&tmp);
		add_assoc_long(&tmp, "count", num_values);
		for (i = 0; i < num_values; i++) {
			add_index_stringl(&tmp, i, ldap_value[i]->bv_val, ldap_value[i]->bv_len);
		}
		ldap_value_free_len(ldap_value);

		zend_hash_str_update(Z_ARRVAL_P(return_value), attribute, strlen(attribute), &tmp);
		add_index_string(return_value, num_attrib, attribute);

		num_attrib++;
#if (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10 || WINDOWS
		ldap_memfree(attribute);
#endif
		attribute = ldap_next_attribute(ld->link, resultentry->data, ber);
	}
#if (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10 || WINDOWS
	if (ber != NULL) {
		ber_free(ber, 0);
	}
#endif

	dprint2("ldap_get_attributes done. count=%d\n", num_attrib);
	add_assoc_long(return_value, "count", num_attrib);
}
/* }}} */

/* {{{ proto array ldap_get_values_len(resource link, resource result_entry, string attribute)
   Get all values with lengths from a result entry */
PHP_FUNCTION(ldap_get_values_len)
{
	zval *link, *result_entry;
	ldap_linkdata *ld;
	ldap_resultentry *resultentry;
	char *attr;
	struct berval **ldap_value_len;
	int i, num_values;
	size_t attr_len;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS(), "rrs", &link, &result_entry, &attr, &attr_len) != SUCCESS) {
		dprint1("ldap_get_values_len called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_get_values_len can't fetch LDAP ctx with ldap link!\n");
		RETURN_FALSE;
	}

	if ((resultentry = (ldap_resultentry *)zend_fetch_resource(Z_RES_P(result_entry), "ldap result entry", le_result_entry)) == NULL) {
		dprint1("ldap_get_values_len can't fetch ldap result entry!\n");
		RETURN_FALSE;
	}

	if ((ldap_value_len = ldap_get_values_len(ld->link, resultentry->data, attr)) == NULL) {
		php_error_docref(NULL, E_WARNING, "Cannot get the value(s) of attribute %s", ldap_err2string(_get_lderrno(ld->link)));
		RETURN_FALSE;
	}

	num_values = ldap_count_values_len(ldap_value_len);
	array_init(return_value);

	for (i=0; i<num_values; i++) {
		add_next_index_stringl(return_value, ldap_value_len[i]->bv_val, ldap_value_len[i]->bv_len);
	}

	add_assoc_long(return_value, "count", num_values);
	ldap_value_free_len(ldap_value_len);
	dprint2("ldap_get_values_len done. len=%d", num_values);

}
/* }}} */

/* {{{ proto string ldap_get_dn(resource link, resource result_entry)
   Get the DN of a result entry */
PHP_FUNCTION(ldap_get_dn)
{
	zval *link, *result_entry;
	ldap_linkdata *ld;
	ldap_resultentry *resultentry;
	char *text;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS(), "rr", &link, &result_entry) != SUCCESS) {
		dprint1("ldap_get_dn called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_get_dn can't fetch LDAP ctx with ldap link!\n");
		RETURN_FALSE;
	}

	if ((resultentry = (ldap_resultentry *)zend_fetch_resource(Z_RES_P(result_entry), "ldap result entry", le_result_entry)) == NULL) {
		dprint1("ldap_get_dn can't fetch ldap result entry!\n");
		RETURN_FALSE;
	}

	text = ldap_get_dn(ld->link, resultentry->data);
	if (text != NULL) {
		RETVAL_STRING(text);
		dprint2("ldap_get_dn done. dn=%s\n",text);
#if (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10 || WINDOWS
		ldap_memfree(text);
#else
		free(text);
#endif
	} else {
		dprint1("ldap_get_dn failed!\n");
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto array ldap_explode_dn(string dn, int with_attrib)
   Splits DN into its component parts */
PHP_FUNCTION(ldap_explode_dn)
{
	zend_long with_attrib;
	char *dn, **ldap_value;
	int i, count;
	size_t dn_len;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS(), "sl", &dn, &dn_len, &with_attrib) != SUCCESS) {
		dprint1("ldap_explode_dn called with wrong arguments!\n");
		return;
	}

	if (!(ldap_value = ldap_explode_dn(dn, with_attrib))) {
		/* Invalid parameters were passed to ldap_explode_dn */
		dprint1("Invalid parameters were passed to ldap_explode_dn!\n");
		RETURN_FALSE;
	}
	VS_SCOPE_EXIT{ ldap_value_free(ldap_value); };

	i=0;
	while (ldap_value[i] != NULL) i++;
	count = i;

	array_init(return_value);

	add_assoc_long(return_value, "count", count);
	for (i = 0; i<count; i++) {
		add_index_string(return_value, i, ldap_value[i]);
	}

	dprint2("ldap_explode_dn done.");
}
/* }}} */

/* {{{ proto string ldap_dn2ufn(string dn)
   Convert DN to User Friendly Naming format */
PHP_FUNCTION(ldap_dn2ufn)
{
	char *dn, *ufn;
	size_t dn_len;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS(), "s", &dn, &dn_len) != SUCCESS) {
		dprint1("ldap_dn2ufn called with wrong arguments!\n");
		return;
	}

	ufn = ldap_dn2ufn(dn);

	if (ufn != NULL) {
		dprint2("ldap_dn2ufn done. dn=%s, ufn=%s\n", dn, ufn);
		RETVAL_STRING(ufn);
#if (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10 || WINDOWS
		ldap_memfree(ufn);
#endif
	} else {
		dprint1("ldap_dn2ufn failed! dn=%s\n", dn);
		RETURN_FALSE;
	}
}
/* }}} */


/* added to fix use of ldap_modify_add for doing an ldap_add, gerrit thomson. */
#define PHP_LD_FULL_ADD 0xff
/* {{{ php_ldap_do_modify
 */
static void php_ldap_do_modify(INTERNAL_FUNCTION_PARAMETERS, int oper)
{
	zval *link, *entry, *value, *ivalue;
	ldap_linkdata *ld;
	char *dn;
	LDAPMod **ldap_mods;
	int i, j, num_attribs, num_values;
	size_t dn_len;
	int *num_berval;
	zend_string *attribute;
	zend_ulong index;
	int is_full_add=0; /* flag for full add operation so ldap_mod_add can be put back into oper, gerrit THomson */

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS(), "rsa", &link, &dn, &dn_len, &entry) != SUCCESS) {
		dprint1("php_ldap_do_modify called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("php_ldap_do_modify can't fetch LDAP ctx!\n");
		RETURN_FALSE;
	}

	num_attribs = zend_hash_num_elements(Z_ARRVAL_P(entry));
	ldap_mods = (LDAPMod **)safe_emalloc((num_attribs+1), sizeof(LDAPMod *), 0);
	num_berval = (int *)safe_emalloc(num_attribs, sizeof(int), 0);
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(entry));

	/* added by gerrit thomson to fix ldap_add using ldap_mod_add */
	if (oper == PHP_LD_FULL_ADD) {
		oper = LDAP_MOD_ADD;
		is_full_add = 1;
	}
	/* end additional , gerrit thomson */

	for (i = 0; i < num_attribs; i++) {
		ldap_mods[i] = (LDAPMod *)emalloc(sizeof(LDAPMod));
		ldap_mods[i]->mod_op = oper | LDAP_MOD_BVALUES;
		ldap_mods[i]->mod_type = NULL;

		if (zend_hash_get_current_key(Z_ARRVAL_P(entry), &attribute, &index) == HASH_KEY_IS_STRING) {
			ldap_mods[i]->mod_type = estrndup(attribute->val, attribute->len);
		} else {
			php_error_docref(NULL, E_WARNING, "Unknown attribute in the data");
			/* Free allocated memory */
			while (i >= 0) {
				if (ldap_mods[i]->mod_type) {
					efree(ldap_mods[i]->mod_type);
				}
				efree(ldap_mods[i]);
				i--;
			}
			efree(num_berval);
			efree(ldap_mods);
			RETURN_FALSE;
		}

		value = zend_hash_get_current_data(Z_ARRVAL_P(entry));

		if (Z_TYPE_P(value) != IS_ARRAY) {
			num_values = 1;
		} else {
			num_values = zend_hash_num_elements(Z_ARRVAL_P(value));
		}

		num_berval[i] = num_values;
		ldap_mods[i]->mod_bvalues = (berval**)safe_emalloc((num_values + 1), sizeof(struct berval *), 0);

/* allow for arrays with one element, no allowance for arrays with none but probably not required, gerrit thomson. */
		if ((num_values == 1) && (Z_TYPE_P(value) != IS_ARRAY)) {
			convert_to_string_ex(value);
			ldap_mods[i]->mod_bvalues[0] = (struct berval *) emalloc (sizeof(struct berval));
			ldap_mods[i]->mod_bvalues[0]->bv_len = Z_STRLEN_P(value);
			ldap_mods[i]->mod_bvalues[0]->bv_val = Z_STRVAL_P(value);
		} else {
			for (j = 0; j < num_values; j++) {
				if ((ivalue = zend_hash_index_find(Z_ARRVAL_P(value), j)) == NULL) {
					php_error_docref(NULL, E_WARNING, "Value array must have consecutive indices 0, 1, ...");
					num_berval[i] = j;
					num_attribs = i + 1;
					RETVAL_FALSE;
					goto errexit;
				}
				convert_to_string_ex(ivalue);
				ldap_mods[i]->mod_bvalues[j] = (struct berval *) emalloc (sizeof(struct berval));
				ldap_mods[i]->mod_bvalues[j]->bv_len = Z_STRLEN_P(ivalue);
				ldap_mods[i]->mod_bvalues[j]->bv_val = Z_STRVAL_P(ivalue);
			}
		}
		ldap_mods[i]->mod_bvalues[num_values] = NULL;
		zend_hash_move_forward(Z_ARRVAL_P(entry));
	}
	ldap_mods[num_attribs] = NULL;

/* check flag to see if do_mod was called to perform full add , gerrit thomson */
	if (is_full_add == 1) {
		if ((i = ldap_add_ext_s(ld->link, dn, ldap_mods, nullptr, nullptr)) != LDAP_SUCCESS) {
			php_error_docref(NULL, E_WARNING, "Add: %s", ldap_err2string(i));
			RETVAL_FALSE;
		} else RETVAL_TRUE;
	} else {
		if ((i = ldap_modify_ext_s(ld->link, dn, ldap_mods, NULL, NULL)) != LDAP_SUCCESS) {
			php_error_docref(NULL, E_WARNING, "Modify: %s", ldap_err2string(i));
			RETVAL_FALSE;
		} else RETVAL_TRUE;
	}

errexit:
	for (i = 0; i < num_attribs; i++) {
		efree(ldap_mods[i]->mod_type);
		for (j = 0; j < num_berval[i]; j++) {
			efree(ldap_mods[i]->mod_bvalues[j]);
		}
		efree(ldap_mods[i]->mod_bvalues);
		efree(ldap_mods[i]);
	}
	efree(num_berval);
	efree(ldap_mods);

	return;
}
/* }}} */

/* {{{ proto bool ldap_add(resource link, string dn, array entry)
   Add entries to LDAP directory */
PHP_FUNCTION(ldap_add)
{
	dprint2("call ldap_add\n");
	/* use a newly define parameter into the do_modify so ldap_mod_add can be used the way it is supposed to be used , Gerrit THomson */
	php_ldap_do_modify(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_LD_FULL_ADD);
}
/* }}} */

/* three functions for attribute base modifications, gerrit Thomson */

/* {{{ proto bool ldap_mod_replace(resource link, string dn, array entry)
   Replace attribute values with new ones */
PHP_FUNCTION(ldap_mod_replace)
{
	dprint2("call ldap_mod_replace\n");
	php_ldap_do_modify(INTERNAL_FUNCTION_PARAM_PASSTHRU, LDAP_MOD_REPLACE);
}
/* }}} */

/* {{{ proto bool ldap_mod_add(resource link, string dn, array entry)
   Add attribute values to current */
PHP_FUNCTION(ldap_mod_add)
{
	dprint2("call ldap_mod_add\n");
	php_ldap_do_modify(INTERNAL_FUNCTION_PARAM_PASSTHRU, LDAP_MOD_ADD);
}
/* }}} */

/* {{{ proto bool ldap_mod_del(resource link, string dn, array entry)
   Delete attribute values */
PHP_FUNCTION(ldap_mod_del)
{
	dprint2("call ldap_mod_del\n");
	php_ldap_do_modify(INTERNAL_FUNCTION_PARAM_PASSTHRU, LDAP_MOD_DELETE);
}
/* }}} */

/* {{{ proto bool ldap_delete(resource link, string dn)
   Delete an entry from a directory */
PHP_FUNCTION(ldap_delete)
{
	zval *link;
	ldap_linkdata *ld;
	char *dn;
	size_t dn_len = 0;
	int rc;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &link, &dn, &dn_len) != SUCCESS) {
		dprint1("ldap_delete called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_delete can't fetch ldap CTX!\n");
		RETURN_FALSE;
	}

	if ((rc = ldap_delete_ext_s(ld->link, dn, nullptr, nullptr)) != LDAP_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Delete: %s", ldap_err2string(rc));
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int ldap_errno(resource link)
   Get the current ldap error number */
PHP_FUNCTION(ldap_errno)
{
	zval *link;
	ldap_linkdata *ld;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &link) != SUCCESS) {
		dprint1("ldap_errno called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_errno can't fetch ldap CTX!\n");
		RETURN_FALSE;
	}

	RETURN_LONG(_get_lderrno(ld->link));
}
/* }}} */

/* {{{ proto string ldap_err2str(int errno)
   Convert error number to error string */
PHP_FUNCTION(ldap_err2str)
{
	zend_long perrno;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &perrno) != SUCCESS) {
		dprint1("ldap_err2str called with wrong arguments!\n");
		return;
	}

	RETURN_STRING(ldap_err2string(perrno));
}
/* }}} */

/* {{{ proto string ldap_error(resource link)
   Get the current ldap error string */
PHP_FUNCTION(ldap_error)
{
	zval *link;
	ldap_linkdata *ld;
	int ld_errno;
	char *message;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &link) != SUCCESS) {
		dprint1("ldap_error called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_error can't fetch ldap CTX!\n");
		RETURN_FALSE;
	}

	ld_errno = _get_lderrno(ld->link);

	message = ldap_err2string(ld_errno);
	dprint2("ldap_error message='%s'\n", message);
	RETVAL_STRING(message);
	return;
}
/* }}} */

/* {{{ proto bool ldap_compare(resource link, string dn, string attr, string value)
   Determine if an entry has a specific value for one of its attributes */
PHP_FUNCTION(ldap_compare)
{
	zval *link;
	char *dn, *attr, *value;
	size_t dn_len, attr_len, value_len;
	ldap_linkdata *ld;
	int err;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsss", &link, &dn, &dn_len, &attr, &attr_len, &value, &value_len) != SUCCESS) {
		dprint1("ldap_compare called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_compare can't fetch ldap CTX!\n");
		RETURN_FALSE;
	}

	struct berval bval = { value_len, value };
	err = ldap_compare_ext_s(ld->link, dn, attr, &bval, nullptr, nullptr);

	switch (err) {
		case LDAP_COMPARE_TRUE:
			RETURN_TRUE;
			break;

		case LDAP_COMPARE_FALSE:
			RETURN_FALSE;
			break;
	}

	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Compare: %s", ldap_err2string(err));
	RETURN_LONG(-1);
}
/* }}} */

/* {{{ proto bool ldap_sort(resource link, resource result, string sortfilter)
   Sort LDAP result entries */
PHP_FUNCTION(ldap_sort)
{
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "ldap_sort support disabled due to custom TrueConf implementation");
	RETURN_FALSE;

	/*zval *link, *result;
	ldap_linkdata *ld;
	char *sortfilter;
	int sflen;
	zend_resource *le;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrs", &link, &result, &sortfilter, &sflen) != SUCCESS) {
		RETURN_FALSE;
	}

	ZEND_FETCH_RESOURCE(ld, ldap_linkdata *, &link, -1, "ldap link", le_link);

	if (zend_hash_index_find(&EG(regular_list), Z_LVAL_P(result), (void **) &le) != SUCCESS || le->type != le_result) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Supplied resource is not a valid ldap result resource");
		RETURN_FALSE;
	}

	if (ldap_sort_entries(ld->link, (LDAPMessage **) &le->ptr, sflen ? sortfilter : NULL, strcmp) != LDAP_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", ldap_err2string(errno));
		RETURN_FALSE;
	}

	RETURN_TRUE;*/
}
/* }}} */

#if (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10
/* {{{ proto bool ldap_get_option(resource link, int option, mixed retval)
   Get the current value of various session-wide parameters */
PHP_FUNCTION(ldap_get_option)
{
	zval *link, *retval;
	ldap_linkdata *ld;
	zend_long option;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlz", &link, &option, &retval) != SUCCESS) {
		dprint1("ldap_get_option called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_get_option can't fetch ldap CTX!\n");
		RETURN_FALSE;
	}

	switch (option) {
	/* options with int value */
	case LDAP_OPT_DEREF:
	case LDAP_OPT_SIZELIMIT:
	case LDAP_OPT_TIMELIMIT:
	case LDAP_OPT_PROTOCOL_VERSION:
	case LDAP_OPT_ERROR_NUMBER:
	case LDAP_OPT_REFERRALS:
#ifdef LDAP_OPT_RESTART
	case LDAP_OPT_RESTART:
#endif
		{
			int val;

			if (ldap_get_option(ld->link, option, &val)) {
				RETURN_FALSE;
			}
			zval_dtor(retval);
			ZVAL_LONG(retval, val);
		} break;
#ifdef LDAP_OPT_NETWORK_TIMEOUT
	case LDAP_OPT_NETWORK_TIMEOUT:
		{
			struct timeval *timeout = NULL;

			if (ldap_get_option(ld->link, LDAP_OPT_NETWORK_TIMEOUT, (void *) &timeout)) {
				if (timeout) {
					ldap_memfree(timeout);
				}
				RETURN_FALSE;
			}
			if (!timeout) {
				RETURN_FALSE;
			}
			zval_dtor(retval);
			ZVAL_LONG(retval, timeout->tv_sec);
			ldap_memfree(timeout);
		} break;
#elif defined(LDAP_X_OPT_CONNECT_TIMEOUT)
	case LDAP_X_OPT_CONNECT_TIMEOUT:
		{
			int timeout;

			if (ldap_get_option(ld->link, LDAP_X_OPT_CONNECT_TIMEOUT, &timeout)) {
				RETURN_FALSE;
			}
			zval_dtor(retval);
			ZVAL_LONG(retval, (timeout / 1000));
		} break;
#endif
	/* options with string value */
	case LDAP_OPT_ERROR_STRING:
#ifdef LDAP_OPT_HOST_NAME
	case LDAP_OPT_HOST_NAME:
#endif
#ifdef HAVE_LDAP_SASL
	case LDAP_OPT_X_SASL_MECH:
	case LDAP_OPT_X_SASL_REALM:
	case LDAP_OPT_X_SASL_AUTHCID:
	case LDAP_OPT_X_SASL_AUTHZID:
#endif
#ifdef LDAP_OPT_MATCHED_DN
	case LDAP_OPT_MATCHED_DN:
#endif
		{
			char *val = NULL;

			if (ldap_get_option(ld->link, option, &val) || val == NULL || *val == '\0') {
				if (val) {
					ldap_memfree(val);
				}
				RETURN_FALSE;
			}
			zval_dtor(retval);
			ZVAL_STRING(retval, val);
			ldap_memfree(val);
		} break;
/* options not implemented
	case LDAP_OPT_SERVER_CONTROLS:
	case LDAP_OPT_CLIENT_CONTROLS:
	case LDAP_OPT_API_INFO:
	case LDAP_OPT_API_FEATURE_INFO:
*/
	default:
		RETURN_FALSE;
	}
	dprint2("ldap_get_option done.\n");
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ldap_set_option(resource link, int option, mixed newval)
   Set the value of various session-wide parameters */
PHP_FUNCTION(ldap_set_option)
{
	zval *link, *newval;
	ldap_linkdata *ld;
	LDAP *ldap;
	zend_long option;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS(), "zlz", &link, &option, &newval) != SUCCESS) {
		dprint1("ldap_set_option called with wrong arguments!\n");
		return;
	}

	if (Z_TYPE_P(link) == IS_NULL) {
		ldap = NULL;
	} else {
		if ((ld = (ldap_linkdata *)zend_fetch_resource_ex(link, "ldap link", le_link)) == NULL) {
			dprint1("ldap_set_option can't fetch ldap CTX!\n");
			RETURN_FALSE;
		}
		ldap = ld->link;
	}

	switch (option) {
	/* options with int value */
	case LDAP_OPT_DEREF:
	case LDAP_OPT_SIZELIMIT:
	case LDAP_OPT_TIMELIMIT:
	case LDAP_OPT_PROTOCOL_VERSION:
	case LDAP_OPT_ERROR_NUMBER:
#ifdef LDAP_OPT_DEBUG_LEVEL
	case LDAP_OPT_DEBUG_LEVEL:
#endif
		{
			int val;

			convert_to_long_ex(newval);
			val = Z_LVAL_P(newval);
			if (ldap_set_option(ldap, option, &val)) {
				RETURN_FALSE;
			}
		} break;
#ifdef LDAP_OPT_NETWORK_TIMEOUT
	case LDAP_OPT_NETWORK_TIMEOUT:
		{
			struct timeval timeout;

			convert_to_long_ex(newval);
			timeout.tv_sec = Z_LVAL_P(newval);
			timeout.tv_usec = 0;
			if (ldap_set_option(ldap, LDAP_OPT_NETWORK_TIMEOUT, (void *) &timeout)) {
				RETURN_FALSE;
			}
		} break;
#elif defined(LDAP_X_OPT_CONNECT_TIMEOUT)
	case LDAP_X_OPT_CONNECT_TIMEOUT:
		{
			int timeout;

			convert_to_long_ex(newval);
			timeout = 1000 * Z_LVAL_P(newval); /* Convert to milliseconds */
			if (ldap_set_option(ldap, LDAP_X_OPT_CONNECT_TIMEOUT, &timeout)) {
				RETURN_FALSE;
			}
		} break;
#endif
		/* options with string value */
	case LDAP_OPT_ERROR_STRING:
#ifdef LDAP_OPT_HOST_NAME
	case LDAP_OPT_HOST_NAME:
#endif
#ifdef HAVE_LDAP_SASL
	case LDAP_OPT_X_SASL_MECH:
	case LDAP_OPT_X_SASL_REALM:
	case LDAP_OPT_X_SASL_AUTHCID:
	case LDAP_OPT_X_SASL_AUTHZID:
#endif
#ifdef LDAP_OPT_MATCHED_DN
	case LDAP_OPT_MATCHED_DN:
#endif
		{
			char *val;
			convert_to_string_ex(newval);
			val = Z_STRVAL_P(newval);
			if (ldap_set_option(ldap, option, val)) {
				RETURN_FALSE;
			}
		} break;
		/* options with boolean value */
	case LDAP_OPT_REFERRALS:
#ifdef LDAP_OPT_RESTART
	case LDAP_OPT_RESTART:
#endif
		{
			void *val;
			convert_to_boolean_ex(newval);
			val = Z_TYPE_P(newval) == IS_TRUE
				? LDAP_OPT_ON : LDAP_OPT_OFF;
			if (ldap_set_option(ldap, option, val)) {
				RETURN_FALSE;
			}
		} break;
		/* options with control list value */
	case LDAP_OPT_SERVER_CONTROLS:
	case LDAP_OPT_CLIENT_CONTROLS:
		{
			LDAPControl *ctrl, **ctrls, **ctrlp;
			zval *ctrlval, *val;
			int ncontrols;
			uint32_t error=0;

			if ((Z_TYPE_P(newval) != IS_ARRAY) || !(ncontrols = zend_hash_num_elements(Z_ARRVAL_P(newval)))) {
				php_error_docref(NULL, E_WARNING, "Expected non-empty array value for this option");
				RETURN_FALSE;
			}
			ctrls = (LDAPControl **)safe_emalloc((1 + ncontrols), sizeof(*ctrls), 0);
			*ctrls = NULL;
			ctrlp = ctrls;
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(newval), ctrlval) {
				if (Z_TYPE_P(ctrlval) != IS_ARRAY) {
					php_error_docref(NULL, E_WARNING, "The array value must contain only arrays, where each array is a control");
					error = 1;
					break;
				}
				if ((val = zend_hash_str_find(Z_ARRVAL_P(ctrlval), "oid", sizeof("oid") - 1)) == NULL) {
					php_error_docref(NULL, E_WARNING, "Control must have an oid key");
					error = 1;
					break;
				}
				ctrl = *ctrlp = (LDAPControl *)emalloc(sizeof(**ctrlp));
				convert_to_string_ex(val);
				ctrl->ldctl_oid = Z_STRVAL_P(val);
				if ((val = zend_hash_str_find(Z_ARRVAL_P(ctrlval), "value", sizeof("value") - 1)) != NULL) {
					convert_to_string_ex(val);
					ctrl->ldctl_value.bv_val = Z_STRVAL_P(val);
					ctrl->ldctl_value.bv_len = Z_STRLEN_P(val);
				} else {
					ctrl->ldctl_value.bv_val = NULL;
					ctrl->ldctl_value.bv_len = 0;
				}
				if ((val = zend_hash_str_find(Z_ARRVAL_P(ctrlval), "iscritical", sizeof("iscritical") - 1)) != NULL) {
					convert_to_boolean_ex(val);
					ctrl->ldctl_iscritical = Z_TYPE_P(val) == IS_TRUE;
				} else {
					ctrl->ldctl_iscritical = 0;
				}

				++ctrlp;
				*ctrlp = NULL;
			} ZEND_HASH_FOREACH_END();
			if (!error) {
				error = ldap_set_option(ldap, option, ctrls);
			}
			ctrlp = ctrls;
			while (*ctrlp) {
				efree(*ctrlp);
				ctrlp++;
			}
			efree(ctrls);
			if (error) {
				RETURN_FALSE;
			}
		} break;
	default:
		RETURN_FALSE;
	}
	dprint2("ldap_set_option done.\n");
	RETURN_TRUE;
}
/* }}} */

#ifdef HAVE_LDAP_PARSE_RESULT
/* {{{ proto bool ldap_parse_result(resource link, resource result, int errcode, string matcheddn, string errmsg, array referrals)
   Extract information from result */
PHP_FUNCTION(ldap_parse_result)
{
	zval *link, *result, *errcode, *matcheddn = nullptr, *errmsg = nullptr, *referrals = nullptr;
	ldap_linkdata *ld;
	LDAPMessage *ldap_result;
	char **lreferrals, **refp;
	char *lmatcheddn, *lerrmsg;
	int rc, lerrcode, myargcount = ZEND_NUM_ARGS();

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrz|zzz", &link, &result, &errcode, &matcheddn, &errmsg, &referrals) != SUCCESS) {
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		RETURN_FALSE;
	}
	if ((ldap_result = (LDAPMessage *)zend_fetch_resource(Z_RES_P(result), "ldap result", le_result)) == NULL) {
		RETURN_FALSE;
	}

	rc = ldap_parse_result(ld->link, ldap_result, &lerrcode,
				myargcount > 3 ? &lmatcheddn : NULL,
				myargcount > 4 ? &lerrmsg : NULL,
				myargcount > 5 ? &lreferrals : NULL,
				NULL /* &serverctrls */,
				0);
	if (rc != LDAP_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to parse result: %s", ldap_err2string(rc));
		RETURN_FALSE;
	}

	zval_dtor(errcode);
	ZVAL_LONG(errcode, lerrcode);

	/* Reverse -> fall through */
	switch (myargcount) {
		case 6:
			zval_dtor(referrals);
			array_init(referrals);
			if (lreferrals != NULL) {
				refp = lreferrals;
				while (*refp) {
					add_next_index_string(referrals, *refp, 1);
					refp++;
				}
				ldap_value_free(lreferrals);
			}
		case 5:
			zval_dtor(errmsg);
			if (lerrmsg == NULL) {
				ZVAL_EMPTY_STRING(errmsg);
			} else {
				ZVAL_STRING(errmsg, lerrmsg, 1);
				ldap_memfree(lerrmsg);
			}
		case 4:
			zval_dtor(matcheddn);
			if (lmatcheddn == NULL) {
				ZVAL_EMPTY_STRING(matcheddn);
			} else {
				ZVAL_STRING(matcheddn, lmatcheddn, 1);
				ldap_memfree(lmatcheddn);
			}
	}
	RETURN_TRUE;
}
/* }}} */
#endif

/* {{{ proto resource ldap_first_reference(resource link, resource result)
   Return first reference */
PHP_FUNCTION(ldap_first_reference)
{
	zval *link, *result;
	ldap_linkdata *ld;
	ldap_resultentry *resultentry;
	LDAPMessage *ldap_result, *entry;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS(), "rr", &link, &result) != SUCCESS) {
		dprint1("ldap_first_reference called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_first_reference can't fetch ldap CTX!\n");
		RETURN_FALSE;
	}

	if ((ldap_result = (LDAPMessage *)zend_fetch_resource(Z_RES_P(result), "ldap result", le_result)) == NULL) {
		dprint1("ldap_first_reference can't fetch ldap result!\n");
		RETURN_FALSE;
	}

	if ((entry = ldap_first_reference(ld->link, ldap_result)) == NULL) {
		dprint1("ldap_first_reference failed!\n");
		RETVAL_FALSE;
	} else {
		dprint2("ldap_first_reference done.\n");
		resultentry = (ldap_resultentry *)emalloc(sizeof(ldap_resultentry));
		RETVAL_RES(zend_register_resource(resultentry, le_result_entry));
		ZVAL_COPY(&resultentry->res, result);
		resultentry->data = entry;
		resultentry->ber = NULL;
	}
}
/* }}} */

/* {{{ proto resource ldap_next_reference(resource link, resource reference_entry)
   Get next reference */
PHP_FUNCTION(ldap_next_reference)
{
	zval *link, *result_entry;
	ldap_linkdata *ld;
	ldap_resultentry *resultentry, *resultentry_next;
	LDAPMessage *entry_next;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS(), "rr", &link, &result_entry) != SUCCESS) {
		dprint1("ldap_next_reference called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_next_reference can't fetch ldap CTX!\n");
		RETURN_FALSE;
	}

	if ((resultentry = (ldap_resultentry *)zend_fetch_resource(Z_RES_P(result_entry), "ldap result entry", le_result_entry)) == NULL) {
		dprint1("ldap_next_reference can't fetch ldap result entry!\n");
		RETURN_FALSE;
	}

	if ((entry_next = ldap_next_reference(ld->link, resultentry->data)) == NULL) {
		dprint1("ldap_next_reference failed!\n");
		RETVAL_FALSE;
	} else {
		dprint2("ldap_next_reference done.\n");
		resultentry_next = (ldap_resultentry *)emalloc(sizeof(ldap_resultentry));
		RETVAL_RES(zend_register_resource(resultentry_next, le_result_entry));
		ZVAL_COPY(&resultentry_next->res, &resultentry->res);
		resultentry_next->data = entry_next;
		resultentry_next->ber = NULL;
	}
}
/* }}} */

#ifdef HAVE_LDAP_PARSE_REFERENCE
/* {{{ proto bool ldap_parse_reference(resource link, resource reference_entry, array referrals)
   Extract information from reference entry */
PHP_FUNCTION(ldap_parse_reference)
{
	zval *link, *result_entry, *referrals;
	ldap_linkdata *ld;
	ldap_resultentry *resultentry;
	char **lreferrals, **refp;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrz", &link, &result_entry, &referrals) != SUCCESS) {
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		RETURN_FALSE;
	}
	if ((resultentry = (ldap_resultentry *)zend_fetch_resource(Z_RES_P(result_entry), "ldap result entry", le_result_entry)) == NULL) {
		RETURN_FALSE;
	}

	if (ldap_parse_reference(ld->link, resultentry->data, &lreferrals, NULL /* &serverctrls */, 0) != LDAP_SUCCESS) {
		RETURN_FALSE;
	}

	zval_dtor(referrals);
	array_init(referrals);
	if (lreferrals != NULL) {
		refp = lreferrals;
		while (*refp) {
			add_next_index_string(referrals, *refp, 1);
			refp++;
		}
		ldap_value_free(lreferrals);
	}
	RETURN_TRUE;
}
/* }}} */
#endif

/* {{{ proto bool ldap_rename(resource link, string dn, string newrdn, string newparent, bool deleteoldrdn);
   Modify the name of an entry */
PHP_FUNCTION(ldap_rename)
{
	zval *link;
	ldap_linkdata *ld;
	int rc;
	char *dn, *newrdn, *newparent;
	size_t dn_len, newrdn_len, newparent_len;
	zend_bool deleteoldrdn;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsssb", &link, &dn, &dn_len, &newrdn, &newrdn_len, &newparent, &newparent_len, &deleteoldrdn) != SUCCESS) {
		dprint1("ldap_rename called with wrong arguments!\n");
		return;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_rename can't fetch ldap CTX!\n");
		RETURN_FALSE;
	}

	if (newparent_len == 0) {
		newparent = NULL;
	}

#if (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10
	rc = ldap_rename_s(ld->link, dn, newrdn, newparent, deleteoldrdn, NULL, NULL);
#else
	if (newparent_len != 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "You are using old LDAP API, newparent must be the empty string, can only modify RDN");
		RETURN_FALSE;
	}
/* could support old APIs but need check for ldap_modrdn2()/ldap_modrdn() */
	rc = ldap_modrdn2_s(ld->link, dn, newrdn, deleteoldrdn);
#endif

	if (rc == LDAP_SUCCESS) {
		dprint2("ldap_rename done. dn=%s, new_dn=%s\n", dn, newrdn);
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

#ifdef HAVE_LDAP_START_TLS_S
/* {{{ proto bool ldap_start_tls(resource link)
   Start TLS */
PHP_FUNCTION(ldap_start_tls)
{
	php_error_docref(NULL TSRMLS_CC, E_WARNING,"ldap_start_tls support disabled due to custom TrueConf implementation");
	RETURN_FALSE;

	/*zval *link;
	ldap_linkdata *ld;
	int rc, protocol = LDAP_VERSION3;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &link) != SUCCESS) {
		return;
	}

	ZEND_FETCH_RESOURCE(ld, ldap_linkdata *, &link, -1, "ldap link", le_link);

	if (((rc = ldap_set_option(ld->link, LDAP_OPT_PROTOCOL_VERSION, &protocol)) != LDAP_SUCCESS) ||
		((rc = ldap_start_tls_s(ld->link, NULL, NULL)) != LDAP_SUCCESS)
	) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,"Unable to start TLS: %s", ldap_err2string(rc));
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}*/
}
/* }}} */
#endif
#endif /* (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10 */

#if defined(LDAP_API_FEATURE_X_OPENLDAP) && defined(HAVE_3ARG_SETREBINDPROC)
/* {{{ _ldap_rebind_proc()
*/
int _ldap_rebind_proc(LDAP *ldap, const char *url, ber_tag_t req, ber_int_t msgid, void *params)
{
	ldap_linkdata *ld;
	int retval;
	zval *cb_url;
	zval **cb_args[2];
	zval *cb_retval;
	zval *cb_link = (zval *) params;
	TSRMLS_FETCH();

	ld = (ldap_linkdata *) zend_fetch_resource(&cb_link TSRMLS_CC, -1, "ldap link", NULL, 1, le_link);

	/* link exists and callback set? */
	if (ld == NULL || ld->rebindproc == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Link not found or no callback set");
		return LDAP_OTHER;
	}

	/* callback */
	MAKE_STD_ZVAL(cb_url);
	ZVAL_STRING(cb_url, estrdup(url), 0);
	cb_args[0] = &cb_link;
	cb_args[1] = &cb_url;
	if (call_user_function_ex(EG(function_table), NULL, ld->rebindproc, &cb_retval, 2, cb_args, 0, NULL TSRMLS_CC) == SUCCESS && cb_retval) {
		convert_to_long_ex(&cb_retval);
		retval = Z_LVAL_P(cb_retval);
		zval_ptr_dtor(&cb_retval);
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "rebind_proc PHP callback failed");
		retval = LDAP_OTHER;
	}
	zval_dtor(cb_url);
	FREE_ZVAL(cb_url);
	return retval;
}
/* }}} */

/* {{{ proto bool ldap_set_rebind_proc(resource link, string callback)
   Set a callback function to do re-binds on referral chasing. */
PHP_FUNCTION(ldap_set_rebind_proc)
{
	zval *link, *callback;
	ldap_linkdata *ld;
	char *callback_name;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz", &link, &callback) != SUCCESS) {
		RETURN_FALSE;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(callback) == IS_STRING && Z_STRLEN_P(callback) == 0) {
		/* unregister rebind procedure */
		if (ld->rebindproc != NULL) {
			zval_dtor(ld->rebindproc);
			ld->rebindproc = NULL;
			ldap_set_rebind_proc(ld->link, NULL, NULL);
		}
		RETURN_TRUE;
	}

	/* callable? */
	if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Two arguments expected for '%s' to be a valid callback", callback_name);
		efree(callback_name);
		RETURN_FALSE;
	}
	efree(callback_name);

	/* register rebind procedure */
	if (ld->rebindproc == NULL) {
		ldap_set_rebind_proc(ld->link, _ldap_rebind_proc, (void *) link);
	} else {
		zval_dtor(ld->rebindproc);
	}

	ALLOC_ZVAL(ld->rebindproc);
	*ld->rebindproc = *callback;
	zval_copy_ctor(ld->rebindproc);
	RETURN_TRUE;
}
/* }}} */
#endif

/* {{{ proto bool ldap_set_rebind_proc(resource link, string callback)
   Set a callback function to do re-binds on referral chasing. */

static unsigned long  g_ldap_refferal_qfc( LDAP*       PrimaryConnection,
						   LDAP*       ReferralFromConnection,
						   wchar_t*      NewDN,
						   char*       HostName,
						   unsigned long       PortNumber,
						   void*       SecAuthIdentity,    // If NULL, use CurrentUser below
						   void*       CurrentUserToken,   // pointer to current user LUID.
						   LDAP*       *ConnectionToUse)
{
	TSRMLS_FETCH();

	ldap_linkdata *ld;
	zval cb_url;
	zval cb_port;

	zval cb_args[2];
	zval cb_retval;
//	zval *cb_link = (zval *) params;
	*ConnectionToUse = NULL;

	ld = global_ld;
	if (!ld)return 0;

	/* link exists and callback set? */
	if (ld == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Link not found or no callback set");
		return LDAP_OTHER;
	}

	/* callback */
	ZVAL_STRING(&cb_url, HostName);
	ZVAL_LONG(&cb_port, PortNumber);
	//	cb_args[0] = &cb_link;
	cb_args[0] = cb_url;
	cb_args[1] = cb_port;

	if (call_user_function_ex(EG(function_table), NULL, &ld->rebindproc, &cb_retval, 2, cb_args, 0, NULL TSRMLS_CC) == SUCCESS && !Z_ISUNDEF(cb_retval)) {
		ldap_linkdata *new_ld = (ldap_linkdata *)zend_fetch_resource_ex(&cb_retval TSRMLS_CC, "ldap link", le_link);
		if (new_ld != NULL && !(Z_TYPE(cb_retval) == IS_NULL || Z_TYPE(cb_retval) == IS_TRUE))
		{
			*ConnectionToUse = new_ld->link;
		}
	}
	zval_dtor(&cb_url);
	zval_dtor(&cb_port);
	return 0;
}

PHP_FUNCTION(ldap_set_rebind_proc)
{
	zval *link, *callback;
	ldap_linkdata *ld;
	zend_string *callback_name;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz", &link, &callback) != SUCCESS) {
		dprint1("ldap_set_rebind_proc called with wrong arguments!\n");
		global_ld = NULL;
		RETURN_FALSE;
	}

	if ((Z_TYPE_P(link) == IS_NULL || Z_TYPE_P(link) == IS_FALSE))
	{
		global_ld = NULL;
		RETURN_FALSE;
	}

	if ((ld = (ldap_linkdata *)zend_fetch_resource(Z_RES_P(link), "ldap link", le_link)) == NULL) {
		dprint1("ldap_set_rebind_proc can't fetch ldap CTX!\n");
		RETURN_FALSE;
	}
	global_ld = ld;

	/* callable? */
	if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Two arguments expected for '%s' to be a valid callback", callback_name->val);
		zend_string_release(callback_name);
		global_ld = NULL;
		RETURN_FALSE;
	}
	zend_string_release(callback_name);

#ifdef _WIN32
	/* register rebind procedure */
	if (Z_ISUNDEF(ld->rebindproc))
	{
		LDAP_REFERRAL_CALLBACK cb;
		unsigned long ldresult = LDAP_SUCCESS;
		cb.SizeOfCallbacks = sizeof(LDAP_REFERRAL_CALLBACK);
		cb.QueryForConnection = g_ldap_refferal_qfc;
		cb.NotifyRoutine = 0;
		cb.DereferenceRoutine = 0;

		ldresult = ldap_set_option(ld->link, LDAP_OPT_REFERRAL_CALLBACK, (void*)&cb);
		if (ldresult != LDAP_SUCCESS)
		{
			global_ld = NULL;
			RETURN_FALSE;
		}
	}
	else {
		zval_ptr_dtor(&ld->rebindproc);
	}
#endif

	ZVAL_COPY(&ld->rebindproc, callback);
	dprint2("ldap_set_rebind_proc done.\n");
	RETURN_TRUE;
}
/* }}} */

PHP_FUNCTION(test_call_rebind_callback)
{
	LDAP* val = nullptr;
	g_ldap_refferal_qfc(val,val,L"","",0,NULL,0, &val);
}

#ifdef STR_TRANSLATION
/* {{{ php_ldap_do_translate
 */
static void php_ldap_do_translate(INTERNAL_FUNCTION_PARAMETERS, int way)
{
	char *value;
	int result, ldap_len;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &value, &value_len) != SUCCESS) {
		return;
	}

	if (value_len == 0) {
		RETURN_FALSE;
	}

	if (way == 1) {
		result = ldap_8859_to_t61(&value, &value_len, 0);
	} else {
		result = ldap_t61_to_8859(&value, &value_len, 0);
	}

	if (result == LDAP_SUCCESS) {
		RETVAL_STRINGL(value, value_len, 1);
		free(value);
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Conversion from iso-8859-1 to t61 failed: %s", ldap_err2string(result));
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ proto string ldap_t61_to_8859(string value)
   Translate t61 characters to 8859 characters */
PHP_FUNCTION(ldap_t61_to_8859)
{
	php_ldap_do_translate(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto string ldap_8859_to_t61(string value)
   Translate 8859 characters to t61 characters */
PHP_FUNCTION(ldap_8859_to_t61)
{
	php_ldap_do_translate(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */
#endif

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_connect, 0, 0, 0)
	ZEND_ARG_INFO(0, hostname)
	ZEND_ARG_INFO(0, port)
	ZEND_ARG_INFO(0, use_ssl)

#ifdef HAVE_ORALDAP
	ZEND_ARG_INFO(0, wallet)
	ZEND_ARG_INFO(0, wallet_passwd)
	ZEND_ARG_INFO(0, authmode)
#endif
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_resource, 0, 0, 1)
	ZEND_ARG_INFO(0, link_identifier)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_bind, 0, 0, 1)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, bind_rdn)
	ZEND_ARG_INFO(0, bind_password)
ZEND_END_ARG_INFO()

#ifdef HAVE_LDAP_SASL
ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_sasl_bind, 0, 0, 1)
	ZEND_ARG_INFO(0, link)
	ZEND_ARG_INFO(0, binddn)
	ZEND_ARG_INFO(0, password)
	ZEND_ARG_INFO(0, sasl_mech)
	ZEND_ARG_INFO(0, sasl_realm)
	ZEND_ARG_INFO(0, sasl_authz_id)
	ZEND_ARG_INFO(0, props)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_read, 0, 0, 3)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, base_dn)
	ZEND_ARG_INFO(0, filter)
	ZEND_ARG_INFO(0, attributes)
	ZEND_ARG_INFO(0, attrsonly)
	ZEND_ARG_INFO(0, sizelimit)
	ZEND_ARG_INFO(0, timelimit)
	ZEND_ARG_INFO(0, deref)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_list, 0, 0, 3)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, base_dn)
	ZEND_ARG_INFO(0, filter)
	ZEND_ARG_INFO(0, attributes)
	ZEND_ARG_INFO(0, attrsonly)
	ZEND_ARG_INFO(0, sizelimit)
	ZEND_ARG_INFO(0, timelimit)
	ZEND_ARG_INFO(0, deref)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_search, 0, 0, 3)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, base_dn)
	ZEND_ARG_INFO(0, filter)
	ZEND_ARG_INFO(0, attributes)
	ZEND_ARG_INFO(0, attrsonly)
	ZEND_ARG_INFO(0, sizelimit)
	ZEND_ARG_INFO(0, timelimit)
	ZEND_ARG_INFO(0, deref)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_count_entries, 0, 0, 2)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, result_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_first_entry, 0, 0, 2)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, result_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_next_entry, 0, 0, 2)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, result_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_get_entries, 0, 0, 2)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, result_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_first_attribute, 0, 0, 2)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, result_entry_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_next_attribute, 0, 0, 2)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, result_entry_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_get_attributes, 0, 0, 2)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, result_entry_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_get_values, 0, 0, 3)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, result_entry_identifier)
	ZEND_ARG_INFO(0, attribute)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_get_values_len, 0, 0, 3)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, result_entry_identifier)
	ZEND_ARG_INFO(0, attribute)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_get_dn, 0, 0, 2)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, result_entry_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_explode_dn, 0, 0, 2)
	ZEND_ARG_INFO(0, dn)
	ZEND_ARG_INFO(0, with_attrib)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_dn2ufn, 0, 0, 1)
	ZEND_ARG_INFO(0, dn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_add, 0, 0, 3)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, dn)
	ZEND_ARG_INFO(0, entry)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_delete, 0, 0, 2)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, dn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_modify, 0, 0, 3)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, dn)
	ZEND_ARG_INFO(0, entry)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_mod_add, 0, 0, 3)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, dn)
	ZEND_ARG_INFO(0, entry)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_mod_replace, 0, 0, 3)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, dn)
	ZEND_ARG_INFO(0, entry)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_mod_del, 0, 0, 3)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, dn)
	ZEND_ARG_INFO(0, entry)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_err2str, 0, 0, 1)
	ZEND_ARG_INFO(0, errno)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_compare, 0, 0, 4)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, dn)
	ZEND_ARG_INFO(0, attribute)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_sort, 0, 0, 3)
	ZEND_ARG_INFO(0, link)
	ZEND_ARG_INFO(0, result)
	ZEND_ARG_INFO(0, sortfilter)
ZEND_END_ARG_INFO()

#if (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10
ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_rename, 0, 0, 5)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, dn)
	ZEND_ARG_INFO(0, newrdn)
	ZEND_ARG_INFO(0, newparent)
	ZEND_ARG_INFO(0, deleteoldrdn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_get_option, 0, 0, 3)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, option)
	ZEND_ARG_INFO(1, retval)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_set_option, 0, 0, 3)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, option)
	ZEND_ARG_INFO(0, newval)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_first_reference, 0, 0, 2)
	ZEND_ARG_INFO(0, link)
	ZEND_ARG_INFO(0, result)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_next_reference, 0, 0, 2)
	ZEND_ARG_INFO(0, link)
	ZEND_ARG_INFO(0, entry)
ZEND_END_ARG_INFO()

#ifdef HAVE_LDAP_PARSE_REFERENCE
ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_parse_reference, 0, 0, 3)
	ZEND_ARG_INFO(0, link)
	ZEND_ARG_INFO(0, entry)
	ZEND_ARG_INFO(1, referrals)
ZEND_END_ARG_INFO()
#endif


#ifdef HAVE_LDAP_PARSE_RESULT
ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_parse_result, 0, 0, 3)
	ZEND_ARG_INFO(0, link)
	ZEND_ARG_INFO(0, result)
	ZEND_ARG_INFO(1, errcode)
	ZEND_ARG_INFO(1, matcheddn)
	ZEND_ARG_INFO(1, errmsg)
	ZEND_ARG_INFO(1, referrals)
ZEND_END_ARG_INFO()
#endif
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_set_rebind_proc, 0, 0, 2)
	ZEND_ARG_INFO(0, link)
	ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

#ifdef STR_TRANSLATION
ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_t61_to_8859, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ldap_8859_to_t61, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()
#endif
/* }}} */

/*
	This is just a small subset of the functionality provided by the LDAP library. All the
	operations are synchronous. Referrals are not handled automatically.
*/
/* {{{ ldap_functions[]
 */
const zend_function_entry ldap_functions[] = {
	PHP_FE(ldap_connect,								arginfo_ldap_connect)
	PHP_FALIAS(ldap_close,		ldap_unbind,			arginfo_ldap_resource)
	PHP_FE(ldap_bind,									arginfo_ldap_bind)
#ifdef HAVE_LDAP_SASL
	PHP_FE(ldap_sasl_bind,								arginfo_ldap_sasl_bind)
#endif
	PHP_FE(ldap_unbind,									arginfo_ldap_resource)
	PHP_FE(ldap_read,									arginfo_ldap_read)
	PHP_FE(ldap_list,									arginfo_ldap_list)
	PHP_FE(ldap_search,									arginfo_ldap_search)
	PHP_FE(ldap_free_result,							arginfo_ldap_resource)
	PHP_FE(ldap_count_entries,							arginfo_ldap_count_entries)
	PHP_FE(ldap_first_entry,							arginfo_ldap_first_entry)
	PHP_FE(ldap_next_entry,								arginfo_ldap_next_entry)
	PHP_FE(ldap_get_entries,							arginfo_ldap_get_entries)
	PHP_FE(ldap_first_attribute,						arginfo_ldap_first_attribute)
	PHP_FE(ldap_next_attribute,							arginfo_ldap_next_attribute)
	PHP_FE(ldap_get_attributes,							arginfo_ldap_get_attributes)
	PHP_FALIAS(ldap_get_values,	ldap_get_values_len,	arginfo_ldap_get_values)
	PHP_FE(ldap_get_values_len,							arginfo_ldap_get_values_len)
	PHP_FE(ldap_get_dn,									arginfo_ldap_get_dn)
	PHP_FE(ldap_explode_dn,								arginfo_ldap_explode_dn)
	PHP_FE(ldap_dn2ufn,									arginfo_ldap_dn2ufn)
	PHP_FE(ldap_add,									arginfo_ldap_add)
	PHP_FE(ldap_delete,									arginfo_ldap_delete)
	PHP_FALIAS(ldap_modify,		ldap_mod_replace,		arginfo_ldap_modify)

/* additional functions for attribute based modifications, Gerrit Thomson */
	PHP_FE(ldap_mod_add,								arginfo_ldap_mod_add)
	PHP_FE(ldap_mod_replace,							arginfo_ldap_mod_replace)
	PHP_FE(ldap_mod_del,								arginfo_ldap_mod_del)
/* end gjt mod */

	PHP_FE(ldap_errno,									arginfo_ldap_resource)
	PHP_FE(ldap_err2str,								arginfo_ldap_err2str)
	PHP_FE(ldap_error,									arginfo_ldap_resource)
	PHP_FE(ldap_compare,								arginfo_ldap_compare)
	PHP_FE(ldap_sort,									arginfo_ldap_sort)

#if (LDAP_API_VERSION > 2000) || HAVE_NSLDAP || HAVE_ORALDAP_10
	PHP_FE(ldap_rename,									arginfo_ldap_rename)
	PHP_FE(ldap_get_option,								arginfo_ldap_get_option)
	PHP_FE(ldap_set_option,								arginfo_ldap_set_option)
	PHP_FE(ldap_first_reference,						arginfo_ldap_first_reference)
	PHP_FE(ldap_next_reference,							arginfo_ldap_next_reference)
#ifdef HAVE_LDAP_PARSE_REFERENCE
	PHP_FE(ldap_parse_reference,						arginfo_ldap_parse_reference)
#endif
#ifdef HAVE_LDAP_PARSE_RESULT
	PHP_FE(ldap_parse_result,							arginfo_ldap_parse_result)
#endif
#ifdef HAVE_LDAP_START_TLS_S
	PHP_FE(ldap_start_tls,								arginfo_ldap_resource)
#endif
#endif

	PHP_FE(ldap_set_rebind_proc,						arginfo_ldap_set_rebind_proc)
	PHP_FE(test_call_rebind_callback,					NULL)

#ifdef STR_TRANSLATION
	PHP_FE(ldap_t61_to_8859,							arginfo_ldap_t61_to_8859)
	PHP_FE(ldap_8859_to_t61,							arginfo_ldap_8859_to_t61)
#endif

	PHP_FE_END
};
/* }}} */

zend_module_entry ldap_module_entry = { /* {{{ */
	STANDARD_MODULE_HEADER,
	"ldap",
	ldap_functions,
	PHP_MINIT(ldap),
	PHP_MSHUTDOWN(ldap),
	NULL,
	NULL,
	PHP_MINFO(ldap),
	NO_VERSION_YET,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

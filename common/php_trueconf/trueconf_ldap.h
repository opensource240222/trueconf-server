#ifndef TRUECONF_LDAP_H
#define TRUECONF_LDAP_H

BEGIN_EXTERN_C()

extern zend_module_entry trueconf_ldap_module_entry;
#define phpext_ldap_ptr &trueconf_ldap_module_entry

#ifdef PHP_WIN32
#	define TRUECONF_LDAP_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_LDAP_API __attribute__ ((visibility("default")))
#else
#	define PHP_LDAP_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(trueconf_ldap);
PHP_MSHUTDOWN_FUNCTION(trueconf_ldap);
PHP_MINFO_FUNCTION(trueconf_ldap);

PHP_FUNCTION(trueconf_ldap_init);
PHP_FUNCTION(trueconf_ldap_deinit);
PHP_FUNCTION(trueconf_ldap_connect);
PHP_FUNCTION(trueconf_ldap_start_cache_update);
PHP_FUNCTION(trueconf_ldap_get_all_groups);
PHP_FUNCTION(trueconf_ldap_get_group_users);
PHP_FUNCTION(trueconf_ldap_get_address_book);
PHP_FUNCTION(trueconf_ldap_get_address_book_of_group);
PHP_FUNCTION(trueconf_ldap_get_all_attributes);
PHP_FUNCTION(trueconf_ldap_search_for_user);
PHP_FUNCTION(trueconf_ldap_search);
PHP_FUNCTION(trueconf_ldap_search_for_trust_user);
PHP_FUNCTION(trueconf_ldap_get_all_users);
PHP_FUNCTION(trueconf_ldap_get_last_cache_update);
PHP_FUNCTION(trueconf_ldap_check_login);
PHP_FUNCTION(trueconf_ldap_check_password);

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_ldap_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_ldap_get_all_groups, 0, 0, 0)
ZEND_ARG_INFO(1, cookie)
ZEND_ARG_INFO(0, page_size)
ZEND_ARG_INFO(0, query_string)
ZEND_ARG_INFO(0, sort_attr)
ZEND_ARG_INFO(0, sort_order)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_ldap_get_group_users, 0, 0, 1)
ZEND_ARG_INFO(0, parent_group)
ZEND_ARG_INFO(1, cookie)
ZEND_ARG_INFO(0, page_size)
ZEND_ARG_INFO(0, custom_attrs)
ZEND_ARG_INFO(0, query_string)
ZEND_ARG_INFO(0, sort_attr)
ZEND_ARG_INFO(0, sort_order)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_ldap_get_address_book, 0, 0, 1)
ZEND_ARG_INFO(0, user_id)
ZEND_ARG_INFO(1, abLimits)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_ldap_get_address_book_of_group, 0, 0, 1)
ZEND_ARG_INFO(0, gid)
ZEND_ARG_INFO(1, abLimits)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_ldap_get_all_attributes, 0, 0, 0)
ZEND_ARG_INFO(0, dn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_ldap_search, 0, 0, 1)
ZEND_ARG_INFO(0, ldap_filter)
ZEND_ARG_INFO(1, cookie)
ZEND_ARG_INFO(0, page_size)
ZEND_ARG_INFO(0, custom_attrs)
ZEND_ARG_INFO(0, sort_attr)
ZEND_ARG_INFO(0, sort_order)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_ldap_search_for_trust_user, 0, 0, 1)
ZEND_ARG_INFO(0, user_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_ldap_get_all_users, 0, 0, 0)
ZEND_ARG_INFO(1, cookie)
ZEND_ARG_INFO(0, page_size)
ZEND_ARG_INFO(0, custom_attrs)
ZEND_ARG_INFO(0, query_string)
ZEND_ARG_INFO(0, sort_attr)
ZEND_ARG_INFO(0, sort_order)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_ldap_check_login, 0, 0, 1)
ZEND_ARG_INFO(0, user_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_ldap_check_password, 0, 0, 3)
ZEND_ARG_INFO(0, login)
ZEND_ARG_INFO(0, password)
ZEND_ARG_INFO(0, distinguishedName)
ZEND_END_ARG_INFO()

#ifdef ZTS
#define TRUECONF_LDAP_G(v) TSRMG(trueconf_ldap_globals_id, zend_ldap_globals *, v)
#else
#define LDAP_G(v) (trueconf_ldap_globals.v)
#endif

END_EXTERN_C()

#endif

#ifndef PHP_LDAP_H
#define PHP_LDAP_H

BEGIN_EXTERN_C()

extern zend_module_entry ldap_module_entry;
#define phpext_ldap_ptr &ldap_module_entry

#ifdef PHP_WIN32
#	define PHP_LDAP_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_LDAP_API __attribute__ ((visibility("default")))
#else
#	define PHP_LDAP_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(ldap);
PHP_MSHUTDOWN_FUNCTION(ldap);
PHP_MINFO_FUNCTION(ldap);

PHP_FUNCTION(ldap_connect);
PHP_FUNCTION(ldap_bind);
PHP_FUNCTION(ldap_sasl_bind);
PHP_FUNCTION(ldap_unbind);
PHP_FUNCTION(ldap_read);
PHP_FUNCTION(ldap_list);
PHP_FUNCTION(ldap_search);
PHP_FUNCTION(ldap_free_result);
PHP_FUNCTION(ldap_count_entries);
PHP_FUNCTION(ldap_first_entry);
PHP_FUNCTION(ldap_next_entry);
PHP_FUNCTION(ldap_get_entries);
PHP_FUNCTION(ldap_first_attribute);
PHP_FUNCTION(ldap_next_attribute);
PHP_FUNCTION(ldap_get_attributes);
PHP_FUNCTION(ldap_get_values_len);
PHP_FUNCTION(ldap_get_dn);
PHP_FUNCTION(ldap_explode_dn);
PHP_FUNCTION(ldap_dn2ufn);
PHP_FUNCTION(ldap_add);
PHP_FUNCTION(ldap_mod_replace);
PHP_FUNCTION(ldap_mod_add);
PHP_FUNCTION(ldap_mod_del);
PHP_FUNCTION(ldap_delete);
PHP_FUNCTION(ldap_errno);
PHP_FUNCTION(ldap_err2str);
PHP_FUNCTION(ldap_error);
PHP_FUNCTION(ldap_compare);
PHP_FUNCTION(ldap_sort);
PHP_FUNCTION(ldap_get_option);
PHP_FUNCTION(ldap_set_option);
PHP_FUNCTION(ldap_parse_result);
PHP_FUNCTION(ldap_first_reference);
PHP_FUNCTION(ldap_next_reference);
PHP_FUNCTION(ldap_parse_reference);
PHP_FUNCTION(ldap_rename);
PHP_FUNCTION(ldap_start_tls);
PHP_FUNCTION(ldap_set_rebind_proc);
PHP_FUNCTION(ldap_set_rebind_proc);
PHP_FUNCTION(test_call_rebind_callback);
PHP_FUNCTION(ldap_t61_to_8859);
PHP_FUNCTION(ldap_8859_to_t61);

#ifdef ZTS
#define LDAP_G(v) TSRMG(ldap_globals_id, zend_ldap_globals *, v)
#else
#define LDAP_G(v) (ldap_globals.v)
#endif

END_EXTERN_C()

#endif

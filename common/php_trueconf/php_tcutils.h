#ifndef PHP_TCUTILS_H
#define PHP_TCUTILS_H

BEGIN_EXTERN_C()

extern zend_module_entry tcutils_module_entry;
#define phpext_tcutils_ptr &tcutils_module_entry

#ifdef PHP_WIN32
#	define PHP_TCUTILS_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_TCUTILS_API __attribute__ ((visibility("default")))
#else
#	define PHP_TCUTILS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(tcutils);
PHP_MINFO_FUNCTION(tcutils);

PHP_FUNCTION(trueconf_get_timezone);
PHP_FUNCTION(trueconf_get_timezone_offset);
PHP_FUNCTION(trueconf_file_exists);
PHP_FUNCTION(trueconf_strcasecmp);
PHP_FUNCTION(trueconf_array_change_key_case);
PHP_FUNCTION(trueconf_array_change_value_case);
PHP_FUNCTION(trueconf_array_has_string_ci);
PHP_FUNCTION(trueconf_array_search);
PHP_FUNCTION(trueconf_num_contains);
PHP_FUNCTION(trueconf_num_starts_with);
PHP_FUNCTION(trueconf_num_ends_with);
PHP_FUNCTION(trueconf_str_cmp);
PHP_FUNCTION(trueconf_str_starts_with);
PHP_FUNCTION(trueconf_str_ends_with);
PHP_FUNCTION(trueconf_str_contains);
PHP_FUNCTION(trueconf_str_contains);
PHP_FUNCTION(ldap_get_default_domain);
PHP_FUNCTION(ldap_get_default_server);
PHP_FUNCTION(trueconf_case_sort);
PHP_FUNCTION(trueconf_get_file_size);
#ifdef  _WIN32
PHP_FUNCTION(trueconf_login_windows_user);
PHP_FUNCTION(trueconf_authorize_user);
PHP_FUNCTION(trueconf_get_windows_timezone_name);
#endif //_WIN32
#ifdef __linux__
PHP_FUNCTION(trueconf_get_linux_timezone_name);
#endif //__linux__

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_str_compare_functions, 0, 0, 2)
ZEND_ARG_INFO(0, strA)
ZEND_ARG_INFO(0, strB)
ZEND_ARG_INFO(0, ignoreCase)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_num_compare_functions, 0, 0, 2)
ZEND_ARG_INFO(0, numA)
ZEND_ARG_INFO(0, numB)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_strcasecmp, 0, 0, 2)
ZEND_ARG_INFO(0, strA)
ZEND_ARG_INFO(0, strB)
ZEND_ARG_INFO(0, locale)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_array_change_entry_case, 0, 0, 1)
ZEND_ARG_INFO(0, inputArray)
ZEND_ARG_INFO(0, caseType)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_array_has_string_ci, 0, 0, 2)
ZEND_ARG_INFO(0, inputArray)
ZEND_ARG_INFO(0, str)
ZEND_ARG_INFO(0, searchInNumeric)
ZEND_END_ARG_INFO()



ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_str_compare_function, 0, 0, 3)
ZEND_ARG_INFO(0, strA)
ZEND_ARG_INFO(0, strB)
ZEND_ARG_INFO(0, compareMethod)
ZEND_ARG_INFO(0, ignoreCase)
ZEND_END_ARG_INFO()



ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_array_search, 0, 0, 2)
ZEND_ARG_INFO(0, needle)
ZEND_ARG_INFO(0, arr)
ZEND_ARG_INFO(0, searchMethod)
ZEND_ARG_INFO(0, ignoreCase)
ZEND_ARG_INFO(0, flipArgs)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_nested_sort, 0, 0, 2)
ZEND_ARG_INFO(1, arr)
ZEND_ARG_INFO(0, keys)
ZEND_ARG_INFO(0, ignoreCase)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_case_sort, 0, 0, 1)
ZEND_ARG_INFO(1, arr)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_trueconf_file_name, 0, 0, 1)
ZEND_ARG_INFO(0, fileName)
ZEND_END_ARG_INFO()

#define TC_PHP_STR_SEARCH_METHOD_SUBSTRING 0
#define TC_PHP_STR_SEARCH_METHOD_STARTS_WITH 1
#define TC_PHP_STR_SEARCH_METHOD_ENDS_WITH 2
#define TC_PHP_STR_SEARCH_METHOD_EQUAL 3

bool php_str_iccmpm_cc(int method, char * subject, int subjectLength, char * needle, int needleLength);

#ifdef ZTS
#define TCUTILS_G(v) TSRMG(tcutils_globals_id, zend_tcutils_globals *, v)
#else
#define TCUTILS_G(v) (tcutils_globals.v)
#endif

END_EXTERN_C()

#endif


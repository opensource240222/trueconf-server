#ifndef PHP_REG_H
#define PHP_REG_H

BEGIN_EXTERN_C()

extern zend_module_entry reg_module_entry;
#define phpext_reg_ptr &reg_module_entry

#ifdef PHP_WIN32
#	define PHP_REG_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_REG_API __attribute__ ((visibility("default")))
#else
#	define PHP_REG_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(reg);
PHP_MINFO_FUNCTION(reg);

PHP_FUNCTION(reg_read_value);
PHP_FUNCTION(reg_write_value);
PHP_FUNCTION(reg_delete_value);
PHP_FUNCTION(reg_get_value_names);
PHP_FUNCTION(reg_create_key);
PHP_FUNCTION(reg_delete_key);
PHP_FUNCTION(reg_get_sub_keys);
PHP_FUNCTION(reg_key_exists);
PHP_FUNCTION(reg_value_exists);
PHP_FUNCTION(reg_read_values);
PHP_FUNCTION(reg_get_key_info);
PHP_FUNCTION(reg_read_values_assoc);

#ifdef ZTS
#define REG_G(v) TSRMG(reg_globals_id, zend_reg_globals *, v)
#else
#define REG_G(v) (reg_globals.v)
#endif

END_EXTERN_C()

#endif

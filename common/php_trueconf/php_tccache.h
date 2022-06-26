#ifndef PHP_TCCACHE_H
#define PHP_TCCACHE_H

BEGIN_EXTERN_C()

extern zend_module_entry tccache_module_entry;
#define phpext_tccache_ptr &tccache_module_entry

#ifdef PHP_WIN32
#	define PHP_TCCACHE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_TCCACHE_API __attribute__ ((visibility("default")))
#else
#	define PHP_TCCACHE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(tccache);
PHP_MINFO_FUNCTION(tccache);

PHP_FUNCTION(tccache_set_timeout);
PHP_FUNCTION(tccache_info);
PHP_FUNCTION(tccache_set);
PHP_FUNCTION(tccache_get);
PHP_FUNCTION(tccache_cas);

PHP_FUNCTION(tccache_exists);
PHP_FUNCTION(tccache_delete);
PHP_FUNCTION(tccache_clear);

PHP_FUNCTION(tccache_test_SetTime);
PHP_FUNCTION(tccache_test_setTimeToClear);

#ifdef ZTS
#define TCCACHE_G(v) TSRMG(tccache_globals_id, zend_tccache_globals *, v)
#else
#define TCCACHE_G(v) (tccache_globals.v)
#endif

END_EXTERN_C()

#endif

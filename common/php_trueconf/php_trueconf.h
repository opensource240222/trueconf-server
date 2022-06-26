#ifndef PHP_TRUECONF_H
#define PHP_TRUECONF_H

BEGIN_EXTERN_C()

extern zend_module_entry trueconf_module_entry;
#define phpext_trueconf_ptr &trueconf_module_entry

#ifdef PHP_WIN32
#	define PHP_TRUECONF_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_TRUECONF_API __attribute__ ((visibility("default")))
#else
#	define PHP_TRUECONF_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(trueconf);
PHP_MSHUTDOWN_FUNCTION(trueconf);
PHP_RINIT_FUNCTION(trueconf);
PHP_RSHUTDOWN_FUNCTION(trueconf);
PHP_MINFO_FUNCTION(trueconf);

#ifdef ZTS
#define TRUECONF_G(v) TSRMG(trueconf_globals_id, zend_trueconf_globals *, v)
#else
#define TRUECONF_G(v) (trueconf_globals.v)
#endif

END_EXTERN_C()

#endif

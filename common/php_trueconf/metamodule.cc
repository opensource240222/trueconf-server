#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "common.h"

#include "php.h"
#include "php_ini.h"

#include "php_trueconf.h"
#include "php_ldap.h"
#include "php_reg.h"
#include "php_tcutils.h"
#include "php_tccache.h"
#include "trueconf_ldap.h"
#include "net/DNSUtils/VS_DNSTools.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/Globals.h"

#include "std/debuglog/VS_Debug.h"

#include <vector>
#include <map>

#define DEBUG_CURRENT_MODULE VS_DM_PHP

zend_module_entry trueconf_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"TrueConf PHP extension",
	NULL,
	PHP_MINIT(trueconf),
	PHP_MSHUTDOWN(trueconf),
	PHP_RINIT(trueconf),
	PHP_RSHUTDOWN(trueconf),
	PHP_MINFO(trueconf),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_TRUECONF_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};

std::vector<zend_function_entry> functions;

const char reg_module_ini_string[] = "reg_module.enabled";
const char ldap_module_ini_string[] = "ldap_module.enabled";
const char tcutils_module_ini_string[] = "tcutils_module.enabled";
const char tccache_module_ini_string[] = "tccache_module.enabled";
const char trueconf_ldap_module_ini_string[] = "trueconf_ldap_module.enabled";

static const struct {
	const char* ini;
	zend_module_entry* module;
} modules[] = {
	{ reg_module_ini_string, &reg_module_entry},
	{ tcutils_module_ini_string, &tcutils_module_entry},
	{ tccache_module_ini_string, &tccache_module_entry},
	{ ldap_module_ini_string, &ldap_module_entry},
	{ trueconf_ldap_module_ini_string, &trueconf_ldap_module_entry},
};

#ifdef _WIN32
const char reg_backend_default[] = "registry:force_lm=true";
#else
const char reg_backend_default[] = "NOT VALID BACKEND";
#endif

PHP_INI_BEGIN()
PHP_INI_ENTRY("trueconf.reg_backend", reg_backend_default, PHP_INI_ALL, NULL)
PHP_INI_ENTRY("trueconf.default_root", "TrueConf\\Server", PHP_INI_ALL, NULL)
PHP_INI_ENTRY("trueconf.debug_level", "0x0", PHP_INI_ALL, NULL)
PHP_INI_ENTRY("trueconf.debug_modules", "0x00000000", PHP_INI_ALL, NULL)

// php.ini entries for trueconf modules
// By default, every module but trueconf_ldap is initialized by default
PHP_INI_ENTRY(reg_module_ini_string, "1", PHP_INI_ALL, NULL)
PHP_INI_ENTRY(ldap_module_ini_string, "1", PHP_INI_ALL, NULL)
PHP_INI_ENTRY(tcutils_module_ini_string, "1", PHP_INI_ALL, NULL)
PHP_INI_ENTRY(tccache_module_ini_string, "1", PHP_INI_ALL, NULL)
PHP_INI_ENTRY(trueconf_ldap_module_ini_string, "1", PHP_INI_ALL, NULL)
PHP_INI_END()

PHP_MINIT_FUNCTION(trueconf)
{
	REGISTER_INI_ENTRIES();
	auto dbgLev = INI_STR("trueconf.debug_level");
	auto dbgModules = INI_STR("trueconf.debug_modules");

	unsigned dLevel = strtoul(dbgLev, nullptr, 16);
	unsigned dModules = strtoul(dbgModules, nullptr, 16);

	VS_SetDebug(dLevel, dModules);
	VS_SetDebugFile(stderr);	// stderr is mapped to apache log
	setbuf(stderr, nullptr);	// switch off bufferizing

	for (auto& module : modules)
	{
		if (1 == zend_ini_long(const_cast<char*>(module.ini), strlen(module.ini), 0) &&
			module.module->module_startup_func != NULL)
			module.module->module_startup_func(INIT_FUNC_ARGS_PASSTHRU);
	}

	return SUCCESS;
}

static std::atomic<bool> inited_meta_trueconf{ false };
static std::mutex init_meta_trueconf_mtx;

PHP_MSHUTDOWN_FUNCTION(trueconf)
{
	for (auto& module: modules)
	{
		if (1 == zend_ini_long(const_cast<char*>(module.ini), strlen(module.ini), 0) &&
			module.module->module_shutdown_func != NULL)
			module.module->module_shutdown_func(SHUTDOWN_FUNC_ARGS_PASSTHRU);
	}
	UNREGISTER_INI_ENTRIES();

	if (inited_meta_trueconf)
	{
		net::dns::destroy();
	}
	
	dprint2( "PHP Trueconf was destroyed successfully\n");
	return SUCCESS;
}

PHP_RINIT_FUNCTION(trueconf)
{
	if (!inited_meta_trueconf)
	{
		std::lock_guard<decltype(init_meta_trueconf_mtx)>_{ init_meta_trueconf_mtx };
			   
		if (inited_meta_trueconf)
		{
			return SUCCESS;
		}

		auto backend_value = INI_STR("trueconf.reg_backend");	// pointer to static string is returned
		auto reg_root = INI_STR("trueconf.default_root");		// pointer to static string is returned

		VS_RegistryKey::SetDefaultRoot(reg_root);
		if (!VS_RegistryKey::InitDefaultBackend(backend_value)) {
			dprint0("PHP Trueconf: registry backend initialization failed! Backend=%s, Default Root='%s'\n", backend_value, reg_root);
			// PHP calls exit(1) if PHP_RINIT_FUNCTION returns anything but SUCCESS.
			// Since PHP_RINIT_FUNCTION is called from multiple threads simultaneously
			// it is possible that two threads will try to call exit(1).
			// But the exit() function is not thread safe so it will lead to a crash.
			// So we can't return FAILURE (and obviosly shouldn't return SUCCESS)
			// and the only safe way to continue is to call exit(1) from here
			// while we are holding a mutex.
			::exit(1);
		}

		net::dns_tools::init_loaded_options();

		std::string server_name;
		if (VS_RegistryKey(false, "").GetString(server_name, "Endpoint") && !server_name.empty())
			g_tr_endpoint_name = std::move(server_name);

		inited_meta_trueconf = true;
	}

	for (auto& module: modules)
	{
		if (1 == zend_ini_long(const_cast<char*>(module.ini), strlen(module.ini), 0) &&
			module.module->request_startup_func != NULL)
			module.module->request_startup_func(INIT_FUNC_ARGS_PASSTHRU);
	}
	dprint2( "Metamodule RINIT_FUNCTION done\n");
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(trueconf)
{
	for (auto& module: modules)
	{
		if (1 == zend_ini_long(const_cast<char*>(module.ini), strlen(module.ini), 0) &&
			module.module->request_shutdown_func != NULL)
			module.module->request_shutdown_func(SHUTDOWN_FUNC_ARGS_PASSTHRU);
	}
	dprint2( "Metamodule RSHUTDOWN_FUNCTION done\n");
	return SUCCESS;
}

PHP_MINFO_FUNCTION(trueconf)
{
	for (auto& module: modules)
	{
		if (1 == zend_ini_long(const_cast<char*>(module.ini), strlen(module.ini), 0) &&
			module.module->info_func != NULL)
			module.module->info_func(ZEND_MODULE_INFO_FUNC_ARGS_PASSTHRU);
	}
}

zend_module_entry *get_module_internal()
{
	functions.clear();

	for (auto& module: modules)
	{
		for (unsigned j = 0; module.module->functions[j].handler != NULL; ++j)
			functions.push_back(module.module->functions[j]);
	}
	zend_function_entry empty;
	memset(&empty, 0, sizeof(empty));
	functions.push_back( empty );

	trueconf_module_entry.functions = &functions[0];
	return &trueconf_module_entry;
}

#ifdef COMPILE_DL_TRUECONF
BEGIN_EXTERN_C()
ZEND_DLEXPORT zend_module_entry *get_module(void) { return get_module_internal(); }
END_EXTERN_C()
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "common.h"

#include "php.h"
#include "php_ini.h"
#include "ext/standard/php_string.h"
#include "ext/standard/info.h"

#include "php_tcutils.h"
#include "vs_zend_parse_parameters.h"

#include "tcauthutils_php.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/ThreadUtils.h"
#include "std-generic/cpplib/utf8.h"
#include "std/cpplib/VS_ThreadPool.h"
#include "std/cpplib/VS_Singleton.h"
#include "std-generic/cpplib/macro_utils.h"
#include "std/cpplib/VS_Replace.h"
#include <boost/algorithm/string/predicate.hpp>
#include "ldap_core/liblutil/tc_ldap.h"

#if defined(_WIN32)
#	include <StrSafe.h>
#	include "Windns.h"
#	pragma comment(lib, "Dnsapi.lib")
#	include <lm.h>
#	include <Dsgetdc.h>
#	pragma comment(lib, "Netapi32.lib")
#	include <Winldap.h>
#	pragma comment(lib, "Wldap32.lib")
#	include <io.h>
#	include <fcntl.h>
#elif defined(__linux__)
#	include <time.h>
#	include <unistd.h>
#	include <netdb.h>
#	include <stdio.h>
#	include <sys/stat.h>
#	include <ldap.h>
#endif

#include <cinttypes>
#include <cstdint>
#include <string>
#include <memory>
#include <mutex>
#include "net/DNSUtils/VS_DNS.h"

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_PHP

static bool USE_DEBUG_LOG = false;
static const char *DEBUG_LOG_FILE_PATH = "c:\\php_trueconf.dll.log";

int write_log(const char *format, ...)
{
	static FILE *logFile  = NULL;

	if (USE_DEBUG_LOG && !logFile)
	{
		logFile = fopen(DEBUG_LOG_FILE_PATH, "w");
	}

	if (!logFile ) return 0;

	va_list args;
	va_start(args, format);
	int res = vfprintf(logFile, format, args);
	va_end(args);
	fflush(logFile);
	return res;
}

const zend_function_entry tcutils_functions[] = {

	PHP_FE(trueconf_get_timezone, NULL)
	PHP_FE(trueconf_strcasecmp, arginfo_trueconf_strcasecmp)
	PHP_FE(trueconf_array_change_key_case, arginfo_trueconf_array_change_entry_case)
	PHP_FE(trueconf_array_change_value_case, arginfo_trueconf_array_change_entry_case)
	PHP_FE(trueconf_array_has_string_ci, arginfo_trueconf_array_has_string_ci)
	PHP_FE(trueconf_num_contains, arginfo_trueconf_num_compare_functions)
	PHP_FE(trueconf_num_starts_with, arginfo_trueconf_num_compare_functions)
	PHP_FE(trueconf_num_ends_with, arginfo_trueconf_num_compare_functions)
	PHP_FE(trueconf_str_cmp, arginfo_trueconf_str_compare_function)
	PHP_FE(trueconf_str_starts_with, arginfo_trueconf_str_compare_functions)
	PHP_FE(trueconf_str_ends_with, arginfo_trueconf_str_compare_functions)
	PHP_FE(trueconf_str_contains, arginfo_trueconf_str_compare_functions)
	PHP_FE(trueconf_array_search, arginfo_trueconf_array_search)
	PHP_FE(trueconf_get_timezone_offset, NULL)
	PHP_FE(trueconf_file_exists, NULL)
	PHP_FE(ldap_get_default_domain, NULL)
	PHP_FE(ldap_get_default_server, NULL)
	PHP_FE(trueconf_case_sort, arginfo_trueconf_case_sort)
	PHP_FE(trueconf_get_file_size, arginfo_trueconf_file_name)
#if defined(_WIN32)
	PHP_FE(trueconf_login_windows_user, NULL)
	PHP_FE(trueconf_authorize_user, NULL)
	PHP_FE(trueconf_get_windows_timezone_name, NULL)
#elif defined(__linux__)
	PHP_FE(trueconf_get_linux_timezone_name, NULL)
#endif
	PHP_FE_END
};

zend_module_entry tcutils_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"tcutils",
	tcutils_functions,
	PHP_MINIT(tcutils),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(tcutils),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_TRUECONF_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};


#ifdef COMPILE_DL_TCUTILS
ZEND_GET_MODULE(tcutils)
#endif

PHP_MINIT_FUNCTION(tcutils)
{
	setlocale(LC_ALL, "");

	REGISTER_LONG_CONSTANT("TC_SEARCH_SUBSTRING", TC_PHP_STR_SEARCH_METHOD_SUBSTRING, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("TC_SEARCH_STARTS_WITH", TC_PHP_STR_SEARCH_METHOD_STARTS_WITH, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("TC_SEARCH_ENDS_WITH", TC_PHP_STR_SEARCH_METHOD_ENDS_WITH, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("TC_SEARCH_EQUAL", TC_PHP_STR_SEARCH_METHOD_EQUAL, CONST_PERSISTENT | CONST_CS);
	return SUCCESS;
}

PHP_MINFO_FUNCTION(tcutils)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Trueconf Helper functions", "enabled");
	php_info_print_table_end();
}



int32_t get_timezone_offset() {
	std::tm utc = {}, local = {};
	time_t curr(time(0));

	utc = *gmtime(&curr);
	local = *localtime(&curr);
	const std::chrono::hours bias(local.tm_hour - utc.tm_hour);
	const std::chrono::minutes biasMin(local.tm_min - utc.tm_min);
	return std::chrono::duration_cast<std::chrono::minutes>(bias + biasMin).count();
}

PHP_FUNCTION(trueconf_get_timezone)
{
	RETURN_LONG( get_timezone_offset() );
}

PHP_FUNCTION(trueconf_get_timezone_offset)
{
	RETURN_LONG(get_timezone_offset());
}

PHP_FUNCTION(trueconf_file_exists)
{
	_char(path);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
		&path, &path_len) != SUCCESS)
	{
		dprint1("trueconf_file_exists: parse error\n");
		RETURN_FALSE;
	}

	const char* pPath = path;
#ifdef _WIN32
	std::string corrected_path;
	if (string_view(pPath, path_len).find_first_of('/') != string_view::npos) {
		corrected_path = path;
		VS_ReplaceAll(corrected_path, "/", "\\");
		pPath = corrected_path.c_str();
	}
#endif

	bool res(false);
	if (FILE *file = fopen(pPath, "r")) {
		fclose(file);
		res =  true;
	}

	dprint2("trueconf_file_exists(%s): %d", pPath, res);
	RETURN_BOOL( res );
}

bool test_ldap_connection(const std::string &name, int port, bool secure = false)
{
	LDAP* ldap = tc_ldap::Connect(name, port, secure);
	if(!ldap)
		return false;

	unsigned long ldresult=LDAP_SUCCESS;
	unsigned long ldap_version = 3;

	ldresult = ldap_set_option(ldap,LDAP_OPT_PROTOCOL_VERSION,(void*)&ldap_version);
	ldap_unbind_ext(ldap, nullptr, nullptr);
	return ldresult == LDAP_SUCCESS;
}
//FILE *flogf = fopen("c:\\php_trueconf.log", "w");

std::string GetDomainFromCannonName(string_view canonName, string_view hostName) {
	if (canonName.length() <= hostName.length())
		return {};
	return std::string(canonName.begin() + hostName.length() + 1, canonName.end());
}

std::string GetComputerCanonicalName(const char* hostName) {
	if (!hostName)
		return {};
	struct addrinfo *result = nullptr;
	VS_SCOPE_EXIT{ freeaddrinfo(result); };

	struct addrinfo hints = {};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;
	size_t res = getaddrinfo(hostName, "ldap", &hints, &result);
	if (res != 0 || !result || !result->ai_canonname)
		return {};
	// result of first node is enough, see AI_CANONNAME: https://linux.die.net/man/3/getaddrinfo
	return result->ai_canonname;
}

std::string GetDefaultDomain() {
#ifdef _WIN32
	WSADATA wsaData = {};
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
		return {};
	VS_SCOPE_EXIT{ WSACleanup(); };
#endif

	char hName[1024] = { 0 };
	if (gethostname(hName, sizeof(hName)) != 0)
		return {};

	auto canonName = GetComputerCanonicalName(hName);
	return GetDomainFromCannonName(canonName, hName);
}

PHP_FUNCTION(ldap_get_default_domain)
{
	auto res = GetDefaultDomain();
	dprint2("ldap_get_default_domain: %s\n", res.c_str());
	RETURN_STRINGL(res.c_str(), res.length());
}

PHP_FUNCTION(ldap_get_default_server)
{
	auto res = net::dns::make_srv_lookup("_ldap._tcp." + GetDefaultDomain()).get();
	assert(!res.first.empty());
	if(res.second || res.first.empty()) //err
		RETURN_STRING("");
	std::string res_str = res.first[0].host; res_str += ':'; res_str += std::to_string(res.first[0].port);
	dprint2("ldap_get_default_server: %s\n",res_str.c_str());

	RETURN_STRINGL(res_str.c_str(), res_str.length());
}

#if defined(_WIN32) // Windows only function: trueconf_login_windows_user

void PerformLogin(zval* return_value, const wchar_t *dns_name, const wchar_t *nb_name, const wchar_t *pass)
{
	write_log("PerformLogin dns_name = %S nb_name = %S pass = %S\n", dns_name, nb_name, pass );

	array_init(return_value);

	HANDLE token;

	long res = LogonUserW(dns_name, NULL, pass, LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT, &token);
	if (!res) res = LogonUserW(dns_name, NULL, pass, LOGON32_LOGON_BATCH, LOGON32_PROVIDER_DEFAULT, &token);

	if (!res)
	{
		unsigned long error = GetLastError();
		write_log("LogonUserW failed. error = %d\n", error);

		add_assoc_long(return_value, "erorrCode", error);
		add_assoc_bool(return_value, "result", false);
		add_assoc_string(return_value, "erorrMessage", "LogonUserW failed.");
		return;
	}

	write_log("LogonUserW ok.\n");

	add_assoc_bool(return_value, "result", true);

	zval groups;
	array_init(&groups);

	DWORD get_groups_res(0);
	PTOKEN_GROUPS ptg(NULL);
	do{
		DWORD error(0);
		DWORD dwLength = 0;
		get_groups_res	= GetTokenInformation(token,TokenGroups,(LPVOID) ptg, 0,&dwLength);
		if ((error = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
		{
			write_log("GetTokenInformation failed. error = %d\n", error);
			add_assoc_long(return_value, "erorrCode", error);
			add_assoc_string(return_value, "erorrMessage", "NetUserGetLocalGroups failed.");
			break;
		}
		ptg = (PTOKEN_GROUPS)malloc(dwLength);
		get_groups_res = GetTokenInformation(token,         // handle to the access token
			TokenGroups,    // get information about the token's groups
			(LPVOID) ptg,   // pointer to TOKEN_GROUPS buffer
			dwLength,       // size of buffer
			&dwLength       // receives required buffer size
			);
		if(!get_groups_res)
		{
			error = GetLastError();
			write_log("GetTokenInformation failed. error = %d\n", error);
			add_assoc_long(return_value, "erorrCode", error);
			add_assoc_string(return_value, "erorrMessage", "GetTokenInformation failed.");
			break;
		}
		///wchar_t *local_system_name(0);
		std::wstring local_system_name;
		DWORD local_system_name_sz(0);
		if(!GetComputerNameW(&local_system_name[0],&local_system_name_sz) && (error = GetLastError())!=ERROR_BUFFER_OVERFLOW)
		{
			write_log("GetComputerName failed. error = %d\n", error);
			add_assoc_long(return_value, "erorrCode", error);
			add_assoc_string(return_value, "erorrMessage", "GetComputerName failed.");
			break;
		}
		local_system_name.resize(local_system_name_sz+1,0);
		if(!GetComputerNameW(&local_system_name[0],&local_system_name_sz))
		{
			error = GetLastError();
			write_log("GetComputerName failed. error = %d\n", error);
			add_assoc_long(return_value, "erorrCode", error);
			add_assoc_string(return_value, "erorrMessage", "GetComputerName failed.");
			break;
		}
		for(unsigned i = 0;i<ptg->GroupCount;i++)
		{
			SID_NAME_USE SidType;
			std::wstring name,domain;
			DWORD name_len = 0,domain_len = 0;

			do{
				error = 0;
				if(LookupAccountSidW( &local_system_name[0], ptg->Groups[i].Sid,
					&name[0], &name_len, &domain[0],
					&domain_len, &SidType ))
				{
					if(!_wcsicmp(&domain[0],&local_system_name[0]))
					{
						write_log("\t%S\n", &name[0]);
						DWORD size_need = WideCharToMultiByte(CP_UTF8, 0, &name[0], -1, NULL,0, 0, 0);
						if(!size_need)
						{
							//error, skip
							write_log("WideCharToMultiByte faild, input name = %s, err = %d\n",name.c_str(),GetLastError());
							break;
						}
						std::string return_name(size_need,0);

						if(!WideCharToMultiByte(CP_UTF8, 0, &name[0], -1, &return_name[0],return_name.size(), 0, 0))
						{
							//error, skip
							write_log("WideCharToMultiByte faild, name = %s, err = %d\n",name.c_str(),GetLastError());
							break;
						}
						add_next_index_string(&groups, return_name.c_str());
					}
				}
				else
				{
					error = GetLastError();
					if(error==ERROR_INSUFFICIENT_BUFFER)
					{
						if(name.size() < name_len)
							name.resize(name_len);
						if(domain.size() < domain_len)
							domain.resize(domain_len);
					}
					else
						write_log("Error = %d\n",error);
				}
			}while(error==ERROR_INSUFFICIENT_BUFFER);
		}
	}while(false);
	add_assoc_zval(return_value, "groups", &groups);
	CloseHandle(token);
	free(ptg);
}

struct PerformLoginArgs
{
	wchar_t *dns_name;
	wchar_t *nb_name;
	wchar_t *pass;
	zval *res;
};

unsigned int WINAPI LoginThread(
						 _In_  LPVOID lpParameter
						 )
{
	vs::SetThreadName("LoginThread");
	PerformLoginArgs *args = (PerformLoginArgs *)lpParameter;
	PerformLogin(args->res, args->dns_name, args->nb_name, args->pass);
	return 0;
}

void PerformLoginInNewThread(zval *ret, const wchar_t *dns_name, const wchar_t *nb_name, const wchar_t *pass)
{
	write_log("PerformLoginInNewThread call...\n");

	PerformLoginArgs a;
	a.dns_name = (wchar_t *)dns_name;
	a.nb_name = (wchar_t *)nb_name;
	a.pass = (wchar_t *)pass;
	a.res = ret;

	HANDLE h = (HANDLE)_beginthreadex(NULL, 0, LoginThread, (void *)&a, 0, NULL);
	WaitForSingleObject(h, INFINITE);

	write_log("PerformLoginInNewThread call... done\n");
}

PHP_FUNCTION(trueconf_login_windows_user)
{
	write_log("trueconf_login_windows_user call...\n");

	_char(login);
	_char(pass);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
		&login, &login_len, &pass, &pass_len) != SUCCESS) {
		dprint1("trueconf_login_windows_user: parse error\n");
			RETURN_FALSE;
	}

	/*{
		write_log("Check user access <begin>\n");
		tc_auth_utils::UserAccessChecker access_checker(login, login_len, passwd, pass_len, HKEY_LOCAL_MACHINE, REG_CHECK_PATH, strlen(REG_CHECK_PATH), GENERIC_READ);

		access_checker.Check();

		write_log("User access check status for user %ws at domain %ws: %d (message: %ws, code: %lu).\n",
			access_checker.GetLoginParser().GetUser(),
			access_checker.GetLoginParser().GetDomain(),
			access_checker.GetAccessStatus(),
			access_checker.GetAccessErrorMessage(),
			access_checker.GetAccessErrorCode());

		write_log("Check user access </end>\n");
	}*/

	wchar_t *buff = (wchar_t *)malloc( (pass_len + login_len + 1) * 2 );
	MultiByteToWideChar(CP_UTF8, 0, login, -1, buff, pass_len + login_len + 1);
	std::wstring _login = buff;
	MultiByteToWideChar(CP_UTF8, 0, pass, -1, buff, pass_len + login_len + 1);
	std::wstring _pass = buff;

	if (_login.find('\\') == std::string::npos  &&
		_login.find('@')  == std::string::npos )
	{
		wchar_t buffer2[1024];

		BOOL isInDomain = true;
		DWORD rv = NetRenameMachineInDomain(0, 0, 0, 0, 0);
		if(rv == NERR_SetupNotJoined || rv == ERROR_NOT_SUPPORTED) {
			isInDomain = false;
		}

		write_log(" isInDomain = %d\n", isInDomain );
		write_log(" NetRenameMachineInDomain(0, 0, 0, 0, 0) = %d\n", rv);

		DWORD len=sizeof(buffer2) / 2;
		long result = GetComputerNameExW(ComputerNameNetBIOS, buffer2, &len);
		if (result)
		{
			WCHAR domain_name[256];
			WKSTA_INFO_100 *info = NULL;
			if (isInDomain && NERR_Success == NetWkstaGetInfo(buffer2, 100, (LPBYTE *)&info) &&
				SUCCEEDED(StringCchCopyW(domain_name, ARRAYSIZE(domain_name), info->wki100_langroup))) {
					_login = _login + L"@" + domain_name;
			} else
			{
				_login = std::wstring(buffer2) + L"\\" +  _login ;
			}

			if (info)
				NetApiBufferFree(info);
		}
	}

	if ( _login.find('\\') != std::string::npos)
	{
		int pos = _login.find('\\');
		_login[pos] = 0;

		const wchar_t *domain = &_login[0];
		const wchar_t *user = &_login[pos + 1];

		std::wstring dns_name = user;

		DOMAIN_CONTROLLER_INFOW *i;
		if (ERROR_SUCCESS == DsGetDcNameW(NULL, domain, NULL, NULL, DS_IS_FLAT_NAME | DS_RETURN_DNS_NAME, &i))
		{
			((dns_name += L"@") += i->DomainName);
		};

		_login[pos] = '\\';

		NetApiBufferFree(i);

		//zval *p = PerformLoginInNewThread(dns_name.c_str(), _login.c_str(), _pass.c_str());
		PerformLogin(return_value, dns_name.c_str(), _login.c_str(), _pass.c_str());

		return;

	} else
		if ( _login.find(L'@') != std::string::npos )
		{
			int pos = _login.find(L'@');
			_login[pos] = 0;

			const wchar_t *user = &_login[0];
			const wchar_t *domain = &_login[pos + 1];

			std::wstring nb_name = user;

			DOMAIN_CONTROLLER_INFOW *i;
			if (ERROR_SUCCESS == DsGetDcNameW(NULL, domain, NULL, NULL, DS_IS_DNS_NAME | DS_RETURN_FLAT_NAME, &i))
			{
				nb_name = std::wstring(i->DomainName) + L"\\" + user;
			};

			_login[pos] = '@';

			NetApiBufferFree(i);
			zval p;
			PerformLoginInNewThread(&p, _login.c_str(), nb_name.c_str(), _pass.c_str());
			RETURN_ZVAL(&p, 1, 1 );
		} else
		{
			zval p;
			PerformLoginInNewThread(&p, _login.c_str(), _login.c_str(), _pass.c_str());
			RETURN_ZVAL(&p, 1, 1 );

		}
}

#endif // Windows only function: trueconf_login_windows_user

#if defined(_WIN32) // Windows only function: trueconf_authorize_user

struct trueconf_authorize_user_params
{
	// input
	int num_args = 0;
	std::string login;
	std::string pass;
	std::string reg_key_path;
	std::string group_name;

	// output
	tc_auth_utils::RegistryAccessCodes result = tc_auth_utils::RACCESS_NONE;
	std::string description;
};

void trueconf_authorize_user_imp(const trueconf_authorize_user_params& p, tc_auth_utils::RegistryAccessCodes& out_res, std::string& out_desc)
{
	// We will try to create registry authorization object, if it has not been created already.
	if (p.num_args >= 3 && !p.reg_key_path.empty())
	{
		DWORD error_code = tc_auth_utils::CreateAuthRegistryObject(
			HKEY_LOCAL_MACHINE,
			p.reg_key_path.c_str(), p.reg_key_path.length(),
			p.group_name.c_str(), p.group_name.length());

		write_log("Creating authorization object... ");

		if (error_code != ERROR_SUCCESS)
		{
			tc_auth_utils::SystemErrorTextualizer et;

			// convert system error code to text message
			et.SetErrorCode(error_code);

			out_res = tc_auth_utils::RACCESS_AUTH_OBJECT_ERROR;
			out_desc = et.GetStringUTF8();

			write_log("FAILED: %ws (error code: %ul).\n", et.GetString(), error_code);

			return;
		}
		write_log("OK.\n");
	}

	// registry path is not specified - authorize user using only login string
	if (p.num_args < 3 && p.reg_key_path.length() == 0)
	{
		int login_type;
		DWORD error_code;
		bool auth_status;

		write_log("Authorizing user using login string only... ");

		// try to authorize user
		auth_status = tc_auth_utils::AuthorizeWithLoginString(
			p.login.c_str(), p.login.length(),
			p.pass.c_str(), p.pass.length(),
			login_type, error_code);

		// User has been authorized
		if (auth_status == true)
		{
			write_log("OK.\n");
			out_res = tc_auth_utils::RACCESS_OK;
		}
		else // error while authorizing user
		{
			if (login_type == tc_auth_utils::LoginParser::LOGIN_ILLEGAL)
			{
				write_log("FAILED: login parse error.\n");
				out_res = tc_auth_utils::RACCESS_PARSE_ERROR;
			}
			else
			{
				tc_auth_utils::SystemErrorTextualizer et; // Numeric Error Code to Textual Message Converter.

				// get error description
				et.SetErrorCode(error_code);

				write_log("FAILED: %ws (error code: %ul).\n", et.GetString(), error_code);

				out_res = (error_code == ERROR_LOGON_FAILURE) ? tc_auth_utils::RACCESS_PARSE_ERROR : tc_auth_utils::RACCESS_SYSTEM_ERROR;
				out_desc = et.GetStringUTF8();
			}
		}

		return;
	}
	else // check user access to Registry Authorization Object
	{
		tc_auth_utils::UserAccessChecker access_checker(
			p.login.c_str(), p.login.length(),
			p.pass.c_str(), p.pass.length(),
			HKEY_LOCAL_MACHINE, p.reg_key_path.c_str(), p.reg_key_path.length(),
			GENERIC_READ);

		access_checker.Check();

		write_log("User access check status for user %ws at domain \"%ws\": %d (message: \"%ws\", code: %lu).\n",
			access_checker.GetLoginParser().GetUser(),
			access_checker.GetLoginParser().GetDomain(),
			access_checker.GetAccessStatus(),
			access_checker.GetAccessErrorMessage(),
			access_checker.GetAccessErrorCode());

		auto status = access_checker.GetAccessStatus();

		out_res = static_cast<tc_auth_utils::RegistryAccessCodes>(status);

		if (status != tc_auth_utils::RACCESS_AUTH_ERROR ||
			status != tc_auth_utils::RACCESS_DENIED)
		{
			out_desc = access_checker.GetAccessErrorMessageUTF8();
		}
	}

}

PHP_FUNCTION(trueconf_authorize_user)
{
	_char(login);
	_char(passw);
	_char(reg_key_path);
	_char(group_name);

	zval res;
	array_init(&res);

	// parse PHP parameters
	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|ss",
		&login, &login_len,
		&passw, &passw_len,
		&reg_key_path, &reg_key_path_len,
		&group_name, &group_name_len) != SUCCESS)
	{
		add_assoc_long(&res, "result", tc_auth_utils::RACCESS_PARSE_ERROR);
		add_assoc_string(&res, "description", "Error while parsing function parameters");
		dprint1("trueconf_authorize_user: parse error\n");
		RETURN_ZVAL(&res, 1, 1);
	}

	// check if we have passed enough parameters
	if (ZEND_NUM_ARGS() < 2)
	{
		add_assoc_long(&res, "result", tc_auth_utils::RACCESS_PARSE_ERROR);
		add_assoc_string(&res, "description", "Insufficient number of parameters");
		RETURN_ZVAL(&res, 1, 1);
	}

	write_log("Check user access <begin>\n");

	auto r = std::make_shared<trueconf_authorize_user_params>();
	r->num_args = ZEND_NUM_ARGS();
	if (login && *login)				r->login = login;
	if (passw && *passw)				r->pass = passw;
	if (reg_key_path && *reg_key_path)	r->reg_key_path = reg_key_path;
	if (group_name && *group_name)		r->group_name = group_name;

	auto m = std::make_shared<std::mutex>();
	auto cv = std::make_shared<vs::condition_variable>();

	std::unique_lock<std::mutex> lock(*m);
	VS_Singleton<VS_ThreadPool>::Instance().Post([m, cv, r]() {
		tc_auth_utils::RegistryAccessCodes out_res;
		std::string out_desc;
		trueconf_authorize_user_imp(*r, out_res, out_desc);

		std::unique_lock<std::mutex> lock_in_thread(*m);
		r->result = out_res;
		r->description = out_desc;
		cv->notify_one();
	});

	auto wait_res = cv->wait_for(lock, std::chrono::seconds(10));

	if (wait_res == std::cv_status::no_timeout)
	{
		add_assoc_long(&res, "result", r->result);
		add_assoc_string(&res, "description", const_cast<char*>(r->description.c_str()));
	}
	else
	{
		add_assoc_long(&res, "result", tc_auth_utils::RACCESS_AUTH_TIMEOUT);
		add_assoc_string(&res, "description", "login timeout");
	}

	write_log("Check user access </end>\n");

	RETURN_ZVAL(&res, 1, 1);
}

#endif // Windows only function: trueconf_authorize_user

bool php_str_iccmpm(int method, char * subject, int subjectLength, char * needle, int needleLength){
	if (!subject || !needle) return false;
	vs::UnicodeConverter<char, wchar_t> converter;
	auto wsubject = converter.Convert(string_view(subject, subjectLength));
	auto wneedle = converter.Convert(string_view(needle, needleLength));

	if (method == 0)		return boost::iequals(wneedle, wsubject);
	else if (method == 1)	return boost::iends_with(wsubject, wneedle);
	else if (method == 2)	return boost::icontains(wsubject, wneedle);

	return false;
}

bool mbstr_strcasecmp(char * strA, int strA_len, char * strB, int strB_len, std::locale l = std::locale()){
	if (!strA || !strB || strA_len == 0 || strB_len == 0) return false;

	auto converter = vs::UnicodeConverter<char, wchar_t>();
	auto wstrA = converter.Convert(string_view(strA, strA_len));
	auto wstrB = converter.Convert(string_view(strB, strB_len));
	return boost::iequals(wstrA, wstrB, l);
}

static int php_array_data_compare_case(const void *a, const void *b TSRMLS_DC) /* {{{ */
{
	Bucket *f;
	Bucket *s;
	zval first;
	zval second;

	f = (Bucket *)a;
	s = (Bucket *)b;

	first = f->val;
	second = s->val;

	if (Z_TYPE(first) != IS_STRING && Z_TYPE(second) != IS_STRING){
		return 0;
	}
	else if (Z_TYPE(first) != IS_STRING){
		return -1;
	}
	else if (Z_TYPE(second) != IS_STRING){
		return 1;
	}


	int result = -1;
	if (mbstr_strcasecmp(Z_STRVAL(first), Z_STRLEN(first), Z_STRVAL(second), Z_STRLEN(second))) result = 0;
	return result;
}


PHP_FUNCTION(trueconf_case_sort) {

	zval *arr(0);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &arr) != SUCCESS) {
		dprint1("trueconf_case_sort: parse error\n");
		return;
	}

	if (zend_hash_sort(Z_ARRVAL_P(arr), php_array_data_compare_case, 0 TSRMLS_CC) == FAILURE)
	{
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_FUNCTION(trueconf_strcasecmp)
{
	_char(strA);
	_char(strB);
	_char(localeName);

	int result(-1);
	std::locale locale;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|s", &strA, &strA_len, &strB, &strB_len, &localeName, &localeName_len) != SUCCESS) {
		dprint1("trueconf_strcasecmp: parse error\n");
		return;
	}

	if (localeName){
		try {
			locale = std::locale(localeName);
		}
		catch (std::runtime_error& er) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s",er.what());
			return;
		}
	}

	if(mbstr_strcasecmp(strA, strA_len, strB, strB_len, locale)) result = 0;

	RETURN_LONG(result > 0 ? 1 : result < 0 ? - 1 : 0);
}

PHP_FUNCTION(trueconf_array_change_key_case)
{
	zval *inputArray, *inputArrayEntry;
	zend_string *strKey(0), *localeName(0);
	zend_ulong keyIndex = 0;
	zend_long changeToUpper = 0;
	HashPosition hashPosition;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|lS", &inputArray, &changeToUpper, &localeName) == FAILURE) {
		dprint1("trueconf_array_change_key_case: parse error\n");
		return;
	}

	array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(inputArray)));

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(inputArray), &hashPosition);

	while ((inputArrayEntry = zend_hash_get_current_data_ex(Z_ARRVAL_P(inputArray), &hashPosition)) != NULL) {
		zval_add_ref(inputArrayEntry);

		switch (zend_hash_get_current_key_ex(Z_ARRVAL_P(inputArray), &strKey, &keyIndex, &hashPosition)) {
		case HASH_KEY_IS_LONG:
			zend_hash_index_update(Z_ARRVAL_P(return_value), keyIndex, inputArrayEntry);
			break;
		case HASH_KEY_IS_STRING:
			std::string res;
			if (changeToUpper != 0)
				res = vs::UTF8ToUpper(string_view(strKey->val, strKey->len));
			else
				res = vs::UTF8ToLower(string_view(strKey->val, strKey->len));

			if (res.size() == 0){
				zend_hash_update(Z_ARRVAL_P(return_value), strKey, inputArrayEntry);
				break;
			}

			zend_hash_str_update(Z_ARRVAL_P(return_value), res.c_str(), res.size(), inputArrayEntry);
			break;
		}

		zend_hash_move_forward_ex(Z_ARRVAL_P(inputArray), &hashPosition);
	}
}


PHP_FUNCTION(trueconf_array_change_value_case)
{
	zval *inputArray, *inputArrayEntry;
	zend_long changeToUpper = 0;
	zend_string *strKey(0), *localeName(0);
	unsigned int keyType = 0;
	zend_ulong keyIndex = 0;
	HashPosition hashPosition;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|lS", &inputArray, &changeToUpper, &localeName) == FAILURE) {
		dprint1("trueconf_array_change_value_case: parse error\n");
		return;
	}

	array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(inputArray)));

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(inputArray), &hashPosition);

	while ((inputArrayEntry = zend_hash_get_current_data_ex(Z_ARRVAL_P(inputArray), &hashPosition)) != NULL) {
		zval_add_ref(inputArrayEntry);

		keyType = zend_hash_get_current_key_ex(Z_ARRVAL_P(inputArray), &strKey, &keyIndex, &hashPosition);

		if (Z_TYPE_P(inputArrayEntry) != IS_STRING) {
			switch (keyType){
			case HASH_KEY_IS_LONG:
				add_index_zval(return_value, keyIndex, inputArrayEntry);
				break;
			case HASH_KEY_IS_STRING:
				add_assoc_zval(return_value, strKey->val, inputArrayEntry);
				break;
			}
			zend_hash_move_forward_ex(Z_ARRVAL_P(inputArray), &hashPosition);
			continue;
		}

		std::string res;
		if (changeToUpper != 0)
			res = vs::UTF8ToUpper(string_view(Z_STRVAL_P(inputArrayEntry), Z_STRLEN_P(inputArrayEntry)));
		else
			res = vs::UTF8ToLower(string_view(Z_STRVAL_P(inputArrayEntry), Z_STRLEN_P(inputArrayEntry)));

		if (res.size() != 0){
			switch (keyType){
			case HASH_KEY_IS_LONG:
				add_index_stringl(return_value, keyIndex, res.c_str(), res.length());
				break;
			case HASH_KEY_IS_STRING:
				add_assoc_stringl(return_value, strKey->val, (char*)res.c_str(), res.length());
				break;
			}
		}

		zend_hash_move_forward_ex(Z_ARRVAL_P(inputArray), &hashPosition);
	}
}


PHP_FUNCTION(trueconf_array_has_string_ci)
{
	zval *array, *entry;
	HashPosition pos;
	_char(str);
	_char(localeName);
	zend_bool searchInNumeric = 0;
	std::locale locale;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "as|bs", &array, &str, &str_len, &searchInNumeric, &localeName, &localeName_len) == FAILURE) {
		dprint1("trueconf_array_has_string_ci: parse error\n");
		return;
	}

	if (localeName){
		try {
			locale = std::locale(localeName);
		}
		catch (std::runtime_error &e) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", e.what());
			return;
		}
	}

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(array), &pos);
	while ((entry = zend_hash_get_current_data_ex(Z_ARRVAL_P(array), &pos)) != NULL) {
		if ((Z_TYPE_P(entry) == IS_DOUBLE || Z_TYPE_P(entry) == IS_LONG) && searchInNumeric){
			convert_to_string(entry);
		}
		else if (Z_TYPE_P(entry) != IS_STRING){
			zend_hash_move_forward_ex(Z_ARRVAL_P(array), &pos);
			continue;
		}

		if (mbstr_strcasecmp(str, str_len, Z_STRVAL_P(entry), Z_STRLEN_P(entry), locale)){
				RETURN_TRUE;
		}

		zend_hash_move_forward_ex(Z_ARRVAL_P(array), &pos);
	}

	RETURN_FALSE;
}




PHP_FUNCTION(trueconf_num_contains)
{
	zend_long subject = 0;
	zend_long needle = 0;
	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &subject, &needle) == FAILURE) {
		dprint1("trueconf_num_contains: parse error\n");
		return;
	}

	needle = abs(needle);
	subject = abs(subject);

	if (subject == needle){
		RETURN_TRUE;
	}

	if (subject < needle){
		RETURN_FALSE;
	}

	long subjectBuffer = 0;
	long needleBuffer = 0;

	while (subject > 0){
		subjectBuffer = subject;
		needleBuffer = needle;

		while (true){
			if (subjectBuffer % 10 == needleBuffer % 10){
				subjectBuffer = subjectBuffer / 10;
				needleBuffer = needleBuffer / 10;
			}
			else {
				subject = subject / 10;
				break;
			}

			if (needleBuffer == 0){
				RETURN_TRUE;
			}
		}

		if (subject < needle){
			RETURN_FALSE;
		}
	}

	RETURN_FALSE;
}


PHP_FUNCTION(trueconf_num_ends_with)
{

	zend_long subject = 0;
	zend_long needle = 0;
	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &subject, &needle) == FAILURE) {
		dprint1("trueconf_num_ends_with: parse error\n");
		return;
	}

	needle = abs(needle);
	subject = abs(subject);

	if (subject == needle){
		RETURN_TRUE;
	}

	if (subject < needle){
		RETURN_FALSE;
	}


	while (true){
		if (subject % 10 != needle % 10){
			RETURN_FALSE;
		}

		needle = needle / 10;
		subject = subject / 10;

		if (needle == 0){
			RETURN_TRUE;
		}
	}

	RETURN_FALSE;
}



PHP_FUNCTION(trueconf_num_starts_with)
{

	zend_long subject = 0;
	zend_long needle = 0;
	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &subject, &needle) == FAILURE) {
		dprint1("trueconf_num_starts_with: parse error\n");
		return;
	}

	needle = abs(needle);
	subject = abs(subject);

	if (subject == needle){
		RETURN_TRUE;
	}

	if (subject < needle){
		RETURN_FALSE;
	}


	while (subject > 0){
		if (subject < needle){
			RETURN_FALSE;
		}
		else if (subject == needle){
			RETURN_TRUE;
		}

		subject = subject / 10;
	}

	RETURN_FALSE;
}


bool php_str_iccmpm_cc(int method, char * subject, int subjectLength, char * needle, int needleLength){
	vs::UnicodeConverter<char, wchar_t> conv;

	auto wSubj = conv.Convert(string_view(subject, subjectLength));
	auto wNeedle = conv.Convert(string_view(needle, needleLength));
	if (wSubj.empty() || wNeedle.empty()) return false;

	switch (method) {
	default:
		return false;
	case TC_PHP_STR_SEARCH_METHOD_SUBSTRING:
		return boost::icontains(wSubj, wNeedle);
	case TC_PHP_STR_SEARCH_METHOD_STARTS_WITH:
		return boost::istarts_with(wSubj, wNeedle);
	case TC_PHP_STR_SEARCH_METHOD_ENDS_WITH:
		return boost::iends_with(wSubj, wNeedle);
	case TC_PHP_STR_SEARCH_METHOD_EQUAL:
		return boost::iequals(wSubj, wNeedle);
	}

	return false;
}

bool php_str_cscmpm(int method, char * subject, int subjectLength, char * needle, int needleLength){
	switch (method){
	default:
		return false;
	case TC_PHP_STR_SEARCH_METHOD_SUBSTRING:
		return boost::contains(string_view(subject, subjectLength), string_view(needle, needleLength));
		break;
	case TC_PHP_STR_SEARCH_METHOD_STARTS_WITH:
		return boost::starts_with(string_view(subject, subjectLength), string_view(needle, needleLength));
		break;
	case TC_PHP_STR_SEARCH_METHOD_ENDS_WITH:
		return boost::ends_with(string_view(subject, subjectLength), string_view(needle, needleLength));
		break;
	case TC_PHP_STR_SEARCH_METHOD_EQUAL:
		return strcmp(subject, needle) == 0;
		break;
	}
}

bool php_str_cmpm(int method, char * subject, int subjectLength, char * needle, int needleLength, bool ignoreCase){
	if (subjectLength == 0 && needleLength == 0){
		return true;
	}
	else if (needleLength == 0 || subjectLength == 0){
		return false;
	}

	if (ignoreCase){
		return php_str_iccmpm_cc(method, subject, subjectLength, needle, needleLength);
	}
	else {
		return php_str_cscmpm(method, subject, subjectLength, needle, needleLength);
	}
}


PHP_FUNCTION(trueconf_str_cmp){
	_char(needle);
	_char(subject);
	zend_long searchMethod = 0;
	zend_bool ignoreCase(0);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssl|b", &subject, &subject_len, &needle, &needle_len, &searchMethod, &ignoreCase) == FAILURE) {
		dprint1("trueconf_str_cmp: parse error\n");
		return;
	}

	if (searchMethod != TC_PHP_STR_SEARCH_METHOD_SUBSTRING
		&& searchMethod != TC_PHP_STR_SEARCH_METHOD_STARTS_WITH
		&& searchMethod != TC_PHP_STR_SEARCH_METHOD_ENDS_WITH
		&& searchMethod != TC_PHP_STR_SEARCH_METHOD_EQUAL){
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unsupported search method");
		RETURN_FALSE;
	}

	RETURN_BOOL(php_str_cmpm(searchMethod, subject, subject_len, needle, needle_len, ignoreCase));
}

PHP_FUNCTION(trueconf_str_starts_with)
{
	_char(subject);
	_char(needle);
	zend_bool ignoreCase(0);
	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|b", &subject, &subject_len, &needle, &needle_len, &ignoreCase) == FAILURE) {
		dprint1("trueconf_str_starts_with: parse error\n");
		return;
	}

	RETURN_BOOL(php_str_cmpm(TC_PHP_STR_SEARCH_METHOD_STARTS_WITH, subject, subject_len, needle, needle_len, ignoreCase));
}

PHP_FUNCTION(trueconf_str_ends_with)
{
	_char(subject);
	_char(needle);

	zend_bool ignoreCase(0);
	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|b", &subject, &subject_len, &needle, &needle_len, &ignoreCase) == FAILURE) {
		dprint1("trueconf_str_ends_with: parse error\n");
		return;
	}

	RETURN_BOOL(php_str_cmpm(TC_PHP_STR_SEARCH_METHOD_ENDS_WITH, subject, subject_len, needle, needle_len, ignoreCase));
}


PHP_FUNCTION(trueconf_str_contains)
{
	_char(subject);
	_char(needle);
	zend_bool ignoreCase(0);
	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|b", &subject, &subject_len, &needle, &needle_len, &ignoreCase) == FAILURE) {
		dprint1("trueconf_str_contains: parse error\n");
		return;
	}

	RETURN_BOOL(php_str_cmpm(TC_PHP_STR_SEARCH_METHOD_SUBSTRING, subject, subject_len, needle, needle_len, ignoreCase));
}


PHP_FUNCTION(trueconf_array_search)
{
	zval *zarray(0);
	_char(needle);
	zend_long searchMethod = 0;
	zend_bool ignoreCase(0);
	zend_bool flipArgs(0);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa|lbb", &needle, &needle_len, &zarray, &searchMethod, &ignoreCase, &flipArgs) == FAILURE) {
		dprint1("trueconf_array_search: parse error\n");
		return;
	}

	if (searchMethod != TC_PHP_STR_SEARCH_METHOD_SUBSTRING
		&& searchMethod != TC_PHP_STR_SEARCH_METHOD_STARTS_WITH
		&& searchMethod != TC_PHP_STR_SEARCH_METHOD_ENDS_WITH
		&& searchMethod != TC_PHP_STR_SEARCH_METHOD_EQUAL){
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unsupported search method");
		RETURN_FALSE;
	}

	zval *zentry;
	HashPosition hashPosition(0);

	zend_string *strKey(0);
	unsigned int keyType = 0;
	zend_ulong keyIndex = 0;
	bool cmpResult = false;

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(zarray), &hashPosition);
	while ((zentry = zend_hash_get_current_data_ex(Z_ARRVAL_P(zarray), &hashPosition)) != NULL) {
		if ((Z_TYPE_P(zentry) == IS_DOUBLE || Z_TYPE_P(zentry) == IS_LONG)){
			convert_to_string(zentry);
		}
		else if (Z_TYPE_P(zentry) != IS_STRING){
			zend_hash_move_forward_ex(Z_ARRVAL_P(zarray), &hashPosition);
			continue;
		}

		if (ignoreCase){
			if (!flipArgs){
				cmpResult = php_str_iccmpm_cc(searchMethod, Z_STRVAL_P(zentry), Z_STRLEN_P(zentry), needle, needle_len);
			}
			else {
				cmpResult = php_str_iccmpm_cc(searchMethod, needle, needle_len, Z_STRVAL_P(zentry), Z_STRLEN_P(zentry));
			}
		} else {
			if (!flipArgs){
				cmpResult = php_str_cmpm(searchMethod, Z_STRVAL_P(zentry), Z_STRLEN_P(zentry), needle, needle_len, ignoreCase);
			}
			else {
				cmpResult = php_str_cmpm(searchMethod, needle, needle_len, Z_STRVAL_P(zentry), Z_STRLEN_P(zentry), ignoreCase);
			}
		}

		if (cmpResult){
			break;
		}

		zend_hash_move_forward_ex(Z_ARRVAL_P(zarray), &hashPosition);
	}

	if (!cmpResult){
		RETURN_FALSE;
	}

	keyType = zend_hash_get_current_key_ex(Z_ARRVAL_P(zarray), &strKey, &keyIndex, &hashPosition);
	switch (keyType){
	case HASH_KEY_IS_LONG:
		RETURN_LONG(keyIndex);
		break;
	case HASH_KEY_IS_STRING:
		RETURN_STRINGL(strKey->val, strKey->len);
		break;
	default: RETURN_FALSE;
	}
}

#ifdef _WIN32
#define DEFAULT_WIN_TIME_ZONE_NAME "UTC"
PHP_FUNCTION(trueconf_get_windows_timezone_name)
{
	DYNAMIC_TIME_ZONE_INFORMATION dtzi{};
	auto result = GetDynamicTimeZoneInformation(&dtzi);
	assert(result != TIME_ZONE_ID_INVALID);
	if (result == TIME_ZONE_ID_INVALID)
	{
		RETURN_STRING(DEFAULT_WIN_TIME_ZONE_NAME);
	}
	auto wlen = wcslen(dtzi.TimeZoneKeyName);
	char buf[128] = {0};
	assert(sizeof(buf) >= wlen + 1);
	wcstombs(buf, dtzi.TimeZoneKeyName, wlen);
	if (strcmp(buf, "Coordinated Universal Time") == 0)
	{
		RETURN_STRING("UTC");
	}
	RETURN_STRING(buf);
}
#endif

#ifdef __linux__

#define DEFAULT_LINUX_TIME_ZONE_NAME "UTC"
PHP_FUNCTION(trueconf_get_linux_timezone_name)
{
	char filename[256] = {0};
	struct stat fstat;
	if (lstat("/etc/localtime", &fstat) == 0)
	{
		if (S_ISLNK(fstat.st_mode))
		{
			int nSize = readlink("/etc/localtime", filename, 256);
			if (nSize > 0)
			{
				filename[nSize] = 0;
				int tz_pos = 0;
				bool first = true;
				for (int i = nSize - 1; i >= 0; --i)
				{
					if (filename[i] == '/')
					{
						if (first)
						{
							first = false;
						}
						else
						{
							tz_pos = i;
							break;
						}
					}
				}
				if (tz_pos == 0)
				{
					RETURN_STRING(filename);
				}
				RETURN_STRING(filename + tz_pos + 1);
			}
		}
	}
	RETURN_STRING(DEFAULT_LINUX_TIME_ZONE_NAME);
}
#endif //__linux__

PHP_FUNCTION(trueconf_get_file_size)
{
	_char(fileName);
	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &fileName, &fileName_len) == FAILURE) {
		dprint1("trueconf_get_file_size: parse error\n");
		RETURN_STRING("0");
	}

	if (!fileName || fileName_len <= 1)
		RETURN_STRING("0");

#if defined(_WIN32)
	auto wfileName = vs::UTF8ToWideCharConvert(fileName);
	if (wfileName.empty())
		RETURN_STRING("0");

	int fd = _wopen(wfileName.c_str(), _O_RDONLY | _O_BINARY);
	if (fd == -1)
		RETURN_STRING("0");
	VS_SCOPE_EXIT { _close(fd); };

	auto fileSize = _filelengthi64(fd);
	if (fileSize == -1)
		RETURN_STRING("0");
#else
	struct stat st;
	int err = ::stat(fileName, &st);
	if (err != 0)
		RETURN_STRING("0");

	if (!S_ISREG(st.st_mode))
		RETURN_STRING("0");

	const auto fileSize = st.st_size;
#endif

	auto res = std::to_string(fileSize);
	RETURN_STRINGL(res.c_str(), res.length());
}
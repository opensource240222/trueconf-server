#ifdef _WIN32
/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 01.08.03     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_Server.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////
#include "net/DNSUtils/VS_DNSTools.h"
#ifdef _WIN32	// not ported

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <mutex>

#define  _WIN32_DCOM
#include <objbase.h>

#include <io.h>
#include <time.h>
#include <conio.h>
#include <errno.h>
#include <limits.h>
#include <direct.h>
#include <process.h>
#include <Windows.h>
#include <shellapi.h>

#include "VS_Server.h"
#include "VS_ServerLib.h"
#include "../Restarter/Restarter.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../../std/cpplib/VS_PerformanceMonitor.h"
#include "../../std/cpplib/VS_Utils.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/utf8_old.h"
#include "../../std/cpplib/VS_Protocol.h"
#include "../../std/debuglog/VS_Debug.h"
#include "std/Globals.h"
#include "../../acs/VS_AcsDefinitions.h"
#include "../../acs/Lib/VS_AcsLibDefinitions.h"
#include "../../SmtpMail/SmtpMailer/VS_SmtpMailer.h"
#include "SecureLib/VS_CryptoInit.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_SecureConstants.h"
#include "SecureLib/VS_UtilsLib.h"
#include "SecureLib/VS_SSLConfigKeys.h"
#include "../../acs/Lib/VS_AcsLib.h"
#include "ProtectionLib/Protection.h"
#include "../../WinFirewallManager/VS_FirewallMgr.h"
#include "tools/Watchdog/VS_RoutersWatchdog.h"

#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <openssl/pem.h>

namespace po = boost::program_options;

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

char   *VS_Server::cl = 0;

static int srvCheckPoint = 0;
static bool srvQuickDestroy = false;
static SERVICE_STATUS_HANDLE srvHandle = NULL;

vs::RegistrationParams VS_Server::reg_params;
const char VS_Server::endpoint_for_config[] = "local";

VS_SimpleStr VS_Server::g_root_key = 0;

int   VS_Server::argc = 0;		char   **VS_Server::argv = 0;
char  VS_Server::sdir[256] = { 0 };
wchar_t VS_Server::wdir[256] = { 0 };

static const char* s_name = nullptr;
static const char* l_name = nullptr;
static const char* r_key = nullptr;
static const char* ws_name = nullptr;
static bool   service = false, restartable = false;
static char   smtp_server[256] = { 0 }, admin_email[256] = { 0 }, smtp_login[256] = {0}, smtp_password[256] = {0};
static unsigned short   smtp_port = 0;
static VS_SmtpAuthType	smtp_auth_type;
static bool   smtp_use_ssl;

static const char   stdout_file_name[] = "Stdout.log";
static const char   stdout_backup_file_name[] = "Stdout.old.log";
static const size_t LOGFILE_SIZE_LIM = 1024;

////////// The Main Function. ///////////////////////////////////////////////////////////

static inline int ServerMain( void );
static inline bool ServerStart( void );
static inline void ServerStop( void );
static inline void FillStartPath( void );

int VS_Server::main(int argc, char *argv[])
{
	::setlocale(LC_ALL, "");
	VS_Server::cl = GetCommandLine();	FillStartPath();
	VS_Server::argc = argc;		VS_Server::argv = argv;
	const int   ret = ServerMain();
	VS_PerformanceMonitor::Instance().Stop();
	return ret;
}
// end main

/////////////////////////////////////////////////////////////////////////////////////////

const char   reason1[] = "Network or Programming Components requested the restart.",
reason2[] = "Impossibility of initialization at start and inquiry of restart.",
report1[] = "The waiting time of a watchdog has expired.";

////////// Restart Function. ////////////////////////////////////////////////////////////

static void __cdecl Restart( const char *reason, const bool hard )
{
	if (reason && *reason)
		dprint1("Restart reason : %s\n",reason);

	if (*admin_email && *smtp_server && smtp_port)
	{
		char   lastLog[4096] = { 0 };	unsigned   index_log = 0;
		if (reason && *reason) {	strcpy( lastLog, reason );	strcat( lastLog, "\n" );
		index_log = (unsigned)strlen( lastLog );	}
		if (index_log)
		{
			unsigned long value = 1;
			VS_RegistryKey cfg_root(false, CONFIGURATION_KEY);
			if (!cfg_root.IsValid())
			{

			} else
			{
				if (cfg_root.GetValue(&value,4,VS_REG_INTEGER_VT,"Restart Notification"))
				{
				}
			}
			if (value!=0)
			{
				VS_SmtpMailer   mailer( smtp_server, smtp_port, smtp_auth_type, smtp_login, smtp_password, smtp_use_ssl );
				char sbj[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1 + 32] = "Restart of server";
				if (!g_tr_endpoint_name.empty()) {		strcat( sbj, " " );	strcat( sbj, g_tr_endpoint_name.c_str());		}
				mailer.Subject( sbj );	mailer.SetTextBody( lastLog );
				mailer.To( admin_email );	mailer.From( admin_email );
				mailer.Send();
			}
		}
	}
	{
		const char   sRstr[] = "tc_rstr.exe", blankStr[]  = " ", doubleQuotes[] = "\"";
		char   appName[MAX_PATH + 256] = { 0 }, cmdLine[sizeof(appName) + 2048] = { 0 }, tmpStr[32] = { 0 };
		strcpy( appName, VS_Server::argv[0] );
		char   *c1 = strrchr( appName, '\\' ), *c2 = strrchr( appName, '/' );
		if (c1 < c2)	c1 = c2;
		if (!c1)	strcpy( appName, sRstr );
		else		strcpy( ++c1, sRstr );
		strcpy( cmdLine, doubleQuotes );	strcat( cmdLine, appName );
		strcat( cmdLine, doubleQuotes );	strcat( cmdLine, blankStr );
		strcat( cmdLine, !hard ? "Soft" : "Hard" );		strcat( cmdLine, blankStr );
		strcat( cmdLine, _ui64toa( (unsigned __int64)GetCurrentProcessId(), tmpStr, 10 ));
		strcat( cmdLine, blankStr );
		if (!service) {		strcat( cmdLine, doubleQuotes );
		strcat( cmdLine, *VS_Server::argv );
		strcat( cmdLine, doubleQuotes );	}
		else	strcat( cmdLine, ws_name );
		strcat( cmdLine, blankStr );	strcat( cmdLine, VS_RESTARTER_CL_MARKER );
		strcat( cmdLine, VS_Server::cl );
		STARTUPINFO   si;	memset( (void *)&si, 0, sizeof(si) );	si.cb = sizeof(si);
		PROCESS_INFORMATION   pi;	memset( (void *)&pi, 0, sizeof(pi) );
		if (!CreateProcess( appName, cmdLine, 0, 0, FALSE, 0, 0, 0, &si, &pi ))
		{
			strcpy( cmdLine, s_name );
			strcat( cmdLine, ": Can't start restarter. Error: " );
			strcat( cmdLine, _ui64toa( (unsigned __int64)GetLastError(), tmpStr, 10 ));
			dprint0( "%s\n", cmdLine );
		}
	}
}
// end Restart

////////// Function of filling of a starting path. //////////////////////////////////////

static inline void FillStartPath( void )
{
	char   *cl = *VS_Server::cl == '\"' ? VS_Server::cl + 1 : VS_Server::cl;
	char   *ecl = (char *)~0;
	{
		char   *c[] = { reinterpret_cast<char*>(strchr( cl, '\t' )),
			reinterpret_cast<char*>(strchr( cl, '/' )),
			reinterpret_cast<char*>(strchr( cl, '\"' )) };
		for (unsigned i = 0; i < sizeof(c)/sizeof(*c); ++i)
			if (c[i] && c[i] < ecl)		ecl = c[i];									}
	if (ecl == (char *)~0)		return;
	char   sdir[sizeof(VS_Server::sdir)];	memset( (void *)sdir, 0, sizeof(sdir) );
	size_t   sz = ecl - cl;		if (!sz)	return;
	if (sz >= sizeof(sdir))		sz = sizeof(sdir) - 1;
	memcpy( (void *)sdir, (void *)cl, sz );		ecl = (char *)~0;
	{	char   *c[] = { strrchr( sdir, '\\' ), strrchr( sdir, '/' ) };
	for (unsigned i = 0; i < sizeof(c)/sizeof(*c); ++i)
		if (c[i] && c[i] < ecl)		ecl = c[i];						}
	if (ecl == (char *)~0)		return;
	++ecl;		*ecl = 0;		strcpy( VS_Server::sdir, sdir );
}
// end FillStartPath

////////// Start As Console Functions. //////////////////////////////////////////////////

VS_RoutersWatchdog * GetWatchDogImp()
{
	static std::unique_ptr<VS_RoutersWatchdog> instance;
	if (!instance)
		instance = std::make_unique<VS_RoutersWatchdog>();
	return instance.get();
}

static inline int StartAsConsole( void )
{
	auto pwd = GetWatchDogImp();
	assert(pwd != nullptr);
	if (!ServerStart())
	{	if (pwd->IsRestart()) {	Sleep(pwd->InSeconds() * 1000 );
	Restart( reason2, false );	}
	ServerStop();	return -1;		}
	const char   pressSrv[] = "Press 'Q' or 'E' for exit.\n%s>";
	bool   exit_flag = false;		printf( pressSrv, s_name );
	while (!exit_flag)
	{	if (!_kbhit())
	{	Sleep( 1000 );
	if (!pwd->Test())
	{
		exit_flag = true;
		dprint1( "%s\n", report1 );
		if (restartable && !pwd->Exit())	Restart( reason1, false );
	}	}
	else
	{	switch (toupper( _getch() ))
	{
			case 'Q' :	case 'E' :	exit_flag = true;	puts( "    Exit." );	break;
			default:	puts( "    Unknown key." );		printf( pressSrv, s_name );
	}	}	}
	ServerStop();	return 0;
}
// end StartAsConsole

////////// Start As Service Functions. //////////////////////////////////////////////////

struct ServiceThreadStruct
{	ServiceThreadStruct() : isValid(false), isStart(false), status(0), specificError(0)
{	startEvent = CreateEvent( 0, TRUE, FALSE, 0 );
mainEvent = CreateEvent( 0, TRUE, FALSE, 0 );
stopEvent = CreateEvent( 0, TRUE, FALSE, 0 );
if (!startEvent || !mainEvent || !stopEvent)
{
	printf("%s: CreateEvent( ... ) error: %lu\n", s_name, GetLastError());
fflush( stdout );	}
isValid = true;
} // end ServiceThreadStruct::ServiceThreadStruct
~ServiceThreadStruct()
{	if (startEvent)		CloseHandle( startEvent );
if (mainEvent)		CloseHandle( mainEvent );
if (stopEvent)		CloseHandle( stopEvent );
} // end ServiceThreadStruct::~ServiceThreadStruct
bool   isValid, isStart;
HANDLE   startEvent, mainEvent, stopEvent;
DWORD   status, specificError;
}; // end ServiceThreadStruct struct
static ServiceThreadStruct   *serviceArgs = 0;
static SERVICE_STATUS			serviceStatus;
static SERVICE_STATUS_HANDLE	serviceStatusHandle;

static void __cdecl ServiceMainThread( void *args )
{
	vs::SetThreadName("ServiceMain");
	const bool   start_res = ServerStart();
	bool isRestartByWatchDog = false;				// ktrushnikov (c) 18:54 31.03.2007
	SetEvent( serviceArgs->startEvent );	fflush( stdout );
	auto pwd = GetWatchDogImp();
	assert(pwd != nullptr);
	if (!start_res)
	{	if (pwd->IsRestart()) {	Sleep(pwd->InSeconds() * 1000 );
	Restart( reason2, false );	}
	goto go_return;		}

	while (1)
	{
		switch (WaitForSingleObject( serviceArgs->mainEvent, 1000 ))
		{
		case WAIT_OBJECT_0 :
			goto go_return;
		case WAIT_TIMEOUT :
			if (!pwd->Test())
			{
				dprint1( "%s\n", report1 );
				isRestartByWatchDog = true; goto go_restart;
			}
		{
			const auto stdout_path = vs::GetLogDirectory() + stdout_file_name;

			std::streamoff file_size = 0;

			{
				std::ifstream log_file(stdout_path, std::ifstream::ate | std::ifstream::binary);
				if (!log_file) break;
				file_size = log_file.tellg();
			}

			decltype(file_size) size_lim = LOGFILE_SIZE_LIM;

			{
				VS_RegistryKey cfg_root(false, CONFIGURATION_KEY);
				unsigned long val;
				if (cfg_root.IsValid() && cfg_root.GetValue(&val, 4, VS_REG_INTEGER_VT, "Max Stdout") && val > 0) {
					size_lim = val;
				}
			}
			size_lim *= 1024 * 1024;

			if (file_size > size_lim) {
				const auto stdout_backup_path = vs::GetLogDirectory() + stdout_backup_file_name;

				extern std::mutex stdout_mtx;
				std::unique_lock<std::mutex> lock(stdout_mtx);

				const bool res_fclose_stdout = fclose(stdout) == 0;
				const bool res_fclose_stderr = fclose(stderr) == 0;
				remove(stdout_backup_path.c_str());
				const bool res_rename = rename(stdout_path.c_str(), stdout_backup_path.c_str()) == 0;
				const bool res_redirect = VS_RedirectOutput(stdout_path.c_str());

				lock.unlock();

				if (!res_fclose_stdout || !res_fclose_stderr || !res_rename || !res_redirect)
				{
					dstream1 << std::boolalpha << "Log rotation failed"
						<< ": close_stdout=" << res_fclose_stdout
						<< ", close_stderr=" << res_fclose_stderr
						<< ", rename=" << res_rename
						<< ", redirect=" << res_redirect
					;
				}
			}
		}
		break;
		default :
			printf("%s: WaitForSingleObject( ... ) error: %lu\n", s_name, GetLastError());
go_restart:
			if (restartable && !pwd->Exit())
				Restart( reason1, false );
			goto go_return;
		}
	}
go_return:
	srvCheckPoint = 0;

	if(!start_res || isRestartByWatchDog) //alex fix{}
	{
		serviceStatus.dwWin32ExitCode		= 0;
		serviceStatus.dwCurrentState		= SERVICE_STOP_PENDING;
		serviceStatus.dwCheckPoint			= srvCheckPoint;
		serviceStatus.dwWaitHint				= srvQuickDestroy ? 15000 : 30000;
		serviceStatus.dwControlsAccepted =0;
		if (!SetServiceStatus( serviceStatusHandle, &serviceStatus ))
		{
			printf("%s: SetServiceStatus( ... ) error: %lu\n", s_name, GetLastError());
		fflush( stdout );
		}	}
	ServerStop();	fflush( stdout );

	if(!start_res || isRestartByWatchDog)
	{	serviceStatus.dwWin32ExitCode		= 0;
	serviceStatus.dwCurrentState		= SERVICE_STOPPED;
	serviceStatus.dwCheckPoint			= 0;
	serviceStatus.dwWaitHint				= 0;
	if (!SetServiceStatus( serviceStatusHandle, &serviceStatus ))
	{
		printf("%s: SetServiceStatus( ... ) error: %lu\n", s_name, GetLastError());
	fflush( stdout );
	}	}

	SetEvent( serviceArgs->stopEvent );
}
// end ServiceMainThread

static void StartServiceMain( void )
{
	const uintptr_t   ntptr = _beginthread( ServiceMainThread, 0, 0 );
	if (!ntptr || ntptr == (uintptr_t)-1L)
	{
		printf("%s: _beginthread( ... ) error: %lu\n", s_name, GetLastError());
	fflush( stdout );	serviceArgs->status = ~0 - 2;
	serviceArgs->specificError = GetLastError();	return;		}
	const DWORD   st = WaitForSingleObject( serviceArgs->startEvent, 120000 );
	if (st != WAIT_OBJECT_0)
	{
		printf("%s: WaitForSingleObject( ... ) error: %lu\n", s_name, GetLastError());
		fflush( stdout );
		serviceArgs->status = ~0 - (st == WAIT_TIMEOUT ? 3 : 4);
		serviceArgs->specificError = GetLastError();
		auto pwd = GetWatchDogImp();
		assert(pwd != nullptr);
		if (restartable && !pwd->Exit())
			Restart( reason1, true );
		else if (!restartable)
			pwd->Shutdown();
		return;
	}
	serviceArgs->isStart = true;
}
// end StartServiceMain

static void StopServiceMain( bool quick=false )
{
	srvQuickDestroy = quick;

	if (serviceArgs->isStart)
	{	SetEvent( serviceArgs->mainEvent );
	if (WaitForSingleObject( serviceArgs->stopEvent, quick?15000:75000 ) == WAIT_OBJECT_0)
		delete serviceArgs;
	}	}
// end StopServiceMain

void __stdcall ServiceHandler( DWORD opcode )
{
	switch (opcode)
	{
	case SERVICE_CONTROL_STOP :
		dprint1("SERVICE_CONTROL_STOP by SCM\n");
		StopServiceMain();
		serviceStatus.dwWin32ExitCode		= 0;
		serviceStatus.dwCurrentState		= SERVICE_STOPPED;
		serviceStatus.dwCheckPoint			= 0;
		serviceStatus.dwWaitHint			= 0;
		break;

	case SERVICE_CONTROL_PAUSE :
		serviceStatus.dwCurrentState		= SERVICE_PAUSED;
		dprint3("SERVICE_CONTROL_PAUSE by SCM\n");
		break;

	case SERVICE_CONTROL_CONTINUE :
		serviceStatus.dwCurrentState		= SERVICE_RUNNING;
		dprint3("SERVICE_CONTROL_CONTINUE by SCM\n");
		break;

	case SERVICE_CONTROL_INTERROGATE :
		dprint3("SERVICE_CONTROL_INTERROGATE by SCM\n");
		break;
	case SERVICE_CONTROL_PARAMCHANGE :
		dprint3("SERVICE_CONTROL_PARAMCHANGE by SCM\n");
		break;

	case SERVICE_CONTROL_SHUTDOWN :
		dprint1("SERVICE_CONTROL_SHUTDOWN by SCM\n");
		StopServiceMain( true );
		serviceStatus.dwWin32ExitCode		= 0;
		serviceStatus.dwCurrentState		= SERVICE_STOPPED;
		serviceStatus.dwCheckPoint			= 0;
		serviceStatus.dwWaitHint			= 0;
		break;

	case SERVICE_CONTROL_NETBINDADD :
		dprint3("SERVICE_CONTROL_NETBINDADD by SCM\n");
		break;
	case SERVICE_CONTROL_NETBINDREMOVE :
		dprint3("SERVICE_CONTROL_NETBINDREMOVE by SCM\n");
		break;
	case SERVICE_CONTROL_NETBINDENABLE :
		dprint3("SERVICE_CONTROL_NETBINDENABLE by SCM\n");
		break;
	case SERVICE_CONTROL_NETBINDDISABLE :
		dprint3("SERVICE_CONTROL_NETBINDDISABLE by SCM\n");
		break;
	default:
		dprint2("oppcode = %ld by SCM\n", opcode);
		;
	}
	if (!SetServiceStatus( serviceStatusHandle, &serviceStatus ))
	{
		printf("%s: SetServiceStatus( ... ) error: %lu\n", s_name, GetLastError());
	fflush( stdout );	}
}
// end ServiceHandler

void __stdcall ServiceStart( DWORD argc, LPTSTR *argv )
{
	if(argc>0)
		ws_name = _strdup(argv[0]);

	serviceArgs = new ServiceThreadStruct;
	if (!serviceArgs || !serviceArgs->isValid)		return;

	serviceStatus.dwServiceType					= SERVICE_WIN32;
	serviceStatus.dwCurrentState				= SERVICE_START_PENDING;
	serviceStatus.dwControlsAccepted			= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE
		| SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_NETBINDCHANGE;
	serviceStatus.dwWin32ExitCode				= 0;
	serviceStatus.dwServiceSpecificExitCode		= 0;
	serviceStatus.dwCheckPoint					= 0;
	serviceStatus.dwWaitHint					= 120000;

	srvHandle = serviceStatusHandle = RegisterServiceCtrlHandler( ws_name, ServiceHandler );
	if (!serviceStatusHandle)
	{	dprint3( "%s: RegisterServiceCtrlHandler( ... ) error: %lu\n", s_name, GetLastError() );
	return;		}


	SetServiceStatus( serviceStatusHandle, &serviceStatus );

	StartServiceMain();

	if (serviceArgs->status != NO_ERROR)
	{	printf( "%s: StartServiceMain( ... ) return ERROR\n", s_name );	fflush( stdout );
	serviceStatus.dwCurrentState				= SERVICE_STOP_PENDING;
	serviceStatus.dwCheckPoint					= 0;
	serviceStatus.dwWaitHint					= 0;
	serviceStatus.dwWin32ExitCode				= serviceArgs->status;
	serviceStatus.dwServiceSpecificExitCode		= serviceArgs->specificError;
	if (!SetServiceStatus( serviceStatusHandle, &serviceStatus ))
		dprint3( "%s: SetServiceStatus( ... ) error: %lu\n", s_name, GetLastError() );
	return;		}

	serviceStatus.dwCurrentState	= SERVICE_RUNNING;
	serviceStatus.dwCheckPoint		= 0;
	serviceStatus.dwWaitHint		= 0;

	if (!SetServiceStatus( serviceStatusHandle, &serviceStatus ))
		dprint3( "%s: SetServiceStatus( ... ) error: %lu\n", s_name, GetLastError() );
}
// end ServiceStart

static inline int StartAsService( void )
{
	fclose( stdin );

	const auto log_dir = vs::GetLogDirectory();
	if (log_dir != "./") // No specific log directory, don't try to call create_directories("./") because it fails in Boost >= 1.60 instead of being no-op.
	{
		boost::system::error_code ec;
		boost::filesystem::create_directories(log_dir, ec);
		if (ec)
			dstream0 << "Can't create directory '" << log_dir << "'";
	}
	const auto stdout_path = log_dir + stdout_file_name;
	VS_RedirectOutput(stdout_path.c_str());

	SERVICE_TABLE_ENTRY dispatchTable[] = {{ const_cast<char*>(ws_name), ServiceStart }, { 0, 0 }};
	if (!StartServiceCtrlDispatcher( dispatchTable ))
	{	dprint3( "%s: StartServiceCtrlDispatcher( ... ) error: %lu", s_name, GetLastError() );
	return -1;		}
	return 0;
}
// end StartAsService

/* !!! Server SSL/TLS key dumping functions !!! */

// 64 Kb
#define BUFSZ (64 * 1024)

bool dump_reg_binary_data(const char *value_name, const char *file_name, bool remove_trailing_zero = false)
{
	std::vector<uint8_t> buf(BUFSZ);
	int size;

	if (!file_name)
		return false;

	// get data from registry
	VS_RegistryKey key(false, CONFIGURATION_KEY, true, false);
	while ((size = key.GetValue(&buf[0], BUFSZ, VS_REG_BINARY_VT, value_name)) < 0)
	{
		// buffer is too small - increase its size
		buf.resize(buf.size() * 2);
	}

	// value not found
	if (size == 0)
	{
		std::cerr << "Registry value " << "\"" << value_name << "\"" << " not found." << std::endl;
		return false;
	}

	// dump data
	std::ofstream fout(file_name, std::ios_base::out | std::ios_base::binary);
	if (remove_trailing_zero && size > 1)
	{
		size = buf[size - 1] == '\0' ? size - 1 : size;
	}
	fout.write((char *)&buf[0], size);
	fout.close();

	return true;
}

void dump_root_ca_cert(const char *where)
{
	VS_GET_PEM_CACERT;
	std::ofstream fout(where, std::ios_base::out | std::ios_base::binary);
	fout.write((char *)PEM_CACERT, strlen(PEM_CACERT));
	fout.close();
}

bool dump_certs_chain(const char *where)
{
	std::vector<uint8_t> end_cert(BUFSZ);
	std::vector<uint8_t> srv_chain(BUFSZ);
	int end_cert_size, srv_chain_size;
	bool status = true;

	VS_RegistryKey key(false, CONFIGURATION_KEY, true, false);

	// get end certificate
	while ((end_cert_size = key.GetValue(&end_cert[0], BUFSZ, VS_REG_BINARY_VT, SRV_CERT_KEY)) < 0)
	{
		// buffer is too small - increase its size
		end_cert.resize(end_cert.size() * 2);
	}


	// get chain
	while ((srv_chain_size = key.GetValue(&srv_chain[0], BUFSZ, VS_REG_BINARY_VT, SRV_CERT_CHAIN_KEY)) < 0)
	{
		// buffer is too small - increase its size
		srv_chain.resize(srv_chain.size() * 2);
	}

	/* !!! dump chain !!! */
	{
		BIO *mem_cert_bio = NULL;
		BIO *out = NULL;
		size_t pending = 0;

		try
		{
			out = BIO_new(BIO_s_mem());
			if (out == NULL)
				throw std::exception();
			BIO_ctrl(out,BIO_C_SET_BUF_MEM_EOF_RETURN,EOF,NULL);

			if (end_cert_size > 0)
			{
				mem_cert_bio = BIO_new_mem_buf(&end_cert[0], end_cert_size);
				if (mem_cert_bio == NULL)
					throw std::exception();
			}

			/* !!! dump certificate chain !!! */
			if (srv_chain_size > 0)
			{
				VS_Container chain_cont;
				// deserialize certificates chain
				chain_cont.Deserialize(&srv_chain[0], srv_chain_size);
				chain_cont.Reset();
				if (!chain_cont.IsValid())
				{
					throw std::exception();
				}

				/* iterate over chain container */
				while (chain_cont.Next())
				{
					if (!!chain_cont.GetName() && (0 == _stricmp(chain_cont.GetName(), CERTIFICATE_CHAIN_PARAM)))
					{
						bool error = false;
						X509 *cert = NULL;
						size_t sz = 0;
						const char *cert_in_chain = (const char*)chain_cont.GetBinValueRef(sz);
						BIO *mem_bio = BIO_new_mem_buf((void *)cert_in_chain, sz);

						try
						{
							cert = PEM_read_bio_X509(mem_bio, 0, 0, 0);
							if (cert == NULL)
								throw std::exception();
							if (PEM_write_bio_X509(out, cert) < 0)
								throw std::exception();
						}
						catch (...)
						{
							error = true;
						}

						BIO_free(mem_bio);
						X509_free(cert);

						if (error)
							throw std::exception();
					}
				}
			}

			/* !!! dump end certificate as PEM data !!! */
			if (mem_cert_bio != NULL)
			{
				X509* cert = PEM_read_bio_X509(mem_cert_bio, 0, 0, 0);
				if (cert == NULL)
					throw std::exception();

				if (PEM_write_bio_X509(out, cert) <= 0)
					throw std::exception();
				X509_free(cert);
			}

			/* !!! write pending PEM data to the file !!! */
			pending = BIO_ctrl_pending(out);
			if (pending > 0)
			{
				std::vector<uint8_t> out_data(pending);
				std::ofstream fout(where, std::ios_base::out | std::ios_base::binary);
				if (BIO_read(out, &out_data[0], pending) != pending)
				{
					fout.close();
					throw std::exception();
				}

				fout.write((const char *)&out_data[0], pending);
				fout.close();
			}
		}
		catch (...)
		{
			status = false;
		}

		BIO_free(mem_cert_bio);
		BIO_free(out);
	}

	return status;
}

std::pair<std::string, std::string> ParseOldStyleOptions(const std::string& token_)
{
	const string_view token(token_);

	assert(!token.empty());
	if (token[0] != '/')
		return {}; // old style options start with '/'

	const auto value_sep_pos = token.find_first_of(":=");
	const auto name = token.substr(1, value_sep_pos - 1);
	if (name.empty())
		return {};
	if (name.find('/') != name.npos)
		return {}; // option name can't contain '/', this token is an absolute Unix file path
	const string_view value = token.substr(value_sep_pos != token.npos ? value_sep_pos + 1 : token.size());

	return std::make_pair(std::string(name), std::string(value));
}

////////// Common Start/Stop Functions. /////////////////////////////////////////////////

static inline int ServerMain( void )
{
	vs::InitOpenSSL();

	s_name = VS_Server::ShortName();	l_name = VS_Server::LongName();
	r_key = VS_Server::RegistryKey();	ws_name = VS_Server::ServiceName();

	if (!s_name || !*s_name || !l_name || !*l_name || !r_key || !*r_key || !ws_name || !*ws_name)
	{	printf( "%s: There are no all names necessary for work of the server application.\n", _MD_STR_(s_name) );
	printf( "\tShort Name: %s, Long Name: %s,\n\tRegistry Key: %s, Windows Service Name %s\n", _MD_STR_(s_name), _MD_STR_(l_name), _MD_STR_(r_key), _MD_STR_(ws_name) );
	return -1;	}

	if (FAILED(CoInitializeEx( NULL, COINIT_MULTITHREADED )))
	{	printf( "%s: DCOM Initialization failed.\n", s_name );
	return -1;	}


	bool      is_instance_config=true;
	unsigned  sz;

	std::string opt_config_file;
	po::options_description command_line_options("Command line only options", 1000);
	command_line_options.add_options()
		("help,h", "produce this help message")
		("ConfigFile,c", po::value<std::string>(&opt_config_file)->value_name("<path>"), "path to the configuration file, by default 'tc_server.cfg' will be used if it exists")
	;

	std::string opt_registry_backend;
	std::string opt_instance;
	std::string opt_endpoint;
	po::options_description general_options("General options", 1000);
	general_options.add_options()
		("RegistryBackend", po::value<std::string>(&opt_registry_backend)->value_name("<string>"), "registry backend specification in the form NAME:[OPTIONS]")
		("Instance", po::value<std::string>(&opt_instance)->value_name("<string>"), "use alternative instance name")
		("Endpoint", po::value<std::string>(&opt_endpoint)->value_name("<string>"), "use alternative endpoint name")
#if defined(_WIN32)
		("Service",  "start as a service")
#endif
		("R", "restart on error")
	;

	po::options_description registration_options("Registration options", 1000);
	registration_options.add_options()
		("Mode", po::value<unsigned>(&VS_Server::reg_params.mode)->value_name("<integer>"), "server registration mode: 1 or 2")
		("ServerName", po::value<std::string>(&VS_Server::reg_params.server_name)->value_name("<string>"), "server name (abc123.trueconf.name#vcs), required in mode 1")
		("ServerID", po::value<std::string>(&VS_Server::reg_params.server_id)->value_name("<string>"), "server id (abc123)")
		("Serial", po::value<std::string>(&VS_Server::reg_params.serial)->value_name("<string>"), "server serial (ABCD-EFGH-IJKL)")
		("File", po::value<std::string>(&VS_Server::reg_params.offline_reg_file)->value_name("<filename>"), "offline registration file, required in mode 2")
	;

	std::string opt_end_cert;
	std::string opt_cert_chain;
	std::string opt_ca_cert;
	std::string opt_private_key;
	po::options_description export_options("Certificate export options", 1000);
	export_options.add_options()
		("EndCert", po::value<std::string>(&opt_end_cert)->value_name("<filename>"), "save end certificate to filename")
		("CertChain", po::value<std::string>(&opt_cert_chain)->value_name("<filename>"), "save certificate chain to filename")
		("CAcert", po::value<std::string>(&opt_ca_cert)->value_name("<filename>"), "save root CA certificate to filename")
		("PrivateKey", po::value<std::string>(&opt_private_key)->value_name("<filename>"), "save private key to filename")
	;

	std::string opt_key;
	std::string opt_expire;
	po::options_description key_options("Key manipulation options", 1000);
	key_options.add_options()
		("Key", po::value<std::string>(&opt_key)->value_name("<string>"), "install key, if key is not present install default key")
		("Expire", po::value<std::string>(&opt_expire)->value_name("<string>"), "expire arg, possible args {Key}")
	;

	auto usage = [&]
	{
		const auto exe_name = boost::filesystem::path(VS_Server::argv[0]).filename().string();
		std::cout << command_line_options;
		std::cout << general_options;
		std::cout << registration_options;
		std::cout << export_options;
		std::cout << key_options;
		std::cout << "Options can be specified in the configuration file one per line in the 'name=value' form.\n";
		std::cout << '\n';
		std::cout << "Registration Mode usage:\n\t" << exe_name << " --Mode=1 --ServerName=name --ServerID=server_id --Serial=serial" << '\n';
		std::cout << "OfflineRegistration Mode usage:\n\t" << exe_name << " --Mode=1 --ServerName=name --ServerID=server_id --Serial=serial --File=file_path" << '\n';
		std::cout << "Registration from file Mode usage:\n\t" << exe_name << " --Mode=2 --File=file_path" << '\n';
	};

	po::variables_map var_map;
	try
	{
		po::options_description options;
		options.add(command_line_options);
		options.add(general_options);
		options.add(registration_options);
		options.add(export_options);
		options.add(key_options);
		po::store(po::parse_command_line(
			VS_Server::argc, VS_Server::argv,
			options,
			po::command_line_style::case_insensitive | po::command_line_style::default_style,
			ParseOldStyleOptions
		), var_map);
		po::notify(var_map);
	}
	catch (std::exception& e)
	{
		std::cout << s_name << ": " << e.what() << '\n';
		usage();
		return -1;
	}

	if (var_map.count("help") > 0)
	{
		usage();
		return 0;
	}

	try
	{
		std::ifstream config_file_stream;
		config_file_stream.open(!opt_config_file.empty() ? opt_config_file.c_str() : "tc_server.cfg");
		if (config_file_stream.is_open())
		{
			po::options_description options;
			options.add(general_options);
			po::store(po::parse_config_file(config_file_stream, options), var_map);
			po::notify(var_map);
		}
		else if (!opt_config_file.empty())
		{
			std::cout << s_name << ": Can't open the configuration file (" + opt_config_file + ")\n";
			return -1;
		}
	}
	catch (std::exception& e)
	{
		std::cout << s_name << ": " << e.what() << '\n';
		usage();
		return -1;
	}

	if (!opt_instance.empty())
	{
		VS_SimpleStr tmp_root_key = r_key;
		tmp_root_key += opt_instance.c_str();
		VS_Server::g_root_key = tmp_root_key;
		r_key = VS_Server::RegistryKey();
	}

	// Settings datbase validating.
	VS_RegistryKey::SetDefaultRoot(r_key ? r_key : "");

	if (opt_registry_backend.empty())
	{
#if defined(_WIN32)
		opt_registry_backend = "registry:force_lm=true";
#else
		std::cout << s_name << ": Registry backend isn't specified\n";
		return -1;
#endif
	}
	if (!VS_RegistryKey::InitDefaultBackend(opt_registry_backend))
	{
		std::cout << s_name << ": Can't initialize registry backend, check 'RegistryBackend' option value\n";
		return -1;
	}

	net::dns_tools::init_loaded_options();

	VS_RegistryKey cfg_root( false, "" );

	/* !!! SSL/TLS dumping operations !!! */
	bool was_dumpopt = false;

	if (!opt_end_cert.empty())
	{
		dump_reg_binary_data(SRV_CERT_KEY, opt_end_cert.c_str(), true);
		was_dumpopt = true;
	}

	if (!opt_cert_chain.empty())
	{
		dump_certs_chain(opt_cert_chain.c_str());
		was_dumpopt = true;
	}

	if (!opt_ca_cert.empty())
	{
		dump_root_ca_cert(opt_ca_cert.c_str());
		was_dumpopt = true;
	}

	if (!opt_private_key.empty())
	{
		dump_reg_binary_data(SRV_PRIVATE_KEY, opt_private_key.c_str(), true);
		was_dumpopt = true;
	}

	if (was_dumpopt)
		return EXIT_SUCCESS;

	if (var_map.count("Key") > 0)
	{
		if (!opt_key.empty())
		{
			printf("Key %s install %s. Please, restart server\n", opt_key.c_str(), VS_ArmInstallKey(opt_key.c_str()) ? "successfull" : (VS_ArmSetDefaultKey(), "failed"));
			return 0;
		}
		else
		{
			printf("Default key installed successfully. Please, restart server\n");
			VS_ArmSetDefaultKey();
			return 0;
		}
	}

	if (!opt_expire.empty())
	{
		if (boost::iequals(opt_expire, "Key"))
		{
			printf("Current key expired successfully. Please, restart server\n");
			VS_ArmExpireInstalledKey();
			return 0;
		}
	}

	if (!opt_endpoint.empty())
	{
		if (opt_endpoint.size() > VS_ACS_MAX_SIZE_ENDPOINT_NAME)
		{
			std::cout << s_name << ": Endpoint name is too long: " << opt_endpoint.size() << '\n';
			return -1;
		}
		is_instance_config = true;
		g_tr_endpoint_name = opt_endpoint;
		sz = g_tr_endpoint_name.length();
	}
	else
	{
		is_instance_config=false;
		/**
			Если запущен в режиме регистрации, то сгенерить имя
		*/
		if (VS_Server::reg_params.mode > 2)
		{
			std::cout << s_name << ": Invalid argument for --Mode: " << VS_Server::reg_params.mode << '\n';
			usage();
			return -1;
		}
		if (VS_Server::reg_params.mode == 1)
		{
			if (VS_Server::reg_params.server_name.size() > VS_ACS_MAX_SIZE_ENDPOINT_NAME)
			{
				std::cout << s_name << ": Server name is too long: " << VS_Server::reg_params.server_name.size() << '\n';
				usage();
				return -1;
			}
			else if (VS_Server::reg_params.server_name.empty())
			{
				std::cout << s_name << ": Server name isn't specified.\n";
				usage();
				return -1;
			}
		}
		if (VS_Server::reg_params.mode == 2)
		{
			if (VS_Server::reg_params.offline_reg_file.empty())
			{
				std::cout << s_name << ": Registration file isn't specified.\n";
				usage();
				return -1;
			}
		}
		if(1 == VS_Server::reg_params.mode)
		{
			char tmp_name[256] = {};
			VS_GenKeyByMD5(tmp_name);
			strcpy(tmp_name+32,".srv.name#tmp");
			g_tr_endpoint_name = tmp_name;
		}
		else
		{
			if (!cfg_root.GetString(g_tr_endpoint_name, VS_SERVER_ENDPOINT_ARG_PREF))
			{
				printf("%s: The endpoint name of the %s is not specified.\n", s_name, l_name);
				return -1;
			}
		}
	}


	{
		char   str[512];

		if(is_instance_config)
		{
			sz = sizeof(str) - sz;
			if ((unsigned)strlen( r_key ) >= sz)
			{
				printf( "%s: The registry key name is too long: %u.\n", s_name, (unsigned)strlen( r_key ) );
				return -1;
			}

			sprintf( str, "%s\\%s", r_key, g_tr_endpoint_name.c_str());
			VS_RegistryKey::SetDefaultRoot(str);
		}

		VS_RegistryKey   key( false, CONFIGURATION_KEY );
		strcpy(str, g_tr_endpoint_name.c_str());
		VS_FilterPath(str);
		wchar_t w_str[512];
		VS_UTF8ToUCS(str, w_str, 512);

		key.GetValue(w_str, sizeof(w_str) - 1, VS_REG_WSTRING_VT, WORKING_DIRECTORY_TAG);
		if (*w_str)
		{
			sz = (unsigned)wcslen( w_str );
			if (sz >= (unsigned)(sizeof(VS_Server::wdir)/sizeof(wchar_t)))
			{
				printf( "%s: The working directory name is too long: %u.\n", s_name, sz );
				return -1;
			}
			wcscpy( VS_Server::wdir, w_str );		*str = 0;
			MakeChangeDir( VS_Server::wdir );
		}
		{	const time_t   tm = time( 0 );
		dprint1( "%s started at %s\n", l_name, ctime( &tm ));		}
		key.GetValue(str, sizeof(str) - 1, VS_REG_STRING_VT, SMTP_ADMIN_EMAIL_TAG);
		if (*str)
		{	sz = (unsigned)strlen( str );
		if (sz >= (unsigned)sizeof(admin_email))
		{	dprint3( "%s: Administrator's e-mail address is too long: %u.\n", s_name, sz );
		return -1;	}
		strcpy( admin_email, str );		*str = 0;
		{	int   port = INT_MIN;
		key.GetValue(&port, sizeof(port), VS_REG_INTEGER_VT, SMTP_PORT_TAG);
		if (port < 1 || port > 65535)
		{	if (port != INT_MIN)
		dprint3( "%s: Administrative SMTP port has incorrect value: %i.\n", s_name, port );
		else	dprint3( "%s: Administrative SMTP port not defined.\n", s_name );
		return -1;	}
		smtp_port = port;
		}
		key.GetValue(str, sizeof(str) - 1, VS_REG_STRING_VT, SMTP_SERVER_TAG);
		if (!*str)
		{	dprint3( "%s: Administrative SMTP Server host not defined.\n", s_name );
		return -1;	}
		sz = (unsigned)strlen( str );
		if (sz >= (unsigned)sizeof(smtp_server))
		{	dprint3( "%s: Administrative SMTP Server host too long: %u.\n", s_name, sz );
		return -1;	}
		strcpy( smtp_server, str );		*str = 0;
		key.GetValue(str, sizeof(str) - 1, VS_REG_STRING_VT, SMTP_LOGIN_TAG);
		if (*str)
		{
			sz = (unsigned)strlen( str );
			if (sz >= (unsigned)sizeof(smtp_login))
			{
				dprint3( "%s: Administrative SMTP login is too long: %u.\n", s_name, sz );
				return -1;
			}
			strcpy(smtp_login, str); *str = 0;
		}
		key.GetValue(str, sizeof(str) - 1, VS_REG_STRING_VT, SMTP_PASSWORD_TAG);
		if (*str)
		{
			sz = (unsigned)strlen( str );
			if (sz >= (unsigned)sizeof(smtp_password))
			{
				dprint3( "%s: Administrative SMTP password is too long: %u.\n", s_name, sz );
				return -1;
			}
			strcpy(smtp_password, str); *str = 0;
		}
		int bufVal = VS_NoAuth;
		key.GetValue(&bufVal, sizeof(bufVal), VS_REG_INTEGER_VT, SMTP_AUTH_TYPE_TAG);
		if (bufVal == VS_SimplePass || bufVal == VS_NTLM)
			smtp_auth_type = static_cast<VS_SmtpAuthType>(bufVal);
		else
			smtp_auth_type = VS_NoAuth;
		bufVal = 0;
		key.GetValue(&bufVal, sizeof(bufVal), VS_REG_INTEGER_VT, SMTP_USE_SSL_TAG);
		smtp_use_ssl = bufVal != 0;
		}
	}

#ifndef _SVKS_M_BUILD_
	// adding to the firewall
	{
		VS_RegistryKey key(false, "", false, true);
		long val = 0;
		key.GetValue(&val, 4, VS_REG_INTEGER_VT, "InFirewall");
		if (!val)
		{
			VS_FirewallMgr *mgr = VS_FirewallMgr::Instance();
			if (mgr && mgr->AddFirewall(VS_WinXPFirewall::Instance()))
			{
				bool fg = mgr->IsValid();
				auto fileName = GetCommandLineW();
				int numArgs = 0;
				LPWSTR * argv = CommandLineToArgvW(fileName, &numArgs);
				if (argv)
				{
					wchar_t appName[] = L"TrueConf Server";
					mgr->AddApplication(argv[0], appName, 0);
					val = 1;
					key.SetValue(&val, 4, VS_REG_INTEGER_VT, "InFirewall");
				}
			}
		}
	}
#endif

	service = var_map.count("Service") > 0;
	restartable = var_map.count("R") > 0;

	VS_PerformanceMonitor::Instance().Start();
	return !service ? StartAsConsole() : StartAsService();
}
// end ServerMain

/*
	this function only for support update configuration from servers ver <= 4.3.7.
	It is not necessary to support this function in crossplatform version.

*/
void RemovePassFromPrivateKey()
{
	// check, if private key is encrypted then decrypt pk using pass, check and save in regisry
	const char SERVER_CERT_PRIVATE_KEY_PASS[] = "cKe4-vbmre 0re=48uhgnpPiGf#$%^kltRR;lmkdpo";
	const char OLD_REG_PRIVATE_KEY_PASS[] = "P[sDubdfgPLk|d)d#4cmdfa658HhdsflvFe5^&*Xl";
	for (auto i = 0; i < 2; i++)
	{
		const char *param_name = i == 0 ? SRV_PRIVATE_KEY : "PrivateKeyOld";
		const char *password = i == 0 ? SERVER_CERT_PRIVATE_KEY_PASS : OLD_REG_PRIVATE_KEY_PASS;

		std::unique_ptr<char, free_deleter> privateKey;
		VS_PKey		private_key;
		VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
		rkey.GetValue(privateKey, VS_REG_BINARY_VT, param_name);
		if (!privateKey)
			return;
		if (!private_key.SetPrivateKey(privateKey.get(), store_PEM_BUF))
		{
			if (private_key.SetPrivateKey(privateKey.get(), store_PEM_BUF, password))
			{
				std::vector<char> clean_privateKey;
				uint32_t sz(0);
				private_key.GetPrivateKey(store_PEM_BUF, 0, &sz);
				if (sz > 0)
				{
					clean_privateKey.resize(sz);
					if (private_key.GetPrivateKey(store_PEM_BUF, clean_privateKey.data(), &sz))
					{
						rkey.SetValue(clean_privateKey.data(), clean_privateKey.size(), VS_REG_BINARY_VT, param_name);
					}
				}
			}
		}
	}
}
static inline bool ServerStart( void )
{
	dstream0 << "Server is starting. Product name: " << VS_Server::LongName() << "; version: " << VS_Server::ProductVersion();
	VS_ReadDebugKeys();
	RemovePassFromPrivateKey();
	GetWatchDogImp()->Init(!restartable ? 0 : Restart, []() {});
	return VS_Server::Start();
}
// end ServerStart

static inline void ServerStop( void )
{
	VS_Server::Stop();
	const time_t   tm = time( 0 );
	dprint1( "%s terminated at %s\n", l_name, ctime( &tm ));
}

bool QuickDestroy()
{
	return srvQuickDestroy;
}

VS_RoutersWatchdog * VS_Server::GetWatchDog()
{
	return GetWatchDogImp();
}

bool VS_Server::PauseDestroy( unsigned long msec )
{
	if(srvHandle==NULL)
		return false;

	srvCheckPoint++;

	SERVICE_STATUS			serviceStatus;

	serviceStatus.dwWin32ExitCode		= 0;
	serviceStatus.dwCurrentState		= SERVICE_STOP_PENDING;
	serviceStatus.dwCheckPoint			= srvCheckPoint;
	serviceStatus.dwWaitHint				= msec;
	serviceStatus.dwControlsAccepted =0;
	serviceStatus.dwServiceType			= SERVICE_WIN32_OWN_PROCESS;
	if (!SetServiceStatus( srvHandle, &serviceStatus ))
	{
		printf("%s: SetServiceStatus( ... ) error: %lu\n", s_name, GetLastError());
	fflush( stdout );
	}

	return true;

}
#endif // _WIN32

/////////////////////////////////////////////////////////////////////////////////////////
#endif

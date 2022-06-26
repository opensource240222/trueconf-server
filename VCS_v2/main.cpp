#ifndef _WIN32
// pid_t - for daemonize
#include <unistd.h>
#include <sys/types.h>
#else
#include <boost/locale/generator.hpp>
#endif
#include "VCSServices_v2.h"
#include "acs_v2/Service.h"
#include "AppServer/Services/VS_AppServerData.h"
#include "newtransport/Router/Router.h"
#include "streams_v2/Router/Router.h"
#include "Bwt/Handler.h"
#include "net/EndpointRegistry.h"
#include "net/InterfaceInfo.h"
#include "net/DNSUtils/VS_DNSTools.h"
#include "SecureLib/VS_CryptoInit.h"
#include "SecureLib/VS_SSLConfigKeys.h"
#include "ServerServices/VS_CheckCert.h"
#include "std/cpplib/ASIOThreadPool.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/ThreadUtils.h"
#include "std-generic/cpplib/deleters.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/event.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/debuglog/VS_Debug.h"
#include "std/clib/daemonize.h"
#include "std/cpplib/NTService.h"
#include "std/cpplib/MakeShared.h"
#include "std/Globals.h"
#include "statuslib/VS_ResolveServerFinder.h"
#include "tools/Server/RegistrationParams.h"
#include "tools/Watchdog/VS_RoutersWatchdog.h"
#include "config.h"
#include "version.h"

#include <boost/filesystem.hpp>
#include <boost/optional/optional.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/system/system_error.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <fstream>

#ifdef _WIN32		// to build VCS_v2 at Windows
#include "../common/tools/Server/VS_Server.h"
//@{ \name Server, Registry Key and Windows Service Functions.
bool VS_Server::Start(void)
{
	return false;
}
void VS_Server::Stop(void)
{}
const char* VS_Server::ShortName() { return "VS_TRUECONF_WS_SERVICE_NAME"; }
const char* VS_Server::LongName() { return "VS_TRUECONF_WS_DISPLAY_NAME"; }
const char* VS_Server::RegistryKey() {
	//if (!!g_root_key)
	//	return g_root_key;
	return "VS_TRUECONF_WS_ROOT_KEY_NAME";
}
const char* VS_Server::ServiceName() { return "VS_TRUECONF_WS_SERVICE_NAME"; }
const char* VS_Server::ProductVersion() { return "STRPRODUCTVER"; }
//@}
#endif

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

namespace po = boost::program_options;

const auto c_service_start_timeout = std::chrono::seconds(5);
const auto c_service_stop_timeout = std::chrono::seconds(30);

static std::pair<std::string, std::string> ParseOldStyleOptions(const std::string& token_)
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

static void error(string_view msg)
{
	std::cerr << "error: " << msg << '\n';
}
template <class... Args>
void error(std::error_code ec, Args&&... args)
{
	std::cerr << "error: ";
	(void)std::initializer_list<int>{((std::cerr << args), 0)...};
	if /*constexpr*/ (sizeof...(Args) > 1)
		std::cerr << ": ";
	std::cerr << ec.message() << '\n';
}
template <class... Args>
void error(boost::system::error_code ec, Args&&... args)
{
	std::cerr << "error: ";
	(void)std::initializer_list<int>{((std::cerr << args), 0)...};
	if /*constexpr*/ (sizeof...(Args) > 1)
		std::cerr << ": ";
	std::cerr << ec.message() << '\n';
}

#ifndef _WIN32
static std::function<int(void)> g_daemon_body;
#endif

int main(int argc, char* argv[]) try
{
	vs::FixThreadSystemMessages();

	std::string opt_config_file;
	po::options_description command_line_options("Command line only options", 1000);
	command_line_options.add_options()
		("help,h", "produce this help message")
		("ConfigFile,c", po::value<std::string>(&opt_config_file)->value_name("<path>"), "path to the configuration file, by default '" TC_SERVER_CFG_PATH "' will be used if it exists")
		("version,v", "prints the product version information")
		;

	std::string opt_registry_backend;
	std::string opt_instance;
	std::string opt_endpoint;
	uint32_t opt_debug = 0;
	std::string opt_log_directory;
#ifndef _WIN32
	std::string opt_pid_file;
#endif
	po::options_description general_options("General options", 1000);
	general_options.add_options()
		("RegistryBackend", po::value<std::string>(&opt_registry_backend)->value_name("<string>"), "registry backend specification in the form NAME:[OPTIONS]")
		("Instance", po::value<std::string>(&opt_instance)->value_name("<string>"), "use alternative instance name")
		("Endpoint", po::value<std::string>(&opt_endpoint)->value_name("<string>"), "use alternative endpoint name")
		("Debug", po::value<uint32_t>(&opt_debug)->value_name("<level>"), "debug level: 1-4, 0=off")
		("LogDirectory", po::value<std::string>(&opt_log_directory)->value_name("<path>"), "directory for storing logs")
		("LogToConsole", "print log messages to the output instead of stdout.log file")
#ifndef _WIN32
		("Daemonize", po::value<std::string>(&opt_pid_file)->value_name("<path to the PID lock-file>"), "daemonize the process")
#endif
#if defined(_WIN32)
		("Service", "start as a service")
#endif
		("R", "restart on error");

	vs::RegistrationParams rp;
	po::options_description registration_options("Registration options", 1000);
	registration_options.add_options()
		("Mode", po::value<unsigned>(&rp.mode)->value_name("<integer>"), vs::RegistrationParams::option_description_mode)
		("ServerName", po::value<std::string>(&rp.server_name)->value_name("<string>"), vs::RegistrationParams::option_description_server_name)
		("ServerID", po::value<std::string>(&rp.server_id)->value_name("<string>"), vs::RegistrationParams::option_description_server_id)
		("Serial", po::value<std::string>(&rp.serial)->value_name("<string>"), vs::RegistrationParams::option_description_serial)
		("File", po::value<std::string>(&rp.offline_reg_file)->value_name("<filename>"), vs::RegistrationParams::option_description_file)
		;

#if 0
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
#endif

	auto usage = [&]
	{
		const auto exe_name = boost::filesystem::path(argv[0]).filename().string();
		std::cout << command_line_options;
		std::cout << general_options;
		std::cout << registration_options;
#if 0
		std::cout << export_options;
		std::cout << key_options;
#endif
		std::cout << "Options can be specified in the configuration file one per line in the 'name=value' form.\n";
		std::cout << '\n';
		std::cout << vs::RegistrationParams::GetUsageMessage(exe_name);
	};

	po::variables_map var_map;
	{
		po::options_description options;
		options.add(command_line_options);
		options.add(general_options);
		options.add(registration_options);
#if 0
		options.add(export_options);
		options.add(key_options);
#endif
		po::store(po::parse_command_line(
			argc, argv,
			options,
			po::command_line_style::case_insensitive | po::command_line_style::default_style,
			ParseOldStyleOptions
		), var_map);
		po::notify(var_map);
	}

	if (var_map.count("version") > 0)
	{
		std::cout << "Product name: " << VS_PRODUCT_NAME << "; version: " << STRPRODUCTVER << std::endl;
		return 0;
	}

	if (var_map.count("help") > 0)
	{
		usage();
		return 0;
	}

	{
		std::ifstream config_file_stream;
		config_file_stream.open(!opt_config_file.empty() ? opt_config_file.c_str() : TC_SERVER_CFG_PATH);
		if (config_file_stream.is_open())
		{
			po::options_description options;
			options.add(general_options);
			po::store(po::parse_config_file(config_file_stream, options), var_map);
			po::notify(var_map);
		}
		else if (!opt_config_file.empty())
			throw std::runtime_error("Can't open the configuration file: " + opt_config_file);
	}

	auto watchdog = vs::make_unique<VS_RoutersWatchdog>();
	auto start_server = [&](void) -> int {

//https://www.boost.org/doc/libs/1_48_0/libs/locale/doc/html/default_encoding_under_windows.html
#ifdef _WIN32
		std::locale::global(boost::locale::generator().generate(""));
		boost::filesystem::path::imbue(std::locale{});
#endif

		// Redirect output unless we were explicitly asked to not do that.
		bool need_redirect_output = var_map.count("LogToConsole") == 0;

		if (!need_redirect_output)
		{
			// In case output won't be redirected disable the output buffering (VS_RedirectOutput does that in the other case).
			setbuf(stdout, nullptr);
			setbuf(stderr, nullptr);
		}

		auto redirect_logs = [&]() {
			const auto& log_dir = vs::GetLogDirectory();

			// Don't try to call create_directories("./") because it fails in Boost >= 1.60 instead of being a no-op.
			if (log_dir != "./")
			{
				boost::system::error_code ec;
				boost::filesystem::create_directories(log_dir, ec);
				if (ec)
					error(ec, "Can't create directory '", log_dir, "'");
			}

			const auto stdout_path = log_dir + "stdout.log";
			VS_RedirectOutput(stdout_path.c_str());
			need_redirect_output = false;
		};

		if (!opt_log_directory.empty())
			vs::SetLogDirectory(opt_log_directory);

		// Redirect output if log directory was specified on the command line or in the config.
		if (need_redirect_output && !opt_log_directory.empty())
			redirect_logs();

		// Initialize VS_RegistryKey
		if (opt_registry_backend.empty())
		{
#if defined(_WIN32)
			opt_registry_backend = "registry:force_lm=true";
#else
			throw std::runtime_error("Registry backend isn't specified");
#endif
		}
		if (!VS_RegistryKey::InitDefaultBackend(opt_registry_backend))
			throw std::runtime_error("Can't initialize registry backend, check 'RegistryBackend' option value");
		VS_RegistryKey::SetDefaultRoot("TrueConf\\Server" + opt_instance);

		vs::InitOpenSSL();
		net::dns_tools::init_loaded_options();

		VS_RegistryKey root_key(false, "", false, true);
		VS_RegistryKey cfg_key(false, "Configuration", false, true);

		// Switch into configured working directory
		std::string work_dir;
		cfg_key.GetString(work_dir, WORKING_DIRECTORY_TAG);
		if (!work_dir.empty())
		{
			boost::system::error_code ec;
			boost::filesystem::create_directories(work_dir, ec);
			if (ec)
				error(ec, "Can't create directory '", work_dir, "'");
			ec = {};
			boost::filesystem::current_path(work_dir, ec);
			if (ec)
				error(ec, "Can't change directory to '", work_dir, "'");
		}

		if (need_redirect_output)
		{
			// We couldn't redirect the output before because we didn't know where to redirect.
			assert(opt_log_directory.empty());

			// Store logs in the working directory.
			assert(vs::GetLogDirectory() == "./");
			redirect_logs();
		}

		// Setup logging
		if (var_map.count("Debug"))
		{
			opt_debug = std::min(opt_debug, 4u);
			{
				int32_t value = opt_debug;
				cfg_key.SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "Debug Level");
			}
			if (opt_debug > 0)
			{
				int32_t value = 0xffffffff;
				cfg_key.SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "Debug Modules");
			}
		}
		VS_ReadDebugKeys();

		// Save current directory as the working directory if it wasn't configured before
		if (work_dir.empty())
		{
			boost::system::error_code ec;
			auto current_dir = boost::filesystem::current_path(ec).native();
			if (ec)
				error(ec, "Can't determine current directory");
#if defined(_WIN32)
			work_dir = vs::UTF16toUTF8Convert(current_dir);
#else
			work_dir = std::move(current_dir);
#endif
			if (!work_dir.empty())
			{
				cfg_key.SetString(work_dir.c_str(), WORKING_DIRECTORY_TAG);
				dstream0 << "Working directory is not set, setting to current directory: '" << work_dir << "'";
			}
		}

		// Load/store endpoint
		std::string endpoint_name;
		if (!opt_endpoint.empty())
		{
			endpoint_name = opt_endpoint;
			root_key.SetString(opt_endpoint.c_str(), "Endpoint");
		}
		else
			root_key.GetString(endpoint_name, "Endpoint");

		g_tr_endpoint_name = endpoint_name;

		rp.Update();
		const auto param_error = rp.Check();
		if (!param_error.empty())
			throw std::runtime_error(param_error);

		if (rp.mode == 1)
		{
			char tmp_name[256] = { 0 };
			VS_GenKeyByMD5(tmp_name);
			strcpy(tmp_name + 32, ".srv.name#tmp");
			endpoint_name = tmp_name;

			// todo(kt): delete Cert/PrivateKey
			if ((cfg_key.HasValue(SRV_CERT_KEY) && !cfg_key.RemoveValue(SRV_CERT_KEY)) ||
				(cfg_key.HasValue(SRV_PRIVATE_KEY) && !cfg_key.RemoveValue(SRV_PRIVATE_KEY)))
			{
				throw std::runtime_error("Can not delete Certificate or PrivateKey to register");
			}
		}
		else if (rp.mode == 0) {
			if (!VS_CheckCert())
				throw std::runtime_error("Certificate not valid");
		}

		if (endpoint_name.empty())
			throw std::runtime_error("Endpoint isn't specified");

		// Load private key, certificate and certificate chain container
		std::string private_key;
		{
			std::unique_ptr<char, free_deleter> p;
			int32_t res = 0;
			if ((res = cfg_key.GetValue(p, VS_REG_BINARY_VT, SRV_PRIVATE_KEY)) > 0)
				private_key.assign(p.get(), res);
		}
		std::unique_ptr<void, free_deleter> certificate;
		size_t certificate_size = 0;
		{
			int32_t res = 0;
			if ((res = cfg_key.GetValue(certificate, VS_REG_BINARY_VT, SRV_CERT_KEY)) > 0)
				certificate_size = static_cast<size_t>(res);
		}
		std::unique_ptr<void, free_deleter> certificate_chain;
		size_t certificate_chain_size = 0;
		{
			int32_t res = 0;
			if ((res = cfg_key.GetValue(certificate_chain, VS_REG_BINARY_VT, SRV_CERT_CHAIN_KEY)) > 0)
				certificate_chain_size = static_cast<size_t>(res);
		}
		VS_Container certificate_chain_cnt;
		certificate_chain_cnt.Deserialize(certificate_chain.get(), certificate_chain_size);
		certificate_chain_cnt.AddValue(CERTIFICATE_PARAM, certificate.get(), certificate_size);

		g_appServer = new VS_AppServerData;
		VS_SCOPE_EXIT { delete g_appServer; g_appServer = nullptr; };

		const unsigned n_threads = std::thread::hardware_concurrency()
			+ 1/*ACS*/
			+ 1/*transport*/
			+ 1/*httpHandler_v2*/
			+ 1/*VCSAuthSrv strand*/
			;

		vs::ASIOThreadPool atp(n_threads);
		atp.Start();
		VS_SCOPE_EXIT
		{
			atp.get_io_service().stop(); // FIXME: Ideally we should be able stop without this.
			atp.Stop();
		};

		// ACS
		auto acs_srv = acs::Service::Create(atp.get_io_service());
		if (!acs_srv)
			throw std::runtime_error("Failed to create acs::Service instance");

		VS_SCOPE_EXIT
		{
			// Stop ACS
			std::cout << "Stopping: Access Connections System\n";
			{
				auto stop_f = acs_srv->Stop();
				if (stop_f.wait_for(c_service_stop_timeout) != std::future_status::ready)
					error("acs::Service::Stop took too long");
				else if (!stop_f.valid())
					error("acs::Service::Stop failed");
			}
			std::cout << "Stopped:  Access Connections System\n";
		};
		{
			std::cout << "Starting: Access Connections System\n";
			auto start_f = acs_srv->Start();
			if (start_f.wait_for(c_service_start_timeout) != std::future_status::ready)
				throw std::runtime_error("acs::Service::Start took too long");
			if (!start_f.valid() || !start_f.get())
				throw std::runtime_error("acs::Service::Start failed");

			unsigned int n_listeners = 0;
			if (net::endpoint::GetCountAcceptTCP(g_tr_endpoint_name, false) > 0)
			{
				n_listeners = acs_srv->AddListeners(g_tr_endpoint_name);
			}
			else {				
				boost::system::error_code ec;
				const auto interfaces = net::GetInterfaceInfo();
				for (const auto& ii : *interfaces)
				{
					for (const auto& i_addr : ii.addr_local_v4)
					{
						ec = acs_srv->AddListener(i_addr, 4307, net::protocol::TCP);
						if (ec)
							error(ec, "acs::Service::AddListener");
						else
							++n_listeners;
					}
					for (const auto& i_addr : ii.addr_local_v6)
					{
						ec = acs_srv->AddListener(i_addr, 4307, net::protocol::TCP);
						if (ec)
							error(ec, "acs::Service::AddListener");
						else
							++n_listeners;
					}
				}
			}
			if (!n_listeners)
				throw std::runtime_error("acs: Couldn't add any listeners");

			auto accept_tcp = [acs_srv](const std::string& host, net::port p) -> bool {
				boost::system::error_code ec;
				auto addr = boost::asio::ip::address::from_string(host, ec);
				if (ec) return false;

				return  acs_srv->AddListener(addr, p, net::protocol::TCP) == boost::system::errc::success;
			};

			//endpoint for configurator
			if (!net::endpoint::ReadOrMakeAcceptTCP(1, "local", accept_tcp, "127.0.0.1", 4307))
				dprint0("\nTrying to accept to local endpoint is failed.\n");

			// initialise AppServerData
			{
				acs::Service::address_list listeners;
				bool NeedAddConnectTCP = n_listeners > 0
					&& net::endpoint::GetCountConnectTCP(g_tr_endpoint_name, false) == 0
					&& net::endpoint::GetCountAcceptTCP(g_tr_endpoint_name, false) == 0;
				if (NeedAddConnectTCP)
				{
					acs_srv->GetListenerList(listeners, net::protocol::TCP);
					for (const auto &addr : listeners)
					{
						net::endpoint::AddConnectTCP({addr.first.to_string().c_str(), addr.second, net::endpoint::protocol_tcp}, g_tr_endpoint_name, false);
					}
				}

				g_appServer->Set(g_tr_endpoint_name.c_str(), false, ST_VCS);		// add our server to list
				g_appServer->SetNetInfo(g_tr_endpoint_name.c_str(), 0);

				if (NeedAddConnectTCP)
					net::endpoint::ClearAllConnectTCP(g_tr_endpoint_name, false);

			}
			std::cout << "Started:  Access Connections System\n";
		}

		// Transport
		auto tr = vs::MakeShared<transport::Router>(atp.get_io_service(), endpoint_name);
		if (!tr)
			throw std::runtime_error("Failed to create transport::Router instance");

		VS_SCOPE_EXIT
		{
			// Stop Transport
			std::cout << "Stopping: Transport Router\n";
			if (tr->IsStarted())
				tr->Stop();
			std::cout << "Stopped:  Transport Router\n";
		};
		{
			tr->SrvPrivateKey().SetPrivateKey(private_key.c_str(), store_PEM_BUF);
			tr->SrvCertChain() = certificate_chain_cnt;
			tr->SetSrvCert(std::string(static_cast<char*>(certificate.get()), certificate_size));
			//tr->Start(acs_srv.get());
		}

		// Add Bwt handler
		auto bwt_handler = std::make_shared<bwt::Handler>();
		acs_srv->AddHandler("Bwt Handler", bwt_handler);

		// Stream Router
		auto sr = std::make_shared<stream::RouterV2>(atp.get_io_service(), endpoint_name);
		VS_SCOPE_EXIT
		{
			// Stop Stream Router
			std::cout << "Stopping: Stream Router\n";
			sr->Stop();
			std::cout << "Stopped:  Stream Router\n";
		};
		{
			std::cout << "Starting: Stream Router\n";
			sr->Start(acs_srv.get());
			std::cout << "Started:  Stream Router\n";
		}

		// it is restart_func, but we do not have restart service on linux from process itself
		// so just shutdown
		vs::event shutdown {true};
		watchdog->Init([&shutdown](const char* str, const bool b) -> void {
			shutdown.set();
		}, [&shutdown]() -> void {
			shutdown.set();
		});

		// Add services
		VCSServices_v2 services(atp.get_io_service());
		VS_SCOPE_EXIT{ services.Destroy(); };
		if (!services.Init(std::move(rp), tr, sr, acs_srv, watchdog.get(), STRPRODUCTVER))
			throw std::runtime_error("Services initialization failed");
		VS_ResolveServerFinder::SetIOService(atp.get_io_service());

		while (!shutdown.wait_for(std::chrono::seconds(10)))
			watchdog->Test();
		return 0;
	};

#ifndef _WIN32
	if (!opt_pid_file.empty())
	{
		int exit_code = EXIT_SUCCESS;
		g_daemon_body = start_server;
		pid_t daemon_pid = rundaemon(DMN_NO_CHDIR | DMN_NO_UMASK, [](void*) {
			try
			{
				return g_daemon_body();
			}
			catch (const std::exception& e)
			{
				error(e.what());
				return EXIT_FAILURE;
			}
		}, nullptr, &exit_code, opt_pid_file.c_str());
		switch (daemon_pid)
		{
		case -1:
			error("Cannot daemonize the process.");
			return EXIT_FAILURE;
			break;
		case -2:
			error("The daemon is already running.");
			return EXIT_FAILURE;
			break;
		case 0: // daemon process
			std::cerr << "Daemon has stopped with the exit code " << exit_code << "." << '\n';
			return exit_code;
			break;
		default: // parent process
			std::cerr << "Daemon's PID: " << daemon_pid << '\n';
			break;
		}
		return EXIT_SUCCESS;
	}
#endif

#ifdef _WIN32
	if (var_map.count("Service") > 0) // start as a Windows Service
	{
		int exit_status = EXIT_FAILURE;

		auto on_stop = [&watchdog](void) -> void {
			if (watchdog->IsInit())
				watchdog->Shutdown();
		};

		fclose(stdin);
		if (!vs::RunNTServiceDispatcher(exit_status, start_server, on_stop))
		{
			return EXIT_FAILURE;
		}

		return exit_status;
	}
#endif

	return start_server();
}
catch (const std::exception& e)
{
	error(e.what());
	return EXIT_FAILURE;
}


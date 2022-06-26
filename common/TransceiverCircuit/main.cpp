#include "VS_RelayProc.h"
#include "SecureLib/VS_CryptoInit.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/VS_PerformanceMonitor.h"
#include "std/cpplib/MakeShared.h"
#include "std-generic/clib/vs_time.h"
#include "tools/Server/VS_Server.h"
#include "std/debuglog/VS_Debug.h"
#include "TransceiverLib/VS_TransceiverAuthenticator.h"
#include "std/VS_TransceiverInfo.h"
#include "std/Globals.h"
#include "version.h"

#include <boost/filesystem/operations.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "std-generic/compat/iomanip.h"
#include <sstream>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include "net/DNSUtils/VS_DNSTools.h"

#if defined(_WIN32)
#include <process.h>
#define getpid _getpid
#else
#include <sys/types.h>
#endif

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

namespace po = boost::program_options;

template <class... Args>
static void error(Args&&... args)
{
	std::cerr << "error: ";
	(void)std::initializer_list<int>{((std::cerr << args), 0)...};
	std::cerr << '\n';
}

template <class... Args>
static void error(boost::system::error_code ec, Args&&... args)
{
	std::cerr << "error: ";
	(void)std::initializer_list<int>{((std::cerr << args), 0)...};
	if /*constexpr*/ (sizeof...(Args) > 1)
		std::cerr << ": ";
	std::cerr << ec.message() << '\n';
}

int main(int argc, char* argv[])
{
	std::string opt_config_file;
	po::options_description command_line_options("Command line only options", 1000);
	command_line_options.add_options()
		("help,h", "produce this help message")
		("ConfigFile,c", po::value<std::string>(&opt_config_file)->value_name("<path>"), (std::string("path to the configuration file, by default '") + ts::DEFAULT_CONFIG_FILE + "' will be used if it exists").c_str())
		("version,v", "prints the product version information")
		;

	std::string opt_registry_backend;
	std::string opt_name;
	std::string opt_server_address;
	std::string opt_server_endpoint;
	std::string opt_root;
	po::options_description required_options("Required options", 1000);
	required_options.add_options()
		(ts::NAME_OPTION_TAG, po::value<std::string>(&opt_name)->value_name("<string>"), "name of this transceiver")
		(ts::REGISTRY_BACKEND_OPTION_TAG, po::value<std::string>(&opt_registry_backend)->value_name("<string>"), "registry backend specification in the form NAME:[OPTIONS]")
		(ts::ROOT_OPTION_TAG, po::value<std::string>(&opt_root)->default_value(ts::DEFAULT_REG_KEY_ROOT)->value_name("<string>"), "registry key root")
		(ts::SERVER_ADDRESS_OPTION_TAG, po::value<std::string>(&opt_server_address)->value_name("<string>"), "address(es) of the server to connect to, format: IP:port[,IP:port]")
		(ts::SERVER_ENDPOINT_OPTION_TAG, po::value<std::string>(&opt_server_endpoint)->value_name("<string>"), "endpoint of the server, format: server.domain.name#vcs")
		;

	uint32_t opt_debug = 0;
	uint32_t opt_debug_modules = 0xffffffff;
	std::string opt_log_directory;
	po::options_description other_options("Other options", 1000);
	other_options.add_options()
		(ts::DEBUG_OPTION_TAG, po::value<uint32_t>(&opt_debug)->value_name("<level>"), "debug level: 1-4, 0=off")
		(ts::DEBUG_MODULES_OPTION_TAG, po::value<uint32_t>(&opt_debug_modules)->value_name("<bitmask>"), "bitmask specifying which modules to debug")
		(ts::LOG_DIRECTORY_OPTION_TAG, po::value<std::string>(&opt_log_directory)->value_name("<path>"), "directory for storing logs")
		;

	auto usage = [&]
	{
		std::cout << command_line_options;
		std::cout << required_options;
		std::cout << other_options;
	};

	po::variables_map var_map;
	try {
		po::options_description options;
		options.add(command_line_options);
		options.add(required_options);
		options.add(other_options);
		po::store(po::parse_command_line(
			argc, argv,
			options,
			po::command_line_style::case_insensitive | po::command_line_style::default_style
		), var_map);
		po::notify(var_map);
	}
	catch (const std::exception& e)
	{
		error("Option parsing error: ", e.what());
		usage();
		return EXIT_FAILURE;
	}

	if (var_map.count("version") > 0)
	{
		std::cout << "Product name: " << VS_PRODUCT_NAME << "; version: " << STRPRODUCTVER << '\n';
		return 0;
	}

	if (var_map.count("help") > 0)
	{
		usage();
		return 0;
	}

	if (!opt_config_file.empty())
	{
		std::ifstream config_file_stream;
		config_file_stream.open(!opt_config_file.empty() ? opt_config_file.c_str() : ts::DEFAULT_CONFIG_FILE);
		if (config_file_stream.is_open())
		{
			try {
				po::options_description options;
				options.add(required_options);
				options.add(other_options);
				po::store(po::parse_config_file(config_file_stream, options), var_map);
				po::notify(var_map);
			}
			catch (const std::exception& e)
			{
				error("Config parsing error: ", e.what());
				usage();
				return EXIT_FAILURE;
			}
		}
		else if (!opt_config_file.empty())
		{
			error("Can't open the configuration file: ", opt_config_file);
			return EXIT_FAILURE;
		}
	}

	auto check_required_option = [&](const char* name) {
		if (var_map.count(name) > 0)
			return true;
		error("Required option is missing: ", name);
		usage();
		return false;
	};
	if (!check_required_option(ts::NAME_OPTION_TAG)) return EXIT_FAILURE;
	if (!check_required_option(ts::REGISTRY_BACKEND_OPTION_TAG)) return EXIT_FAILURE;
	if (!check_required_option(ts::ROOT_OPTION_TAG)) return EXIT_FAILURE;
	if (!check_required_option(ts::SERVER_ADDRESS_OPTION_TAG)) return EXIT_FAILURE;
	if (!check_required_option(ts::SERVER_ENDPOINT_OPTION_TAG)) return EXIT_FAILURE;

	if (!opt_log_directory.empty())
		vs::SetLogDirectory(opt_log_directory);

	// Ensure path for logs exists
	const auto log_dir = vs::GetLogDirectory() + ts::LOG_DIRECTORY_NAME;
	boost::system::error_code ec;
	boost::filesystem::create_directories(log_dir, ec);
	if (ec)
	{
		error(ec, "Can't create directory '", log_dir, "'");
		return EXIT_FAILURE;
	}

	// Redirrect output to file
	auto now(std::time(0));
	tm now_tm;
	std::ostringstream log_name;
	log_name << log_dir << '/' << vs::put_time(localtime_r(&now, &now_tm), "%Y-%m-%d-%H-%M-%S") << '.' << getpid() << ".log";
	VS_RedirectOutput(log_name.str().c_str());

	vs::InitOpenSSL();

	if (!VS_RegistryKey::InitDefaultBackend(opt_registry_backend))
	{
		error("Can't initialize registry backend, check 'RegistryBackend' option value");
		return EXIT_FAILURE;
	}
	VS_RegistryKey::SetDefaultRoot(opt_root);

	net::dns_tools::init_loaded_options();

	if (var_map.count("Debug"))
	{
		// We don't want to write these options to the registry because it might affect the server.
		VS_SetDebug(std::min(opt_debug, 4u), opt_debug_modules);
	}
	else
		VS_ReadDebugKeys();

	auto trans_key = auth::Transceiver::ReadSharedKey(opt_name);
	if (trans_key.empty()) {
		dstream0 << "Error\t Can't find " << TRANSCEIVER_SHARED_KEY_TAG << ".\n";
		return -1;
	}

	VS_PerformanceMonitor::Instance().Start();

	dstream0 << "Starting transceiver with name='" << opt_name << "'\n";
	auto app = vs::MakeShared<VS_RelayProc>(opt_server_endpoint.c_str(), opt_server_address.c_str(), opt_name, (const unsigned char*)trans_key.c_str(), trans_key.length());
	app->Run();

	VS_PerformanceMonitor::Instance().Stop();

	return 0;
}

bool VS_Server::Start()                 { assert(false && "Call to server specific API"); return false; }
void VS_Server::Stop()                  { assert(false && "Call to server specific API"); }
const char* VS_Server::ShortName()      { assert(false && "Call to server specific API"); return nullptr; }
const char* VS_Server::LongName()       { assert(false && "Call to server specific API"); return nullptr; }
const char* VS_Server::RegistryKey()    { assert(false && "Call to server specific API"); return nullptr; }
const char* VS_Server::ServiceName()    { assert(false && "Call to server specific API"); return nullptr; }
const char* VS_Server::ProductVersion() { assert(false && "Call to server specific API"); return nullptr; }

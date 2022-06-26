
#include "../../Bwt/RunTest.h"
#include "../../Bwt/ContentHandler.h"
#include "TestBwtCmdline.h"
#include <boost/filesystem.hpp>
#include "std-generic/compat/iomanip.h"
#include <iostream>
#include "../../std/cpplib/VS_RegistryKey.h"
#include "std-generic/clib/vs_time.h"
#include "../../net/EndpointRegistry.h"

#include <VSClient/VS_ApplicationInfo.h>
#include <stdio.h>
#include <inttypes.h>

inline const char *StrOutIn(const unsigned type)
{
	return !type ? "outbound" : "inbound";
}

class Intermediate : public bwt::Intermediate
{
public:
	Intermediate( FILE *file_out, FILE *file_in)
		: file_out(file_out), file_in(file_in),error(false) {}
	~Intermediate()
	{
		if (out_bps > 0 || in_bps > 0)
		{
			puts("\n       Response(ms),  Jitters(min/max)(ms)");
			printf("Out.     %7" PRId64 ",        %7" PRId64 "/%-7" PRId64 "\n", out_response_ms, out_jitter_min_ms, out_jitter_max_ms);
			printf("In.      %7" PRId64 ",        %7" PRId64 "/%-7" PRId64 "\n", in_response_ms, in_jitter_min_ms, in_jitter_max_ms);
			puts("\nTest for connection was finished.");
		}
	}

	bool Result(const unsigned status, const void *inf, const unsigned mark)
	{
		bwt::Intermediate::Result(status, inf, mark);
		switch (status)
		{
		case VS_BWT_ST_INTER_RESULT:
		{
		if (out_bytes > 0 || in_bytes > 0)
		{
			printf("Out. Bytes: %.0f, Bps: %.0f.   In. Bytes: %.0f, Bps: %.0f.   \r", out_bytes, out_bps, in_bytes, in_bps);
		}
		return true;
		}
		case VS_BWT_ST_START_CONNECT:
			printf("Connection with type %s is starting.\n", StrOutIn(mark));
			return true;
		case VS_BWT_ST_CONNECT_ATTEMPT:
		{
			bwt::Endpoint   *endpoint = (bwt::Endpoint *)inf;
			printf("Attempt of connection number %u\n\tHost: %s, Port: %s, Protocol: %s.\n", mark, endpoint->host.c_str(), endpoint->port.c_str(), "Tcp");
			return true;
		}
		case VS_BWT_ST_CONNECT_OK:
			printf("Connection with type %s was established.\n", StrOutIn(mark));
			return true;
		case VS_BWT_ST_CONNECT_ERROR:
			printf("Connection with type %s was not established.\n", StrOutIn(mark));
			printf("%s", reinterpret_cast<const char*>(inf)); printf("\n");
			error = true;
			return true;
		case VS_BWT_ST_START_HANDSHAKE:
			printf("Handshake for %s connection is starting.\n", StrOutIn(mark));
			return true;
		case VS_BWT_ST_HANDSHAKE_OK:
			printf("Handshake for %s connection was successful.\n", StrOutIn(mark));
			return true;
		case VS_BWT_ST_HANDSHAKE_ERROR:
			puts("Handshake for connection was unsuccessful.");
			if (mark == 1)		puts("\tThe name of a server is incorrectly specified.");
			printf("%s", reinterpret_cast<const char*>(inf)); printf("\n");
			error = true;
			return true;
		case VS_BWT_ST_NO_RESOURCES:
			printf("Refused in access to resources for %s connection.\n", StrOutIn(mark));
			return true;
		case VS_BWT_ST_START_TEST:
			puts("Test for connection is starting.\n");
			return true;
		case VS_BWT_ST_FINISH_TEST:
			return true;
		case VS_BWT_ST_CONNECTION_DIED:
			if ( reinterpret_cast<const char*>(inf) != std::string("End of file"))
			{
				printf("Connection with type %s died.\n", StrOutIn(mark));
				printf("%s", reinterpret_cast<const char*>(inf)); printf("\n");
				error = true;
			}
			return true;
		case VS_BWT_ST_CONNECTION_ERROR:
			printf("Connection with type %s has broken.\n", StrOutIn(mark));
			printf("%s", reinterpret_cast<const char*>(inf)); printf("\n");
			error = true;
			return true;
		default:
			printf("Unknown status: %u\n", status);
			return true;
		}
		}

		void SaveToFile()
		{
			if (file_out)
				fprintf(file_out, "%" PRId64 ",%" PRId64 ",%.0f\n", loc_offset_ms, m_jitter_ms, out_bps);
			if (file_in)
				fprintf(file_in, "%" PRId64 ",%" PRId64 ",%.0f\n", loc_offset_ms, m_jitter_ms, in_bps);
		}

		FILE   *file_out, *file_in;
		bool error;
};// end Intermediate class


const char   *registry_roots[] = { "Online", "Client", "OnlineTwin", "ClientTwin" };

bool VS_ReadAS(char * server, bool Default)
{
	if (!server)
		return false;
	*server = 0;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	return key.GetValue(server, 255, VS_REG_STRING_VT, Default ? REG_DefaultServer : REG_Server) > 0;
}

bwt::Endpoint get_current_endpoint()
{
	char   str[512] = { 0 };
	if (!VS_ReadAS(str) || !*str)		return bwt::Endpoint();
	bwt::Endpoint endpoint;
	endpoint.endpoint = str;
	return endpoint;
}

inline bool set_root_registry(const char *registry)
{
	char   str[512] = { 0 };
	sprintf(str, "TrueConf\\%s", registry);
	VS_RegistryKey::SetDefaultRoot(str);
	VS_RegistryKey   key(true, "Current configuration");
	return key.IsValid();
}

bwt::Endpoint get_registry_endpoint(string_view endpoint_name)
{
	std::vector<bwt::Endpoint> endpoints;
	const uint32_t tcp_count = net::endpoint::GetCountConnectTCP(endpoint_name);
	for (uint32_t i = 1; i <= tcp_count; ++i)
	{
		auto tcp = net::endpoint::ReadConnectTCP(i, endpoint_name);
		endpoints.emplace_back(endpoint_name, tcp->host, std::to_string(tcp->port));
	}
	for (uint32_t i = 0; i < endpoints.size(); i++)
	{
		if (endpoints[i].port == "4307")
		{
			return endpoints[i];
		}
	}
	return !endpoints.empty() ? endpoints[0] : bwt::Endpoint();
}

FILE*  create_csv(bwt::Endpoint endpoint, unsigned test_mode, unsigned file_mode)
{
	if (!(test_mode & file_mode))
	{
		return nullptr;
	}
	std::string endpoint_str = endpoint.endpoint;
	std::replace_if(endpoint_str.begin(), endpoint_str.end(),
		[](char c)
	{
		return !(isalpha(c) || isdigit(c));
	}, '_');
	auto now = std::time(0);
	tm now_tm;
	std::stringstream ss;
	ss << endpoint_str << '/' << vs::put_time(localtime_r(&now, &now_tm), "%Y_%m_%d_%H_%M_%S") << (file_mode & VS_BWT_MODE_OUT ? "_OUT.csv" : "_IN.csv");
	boost::filesystem::create_directory(endpoint_str);
	FILE* file = fopen(ss.str().c_str(), "w");
	if (file)
	{
		fputs("Offset.Ms,Send.Ms,Bytes.Seconds\n", file);
	}
	return file;
}

int main( int argc, char *argv[] )
{
	try
	{
		FILE *file_out = NULL, *file_in = NULL;
		bwt::Endpoint endpoint;
		bwt::cmdline cmdline(VS_BWT_MODE_HALFDUPLEX, 5, VS_ACS_BWT_HEX_BUFFER_SIZE, 0);

		cmdline.parse(argc, argv);

		if (cmdline.get_help())
		{
			cmdline.usage();
			return -1;
		}

		const auto registry_backend = cmdline.get_registry_backend();
		if (registry_backend.empty())
		{
			std::cout << "Registry backend isn't specified!\n";
			cmdline.usage();
			return -1;
		}

		if (!VS_RegistryKey::InitDefaultBackend(registry_backend))
		{
			std::cout << "Can't initialize registry backend!\n";
			return -1;
		}

		if (!cmdline.unrecognized().empty())
		{
			endpoint.host = cmdline.unrecognized()[0];
			endpoint.endpoint = cmdline.get_target_endpoint();
			if (endpoint.endpoint.empty())
			{
				endpoint.endpoint = endpoint.host + "#as";
			}
			endpoint.port = cmdline.unrecognized().size() > 1 ? endpoint.port = cmdline.unrecognized()[1] : "80";
		}

		if (endpoint.host.empty())
		{
			endpoint.endpoint = cmdline.get_target_endpoint();
			for (size_t i = 0; i < (sizeof(registry_roots) / sizeof(char**)) && endpoint.endpoint.empty(); i++)
			{
				set_root_registry(registry_roots[i]);
				endpoint = get_current_endpoint();
			}
			endpoint = get_registry_endpoint(endpoint.endpoint);
		}

		if (cmdline.get_generate_files())
		{
			file_in = create_csv(endpoint, cmdline.get_test_mode(), VS_BWT_MODE_IN);
			file_out = create_csv(endpoint, cmdline.get_test_mode(), VS_BWT_MODE_OUT);
		}

		auto callback = std::make_shared<Intermediate>(file_out, file_in);
		bwt::RunTest(endpoint, callback, cmdline.get_test_mode(), cmdline.get_duration(), cmdline.get_frame_size(), cmdline.get_period());
	}
	catch (std::exception& e)
	{
		printf("Exception: %s\n", e.what());
	}
	return -1;
}


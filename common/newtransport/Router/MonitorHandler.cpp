#include "MonitorHandler.h"
#include "Monitor.h"
#include "Router.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"

#include <openssl/sha.h>

#define   VS_MONITOR_PRIMARY_FIELD   { '_','V','S','_','M','O','N','I','T','O','R','_',0 }
#define   VS_MONITOR_PRIMARY_FIELD_LENGTH 12

namespace transport {

const static char VS_Monitor_PrimaryField[VS_MONITOR_PRIMARY_FIELD_LENGTH+1] = VS_MONITOR_PRIMARY_FIELD;

struct MonitorHeader
{
#pragma pack(1)
	char			primary_field[VS_MONITOR_PRIMARY_FIELD_LENGTH+1];
	uint64_t		random_value;
	int64_t			valid_until;
	char			sha256[32];
	Monitor::TmRequest request;
#pragma pack()
};

bool CheckSignature(const MonitorHeader* monitor_header)
{
	VS_RegistryKey	key(false, CONFIGURATION_KEY);
	std::string secret_data;
	key.GetString(secret_data, "Secret Data");
	std::vector<char> buffer(VS_MONITOR_PRIMARY_FIELD_LENGTH + 1 + secret_data.size() + 1 + sizeof(uint64_t) + sizeof(int64_t));
	auto ptr = buffer.data();
	strncpy(buffer.data(), monitor_header->primary_field, VS_MONITOR_PRIMARY_FIELD_LENGTH);
	ptr += VS_MONITOR_PRIMARY_FIELD_LENGTH + 1;
	strncpy(ptr, secret_data.data(), secret_data.size());
	ptr += secret_data.size() + 1;
	memcpy(ptr, &monitor_header->random_value, sizeof(uint64_t));
	ptr += sizeof(uint64_t);
	memcpy(ptr, &monitor_header->valid_until, sizeof(int64_t));
	ptr += sizeof(int64_t);
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, buffer.data(), ptr-buffer.data());
	SHA256_Final(hash, &sha256);
	return memcmp(monitor_header->sha256, hash, SHA256_DIGEST_LENGTH)==0;
}

	MonitorHandler::MonitorHandler(Router* router) :m_router(router)
	{
	}

	acs::Response MonitorHandler::Protocol(const acs::Handler::stream_buffer& buffer, unsigned channel_token)
	{
		if (buffer.size() < sizeof(MonitorHeader))
		{
			return acs::Response::next_step;
		}
		const MonitorHeader*   monitor_header = reinterpret_cast<const MonitorHeader*>(buffer.data());
		if ((strncpy((char*)monitor_header->primary_field, VS_Monitor_PrimaryField, sizeof(monitor_header->primary_field))) ||
			(buffer.size() > sizeof(MonitorHeader)) ||
			(std::chrono::system_clock::from_time_t(monitor_header->valid_until) < std::chrono::system_clock::now()) ||
			!CheckSignature(monitor_header))
		{
			return acs::Response::not_my_connection;
		}
		return acs::Response::accept_connection;
	}

	void MonitorHandler::Accept(boost::asio::ip::tcp::socket&& sock, acs::Handler::stream_buffer&& buffer)
	{
		const MonitorHeader*   monitor_header = reinterpret_cast<const MonitorHeader*>(buffer.data());
		m_router->AddMonitorEndpoint(std::move(sock), monitor_header->request);
	}

}

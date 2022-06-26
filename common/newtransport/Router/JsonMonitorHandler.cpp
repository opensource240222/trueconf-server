#include "JsonMonitorHandler.h"
#include "Monitor.h"
#include "Router.h"
#include "../../net/BufferedConnection.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"
#include "../../std/cpplib/json/reader.h"
#include "../../std/cpplib/json/writer.h"
#include "../../std/debuglog/VS_Debug.h"

#include <openssl/sha.h>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT
#define GET_STRING_FROM_JSON(name,str_name,container, default_value) std::string name;	\
	do { json::Object::const_iterator it = container.Find( #str_name );		\
	if (it != container.End() ) name = (const json::String) it->element;	\
																else	name = default_value; \
					} while( 0 )
#define   VS_MONITOR_PRIMARY_FIELD   { '_','V','S','_','M','O','N','I','T','O','R','_',0 }
#define   VS_MONITOR_PRIMARY_FIELD_LENGTH 12

namespace transport {

const static char VS_Monitor_PrimaryField[VS_MONITOR_PRIMARY_FIELD_LENGTH + 1] = VS_MONITOR_PRIMARY_FIELD;

class JsonMonitorConnection : public net::BufferedConnection<>
{
	using base_t = net::BufferedConnection<>;
public:
	JsonMonitorConnection(boost::asio::io_service& ios)
		: base_t(ios)
	{
	}
	void Start(boost::asio::ip::tcp::socket&& socket, const std::weak_ptr<Router>& router)
	{
		json::Object obj;
		Monitor::TmReply tm_reply;
		auto r = router.lock();
		if (r)
			r->GetMonitorInfo(tm_reply);
		tm_reply.ToJson(obj);
		std::stringstream ss;
		json::Writer::Write(obj, ss);
		SetSocket(std::move(socket));
		auto ssstr = ss.str();// Don't call str() twice
		Send(reinterpret_cast<const uint8_t*>(ssstr.data()), ssstr.size());
		Shutdown();
	}
};

JsonMonitorHandler::JsonMonitorHandler(const std::weak_ptr<Router>& router): m_router(router)
{
}

acs::Response JsonMonitorHandler::Protocol(const acs::Handler::stream_buffer& buffer, unsigned channel_token)
{
	std::string buffer_str(buffer.data(), buffer.data() + buffer.size());
	size_t i = buffer_str.find_first_not_of(" /t");
	if (i != std::string::npos && buffer_str[i] != '{')
	{
		return acs::Response::not_my_connection;
	}
	int bracket_count = 0;
	for (size_t i = 0; i < buffer.size(); ++i)
	{		if (buffer[i] == '{')
			bracket_count++;
		else if (buffer[i] == '}')
			bracket_count--;
	}
	if (bracket_count < 0)
	{
		return acs::Response::not_my_connection;
	}
	else if (bracket_count>0)
	{
		return acs::Response::next_step;
	}
	else
	{
		return acs::Response::accept_connection;
	}
}

void Sign(const std::string& request_data, uint64_t rnd, int64_t valid_until, std::string& strhash)
{
	VS_RegistryKey	key(false, CONFIGURATION_KEY);
	std::string secret_data;
	key.GetString(secret_data, "Secret Data");
	std::vector<char> buffer;
	buffer.insert(buffer.end(), request_data.data(), request_data.data() + request_data.size());
	buffer.insert(buffer.end(), secret_data.begin(), secret_data.end());
	buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&rnd), reinterpret_cast<uint8_t*>(&rnd) + sizeof(rnd));
	buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&valid_until), reinterpret_cast<uint8_t*>(&valid_until) + sizeof(valid_until));
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, buffer.data(), buffer.size());
	SHA256_Final(hash, &sha256);
	strhash.resize(64);
	int i = 0;
	for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		sprintf(const_cast<char*>(strhash.data()) + (i * 2), "%02x", hash[i]);
	}
}

void JsonMonitorHandler::Accept(boost::asio::ip::tcp::socket&& socket, acs::Handler::stream_buffer&& buffer)
{
	json::Object obj;
	std::stringstream data(std::string(buffer.data(), buffer.data() + buffer.size()));
	json::Reader::Read(obj, data);
	GET_STRING_FROM_JSON(request_data, request_data, obj, "");
	GET_STRING_FROM_JSON(hash, hash, obj, "");
	GET_STRING_FROM_JSON(valid_until, valid_until, obj, "");
	GET_STRING_FROM_JSON(rnd, rnd, obj, "");
	GET_STRING_FROM_JSON(request, rnd, obj, "");
	std::string hash2;
	Sign(request_data, ::atoll(rnd.c_str()), ::atoll(valid_until.c_str()), hash2);
	if (hash != hash2)
	{
		dprint0("Wrong hash\n");
		socket.close();
		return;
	}
	std::make_shared<JsonMonitorConnection>(socket.get_io_service())->Start(std::move(socket), m_router);
}

}

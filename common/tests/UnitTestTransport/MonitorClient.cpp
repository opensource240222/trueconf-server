#include "MonitorClient.h"
#include "std/debuglog/VS_Debug.h"
#include "std/cpplib/json/reader.h"
#include "std/cpplib/json/writer.h"
#include <chrono>
#include "newtransport/Router/Monitor.h"

using namespace boost::asio;

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT
#define GET_STRING_FROM_JSON(name,str_name,container, default_value) std::string name;	\
	do { json::Object::const_iterator it = container.Find( #str_name );		\
	if (it != container.End() ) name = (const json::String) it->element;	\
																																																																else	name = default_value; \
							} while( 0 )
#define   VS_MONITOR_PRIMARY_FIELD   { '_','V','S','_','M','O','N','I','T','O','R','_',0 }
#define   VS_MONITOR_PRIMARY_FIELD_LENGTH 12

const static char VS_Monitor_PrimaryField[VS_MONITOR_PRIMARY_FIELD_LENGTH + 1] = VS_MONITOR_PRIMARY_FIELD;

namespace transport {
void Sign(const std::string& request_data, uint64_t rnd, int64_t valid_until, std::string& strhash);
}

MonitorClient::MonitorClient(boost::asio::io_service& io_serv) :m_io_serv(io_serv)
{
}

void MonitorClient::Start()
{
	ip::tcp::socket socket(m_io_serv);
	ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 4307);
	boost::system::error_code ec;
	socket.connect(ep, ec);
	if (ec)
	{
		dprint0("Failed to connect to 127.0.0.1 4307");
		return;
	}
	json::Object obj;
	uint64_t rnd = 0x128;
	obj["request_data"] = json::String(VS_Monitor_PrimaryField);
	obj["rnd"] = json::String(std::to_string(rnd));
	time_t valid_until=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + std::chrono::minutes(60));
	obj["valid_until"] = json::String(std::to_string(valid_until));
	obj["request"] = json::String(std::to_string(0));
	std::string hash2;
	transport::Sign(VS_Monitor_PrimaryField, rnd, valid_until, hash2);
	obj["hash"] = json::String(hash2);
	transport::Monitor::TmReply tm_reply;
	tm_reply.ToJson(obj);
	std::stringstream ss;
	json::Writer::Write(obj, ss);
	int res=socket.send(boost::asio::buffer(reinterpret_cast<const uint8_t*>(ss.str().c_str()), ss.str().length()));
	std::vector<uint8_t> buf(1024);
	res = socket.receive(boost::asio::buffer(buf));

}


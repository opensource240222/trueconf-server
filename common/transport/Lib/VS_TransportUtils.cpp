#if defined(_WIN32) // Not ported yet

#include "VS_TransportUtils.h"
#include "../../acs/Lib/VS_AcsLib.h"
#include "VS_TransportLib.h"
#include "../../acs/connection/VS_ConnectionTCP.h"
#include "std-generic/cpplib/deleters.h"
#include "std/cpplib/ThreadUtils.h"

#include <thread>
#include <memory>
#include <chrono>

bool VS_GetServerNameByAddress(const std::string& addr, const uint16_t port, const std::function<void(const std::string&, const ServerNameByAddressResult&)>&callback, const bool isAsync)
{

	if (isAsync)
	{
		std::thread th([addr, callback, port]()
		{
			vs::SetThreadName("GetServerName");
			VS_GetServerNameByAddress(addr, port, callback, false);
		}
			);
		th.detach();
		return true;
	}
	try
	{
		auto temp_epname = "you.are.who" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
		std::unique_ptr<net::HandshakeHeader, free_deleter> hs(VS_FormTransportHandshake(temp_epname.c_str(), "hwo.are.you", 1, true));
		if (hs == nullptr)
			throw(std::make_pair(std::string(""), ServerNameByAddressResult::err_unknow));

		auto mills = 5000ul;
		VS_ConnectionTCP conn;
		if (!conn.Connect(addr.c_str(),port, mills))
			throw(std::make_pair(std::string(""), ServerNameByAddressResult::err_connect_faild));

		if (!conn.CreateOvReadEvent() || !conn.CreateOvWriteEvent() ||
			!VS_WriteZeroHandshake(&conn, hs.get(), mills))
			throw(std::make_pair(std::string(""), ServerNameByAddressResult::err_write_faild));

		char *server_id = 0;
		char *user_id = 0;
		unsigned short  maxConnSilenceMs = 0;
		unsigned char	resultCode = hserr_antikyou, fatalSilenceCoef = 0, retHops = 0;
		bool tcpKeepAliveSupport(false);
		{
			net::HandshakeHeader* rzh = nullptr;
			if (!VS_ReadZeroHandshake(&conn, &rzh, mills))
			{
				hs.reset(rzh);
				throw(std::make_pair(std::string(""), ServerNameByAddressResult::err_read_faild));
			}
			hs.reset(rzh);
		}

		VS_TransformTransportReplyHandshake(hs.get(),
			resultCode, maxConnSilenceMs, fatalSilenceCoef,
			retHops, server_id,
			user_id, tcpKeepAliveSupport);
		if (!server_id)
			throw(std::make_pair(std::string(""), ServerNameByAddressResult::err_unknow));

		throw(std::make_pair(std::string(server_id), ServerNameByAddressResult::res_ok));
	}
	catch (const std::pair<std::string, ServerNameByAddressResult>& res)
	{
		callback(res.first, res.second);
		return true;
	}
	return false;

}

#endif

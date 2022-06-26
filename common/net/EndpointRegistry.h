#pragma once

#include "Port.h"
#include "std-generic/attributes.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/macro_utils.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace net { namespace endpoint {

extern const char protocol_tcp[];              // Clean TCP (O'K)
extern const char protocol_socks[];            // TCP through Socks Proxy (O'K)
extern const char protocol_http_tnl[];         // HTTP CONNECT through HTTP Proxy (O'K)
extern const char protocol_http_tnl_msq[];     // "HttpTnl" itself Target Visicron Server Application (O'K)
extern const char protocol_ssl[];              // Clean SSL (not yet...)
extern const char protocol_socks_ssl[];        // SSL through Socks Proxy (not yet...)
extern const char protocol_https[];            // SSL through HTTP Proxy (not yet...)
extern const char protocol_http[];             // IP over HTTP (not yet...)
extern const char protocol_internet_options[]; // Information From Windows Internet Options
extern const char protocol_nhp[];              // Nat hole punching
extern const char protocol_direct_udp[];       // DirectUdp
extern const char protocol_ras[];              // H225 Registration, Admission, Status
extern const char protocol_t120[];             // T.119 Data protocols for multimedia conferencing

bool CreateFromID(string_view id, bool user);

struct ConnectTCP
{
	std::string host;
	net::port port;
	std::string protocol_name;

	std::string socks_host;
	net::port socks_port;
	std::string socks_user;
	std::string socks_password;
	unsigned char socks_version;

	std::string http_host;
	net::port http_port;
	std::string http_user;
	std::string http_password;

	// Not serialized
	bool is_ipv6;

	VS_FORWARDING_DEF_CTOR13(ConnectTCP, host, port, protocol_name, socks_host, socks_port, socks_user, socks_password, socks_version, http_host, http_port, http_user, http_password, is_ipv6) {}

	bool Serialize(void* buffer, size_t& size) const;
	VS_NODISCARD std::vector<uint8_t> Serialize() const;
	bool Deserialize(const void* buffer, size_t size);
};
struct ConnectTCPFromIDResult
{
	VS_FORWARDING_CTOR2(ConnectTCPFromIDResult, res, connect_tcp){}
	bool res;
	ConnectTCP connect_tcp;
};
ConnectTCPFromIDResult GetConnectTCPFromID(string_view id);
struct AcceptTCP
{
	std::string host;
	net::port port;
	std::string protocol_name;

	VS_FORWARDING_DEF_CTOR3(AcceptTCP, host, port, protocol_name) {}

	bool Serialize(void* buffer, size_t& size) const;
	VS_NODISCARD std::vector<uint8_t> Serialize() const;
	bool Deserialize(const void* buffer, size_t size);
};

struct AcceptUDP
{
	std::string host;
	net::port port;
	std::string protocol_name;

	VS_FORWARDING_DEF_CTOR3(AcceptUDP, host, port, protocol_name) {}

	bool Serialize(void* buffer, size_t& size) const;
	VS_NODISCARD std::vector<uint8_t> Serialize() const;
	bool Deserialize(const void* buffer, size_t size);
};

VS_NODISCARD std::vector<uint8_t> Serialize(bool is_connect_keys, string_view endpoint_name, bool is_current_user = true);
bool Deserialize(bool is_connect_keys, const void* buffer, size_t size, string_view endpoint_name, bool is_current_user = true);
//bool Deserialize(const void* connect_buffer, size_t connect_size, const void* accept_buffer, size_t accept_size, string_view endpoint_name, bool is_current_user = true);

bool GetFromBuffer(unsigned index, ConnectTCP& c_tcp, const void* buffer, size_t size);
bool GetFromBuffer(unsigned index, AcceptTCP& a_tcp, const void* buffer, size_t size);
bool GetFromBuffer(unsigned index, AcceptUDP& a_udp, const void* buffer, size_t size);
VS_NODISCARD unsigned GetCountConnectTCPFromBuffer(const void* buffer, size_t size);
VS_NODISCARD unsigned GetCountAcceptTCPFromBuffer(const void* buffer, size_t size);
VS_NODISCARD unsigned GetCountAcceptUDPFromBuffer(const void* buffer, size_t size);

void Remove(string_view endpoint_name, bool is_current_user = true);

VS_DEPRECATED bool IsPermanent(string_view endpoint_name, bool is_current_user = true);
VS_DEPRECATED void SetAsPermanent(string_view endpoint_name, bool is_current_user = true);
VS_DEPRECATED void ResetAsPermanent(string_view endpoint_name, bool is_current_user = true);

VS_NODISCARD unsigned GetCountConnectTCP(string_view endpoint_name, bool is_current_user = true);
VS_NODISCARD std::unique_ptr<ConnectTCP> ReadConnectTCP(unsigned index, string_view endpoint_name, bool is_current_user = true);
void MakeFirstConnectTCP(unsigned index, string_view endpoint_name, bool is_current_user = true);
unsigned AddConnectTCP(const ConnectTCP& c_tcp, string_view endpoint_name, bool is_current_user = true);
VS_DEPRECATED void AddFirstConnectTCP(const ConnectTCP& c_tcp, string_view endpoint_name, bool is_current_user = true);
void DeleteConnectTCP(unsigned index, string_view endpoint_name, bool is_current_user = true);
bool UpdateConnectTCP(unsigned index, const ConnectTCP& c_tcp, string_view endpoint_name, bool is_current_user = true);
void ClearAllConnectTCP(string_view endpoint_name, bool is_current_user = true);

VS_NODISCARD unsigned GetCountAcceptTCP(string_view endpoint_name, bool is_current_user = true);
template<class Acceptor>
bool ReadOrMakeAcceptTCP(unsigned index, string_view endpoint_name, Acceptor&& acceptor, string_view defHost, net::port defPort, bool is_current_user = true);
VS_NODISCARD std::unique_ptr<AcceptTCP> ReadAcceptTCP(unsigned index, string_view endpoint_name, bool is_current_user = true);
void MakeFirstAcceptTCP(unsigned index, string_view endpoint_name, bool is_current_user = true);
unsigned AddAcceptTCP(const AcceptTCP& a_tcp, string_view endpoint_name, bool is_current_user = true);
VS_DEPRECATED void AddFirstAcceptTCP(const AcceptTCP& a_tcp, string_view endpoint_name, bool is_current_user = true);
void DeleteAcceptTCP(unsigned index, string_view endpoint_name, bool is_current_user = true);
bool UpdateAcceptTCP(unsigned index, const AcceptTCP& a_tcp, string_view endpoint_name, bool is_current_user = true);
void ClearAllAcceptTCP(string_view endpoint_name, bool is_current_user = true);

VS_NODISCARD unsigned GetCountAcceptUDP(string_view endpoint_name, bool is_current_user = true);
VS_NODISCARD std::unique_ptr<AcceptUDP> ReadAcceptUDP(unsigned index, string_view endpoint_name, bool is_current_user = true);
void MakeFirstAcceptUDP(unsigned index, string_view endpoint_name, bool is_current_user = true);
unsigned AddAcceptUDP(const AcceptUDP& a_udp, string_view endpoint_name, bool is_current_user = true);
VS_DEPRECATED void AddFirstAcceptUDP(const AcceptUDP& a_udp, string_view endpoint_name, bool is_current_user = true);
void DeleteAcceptUDP(unsigned index, string_view endpoint_name, bool is_current_user = true);
bool UpdateAcceptUDP(unsigned index, const AcceptUDP& a_udp, string_view endpoint_name, bool is_current_user = true);
void ClearAllAcceptUDP(string_view endpoint_name, bool is_current_user = true);

}

template<class Acceptor>
bool endpoint::ReadOrMakeAcceptTCP(unsigned index, string_view endpoint_name, Acceptor && acceptor, string_view defHost, net::port defPort, bool is_current_user)
{
	auto local_accept = net::endpoint::ReadAcceptTCP(index, endpoint_name);
	bool recreate_ep = !local_accept;
	std::string local_host = local_accept && !local_accept->host.empty() ? local_accept->host : static_cast<std::string>(defHost);
	net::port port = local_accept && local_accept->port > 0 ? local_accept->port : defPort;
	bool local_accept_res(false);
	while (!(local_accept_res = acceptor(local_host, port)) && port < UINT16_MAX)
	{
		++port;
		recreate_ep = true;
	}
	if (!local_accept_res)
	{
		net::endpoint::Remove(endpoint_name);
		return false;
	}
	else if (recreate_ep)
	{
		net::endpoint::Remove(endpoint_name);
		net::endpoint::AddAcceptTCP({ local_host, port, net::endpoint::protocol_tcp }, endpoint_name);
	}

	return true;
}
}
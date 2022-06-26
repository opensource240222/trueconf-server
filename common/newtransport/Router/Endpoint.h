#pragma once

#include "EndpointBase.h"
#include "Connection.h"
#include "MessageStats.h"

#include <boost/asio/steady_timer.hpp>

namespace transport {

class Router;
class Message;
class SecureProvider;

class Endpoint
	: public EndpointBase
	, public Connection<>
{
	using base_t = Connection<>;
public:
	using socket_type = base_t::socket_type;

	Endpoint(boost::asio::io_service& ios, std::shared_ptr<Router> router, string_view endpoint_id);
	~Endpoint();

	void SetConnection(socket_type&& socket, unsigned version, unsigned secure_handshake_version, uint8_t hops, bool tcp_keep_alive_support);
	void StartConnection();
	void Close() override;
	void Shutdown() override;

	void ProcessMessage(const Message& message) override;
	void SendToPeer(const Message& message) override;
	void SendPing() override;
	void SendDisconnect() override;

	std::string GetRemoteIp() override;

	void FillMonitorStruct(Monitor::TmReply::Endpoint& endpoint) override;

private:
	bool IsSecure() const;
	bool IsDisconnected() const;
	void Periodic();

	// Connection interface
	void OnHandshakeReply(HandshakeResult result, uint16_t max_conn_silence_ms, uint8_t fatal_silence_coef, uint8_t hops, const char* server_id, const char* client_id, bool tcp_keep_alive_support) override;
	void OnMessage(Message&& message) override;

	// BufferedConnection interface
	size_t OnReceive(const void* data, size_t size) override;
	void OnSend(size_t bytes_transferred) override;
	bool OnError(const boost::system::error_code& ec) override;
	void OnClose() override;
	string_view LogID() const override;

	boost::asio::steady_timer m_timer;
	std::unique_ptr<SecureProvider> m_sec_storage;
	unsigned m_version;
	unsigned m_secure_handshake_version;
	uint16_t m_max_conn_silence_ms;
	uint8_t m_fatal_silence_coef;
	bool m_tcp_keep_alive_support;
	std::chrono::steady_clock::time_point m_last_connect_time;
	std::chrono::steady_clock::time_point m_last_disconnect_time;
	std::chrono::steady_clock::time_point m_last_receive_time;
	std::chrono::steady_clock::time_point m_last_send_time;
	MessageStats m_recv_stats;
	MessageStats m_send_stats;

	struct LogPrefix;
};
}

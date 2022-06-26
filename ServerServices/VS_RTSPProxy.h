#pragma once

#include "acs_v2/Handler.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <vector>

namespace ts { struct IPool; }
class VS_RTSPProxySession;
class VS_RTSPReply;

class VS_RTSPProxy : public acs::Handler
{
public:
	explicit VS_RTSPProxy(std::shared_ptr<ts::IPool> transceivers_pool);

	// acs::Handler
	acs::Response Protocol(const stream_buffer& buffer, unsigned channel_token) override;
	void Accept(boost::asio::ip::tcp::socket&& socket, stream_buffer&& buffer) override;

	// Close all existing connections, reject all new connection
	void Close();

private:
	void CloseWithError(boost::asio::ip::tcp::socket&& socket, string_view reply);
	void Cleanup();

private:
	std::atomic<bool> m_running;
	std::shared_ptr<ts::IPool> m_transceivers_pool;
	std::vector<std::weak_ptr<VS_RTSPReply>> m_replies;
	std::vector<std::weak_ptr<VS_RTSPProxySession>> m_sessions;
	std::chrono::steady_clock::time_point m_last_cleanup_time;
};

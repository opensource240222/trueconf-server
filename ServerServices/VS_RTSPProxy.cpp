#include "ServerServices/VS_RTSPProxy.h"
#include "AppServer/Services/VS_Storage.h"
#include "ServerServices/VS_RTSPRequestParser.h"
#include "TransceiverLib/TransceiversPoolInterface.h"
#include "TransceiverLib/VS_TransceiverProxy.h"
#include "net/Address.h"
#include "net/Port.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/ignore.h"
#include "std/cpplib/MakeShared.h"
#include "std/cpplib/VS_Replace.h"
#include "std/VS_TransceiverInfo.h"
#include "std/debuglog/VS_Debug.h"

#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/asio/write.hpp>
#include <boost/regex.hpp>

#include "std-generic/compat/memory.h"
#include <algorithm>
#include <cassert>

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

#define RTSP_VERBOSE_LOGS 0

static const boost::regex rtsp_start_line_re(
	"[A-za-z]+" // Method
	" +"
	"([^ ]+)" // URL
	" +"
	"[Rr][Tt][Ss][Pp]/[0-9]\\.[0-9]" // RTSP version
	, boost::regex::optimize);

static const boost::regex rtsp_vs_url_re(
	"(?:[Rr][Tt][Ss][Pp]://[^/]*)?" // Optional scheme, host and port
	"/c/" // Group conf prefix
	"([^/? ]+)" // CID
	, boost::regex::optimize);

static string_view::size_type FindCRLF(string_view str)
{
	for (string_view::size_type pos = 0; pos + 1 < str.size(); )
	{
		if (str[pos+1] == '\n')
		{
			if (str[pos] == '\r')
				return pos;
			else
				pos += 2;
		}
		else
			pos += 1;
	}
	return string_view::npos;
}

#if RTSP_VERBOSE_LOGS
static std::string EscapeCRLF(string_view str)
{
	std::string result(str);
	VS_ReplaceAll(result, "\r", "\\r");
	VS_ReplaceAll(result, "\n", "\\n");
	return result;
}
#endif

class VS_RTSPReply : public vs::enable_shared_from_this<VS_RTSPReply>
{
public:
	void Start();
	void Close();

protected:
	VS_RTSPReply(boost::asio::ip::tcp::socket &&socket, string_view status);

private:
	boost::asio::ip::tcp::socket m_socket;
	std::string m_reply;
};

class VS_RTSPProxySession : public vs::enable_shared_from_this<VS_RTSPProxySession>
{
public:
	void Start(const net::address& transceiver_address, net::port transceiver_port);
	void Close();

protected:
	VS_RTSPProxySession(boost::asio::ip::tcp::socket &&in_socket, const void *in_data, size_t in_size, string_view secret);

private:
	void HandleInputData(size_t size);
	void ReadFromIn();
	void ReadFromOut();
	void WriteToIn(size_t size);
	void WriteToOut();
	static std::string ConstructSecretHeader(string_view secret);

private:
	struct ConnectionInfo
	{
		ConnectionInfo(boost::asio::ip::tcp::socket&& socket_, const void* data, size_t size); // Used for incoming connection
		explicit ConnectionInfo(boost::asio::io_service& ios); // Used for outgoing connection

		boost::asio::mutable_buffers_1 PrepareReadBuffer(size_t max_size);

		boost::asio::ip::tcp::socket socket;
		size_t read_buffer_size;
		std::unique_ptr<unsigned char[]> read_buffer;
	};

	ConnectionInfo m_in;
	ConnectionInfo m_out;
	std::string m_log_id;
	const std::string m_secret_header;

	VS_RTSPRequestParser m_parser;
	unsigned m_is_setup_request : 1;
	unsigned m_is_trueconf_secret_header : 1;
	unsigned m_is_transport_header : 1;
	std::vector<std::string> m_transport_headers;
	std::vector<boost::asio::const_buffer> m_write_chunks;
};

VS_RTSPProxy::VS_RTSPProxy(std::shared_ptr<ts::IPool> transceivers_pool)
	: m_running(true)
	, m_transceivers_pool(std::move(transceivers_pool))
{
}

acs::Response VS_RTSPProxy::Protocol(const stream_buffer& buffer, unsigned channel_token)
{
	static constexpr size_t c_max_start_line_length = 1024;

	string_view start_line(reinterpret_cast<const char*>(buffer.data()), std::min(buffer.size(), c_max_start_line_length));
	const auto crlf_pos = FindCRLF(start_line);
	if (crlf_pos == string_view::npos)
	{
		if (buffer.size() < c_max_start_line_length)
			return acs::Response::next_step;
		else
		{
#if RTSP_VERBOSE_LOGS
			dstream4 << "RTSPProxy: Rejecting connection: no CRLF in first " << c_max_start_line_length << " bytes";
#endif
			return acs::Response::not_my_connection;
		}
	}

	start_line = start_line.substr(0, crlf_pos);
	if (boost::regex_match(start_line.begin(), start_line.end(), rtsp_start_line_re))
		return acs::Response::accept_connection;
	else
	{
#if RTSP_VERBOSE_LOGS
		dstream4 << "RTSPProxy: Rejecting connection: invalid start line: " << start_line;
#endif
		return acs::Response::not_my_connection;
	}
}

void VS_RTSPProxy::Accept(boost::asio::ip::tcp::socket&& socket, stream_buffer&& buffer)
{
	if (!m_running.load(std::memory_order_acquire))
	{
		// Don't send a reply because we are stopping and don't wait for a write to complete.
		socket.close();
		return;
	}

	Cleanup();

	string_view start_line(reinterpret_cast<const char*>(buffer.data()), buffer.size());
	const auto crlf_pos = FindCRLF(start_line);
	assert(crlf_pos != string_view::npos);

	start_line = start_line.substr(0, crlf_pos);
	boost::match_results<string_view::iterator> sl_match;
	boost::regex_match(start_line.begin(), start_line.end(), sl_match, rtsp_start_line_re);
	assert(!sl_match.empty());
	assert(sl_match.length(1) > 0);

	boost::match_results<string_view::iterator> url_match;
	if (!boost::regex_search(sl_match[1].first, sl_match[1].second, url_match, rtsp_vs_url_re, boost::match_continuous))
		return CloseWithError(std::move(socket), "404 Not Found: missing prefix");
	assert(!url_match.empty());

	std::string cid = url_match.str(1);

	VS_ConferenceDescription cd;
	if (g_storage->FindConferenceByCallID(cid, cd) != 0)
		return CloseWithError(std::move(socket), "404 Not Found");

	if (!cd.m_isBroadcastEnabled)
		return CloseWithError(std::move(socket), "403 Forbidden: broadcast is disabled");

	const auto tr_proxy = m_transceivers_pool->GetTransceiverProxy(cd.m_name.m_str);
	if (!tr_proxy)
		return CloseWithError(std::move(socket), "500 Internal Server Error: no transceiver");

	const auto address = tr_proxy->GetRemoteAddress();
	if (address.is_unspecified())
		return CloseWithError(std::move(socket), "500 Internal Server Error: no address");

	const auto port = tr_proxy->GetLive555Port();
	if (port == 0)
		return CloseWithError(std::move(socket), "500 Internal Server Error: no port");

	auto secret = tr_proxy->GetLive555Secret();
	if (secret.empty())
		return CloseWithError(std::move(socket), "500 Internal Server Error: no secret key");

	auto session = vs::MakeShared<VS_RTSPProxySession>(std::move(socket), buffer.data(), buffer.size(), std::move(secret));
	m_sessions.emplace_back(session);
	session->Start(address, port);
}

void VS_RTSPProxy::Close()
{
	m_running.store(false, std::memory_order_release);
	for (const auto& p : m_replies)
		if (auto reply = p.lock())
			reply->Close();
	for (const auto& p : m_sessions)
		if (auto session = p.lock())
			session->Close();
}

void VS_RTSPProxy::CloseWithError(boost::asio::ip::tcp::socket&& socket, string_view status)
{
	dstream2 << "RTSPProxy: Sending error \"" << status << "\" to " << socket.remote_endpoint(vs::ignore<boost::system::error_code>());

	auto reply = vs::MakeShared<VS_RTSPReply>(std::move(socket), status);
	m_replies.emplace_back(reply);
	reply->Start();
}

void VS_RTSPProxy::Cleanup()
{
	const auto now = std::chrono::steady_clock::now();
	if (now - m_last_cleanup_time > std::chrono::seconds(60))
	{
		m_last_cleanup_time = now;
		m_sessions.erase(std::remove_if(m_sessions.begin(), m_sessions.end(), [](const std::weak_ptr<VS_RTSPProxySession>& p) { return p.expired(); }), m_sessions.end());
		m_replies.erase(std::remove_if(m_replies.begin(), m_replies.end(), [](const std::weak_ptr<VS_RTSPReply>& p) { return p.expired(); }), m_replies.end());
	}
}

VS_RTSPReply::VS_RTSPReply(boost::asio::ip::tcp::socket&& socket, string_view status)
	: m_socket(std::move(socket))
{
	m_socket.shutdown(boost::asio::socket_base::shutdown_receive, vs::ignore<boost::system::error_code>());

	m_reply.reserve(128);
	m_reply += "RTSP/1.0 ";
	m_reply += status;
	m_reply += "\r\n\r\n";
}

void VS_RTSPReply::Start()
{
	boost::asio::async_write(m_socket, boost::asio::buffer(m_reply.data(), m_reply.size()), [this, self = shared_from_this()](const boost::system::error_code& ec, size_t /*bytes_transferred*/)
	{
		if (ec == boost::asio::error::operation_aborted)
			return;

#if RTSP_VERBOSE_LOGS
		if (ec)
			dstream2 << "RTSPProxy: Failed to send error reply to " << m_socket.remote_endpoint(vs::ignore<boost::system::error_code>()) << ": (" << ec.value() << ") " << ec.message();
		else
			dstream4 << "RTSPProxy: Sent error reply to " << m_socket.remote_endpoint(vs::ignore<boost::system::error_code>());
#endif

		m_socket.close(vs::ignore<boost::system::error_code>());
	});
}

void VS_RTSPReply::Close()
{
	m_socket.close();
}

VS_RTSPProxySession::ConnectionInfo::ConnectionInfo(boost::asio::ip::tcp::socket&& socket_, const void* data, size_t size)
	: socket(std::move(socket_))
	, read_buffer_size(size)
	, read_buffer(vs::make_unique_default_init<unsigned char[]>(read_buffer_size))
{
	std::memcpy(read_buffer.get(), data, size);
}

VS_RTSPProxySession::ConnectionInfo::ConnectionInfo(boost::asio::io_service& ios)
	: socket(ios)
	, read_buffer_size(1024)
	, read_buffer(vs::make_unique_default_init<unsigned char[]>(read_buffer_size))
{
}

boost::asio::mutable_buffers_1 VS_RTSPProxySession::ConnectionInfo::PrepareReadBuffer(size_t max_size)
{
	if (read_buffer_size < max_size)
	{
		const auto available = socket.available(vs::ignore<boost::system::error_code>{});
		if (available > read_buffer_size)
		{
			read_buffer_size = std::min(max_size, std::max(available, read_buffer_size * 3 / 2));
			read_buffer = vs::make_unique_default_init<unsigned char[]>(read_buffer_size);
		}
	}
	return boost::asio::mutable_buffers_1(read_buffer.get(), read_buffer_size);
}

VS_RTSPProxySession::VS_RTSPProxySession(boost::asio::ip::tcp::socket&& in_socket, const void* in_data, size_t in_size, string_view secret)
	: m_in(std::move(in_socket), in_data, in_size)
	, m_out(m_in.socket.get_io_service())
	, m_secret_header(ConstructSecretHeader(secret))
	, m_is_setup_request(0)
	, m_is_trueconf_secret_header(0)
	, m_is_transport_header(0)
{
}

void VS_RTSPProxySession::Start(const net::address& transceiver_address, net::port transceiver_port)
{
	m_out.socket.async_connect(boost::asio::ip::tcp::endpoint(transceiver_address, transceiver_port), [this, self = shared_from_this()](const boost::system::error_code& ec)
	{
		if (ec == boost::asio::error::operation_aborted)
			return;

		if (ec)
		{
			dstream2 << "RTSPProxy: connect failed: (" << ec.value() << ") " << ec.message();
			Close();
		}

		std::ostringstream ss;
		ss << "RTSPProxy(";

		const auto in_ep = m_in.socket.remote_endpoint(vs::ignore<boost::system::error_code>());
		if (in_ep.protocol() == boost::asio::ip::tcp::v4())
			ss << in_ep.address().to_string(vs::ignore<boost::system::error_code>());
		else
			ss << '[' << in_ep.address().to_string(vs::ignore<boost::system::error_code>()) << ']';
		ss << ':' << in_ep.port();

		ss << "<->";

		const auto out_ep = m_out.socket.remote_endpoint(vs::ignore<boost::system::error_code>());
		if (out_ep.protocol() == boost::asio::ip::tcp::v4())
			ss << out_ep.address().to_string(vs::ignore<boost::system::error_code>());
		else
			ss << '[' << out_ep.address().to_string(vs::ignore<boost::system::error_code>()) << ']';
		ss << ':' << out_ep.port();

		ss << "): ";
		m_log_id = ss.str();

		dstream4 << m_log_id << "new connection";

		// Note: At the start the read buffer of the incoming connection has exactly the same size as the initial data.
		HandleInputData(m_in.read_buffer_size);
		ReadFromOut();
	});
}

void VS_RTSPProxySession::Close()
{
#if RTSP_VERBOSE_LOGS
	dstream4 << m_log_id << "close";
#endif
	m_in.socket.shutdown(boost::asio::socket_base::shutdown_both, vs::ignore<boost::system::error_code>());
	m_out.socket.shutdown(boost::asio::socket_base::shutdown_both, vs::ignore<boost::system::error_code>());
}

void VS_RTSPProxySession::HandleInputData(size_t size)
{
	m_transport_headers.clear();
	m_write_chunks.clear();

	// If 'part' refers to memory that immediately follows the last chunk then extends the last chunk appropriately.
	// Otherwise adds 'part' as a new chunk.
	auto append = [&](string_view part) {
		if (m_write_chunks.empty())
		{
			m_write_chunks.emplace_back(part.data(), part.size());
			return;
		}
		auto& last = m_write_chunks.back();
		const auto last_chunk_end = boost::asio::buffer_cast<const char*>(last) + boost::asio::buffer_size(last);
		const auto new_chunk_begin = part.data();
		if (last_chunk_end != new_chunk_begin)
		{
			m_write_chunks.emplace_back(part.data(), part.size());
			return;
		}

		last = boost::asio::const_buffer(boost::asio::buffer_cast<const char*>(last), boost::asio::buffer_size(last) + part.size());
	};

	const string_view data(reinterpret_cast<const char*>(m_in.read_buffer.get()), size);
#if RTSP_VERBOSE_LOGS
	dstream4 << m_log_id << "in: parsing data:\n\t" << EscapeCRLF(data);
#endif
	const bool success = m_parser.Parse(data,
		[&](string_view method) {
			append(method);
			if (method == "SETUP")
			{
				m_is_setup_request = 1;
				return VS_RTSPRequestParser::Action::skip_line;
			}
			else
				m_is_setup_request = 0;
			return VS_RTSPRequestParser::Action::skip;
		},
		[&](string_view /*uri*/) { assert(false); return VS_RTSPRequestParser::Action::skip_line; },
		[&](string_view /*version*/) { assert(false); return VS_RTSPRequestParser::Action::skip_line; },
		[&](string_view name) {
			assert(m_is_setup_request);

			// Filter out existing secret headers
			if (strncasecmp(name.data(), ts::RTSP_SECRET_HEADER, name.size()) == 0)
			{
				m_is_trueconf_secret_header = 1;
				return VS_RTSPRequestParser::Action::skip;
			}
			else
				m_is_trueconf_secret_header = 0;

			append(name);
			if (strncasecmp(name.data(), "Transport", name.size()) == 0)
			{
				m_is_transport_header = 1;
				return VS_RTSPRequestParser::Action::parse;
			}
			else
				m_is_transport_header = 0;
			return VS_RTSPRequestParser::Action::skip;
		},
		[&](string_view value) {
			assert(m_is_transport_header);

			const string_view destination_param("destination=");
			const auto destination_value = m_in.socket.remote_endpoint(vs::ignore<boost::system::error_code>()).address().to_string(vs::ignore<boost::system::error_code>());

			m_transport_headers.emplace_back();
			auto& buffer = m_transport_headers.back();
			buffer.reserve(value.size() + destination_param.size() + destination_value.size() + 2);

			typedef boost::algorithm::split_iterator<string_view::const_iterator> split_iterator;
			split_iterator spec_it(value, boost::algorithm::token_finder([](char x) { return x == ','; }));
			while (true)
			{
				const string_view spec(&*spec_it->begin(), spec_it->end() - spec_it->begin());
				split_iterator param_it(spec, boost::algorithm::token_finder([](char x) { return x == ';'; }));
				while (true)
				{
					const string_view param(&*param_it->begin(), param_it->end() - param_it->begin());

					// Filter out existing destination= parameters
					if (param.substr(0, destination_param.size()) != destination_param)
						buffer += param;

					++param_it;
					if (param_it.eof())
						break;
					buffer += ';';
				}

				if (!destination_value.empty())
				{
					buffer += ';';
					buffer += destination_param;
					buffer += destination_value;
				}

				++spec_it;
				if (spec_it.eof())
					break;
				buffer += ',';
			}
			append(buffer);
		},
		[&]() {
			if (m_is_setup_request)
				append(m_secret_header);
		},
		[&](string_view data) {
			// Filter out existing secret headers
			if (m_is_trueconf_secret_header)
				return;
			append(data);
		},
		[&](string_view::size_type pos) {
			dstream4 << m_log_id << "in: parsing failed at position " << pos << " in:\n" << data;
		}
	);

	if (!success)
		Close();
	else if (m_write_chunks.empty())
		ReadFromIn();
	else
		WriteToOut();
}

void VS_RTSPProxySession::ReadFromIn()
{
	const auto buffer = m_in.PrepareReadBuffer(4096);
#if RTSP_VERBOSE_LOGS
	dstream4 << m_log_id << "in: read start: size=" << boost::asio::buffer_size(buffer);
#endif
	m_in.socket.async_receive(buffer, [this, self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (ec == boost::asio::error::operation_aborted)
			return;

		if (ec)
		{
			if (ec != boost::asio::error::eof && ec != boost::asio::error::shut_down)
				dstream2 << m_log_id << "in: read failed: (" << ec.value() << ") " << ec.message();
			Close();
			return;
		}

		HandleInputData(bytes_transferred);
	});
}

void VS_RTSPProxySession::ReadFromOut()
{
	const auto buffer = m_out.PrepareReadBuffer(0x10000);
#if RTSP_VERBOSE_LOGS
	dstream4 << m_log_id << "out: read start: size=" << boost::asio::buffer_size(buffer);
#endif
	m_out.socket.async_receive(buffer, [this, self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (ec == boost::asio::error::operation_aborted)
			return;

		if (ec)
		{
			if (ec != boost::asio::error::eof && ec != boost::asio::error::shut_down)
				dstream2 << m_log_id << "out: read failed: (" << ec.value() << ") " << ec.message();
			Close();
			return;
		}

		WriteToIn(bytes_transferred);
	});
}

void VS_RTSPProxySession::WriteToIn(size_t size)
{
	const auto buffer = boost::asio::buffer(m_out.read_buffer.get(), size);
#if RTSP_VERBOSE_LOGS
	dstream4 << m_log_id << "in: write start: size=" << boost::asio::buffer_size(buffer);
#endif
	boost::asio::async_write(m_in.socket, buffer, [this, self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (ec == boost::asio::error::operation_aborted)
			return;

		if (ec)
		{
			if (ec != boost::asio::error::eof && ec != boost::asio::error::shut_down)
				dstream2 << m_log_id << "in: write failed: (" << ec.value() << ") " << ec.message();
			Close();
			return;
		}

		ReadFromOut();
	});
}

void VS_RTSPProxySession::WriteToOut()
{
	const auto& buffer = m_write_chunks; // TODO: Use span
#if RTSP_VERBOSE_LOGS
	auto ds = dstream4;
	ds << m_log_id << "out: write start: size=" << boost::asio::buffer_size(buffer) << ", parts:\n";
	for (const auto& part : m_write_chunks)
		ds << '\t' << EscapeCRLF(string_view(boost::asio::buffer_cast<const char*>(part), boost::asio::buffer_size(part))) << '\n';
#endif
	boost::asio::async_write(m_out.socket, buffer, [this, self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (ec == boost::asio::error::operation_aborted)
			return;

		if (ec)
		{
			if (ec != boost::asio::error::eof && ec != boost::asio::error::shut_down)
				dstream2 << m_log_id << "out: write failed: (" << ec.value() << ") " << ec.message();
			Close();
			return;
		}

		ReadFromIn();
	});
}

std::string VS_RTSPProxySession::ConstructSecretHeader(string_view secret)
{
	std::string result;
	result.reserve(strlen(ts::RTSP_SECRET_HEADER) + 2/* ": " */ + secret.size() + 2/* "\r\n" */);
	result += ts::RTSP_SECRET_HEADER;
	result += ": ";
	result += secret;
	result += "\r\n";
	return result;
}

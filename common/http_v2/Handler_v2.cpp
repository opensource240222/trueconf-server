#include "http_v2/Handler_v2.h"
#include "std-generic/cpplib/ignore.h"
#include <boost/asio/write.hpp>

namespace http_v2 {

void ACSHandler::SetHttpRouter(const std::weak_ptr<http::Router>& http_router)
{
	m_http_router = http_router;
}

static acs::Response ConvertResult(http::Router::FindHandlerResult result)
{
	if (result == http::Router::FindHandlerResult::accept)
		return acs::Response::accept_connection;
	else if (result == http::Router::FindHandlerResult::not_my)
		return acs::Response::not_my_connection;
	else if (result == http::Router::FindHandlerResult::need_more)
		return acs::Response::next_step;
	else
		return acs::Response::not_my_connection;
}

acs::Response ACSHandler::Protocol(const acs::Handler::stream_buffer& buffer, unsigned /*channel_token*/)
{
	assert(!buffer.empty());
	auto r = m_http_router.lock();
	if (!r)
		return acs::Response::not_my_connection;
	if (!r->CanProcessRequest())
		return acs::Response::not_my_connection;
	auto res = r->FindHandler(string_view(reinterpret_cast<const char*>(buffer.data()), buffer.size()), vs::ignore<std::shared_ptr<http::handlers::Interface>>());
	return ConvertResult(res);
}

void ACSHandler::Accept(boost::asio::ip::tcp::socket&& sock, acs::Handler::stream_buffer&& buffer)
{
	assert(!buffer.empty());
	auto r = m_http_router.lock();
	if (!r)
		return;
	std::shared_ptr<http::handlers::Interface> ptr;
	r->FindHandler(string_view(reinterpret_cast<const char*>(buffer.data()), buffer.size()), ptr);
	if (!ptr)
		return;
	if (!r->BeginProcessRequest())
		return;
	
	struct Handler final
	{
		typedef boost::asio::ip::tcp::socket socket_t;

		Handler(socket_t &&aSocket) noexcept : socket(std::move(aSocket)) {}
		socket_t socket;
		std::string result;
	};

	std::string tmp(reinterpret_cast<const char*>(buffer.data()), buffer.size());
	m_strand.post([r, ptr, handler = std::make_shared<Handler>(std::move(sock)), in_buff = std::move(tmp)]() mutable {
		auto endp = handler->socket.remote_endpoint(vs::ignore<boost::system::error_code>());
		auto out = ptr->HandleRequest(in_buff, endp.address(), endp.port());
		if (out.is_initialized()) {
			handler->result = std::move(out.get());
			const auto handler_raw = handler.get();
			boost::asio::async_write(handler_raw->socket, boost::asio::buffer(handler_raw->result),
				[handler = std::move(handler), r](const boost::system::error_code& /*err*/, std::size_t /*bytes_transfered*/) noexcept
			{
				// todo(kt): check bytes_transfered == saved_buffer.size() ?
				r->EndProcessRequest();
				handler->socket.shutdown(boost::asio::socket_base::shutdown_both, vs::ignore<boost::system::error_code>());
			});
		}
		else
			r->EndProcessRequest();
	});
}

} //namespace http

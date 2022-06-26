#ifdef _WIN32
#include "VS_HttpHandler_v2.h"
#include "std-generic/cpplib/ignore.h"
#include "std-generic/cpplib/scope_exit.h"

#include <cassert>

VS_ACS_Response VS_HttpHandler_v2::Connection(unsigned long *in_len)
{
	if (in_len)
		*in_len = 1024;
	return VS_ACS_Response::vs_acs_next_step;
}

static VS_ACS_Response ConvertResult(http::Router::FindHandlerResult result)
{
	if (result == http::Router::FindHandlerResult::accept)
		return VS_ACS_Response::vs_acs_accept_connections;
	else if (result == http::Router::FindHandlerResult::not_my)
		return VS_ACS_Response::vs_acs_connection_is_not_my;
	else if (result == http::Router::FindHandlerResult::need_more)
		return VS_ACS_Response::vs_acs_next_step;
	assert(false);
	return vs_acs_connection_is_not_my;
}

VS_ACS_Response VS_HttpHandler_v2::Protocol(const void *in_buffer, unsigned long *in_len,
	void **out_buffer, unsigned long *out_len,
	void **context)
{
	if (!in_buffer || !in_len)
		return VS_ACS_Response::vs_acs_connection_is_not_my;
	auto r = m_http_router.lock();
	if (!r)
		return VS_ACS_Response::vs_acs_connection_is_not_my;
	if (!r->CanProcessRequest())
		return VS_ACS_Response::vs_acs_connection_is_not_my;
	auto res = r->FindHandler(string_view(reinterpret_cast<const char*>(in_buffer), *in_len), vs::ignore<std::shared_ptr<http::handlers::Interface>>());
	auto out_res = ConvertResult(res);
	if (out_res == VS_ACS_Response::vs_acs_next_step && (++*in_len > 64000))
		out_res = VS_ACS_Response::vs_acs_connection_is_not_my;
	return out_res;
}

void VS_HttpHandler_v2::Accept(VS_ConnectionTCP *conn, const void *in_buffer,
	const unsigned long in_len, const void *context)
{
	auto r = m_http_router.lock();
	if (!r)
		return;
	if (!in_buffer || !in_len)
		return;
	std::shared_ptr<http::handlers::Interface> ptr;
	auto res = r->FindHandler(string_view(reinterpret_cast<const char*>(in_buffer), in_len), ptr);
	if (!ptr)
		return;
	if (!r->BeginProcessRequest())
		return;
	std::string tmp(reinterpret_cast<const char*>(in_buffer), in_len);
	m_strand.post([r, ptr, conn, in_buff = std::move(tmp)]() {
		VS_SCOPE_EXIT{ r->EndProcessRequest(); };
		auto out = ptr->HandleRequest(in_buff,
			net::address::from_string(conn->GetPeerAddress().GetHostByIp(), vs::ignore<boost::system::error_code>()),
			conn->GetPeerAddress().port());
		if (out)
		{
			unsigned long mills(1000);
			conn->Send(out->c_str(), out->length(), mills);
		}
		conn->Close();
		delete conn;
	});
}

#endif // _WIN32

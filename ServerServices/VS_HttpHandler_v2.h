#ifdef _WIN32
#pragma once

#include "acs/AccessConnectionSystem/VS_AccessConnectionHandler.h"
#include "acs/AccessConnectionSystem/VS_AccessConnectionHandlerTypes.h"
#include "http/Router.h"

#include "boost/asio/io_service.hpp"
#include <boost/asio/strand.hpp>

class VS_HttpHandler_v2 : public VS_AccessConnectionHandler
{
	boost::asio::io_service::strand m_strand;
	std::string m_handlerName;
	std::weak_ptr<http::Router> m_http_router;
public:
	VS_HttpHandler_v2(boost::asio::io_service& ios) : m_strand(ios)
	{}
	bool IsValid(void) const override { return true; }
	bool Init(const char *handler_name) override
	{
		if (!handler_name)
			return false;
		m_handlerName = handler_name;
		return true;
	}
	VS_ACS_Response	Connection(unsigned long *in_len) override;
	VS_ACS_Response Protocol(const void *in_buffer, unsigned long *in_len,
		void **out_buffer, unsigned long *out_len,
		void **context) override;
	void Accept(VS_ConnectionTCP *conn, const void *in_buffer,
		const unsigned long in_len, const void *context) override;
	void Destructor(const void *context) override
	{}
	void Destroy(const char *handler_name) override
	{}
	char* HandlerName(void) const override {
		return const_cast<char*>(m_handlerName.c_str());
	}

	void SetHttpRouter(const std::weak_ptr<http::Router>& http_router)
	{
		m_http_router = http_router;
	}
};

#endif	// _WIN32

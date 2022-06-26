#pragma once
#include "http/handlers/Interface.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/compat/map.h"
#include "std-generic/compat/functional.h"

#include <atomic>

namespace http {

static const uint32_t MAX_HTTP_CONCURRENT_PROCESSING_REQUEST = 100;

class Router
{
	std::atomic<uint32_t> m_concurrent_processing_requests{ 0 };
	vs::map<std::string, std::shared_ptr<http::handlers::Interface>, vs::str_less> m_request_handlers;
public:
	void AddHandler(string_view key, const std::shared_ptr<http::handlers::Interface>& handler);

	enum class FindHandlerResult : uint8_t
	{
		accept,
		not_my,
		need_more
	};
	FindHandlerResult FindHandler(string_view buffer_sv, std::shared_ptr<http::handlers::Interface>& handler);

	bool CanProcessRequest() { return m_concurrent_processing_requests <= MAX_HTTP_CONCURRENT_PROCESSING_REQUEST; }
	bool BeginProcessRequest() {
		if (m_concurrent_processing_requests.fetch_add(1) < MAX_HTTP_CONCURRENT_PROCESSING_REQUEST)
			return true;
		else
		{
			m_concurrent_processing_requests.fetch_sub(1); // Undo increment
			return false;
		}
	}
	void EndProcessRequest() { --m_concurrent_processing_requests; }
};

}

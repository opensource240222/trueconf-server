#pragma once

#include "MessageStats.h"
#include "std/cpplib/json/elements.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#define   VS_TR_PREFIX_MON_PIPE   "transport"
extern const char   VS_TrPrefixMonPipe[];		// = VS_TR_PREFIX_MON_PIPE

#define   TM_TYPE_UNKNOWN					0


namespace transport
{
	struct Monitor
	{
		union TmRequest
		{
			TmRequest(void) {}
			uint8_t   type;
		};

		struct TmReply
		{
			TmReply(void) {  }
			struct Endpoint
			{
				uint8_t	ep_type;
				std::string			id;
				std::string			username;
				std::string			protocol;
				std::chrono::steady_clock::time_point last_connect_date_time, last_disconnect_date_time;
				MessageStats recv_stats, send_stats;
				std::string local_host, remote_host;
				uint16_t	local_port, remote_port;
				void ToJson(json::Object& obj);
			};

			struct Service
			{
				std::string service_name;
				std::string service_type;
				MessageStats recv_stats, send_stats;
				void ToJson(json::Object& obj);
			};

			void ToJson(json::Object& obj);

			std::vector<Endpoint> endpoints;
			std::vector<Service> services;
		};

	};
}

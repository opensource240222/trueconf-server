#pragma once

#include "std-generic/cpplib/macro_utils.h"

#include <cstdint>
#include <string>
#include <vector>

#include "std/cpplib/json/elements.h"

namespace acs {
	struct Monitor
	{
		union AcsRequest
		{
			AcsRequest(void) {}
			uint8_t   type;
		};

		struct AcsReply
		{
			AcsReply(void) {  }
			struct Handler
			{
				VS_FORWARDING_CTOR3(Handler, name, processed_connections, accepted_connections) {}
				std::string name;
				unsigned processed_connections;
				unsigned accepted_connections;
				void ToJson(json::Object& obj) const;
			};

			struct Listener
			{
				std::string ip;
				uint16_t port;
				std::string protocol;
				uint16_t rcvd_connections = 0;
				uint16_t unseccessful_connections = 0;
				void ToJson(json::Object& obj) const;
			};

			void ToJson(json::Object& obj) const;

			std::vector<Handler> handlers;
			std::vector<Listener> listeners;
		};

	};

}
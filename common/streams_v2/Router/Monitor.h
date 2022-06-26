#pragma once
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#include "std/cpplib/json/elements.h"

namespace stream {
	struct Monitor
	{
		union StreamRequest
		{
			StreamRequest(void) {}
			uint8_t   type;
		};

		struct StreamReply
		{
			StreamReply(void) {  }
			struct Conference
			{
				std::string name;
				std::chrono::system_clock::time_point creation_time;
				std::chrono::steady_clock::duration max_absence_ms;
				uint16_t num_participants;
				void ToJson(json::Object& obj) const;
			};

			struct Participant
			{
				std::string conf_name;
				std::string part_name;
				std::chrono::system_clock::time_point creation_time;
				std::string receiver;
				//uint16_t r_reconnects;
				time_t r_last_connect;
				time_t r_last_disconnect;
				std::string r_local_addr;
				std::string r_remote_addr;
				uint16_t r_bytes=0;
				uint16_t r_bytes_bandwith=0;
				uint16_t r_frames=0;
				uint16_t r_frames_bandwith=0;

				std::string sender;
				//uint16_t s_reconnects;
				time_t s_last_connect;
				time_t s_last_disconnect;
				std::string s_local_addr;
				std::string s_remote_addr;
				uint16_t s_bytes=0;
				uint16_t s_bytes_bandwith=0;
				uint16_t s_frames=0;
				uint16_t s_frames_bandwith=0;
				void ToJson(json::Object& obj) const;
			};

			void ToJson(json::Object& obj) const;

			std::vector<Conference> conferences;
			std::vector<Participant> participants;
		};

	};

}
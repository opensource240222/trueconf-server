#include "Monitor.h"

namespace stream {

	void Monitor::StreamReply::ToJson(json::Object & obj) const
	{
		json::Array conference_array, participants_array;
		for (size_t i = 0; i < conferences.size(); i++)
		{
			json::Object obj;
			conferences[i].ToJson(obj);
			conference_array.Insert(obj);
		}

		for (size_t i = 0; i < participants.size(); i++)
		{
			json::Object obj;
			participants[i].ToJson(obj);
			participants_array.Insert(obj);
		}

		obj["conference_array"] = conference_array;
		obj["participants_array"] = participants_array;
	}

	void Monitor::StreamReply::Participant::ToJson(json::Object & obj) const
	{
		obj["conf_name"] = json::String(conf_name);
		obj["part_name"] = json::String(part_name);
		obj["creation_time"]= json::String(std::to_string(creation_time.time_since_epoch().count()));

		obj["receiver"]= json::String(receiver);
		obj["r_last_connect"] = json::String(std::to_string(r_last_connect));
		obj["r_last_disconnect"]= json::String(std::to_string(r_last_disconnect));
		obj["r_local_addr"]= json::String(r_local_addr);
		obj["r_remote_addr"]= json::String(r_remote_addr);
		obj["r_bytes"]= json::String(std::to_string(r_bytes));
		obj["r_bytes_bandwith"]= json::String(std::to_string(r_bytes_bandwith));;
		obj["r_frames"]= json::String(std::to_string(r_frames));
		obj["r_frames_bandwith"]= json::String(std::to_string(r_frames_bandwith));

		obj["sender"]= json::String(sender);
		obj["s_last_connect"]= json::String(std::to_string(s_last_connect));
		obj["s_last_disconnect"]= json::String(std::to_string(s_last_disconnect));
		obj["s_local_addr"]= json::String(s_local_addr);
		obj["s_remote_addr"]= json::String(s_remote_addr);
		obj["s_bytes"]= json::String(std::to_string(s_bytes));
		obj["s_bytes_bandwith"]= json::String(std::to_string(s_bytes_bandwith));
		obj["s_frames"]= json::String(std::to_string(s_frames));
		obj["s_frames_bandwith"]= json::String(std::to_string(s_frames_bandwith));


	}

	void Monitor::StreamReply::Conference::ToJson(json::Object & obj) const
	{
		obj["name"] = json::String(name);
		obj["creation_time"] = json::String(std::to_string(creation_time.time_since_epoch().count()));
		obj["num_participants"] = json::String(std::to_string(num_participants));
		obj["max_silence_ms"] = json::String(std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(max_absence_ms).count()));
	}

}

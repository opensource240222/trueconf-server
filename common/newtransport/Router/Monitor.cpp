#include "Monitor.h"

namespace transport {

void Monitor::TmReply::Endpoint::ToJson(json::Object& obj)
{
	obj["id"] = json::String(id);
	obj["username"] = json::String(username);
	obj["protocol"] = json::String(protocol);

	obj["last_connect_date_time"] = json::String(std::to_string(std::chrono::duration_cast<std::chrono::seconds>(last_connect_date_time.time_since_epoch()).count()));
	obj["last_disconnect_date_time"] = json::String(std::to_string(std::chrono::duration_cast<std::chrono::seconds>(last_disconnect_date_time.time_since_epoch()).count()));
	obj["recv_message_count"] = json::String(std::to_string(recv_stats.m_message_count));
	obj["recv_total_size"] = json::String(std::to_string(recv_stats.m_total_recv_size));
	obj["recv_min_message_size"] = json::String(std::to_string(recv_stats.m_min_message_size));
	obj["recv_max_message_size"] = json::String(std::to_string(recv_stats.m_max_message_size));
	obj["recv_avg_message_size"] = json::String(std::to_string(recv_stats.m_average_message_size));
	obj["send_message_count"] = json::String(std::to_string(send_stats.m_message_count));
	obj["send_total_size"] = json::String(std::to_string(send_stats.m_total_recv_size));
	obj["send_min_message_size"] = json::String(std::to_string(send_stats.m_min_message_size));
	obj["send_max_message_size"] = json::String(std::to_string(send_stats.m_max_message_size));
	obj["send_avg_message_size"] = json::String(std::to_string(send_stats.m_average_message_size));
	obj["local_host"] = json::String(local_host);
	obj["local_port"] = json::String(std::to_string(local_port));
	obj["remote_host"] = json::String(remote_host);
	obj["remote_port"] = json::String(std::to_string(remote_port));
}

void Monitor::TmReply::Service::ToJson(json::Object& obj)
{
	obj["id"] = json::String(service_name);
	obj["type"] = json::String(service_type);
	obj["recv_message_count"] = json::String(std::to_string(recv_stats.m_message_count));
	obj["recv_total_size"] = json::String(std::to_string(recv_stats.m_total_recv_size));
	obj["recv_min_message_size"] = json::String(std::to_string(recv_stats.m_min_message_size));
	obj["recv_max_message_size"] = json::String(std::to_string(recv_stats.m_max_message_size));
	obj["recv_avg_message_size"] = json::String(std::to_string(recv_stats.m_average_message_size));
	obj["send_message_count"] = json::String(std::to_string(send_stats.m_message_count));
	obj["send_total_size"] = json::String(std::to_string(send_stats.m_total_recv_size));
	obj["send_min_message_size"] = json::String(std::to_string(send_stats.m_min_message_size));
	obj["send_max_message_size"] = json::String(std::to_string(send_stats.m_max_message_size));
	obj["send_avg_message_size"] = json::String(std::to_string(send_stats.m_average_message_size));
}

void Monitor::TmReply::ToJson(json::Object& obj)
{
	json::Object ans;
	json::Array endpoint_array, service_array;
	for (size_t i = 0; i < endpoints.size(); i++)
	{
		json::Object obj;
		endpoints[i].ToJson(obj);
		endpoint_array.Insert(obj);
	}
	for (size_t i = 0; i < services.size(); i++)
	{
		json::Object obj;
		services[i].ToJson(obj);
		service_array.Insert(obj);
	}
	obj["endpoints"] = endpoint_array;
	obj["services"] = service_array;
}

}

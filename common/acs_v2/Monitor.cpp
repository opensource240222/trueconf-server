#include "Monitor.h"

namespace acs {

	void Monitor::AcsReply::ToJson(json::Object& obj) const
	{
		json::Array handler_array, listeners_array;
		for (size_t i = 0; i < handlers.size(); i++)
		{
			json::Object obj;
			handlers[i].ToJson(obj);
			handler_array.Insert(obj);
		}
		for (size_t i = 0; i < listeners.size(); i++)
		{
			json::Object obj;
			listeners[i].ToJson(obj);
			listeners_array.Insert(obj);
		}

		obj["handler_array"] = handler_array;
		obj["listeners_array"] = listeners_array;
	}

	void Monitor::AcsReply::Handler::ToJson(json::Object& obj) const
	{
		obj["name"] = json::String(name);
		obj["processed_connections"] = json::String(std::to_string(processed_connections));
		obj["accepted_connections"] = json::String(std::to_string(accepted_connections));
	}


	void Monitor::AcsReply::Listener::ToJson(json::Object & obj) const
	{
		obj["ip"] = json::String(ip);
		obj["port"] = json::String(std::to_string(port));
		obj["protocol"] = json::String(protocol);
		obj["recieved_connections"] = json::String(std::to_string(rcvd_connections));
		obj["unseccessful_connections"] = json::String(std::to_string(unseccessful_connections));
	}

}

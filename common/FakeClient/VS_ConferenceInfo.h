#pragma once

#include <string>
#include <iomanip>

struct VS_ConferenceInfo {
	VS_ConferenceInfo(bool group_conf, bool public_conf) :is_group_conf(group_conf), is_public_conf(public_conf) {}
	VS_ConferenceInfo(bool group_conf, bool public_conf, const std::string &topic_) :is_group_conf(group_conf), is_public_conf(public_conf), topic(topic_){}
	bool is_group_conf = false;
	bool is_public_conf = false;
	std::string topic;
	std::string ID;

	std::ostream& operator<<(std::ostream& os) {
		return os << "VS_ConferenceInfo: " << std::boolalpha << "is_group_conf = '" << is_group_conf << "' is_public_conf = '" << is_public_conf << "' topic = '" << topic << "', ID = '" << ID << '\'';
	}
};
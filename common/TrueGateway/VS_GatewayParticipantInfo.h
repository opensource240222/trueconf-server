#pragma once

#include <string>

/* gateway  */
namespace gw{
struct Participant {
	Participant() = default;
	Participant(string_view id, bool allowedToJoinGRK)
		:callID(id), allowedToJoinGroupConf(allowedToJoinGRK) {}
	std::string callID;
	bool allowedToJoinGroupConf = false;

	std::ostream& operator<<(std::ostream& os) {
		return os << "gw::Participant: " << std::boolalpha << "callID = '" << callID << "' allowedToJoinGroupConf = '" << allowedToJoinGroupConf << '\'';
	}
};
}
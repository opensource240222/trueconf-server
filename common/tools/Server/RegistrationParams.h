#pragma once
#include <string>

namespace vs{

struct RegistrationParams
{
	unsigned	mode = 0;
	std::string server_name;
	std::string server_id;
	std::string serial;
	std::string offline_reg_file;
};

} // namespace vs
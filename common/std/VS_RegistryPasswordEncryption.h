#pragma once
#include <string>
#include "std-generic/cpplib/string_view.h"

namespace sec {
	std::string EncryptRegistryPassword(string_view user_name, string_view unencrypted_password);
	std::string DecryptRegistryPassword(string_view user_name, string_view password_from_registry);
} //namespace sec

#pragma once

#include <string>

namespace vs
{
	extern bool escape_unsafe_html_tags(const char *in, std::string &out);
	extern bool escape_unsafe_html_tags(const std::string &in, std::string &out);
}

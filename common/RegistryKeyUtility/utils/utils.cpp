#include "utils.h"
#include <boost/regex.hpp>
#include <assert.h>

#include "../constants/Constants.h"


bool is_valid_key_name(const string_view str)
{
	return string_view::npos == str.find(std::string{ DELIMITER } +DELIMITER);
}

string_view cut_front_end_delimeter(string_view str)
{
	if (!str.empty() && str.front() == DELIMITER)
		str.remove_prefix(1);
	if (!str.empty() && str.back() == DELIMITER)
		str.remove_suffix(1);
	return str;
}

static const boost::regex REGEX_INT{ "^-?(0|[1-9][0-9]*)$" };

bool is_int_numbers(const char* in)
{
	assert(in != nullptr);
	return boost::regex_match(in, REGEX_INT);
}

static const boost::regex REGEX_BASE64{ R"(^(?:[A-Za-z0-9+\/]{4})*(?:[A-Za-z0-9+\/]{2}==|[A-Za-z0-9+\/]{3}=)?$)" };

bool is_base64(const char* in)
{
	assert(in != nullptr);
	return boost::regex_match(in, REGEX_BASE64);
}

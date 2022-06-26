#include "escape.h"
#include "../../std/cpplib/curl_deleters.h"
#include "../exceptions/SIPURIEscapeException.h"
#include "std/debuglog/VS_Debug.h"
#include <curl/curl.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>

#include <memory>

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

namespace vs
{
	static constexpr char USER_DOMAIN_SEPARATOR{ '@' };
	static constexpr char USER_PART[] = R"({}<>"`|^[]:@%\)";
	static const boost::regex REGEX_URI_SYMBOL("(\\S*(%([a-fA-F0-9]{2}))\\S*)+", boost::regex::optimize);

	static constexpr struct
	{
		const char *from;
		const char *to;
	} REPLACE_SYMBOL[] = {
		// https://tools.ietf.org/html/rfc3261#section-25.1
		// user-unreserved  =  "&" / "=" / "+" / "$" / "," / ";" / "?" / "/"
		{ "%26", "&" },
		{ "%3D", "=" },
		{ "%2B", "+" },
		{ "%24", "$" },
		{ "%2C", "," },
		{ "%3B", ";" },
		{ "%3F", "?" },

		// unreserved  =  alphanum / mark
		// mark = "-" / "_" / "." / "!" / "~" / "*" / "'" / "(" / ")"
		// "-" / "_" / "." /  "~" is not escaped in url so we don't unescape them
		// '/' is allowed, but we use it in call_id/trans_id so must remain escaped
		// '(' ')' are allowed, but they will be cleaned like visual separators
		{ "%27", "'" },
		{ "%2A", "*" }
	};

	static inline bool contains_not_allowed_in_user_part(const string_view utf8_str) {
		return utf8_str.find_first_of(USER_PART) != string_view::npos ||													// '%' is allowed but used in escaping
			std::find_if(utf8_str.cbegin(), utf8_str.cend(), [](const char ch) {return ch < 33; }) != utf8_str.cend();	// all control chars and space
	}

	static inline std::pair<string_view, string_view> find_first_at_second(const string_view str)
	{
		const std::size_t at_pos = str.find_last_of(USER_DOMAIN_SEPARATOR);
		//user@domain@server -> user@domain; user@server -> user; user -> user
		if (at_pos == string_view::npos)
		{
			return { str, {} };
		}
		return { str.substr(0, at_pos),  str.substr(at_pos) };
	}

	template<typename T>
 	static std::string execute(T &&func, const string_view str)
	{
		auto&& user_server = find_first_at_second(str);
		std::string result{ func(user_server.first) };
		if (!user_server.second.empty())
		{
			result += user_server.second;
		}
		return result;
	}

	static std::unique_ptr<CURL, CURL_deleter> get_curl()
	{
		std::unique_ptr<CURL, CURL_deleter> curl(::curl_easy_init());
		if (!curl)
		{
			throw SIPURIEscapeException(__LINE__, __FILE__, "error curl_easy_init");
		}
		return curl;
	}

	std::string sip_uri_unescape(const string_view input)
	{
		return execute([](const string_view str)
		{
			try
			{
				if (boost::regex_match(str.cbegin(), str.cend(), REGEX_URI_SYMBOL))
				{
					auto &&curl = get_curl();
					int outlength = 0;
					std::unique_ptr<char, curl_free_deleter> unescaped(::curl_easy_unescape(curl.get(), str.data(), str.length(), &outlength));
					if (unescaped)
					{
						return std::string{ unescaped.get(), unescaped.get() + outlength };
					}
				}
				return std::string(str);
			}
			catch (const std::runtime_error &ex)
			{
				dstream1 << "sip_uri_unescape() error " << ex.what() << "\n";
				return std::string();
			}
		}, input);
	}

	std::string sip_uri_escape(const string_view input)
	{

		return execute([](const string_view str)
		{
			if (contains_not_allowed_in_user_part(str))
			{
				std::unique_ptr<CURL, CURL_deleter> curl = get_curl();
				std::unique_ptr<char, curl_free_deleter> escaped(::curl_easy_escape(curl.get(), str.data(), str.length()));
				if (escaped)
				{
					std::string res{ escaped.get() };
					for (const auto& val : REPLACE_SYMBOL)
					{
						boost::replace_all(res, val.from, val.to);
					}
					return res;
				}
			}
			return std::string(str);
		}, input);
	}
}

#undef DEBUG_CURRENT_MODULE
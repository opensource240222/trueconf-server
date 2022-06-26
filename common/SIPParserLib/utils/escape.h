#pragma once
#include "std-generic/cpplib/string_view.h"

namespace vs
{
	/*!
		Convert input utf8 string to sip uri string (convert before symbol '@').
		\param input - utf8 string.
		\throw SIPURIEscapeException in case of error init libcurl.
		\return sip uri string.
	*/
	std::string sip_uri_escape(const string_view input);

	/*!
		Convert input sip uri string to utf8 string (convert before symbol '@').
		\param input - sip uri string.
		\throw SIPURIEscapeException in case of error init libcurl.
		\return utf8 string.
	*/
	std::string sip_uri_unescape(const string_view input);
}

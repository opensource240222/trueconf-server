#pragma once

#include "std-generic/cpplib/string_view.h"

template<class F>
void ForEachHost(string_view addresses, F&& handler)
{
	while (!addresses.empty())
	{
		const size_t comma_pos = addresses.find(',');
		const string_view address = addresses.substr(0, comma_pos);

		const size_t last_colon_pos = address.rfind(':');

		// ']' after last ':' => IPv6 without port
		const size_t port_delim_pos = address.find(']', last_colon_pos) == string_view::npos
			? last_colon_pos
			: string_view::npos;

		const string_view host = address.substr(0, port_delim_pos);

		string_view port;
		if (port_delim_pos != string_view::npos)
			port = address.substr(port_delim_pos + 1);

		handler(host, port);

		if (comma_pos != string_view::npos)
			addresses.remove_prefix(comma_pos + 1);
		else
			addresses = "";
	}
}

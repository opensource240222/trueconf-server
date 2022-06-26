#pragma once

#include "VS_DNS.h"

namespace net
{
	namespace dns_tools
	{
		template<typename Query>
		net::address single_make_a_aaaa_lookup(Query &&query, int flags = net::dns::flags_t::real_inet | net::dns::flags_t::use_cache) noexcept
		{
			{
				boost::system::error_code ec;
				auto addr = net::address::from_string(query, ec);
				if (!ec) // no error
					return addr;
			}

			auto res = net::dns::make_a_aaaa_lookup(std::forward<Query>(query), flags).get();
			for (auto item : { &res.first, &res.second })
			{
				assert(item);
				if (!item->ec) // no error
				{
					assert(!item->host.addrs.empty());
					assert(!item->host.addrs.front().is_unspecified());
					return item->host.addrs.front();
				}
			}
			return {};
		}

		void init_loaded_options();

	} //namespace dns_tools
} //namespace net
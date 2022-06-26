#pragma once

#include "VS_DNS.h"

namespace net
{
	namespace dns
	{
		struct Resolver
		{
			virtual void Init(const init_options &options) = 0;
			virtual void Destroy() = 0;
			virtual ~Resolver() {}
			virtual	void Make_A_Lookup(std::string hostname, a_callback cb, int flags) = 0;
			virtual void Make_AAAA_Lookup(std::string hostname, aaaa_callback cb, int flags) = 0;
			virtual void Make_SRV_Lookup(std::string hostname, srv_callback cb, int flags) = 0;
			virtual void Make_PTR_Lookup(net::address addr, ptr_callback cb, int flags) = 0;
			virtual void Make_NAPTR_Lookup(std::string query, naptr_type type, naptr_callback cb, int flags) = 0;
			virtual void Make_A_AAAA_Lookup(std::string hostname, a_aaaa_callback cb, int flags) = 0;
			virtual bool RunningInThisThread() const = 0;
			virtual std::future<void> CancelAllRequests() = 0;
			virtual void PrintCache(std::ostream &) = 0;
		};
	}
}

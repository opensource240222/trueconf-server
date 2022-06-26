#include "VS_DNSTools.h"

#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"

void net::dns_tools::init_loaded_options()
{
	net::dns::init_options op = net::dns::default_init_options();

	std::string servers;

	VS_RegistryKey key(false, CONFIGURATION_KEY);
	if(key.IsValid())
	{
		if(key.GetString(servers, "DNS Servers") && !servers.empty())
		{
			op.servers = servers.c_str();
		}

		int value;

		if(key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "DNS Timeout") > 0)
		{
			op.timeout = std::chrono::milliseconds(value);
		}

		if (key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "DNS CachePositiveTTL") > 0)
		{
			op.cachePositiveTTL = std::chrono::seconds(value);
		}

		if (key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "DNS CacheNegativeTTL") > 0)
		{
			op.cacheNegativeTTL = std::chrono::seconds(value);
		}

		if (key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "DNS CacheMaxRecords") > 0)
		{
			op.cacheMaxRecords = value;
		}

		if (key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "DNS Tries") > 0)
		{
			op.tries = value;
		}

		if (key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "DNS PrimaryFlag") > 0)
		{
			op.primaryFlag = !!value;
		}
	}

	net::dns::init(op);
}

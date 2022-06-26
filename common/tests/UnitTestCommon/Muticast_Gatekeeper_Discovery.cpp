#if 0 // TODO: Rewrite to use the new UDP Router

#include <utility>
#include <memory>
#include <iostream>
#include "../../../TrueGateway/net/VS_NetAcceptor.h"
#include "../../../TrueGateway/net/VS_UDPConnRouter.h"
#include "../../../std/cpplib/VS_SimpleStr.h"
#include "../../../acs/Lib/VS_AcsLib.h"

#include <gtest/gtest.h>

namespace multicast_GK_discovery {
	class MulticastGKDiscovery :
		public ::testing::Test,
		public VS_NetAcceptor
	{
	protected:
		MulticastGKDiscovery()
		{}

		virtual void SetUp()
		{}

		virtual void TearDown()
		{}
	};

	TEST_F(MulticastGKDiscovery, udp_router_loosing_32512)
	{
		VS_IPPortAddress addr;
		VS_SimpleStr addr_string;
		char host_addr[64] = { 0 };
		char host[512] = { 0 };

		// get connection data
		ASSERT_TRUE(VS_GetDefaultHostName(host, 512));
		{
			char *p[1];
			p[0] = host_addr;
			VS_GetHostsByName(host, p, 1, 64);
		}
		// create router for H225 Multicast Discovery
		ASSERT_TRUE(AddUDPAddress(host_addr, 0, "224.0.1.41", 11718, e_RAS));

		// get router and validate address parameter
		auto routers = GetUDPRouters();
		ASSERT_EQ(routers.size(), 1);
		ASSERT_TRUE(routers[0]->GetBindAddr(addr));
		ASSERT_TRUE(addr.GetAddrString(addr_string));
		{
			std::string expected_conn_string = "UDP:";
			expected_conn_string += host_addr;
			expected_conn_string += ":";
			expected_conn_string += std::to_string(11718);
			ASSERT_STRCASEEQ(addr_string.m_str, expected_conn_string.c_str());
		}
	}
}

#endif

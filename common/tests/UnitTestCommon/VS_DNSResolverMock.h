#pragma once

#include <gmock/gmock.h>

#include "tests/common/GMockOverride.h"
#include "net/DNSUtils/VS_DNSResolver.h"

namespace net {
	namespace dns
	{
		struct ResolverMock : public net::dns::Resolver
		{
			void DelegateTo(net::dns::Resolver *impl)
			{
				using ::testing::_;
				using ::testing::Invoke;

				ON_CALL(*this, Init(_)).WillByDefault(Invoke(impl, &Resolver::Init));
				ON_CALL(*this, Destroy()).WillByDefault(Invoke(impl, &Resolver::Destroy));
				ON_CALL(*this, Make_A_Lookup(_, _, _)).WillByDefault(Invoke(impl, &Resolver::Make_A_Lookup));
				ON_CALL(*this, Make_AAAA_Lookup(_, _, _)).WillByDefault(Invoke(impl, &Resolver::Make_AAAA_Lookup));
				ON_CALL(*this, Make_SRV_Lookup(_, _, _)).WillByDefault(Invoke(impl, &Resolver::Make_SRV_Lookup));
				ON_CALL(*this, Make_PTR_Lookup(_, _, _)).WillByDefault(Invoke(impl, &Resolver::Make_PTR_Lookup));
				ON_CALL(*this, Make_NAPTR_Lookup(_, _, _, _)).WillByDefault(Invoke(impl, &Resolver::Make_NAPTR_Lookup));
				ON_CALL(*this, Make_A_AAAA_Lookup(_, _, _)).WillByDefault(Invoke(impl, &Resolver::Make_A_AAAA_Lookup));
				ON_CALL(*this, RunningInThisThread()).WillByDefault(Invoke(impl, &Resolver::RunningInThisThread));
				ON_CALL(*this, CancelAllRequests()).WillByDefault(Invoke(impl, &Resolver::CancelAllRequests));
				ON_CALL(*this, PrintCache(_)).WillByDefault(Invoke(impl, &Resolver::PrintCache));
			}

			MOCK_METHOD1_OVERRIDE(Init, void(const init_options &options));
			MOCK_METHOD0_OVERRIDE(Destroy, void());
			MOCK_METHOD3_OVERRIDE(Make_A_Lookup, void(std::string, a_callback, int));
			MOCK_METHOD3_OVERRIDE(Make_AAAA_Lookup, void(std::string, aaaa_callback, int));
			MOCK_METHOD3_OVERRIDE(Make_SRV_Lookup, void(std::string, srv_callback, int));
			MOCK_METHOD3_OVERRIDE(Make_PTR_Lookup, void(net::address, ptr_callback, int));
			MOCK_METHOD4_OVERRIDE(Make_NAPTR_Lookup, void(std::string, naptr_type, naptr_callback, int));
			MOCK_METHOD3_OVERRIDE(Make_A_AAAA_Lookup, void(std::string, a_aaaa_callback, int));
			MOCK_CONST_METHOD0_OVERRIDE(RunningInThisThread, bool());
			MOCK_METHOD0_OVERRIDE(CancelAllRequests, std::future<void>());
			MOCK_METHOD1_OVERRIDE(PrintCache, void(std::ostream &));
		};
	}
}
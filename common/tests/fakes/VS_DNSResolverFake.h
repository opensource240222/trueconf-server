#pragma once

#include "net/DNSUtils/VS_DNSResolver.h"

namespace test { namespace net { namespace dns {

		struct ResolverFake : public ::net::dns::Resolver
		{
			void Init(const ::net::dns::init_options &options) override {};
			void Destroy() override {}
			virtual ~ResolverFake() {}

			void Make_A_Lookup(std::string /*hostname*/, ::net::dns::a_callback cb, int /*flags*/) override
			{
				return cb({}, ::net::dns::errors::not_imp);
			}

			void Make_AAAA_Lookup(std::string /*hostname*/, ::net::dns::aaaa_callback cb, int /*flags*/) override
			{
				return cb({}, ::net::dns::errors::not_imp);
			}

			void Make_SRV_Lookup(std::string /*hostname*/, ::net::dns::srv_callback cb, int /*flags*/) override
			{
				return cb({}, ::net::dns::errors::not_imp);
			}

			void Make_PTR_Lookup(::net::address/*addr*/, ::net::dns::ptr_callback cb, int /*flags*/) override
			{
				return cb({}, ::net::dns::errors::not_imp);
			}

			void Make_NAPTR_Lookup(std::string /*query*/, ::net::dns::naptr_type /*type*/, ::net::dns::naptr_callback cb, int /*flags*/) override
			{
				return 	cb({}, ::net::dns::errors::not_imp);
			}

			void Make_A_AAAA_Lookup(std::string /*hostname*/, ::net::dns::a_aaaa_callback cb, int /*flags*/) override
			{
				return cb({{}, ::net::dns::errors::not_imp }, { {}, ::net::dns::errors::not_imp });
			}

			bool RunningInThisThread() const override
			{
				return false;
			}

			std::future<void> CancelAllRequests() override
			{
				return ReturnFakeFuture();
			}

			void PrintCache(std::ostream &) override
			{
				//stub
			}

		private:
			static inline std::future<void> ReturnFakeFuture()
			{
				std::promise<void> p;
				p.set_value();
				return p.get_future();
			}
		};

} } }

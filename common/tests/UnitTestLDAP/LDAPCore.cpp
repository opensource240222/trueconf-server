#ifdef _WIN32
#include <gtest/gtest.h>

//#include "std/cpplib/VS_RegistryKey.h"
//#include "std/cpplib/VS_RegistryConst.h"
//#include "std/cpplib/ignore.h"
#include "ldap_core/VS_LDAPCore.h"
//#include "ldap_core/CfgHelper.h"
//#include "ldap_core/VS_LDAPConst.h"

#include <chrono>
#include <thread>

namespace
{
	class LDAPCore : public ::testing::Test,
		public tc::LDAPCore
	{
	public:
		explicit LDAPCore() : tc::LDAPCore({}) {}

	private:
		tc::ldap_error_code_t GetAllGroupsOfUser(const tc::ldap_user_info& user, const unsigned long user_primary_group_id, const std::string& objectSid, std::set<tc::group_dn>& ret) override
		{	return LDAP_PARAM_ERROR;	}
	};

	struct TestDataItem
	{
		std::wstring test_url;
		std::string host;
		unsigned short port;
		std::string NewDN;
		bool IsSecure;
	};

	const TestDataItem test_data[] = {		// format: test_url, host, port, NewDistinguishedName, IsSecure
		// real test
		{ L"ldap://tc43.truetest.loc/DC=tc43,DC=truetest,DC=loc", "tc43.truetest.loc", tc::LDAP_PORT_INIT, "DC=tc43,DC=truetest,DC=loc", false },
		{ L"ldap://ForestDnsZones.truetest.loc/DC=ForestDnsZones,DC=truetest,DC=loc", "ForestDnsZones.truetest.loc", tc::LDAP_PORT_INIT, "DC=ForestDnsZones,DC=truetest,DC=loc", false },
		{ L"ldap://truetest.loc/CN=Configuration,DC=truetest,DC=loc", "truetest.loc", tc::LDAP_PORT_INIT, "CN=Configuration,DC=truetest,DC=loc", false },

		// theoretic test
		{ L"ldaps://192.168.1.1:1234/", "192.168.1.1", 1234, "", true },
//		{ L"ldaps://[fd00:7:495:1::111]/", "[fd00:7:495:1::111]", 0, "", true },
//		{ L"://", "", 0, L"", 0 },
		{ L"ldap://ldaps/", "ldaps", tc::LDAP_PORT_INIT, "", false },
		{ L"ldaps://ldap/", "ldap", tc::LDAP_SECURE_PORT_INIT, "", true },

		// test from interntet (rfc2255, rfc4516, etc)
		{ L"ldap://example.ibm.com/c=US?o,description?one?o=ibm", "example.ibm.com", tc::LDAP_PORT_INIT, "c=US", false },
		{ L"ldap:///", "", tc::LDAP_PORT_INIT, "", false },
		{ L"ldap:///??sub??!bindname=cn=Manager%2co=Foo", "", tc::LDAP_PORT_INIT, "", false },
		{ L"ldap://ldap.example.net", "ldap.example.net", tc::LDAP_PORT_INIT, "", false },
		{ L"ldap://ldap.example.net/", "ldap.example.net", tc::LDAP_PORT_INIT, "", false },
		{ L" ldap ://ldap.example.net/?", "ldap.example.net", tc::LDAP_PORT_INIT, "", false },
		// rfc4516 section#4 Examples
		{ L"ldap:///o=University%20of%20Michigan,c=US", "", tc::LDAP_PORT_INIT, "o=University of Michigan,c=US", false},

//		{ L"", "", 0, L"", 0}		// todo(kt): this should EXPECT_FALSE
		// todo(kt): for these from interntet also (rfc2255, rfc4516):
		// ldap:///o=University%20of%20Michigan,c=US
		// ldap://ldap.itd.umich.edu/o=University%20of%20Michigan,c=US
		// ldap://ldap.itd.umich.edu/o=University%20of%20Michigan,c=US??sub?(cn=Babs%20Jensen)
		// ldap://ldap.itd.umich.edu/c=GB?objectClass?one
		// ldap:///??sub??!bindname=cn=Manager%2co=Foo
		// ldap://ldap.netscape.com/o=Airius.com?objectClass?one
		// LDAP://ldap1.example.com/c=GB?objectClass?ONE
		// ldap://ldap.example.net
		// ldap://ldap.example.net/
		// ldap://localhost:389/ou=People,o=JNDITutorial
	};

	TEST_F(LDAPCore, ParseLdapUrl)
	{
		std::string host;
		unsigned long port(0);
		std::wstring NewDN;

		for (const auto &t : test_data)
		{
			std::string host;
			unsigned short port(0);
			std::wstring NewDN;
			bool IsSecure(false);
			tc::referral_t r;
			ASSERT_TRUE(ParseReferral(t.test_url, r));
			ASSERT_EQ(t.host, r.host);
			ASSERT_EQ(t.port, r.port);
			ASSERT_EQ(t.NewDN, r.baseDN);
			ASSERT_EQ(t.IsSecure, r.IsSecure);
		}
	}

	TEST_F(LDAPCore, isUselessDN){
		m_skip_referrals = tc::DEFAULT_SKIP_REFERRALS;

		for (const auto &t : test_data)
		{
			tc::referral_t r;
			ParseReferral(t.test_url, r);
			if (t.test_url.find(L"DC=ForestDnsZones") != std::wstring::npos ||
				t.test_url.find(L"DC=DomainDnsZones") != std::wstring::npos ||
				t.test_url.find(L"CN=Configuration") != std::wstring::npos ||
				t.test_url.find(L"CN=Schema") != std::wstring::npos){
				ASSERT_TRUE(isUselessDN(r.baseDN));
			}
			else{
				ASSERT_FALSE(isUselessDN(r.baseDN));
			}
		}
	}

}  // namespace
#endif
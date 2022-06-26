#include "std/cpplib/VS_Policy.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <functional>
#include <thread>

const std::string ip("127.0.0.1");
const std::string login("user");

class PolicyTest : public ::testing::Test
{
public:
	PolicyTest(VS_Policy::PolicySettings aDs = {}, std::string aName = CONFIGURATION_KEY)
		: ds(aDs)
		, m_name(std::move(aName)) {}
protected:
	void SetUp() override
	{
		// kt: override default (values from old test)
		ds.max_fail_before_delay = 8;
		ds.delay_ttl = std::chrono::seconds(2 * 60 * 60);

		VS_RegistryKey key(false, m_name, false, true);
		ASSERT_TRUE(key.IsValid());
		unsigned unsigned_use_ip = ds.use_ip ? 1 : 0;
		unsigned unsigned_use_login = ds.use_login ? 1 : 0;
		auto observe_interval = std::chrono::duration_cast<std::chrono::seconds>(ds.observe_interval).count();
		auto ban_time = std::chrono::duration_cast<std::chrono::seconds>(ds.ban_time).count();
		auto delay_time = std::chrono::duration_cast<std::chrono::seconds>(ds.delay_time).count();
		auto delay_interval = std::chrono::duration_cast<std::chrono::seconds>(ds.delay_interval).count();
		auto delay_ttl = std::chrono::duration_cast<std::chrono::seconds>(ds.delay_ttl).count();

		ASSERT_TRUE(key.SetValue(&unsigned_use_ip, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_USE_IP));
		ASSERT_TRUE(key.SetValue(&unsigned_use_login, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_USE_LOGIN));
		ASSERT_TRUE(key.SetValue(&ds.max_fail_before_ban, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_MAX_FAIL_BEFORE_BAN));
		ASSERT_TRUE(key.SetValue(&ds.max_fail_before_delay, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_MAX_FAIL_BEFORE_DELAY));
		ASSERT_TRUE(key.SetValue(&delay_time, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_DELAY_TIME));
		ASSERT_TRUE(key.SetValue(&ban_time, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_BAN_TIME));
		ASSERT_TRUE(key.SetValue(&ds.access_deny_on_ban, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_SEND_ACCESS_DENY));
		ASSERT_TRUE(key.SetValue(&ds.silent_on_ban, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_SILENT_ON_BAN));
		ASSERT_TRUE(key.SetValue(&delay_ttl, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_DELAY_TTL));
		ASSERT_TRUE(key.SetValue(&delay_interval, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_DELAY_INTERVAL));
		ASSERT_TRUE(key.SetValue(&observe_interval, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_OBSERVE_INTERVAL));
	}

	void TearDown() override
	{
		VS_RegistryKey key(false, "", false);
		if(key.IsValid())
		{
			key.RemoveKey(m_name);
		}
	}

	VS_Policy::PolicySettings ds;
private:
	std::string m_name;
};

class PolicySettingsInitTest : public PolicyTest
{
public:
	PolicySettingsInitTest() : PolicyTest(VS_Policy::PolicySettings{
		1,	// use_ban
		1,	// use_login
		0,	// access_deny_on_ban
		1,	// silent_on_ban
		10,	// max_fail_before_ban = msgs in observe_interval (~10 logins per 1 minues)
		8,  // max_fail_before_delay
		std::chrono::seconds(5),		// delay_interval
		std::chrono::seconds(60 * 60),	// delay_time
		std::chrono::hours(24),			// ban_time
		std::chrono::seconds(2*60*60),		// delay_ttl
		std::chrono::minutes(1)			// observe_interval
	}, std::string(CONFIGURATION_KEY) + "\\TEST\\test") {}
};


class PolicySettingsTest final : public VS_Policy::PolicySettingsInitInterface
{
public:
	static const unsigned int TYPE = 555;
public:
	PolicySettingsTest()
		: m_settings { PolicyEndpointSettings{ TYPE , "TEST" , "test", nullptr } }
	{
	}
	unsigned int GetPolicyEndpointSettings(const PolicyEndpointSettings *&p) const noexcept override
	{
		p = m_settings;
		return sizeof(m_settings) / sizeof(*m_settings);
	}
private:
	PolicyEndpointSettings m_settings[1];
};

static inline const VS_Policy::PolicySettingsInitInterface *make_policy_settings()
{
	static PolicySettingsTest settings{};
	return &settings;
}

struct VS_PolicyExtends final : public VS_Policy
{
	template<typename ...Args>
	VS_PolicyExtends(Args&& ...args) : VS_Policy(std::forward<Args>(args)...) {}
	using VS_Policy::clock;
};

static inline VS_PolicyExtends make_policy(const char* protocol, const VS_Policy::PolicySettingsInitInterface *sett)
{
	return sett != nullptr ? VS_PolicyExtends(protocol, sett) : VS_PolicyExtends(protocol);
}

//====================================================================================

static void TestSimpleBan(int type, const char *protocol, const VS_Policy::PolicySettings &ds, const VS_Policy::PolicySettingsInitInterface *sett = nullptr)
{
	VS_PolicyExtends lp = make_policy(protocol, sett);
	for (unsigned i = 0; i < ds.max_fail_before_ban; i++) {

		ASSERT_TRUE(lp.Request(ip, login, type));
		lp.SetResult(ip, login, false, type);
	}
	ASSERT_FALSE(lp.Request(ip, login, type));
}

TEST_F(PolicyTest, TestSimpleBan)
{
	TestSimpleBan(VS_Policy::DEFAULT_TYPE, "protocol", ds);
}

TEST_F(PolicySettingsInitTest, TestSimpleBan)
{
	TestSimpleBan(PolicySettingsTest::TYPE, "settings_init", ds, make_policy_settings());
}

//====================================================================================

static void TestPolicySimpleBanCb(int type, const char *protocol, const VS_Policy::PolicySettings& ds, const VS_Policy::PolicySettingsInitInterface *sett = nullptr)
{
	VS_PolicyExtends lp = make_policy(protocol, sett);
	for (unsigned i = 0; i < ds.max_fail_before_ban; i++) {
		lp.Request(ip, login,
			[](bool lr) {
				ASSERT_TRUE(lr);
			}, type);
		lp.SetResult(ip, login, false, type);
	}
	lp.Request(ip, login,
		[](bool lr) {
			ASSERT_FALSE(lr);
		}, type);
}

TEST_F(PolicyTest, SimpleBanCb)
{
	TestPolicySimpleBanCb(VS_Policy::DEFAULT_TYPE, "protocol", ds);
}

TEST_F(PolicySettingsInitTest, SimpleBanCb)
{
	TestPolicySimpleBanCb(PolicySettingsTest::TYPE, "settings_init", ds, make_policy_settings());
}

//====================================================================================

static void TestPolicyResetFails(int type, const char *protocol, const VS_Policy::PolicySettings& ds, const VS_Policy::PolicySettingsInitInterface *sett = nullptr)
{
	VS_PolicyExtends lp = make_policy(protocol, sett);
	for (unsigned i = 0; i < ds.max_fail_before_ban / 2; i++) {
		ASSERT_TRUE(lp.Request(ip, login, type));
		lp.SetResult(ip, login, false, type);
	}
	/*after ds.max_fail_before_ban/2 fails still should be ok */
	for (unsigned i = 0; i < 10 * ds.max_fail_before_ban; i++) {

		ASSERT_TRUE(lp.Request(ip, login, type));
		lp.SetResult(ip, login, true, type);
	}
}

TEST_F(PolicyTest, ResetFails)
{
	TestPolicyResetFails(VS_Policy::DEFAULT_TYPE, "protocol", ds);
}

TEST_F(PolicySettingsInitTest, ResetFails)
{
	TestPolicyResetFails(PolicySettingsTest::TYPE, "settings_init", ds, make_policy_settings());
}

//====================================================================================

static void TestPolicyOutOfObserveInterval(int type, const char* protocol, const VS_Policy::PolicySettings& ds, const VS_Policy::PolicySettingsInitInterface *sett = nullptr)
{
	VS_PolicyExtends lp = make_policy(protocol, sett);
	for (unsigned i = 0; i < 10 * ds.max_fail_before_ban; i++) {
		ASSERT_TRUE(lp.Request(ip, login, type));
		lp.SetResult(ip, login, false, type);
		lp.clock().add_diff(ds.observe_interval + std::chrono::seconds(1));
	}
}

TEST_F(PolicyTest, OutOfObserveInterval)
{
	TestPolicyOutOfObserveInterval(VS_Policy::DEFAULT_TYPE, "protocol", ds);
}

TEST_F(PolicySettingsInitTest, OutOfObserveInterval)
{
	TestPolicyOutOfObserveInterval(PolicySettingsTest::TYPE, "settings_init", ds, make_policy_settings());
}

//====================================================================================

static void TestPolicyExpiredBan(int type, const char* protocol, const VS_Policy::PolicySettings& ds, const VS_Policy::PolicySettingsInitInterface *sett = nullptr)
{
	VS_PolicyExtends lp = make_policy(protocol, sett);
	for (unsigned i = 0; i < ds.max_fail_before_ban; i++) {
		ASSERT_TRUE(lp.Request(ip, login, type));
		lp.SetResult(ip, login, false, type);
	}
	ASSERT_FALSE(lp.Request(ip, login, type));
	lp.clock().set_diff(ds.ban_time - std::chrono::seconds(10));
	ASSERT_FALSE(lp.Request(ip, login, type)); /*should be still banned */
	lp.clock().set_diff({}); /* update ban_expire_time with m_diff=0 */
	ASSERT_FALSE(lp.Request(ip, login, type));
	lp.clock().set_diff(ds.ban_time + std::chrono::seconds(1));
	ASSERT_TRUE(lp.Request(ip, login, type));
}

TEST_F(PolicyTest, ExpiredBan)
{
	TestPolicyExpiredBan(VS_Policy::DEFAULT_TYPE, "protocol", ds);
}

TEST_F(PolicySettingsInitTest, ExpiredBan)
{
	TestPolicyExpiredBan(PolicySettingsTest::TYPE, "settings_init", ds, make_policy_settings());
}

//====================================================================================

static void TestPolicyDelay(int type, const char* protocol, const VS_Policy::PolicySettings& ds, const VS_Policy::PolicySettingsInitInterface *sett = nullptr)
{
	/* to have at least one delayed call with true result max_fail_before_delay should be less than max_fail_before_ban
	 otherwise all accumulated delay will be called with false */
	ASSERT_TRUE(ds.max_fail_before_delay < ds.max_fail_before_ban);
	VS_PolicyExtends lp = make_policy(protocol, std::move(sett));
	int delay_call_count = 0;
	for (unsigned i = 0; i < ds.max_fail_before_delay; i++) {
		ASSERT_TRUE(lp.Request(ip, login, type));
		lp.SetResult(ip, login, false, type);
	}
	for (unsigned i = 0; i < (ds.max_fail_before_ban - ds.max_fail_before_delay - 1); i++) {
		lp.Request(ip, login, [&delay_call_count](bool lr) {
			ASSERT_TRUE(lr);
			delay_call_count++;
			}, type);
	}

	ASSERT_EQ(delay_call_count, 0);
	for (unsigned i = 0; i < (ds.max_fail_before_ban - ds.max_fail_before_delay - 1); i++) {

		int prev_delay_call_count = delay_call_count;
		lp.clock().add_diff((i == 0) ? ds.delay_time + std::chrono::seconds(1) : ds.delay_interval + std::chrono::seconds(1));
		lp.Timeout();
		ASSERT_EQ(delay_call_count, prev_delay_call_count + 1);
	}
}

TEST_F(PolicyTest, Delay)
{
	TestPolicyDelay(VS_Policy::DEFAULT_TYPE, "protocol", ds);
}


TEST_F(PolicySettingsInitTest, Delay)
{
	TestPolicyDelay(PolicySettingsTest::TYPE, "settings_init", ds, make_policy_settings());
}

//====================================================================================

static void TestPolicyDelayTTL(int type, const char* protocol, const VS_Policy::PolicySettings& ds, const VS_Policy::PolicySettingsInitInterface *sett = nullptr)
{
	VS_PolicyExtends lp = make_policy(protocol, std::move(sett));
	for (unsigned i = 0; i < ds.max_fail_before_delay; i++) {
		ASSERT_TRUE(lp.Request(ip, login, type));
		lp.SetResult(ip, login, false, type);
	}
	lp.Request(ip, login, [](bool lr) {
		ASSERT_FALSE(lr);
		}, type);
	lp.clock().add_diff(ds.delay_ttl + std::chrono::seconds(1));
	lp.Timeout();
}


TEST_F(PolicyTest, DelayTTL)
{
	TestPolicyDelayTTL(VS_Policy::DEFAULT_TYPE, "protocol", ds);
}


TEST_F(PolicySettingsInitTest, DelayTTL)
{
	TestPolicyDelayTTL(PolicySettingsTest::TYPE, "settings_init", ds, make_policy_settings());
}

//====================================================================================
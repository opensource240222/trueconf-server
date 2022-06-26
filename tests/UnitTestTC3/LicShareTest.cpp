#include "tests/common/GMockOverride.h"
#include "../LicenseLib/VS_LicenseSharer.h"
#include "AppServer/Services/VS_Storage.h"
#include "../common/std/cpplib/VS_RegistryConst.h"
#include "../common/std/cpplib/VS_RegistryKey.h"
#include "std-generic/compat/chrono.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace tc3_test
{
VS_License MakeSimpleLic(int resourses_count) {
	VS_License lic;
	lic.m_id = 1;
	lic.m_onlineusers = resourses_count;
	lic.m_terminal_pro_users = resourses_count;
	lic.m_gateways = resourses_count;
	lic.m_max_guests = resourses_count;
	lic.m_restrict |= VS_License::ENTERPRISE_SLAVE;
	lic.m_restrict |= VS_License::ENTERPRISE_MASTER;
	lic.m_error = 0;
	return lic;
}

struct StorageMock : public VS_Storage {
	StorageMock(const std::string& our_endpoint) :VS_Storage(our_endpoint.c_str()) {}
	MOCK_METHOD0_OVERRIDE(CountOnlineUsers, int());
	MOCK_METHOD0_OVERRIDE(CountGroupConferences, int());
	MOCK_METHOD0_OVERRIDE(CountOnlineGateways, int());
	MOCK_METHOD0_OVERRIDE(CountOnlineGuests, int());
	MOCK_METHOD0_OVERRIDE(CountOnlineTerminalPro, int());
};

struct LicenseWrapMock : public VS_LicensesWrap {
	MOCK_METHOD1_OVERRIDE( ShareMyLicense, const VS_License(const VS_License& to_share));
	MOCK_METHOD1_OVERRIDE( ReturnBackSharedResourses, void(const VS_License& what_was_shared));
	MOCK_METHOD1_OVERRIDE( AddSharedLicense, void(const VS_License& lic));
	MOCK_CONST_METHOD0_OVERRIDE( IsMaster, bool());
	MOCK_CONST_METHOD0_OVERRIDE( IsSlave, bool());
	MOCK_CONST_METHOD0_OVERRIDE( GetSharedLicSum, const VS_License&());
	MOCK_METHOD0_OVERRIDE(ClearSharedLicense, void());

	void AddSharedLicenseReal(VS_License& lic) {
		VS_LicensesWrap::AddSharedLicense(lic);
	}
};

struct TransportHelperFake : public VS_TransportRouterServiceHelper {
	bool PostRequest(const char* to_server, const char* to_user, const VS_Container &cnt,
		const char* add_string=0,const char* to_service=0,
		unsigned timeout=default_timeout,
		const char* from_service=0, const char* from_user=0){ tmp_cnt = cnt; return true;}

	bool	Init(const char *our_endpoint,
		const char *our_service,
		const bool permittedAll = false) {return true;}
	const char* OurEndpoint() const override { return g_tr_endpoint_name.c_str(); }

	static VS_Container tmp_cnt;
};

VS_Container TransportHelperFake::tmp_cnt;

struct SharedLicTestInitializer {
	void SetUpEnv() {
		g_storage = new StorageMock(g_tr_endpoint_name);
		p_licWrap = new LicenseWrapMock;

		master.SetTransport(&transport);
		slave.SetTransport(&transport);
	}

	void TearDownEnv() {
		delete g_storage; g_storage = nullptr;
		delete p_licWrap; p_licWrap = nullptr;

		VS_RegistryKey k(false, "", false);
		k.RemoveKey(SHARED_LIC_KEY_NAME);
	}

	bool AddLicense(VS_Container& cnt, const VS_License & l) {
		size_t licBuffLen(0);
		std::unique_ptr<uint8_t[]> licBuff(nullptr);
		l.Serialize(licBuff, licBuffLen);
		if (licBuffLen <= 0) {
			return false;
		}

		return cnt.AddValue(LICENSE_TAG, licBuff.get(), licBuffLen);
	}

	void SetMyUsedResourses(const int count) {
		using ::testing::Return;

		auto storage_mock = static_cast<StorageMock*>(g_storage);
		ON_CALL(*storage_mock, CountOnlineUsers()).WillByDefault(Return(count));
		ON_CALL(*storage_mock, CountGroupConferences()).WillByDefault(Return(count));
		ON_CALL(*storage_mock, CountOnlineGateways()).WillByDefault(Return(count));
		ON_CALL(*storage_mock, CountOnlineGuests()).WillByDefault(Return(count));
		ON_CALL(*storage_mock, CountOnlineTerminalPro()).WillByDefault(Return(count));
	}

	TransportHelperFake transport;
	lic::Sharer master;
	lic::Sharer slave;
};

struct SharedLicenseTest : public SharedLicTestInitializer, public ::testing::Test{
	void SetUp() override {
		SharedLicTestInitializer::SetUpEnv();
	}
	void TearDown() override {
		SharedLicTestInitializer::TearDownEnv();
	}

	void SetAllowAnySlave(bool allow) {
		VS_RegistryKey shared_lic(false, SHARED_LIC_KEY_NAME,false,true);

		int32_t allow_any_slave = allow ? 1 : 0;
		shared_lic.SetValue(&allow_any_slave, sizeof(allow_any_slave), VS_REG_INTEGER_VT, ALLOW_ANY_SLAVE_TAG);
	}

	std::map<int, lic::Sharer::ReturnLicenseReq>& GetLicenseOverhead() {
		return slave.m_licOverhead;
	}
	const std::string slave_server = "slave";
	const std::string master_server = "master";
};

TEST_F(SharedLicenseTest,SuppressCheckOWhenLicenseNotSyncronized) {
	using ::testing::Return;

	auto pLicWrap = static_cast<LicenseWrapMock*>(p_licWrap);
	ON_CALL(*pLicWrap, IsSlave()).WillByDefault(Return(true));
	slave.Init();

	auto lic = MakeSimpleLic(10);
	slave.RequestLicense(lic, master_server);
	EXPECT_TRUE(transport.tmp_cnt.IsValid());	// request is sent

	transport.tmp_cnt.Clear();
	slave.SendSharedLicenseCheck(master_server);
	EXPECT_FALSE(transport.tmp_cnt.IsValid());	// request is not sent
}

TEST_F(SharedLicenseTest, SendCheckOnlyWhenLicensesSyncronized) {
	using ::testing::Return;
	using ::testing::ReturnRef;
	using ::testing::_;

	auto pLicWrap = static_cast<LicenseWrapMock*>(p_licWrap);
	ON_CALL(*pLicWrap, IsSlave()).WillByDefault(Return(true));
	ON_CALL(*pLicWrap, IsMaster()).WillByDefault(Return(true));

	slave.Init();
	master.Init();

	auto lic = MakeSimpleLic(10);
	slave.RequestLicense(lic, master_server);
	EXPECT_TRUE(transport.tmp_cnt.IsValid());	// request is sent

	ON_CALL(*pLicWrap, ShareMyLicense(_)).WillByDefault(Return(lic));
	ON_CALL(*pLicWrap, GetSharedLicSum()).WillByDefault(ReturnRef(lic));
	master.ProcessShareLicenseRequest(TransportHelperFake::tmp_cnt, slave_server);
	uint64_t licID(0);
	slave.ReceiveLicenseShare(TransportHelperFake::tmp_cnt, licID);

	transport.tmp_cnt.Clear();
	slave.SendSharedLicenseCheck(master_server);
	EXPECT_TRUE(transport.tmp_cnt.IsValid());	// request is sent
}

TEST_F(SharedLicenseTest, ShareResources) {
	using ::testing::Return;
	using ::testing::_;
	using ::testing::ReturnRef;
	using ::testing::AtLeast;

	auto pLicWrap = static_cast<LicenseWrapMock*>(p_licWrap);
	ON_CALL(*pLicWrap, IsSlave()).WillByDefault(Return(true));
	ON_CALL(*pLicWrap, IsMaster()).WillByDefault(Return(true));

	slave.Init();
	master.Init();
	SetAllowAnySlave(true);

	auto lic = MakeSimpleLic(10);
	slave.RequestLicense(lic, master_server);

	EXPECT_CALL(*pLicWrap, ShareMyLicense(_)).Times(AtLeast(2));
	ON_CALL(*pLicWrap, ShareMyLicense(_)).WillByDefault(Return(lic));
	master.ProcessShareLicenseRequest(TransportHelperFake::tmp_cnt, slave_server);
	master.RestoreSharedLicenses();

	// verify we have saved information in registry
	std::string sh_lic_key_name = SHARED_LIC_KEY_NAME; sh_lic_key_name += "\\"; sh_lic_key_name += slave_server;
	VS_RegistryKey sh_lic_key(false, sh_lic_key_name);
	ASSERT_TRUE(sh_lic_key.IsValid());
	EXPECT_TRUE(sh_lic_key.HasValue(LICENSE_TAG));

	// verify master aswered positively
	lic::ShareResult result = lic::ShareResult::NO_SHARED;
	TransportHelperFake::tmp_cnt.GetValueI32(RESULT_PARAM, result);
	EXPECT_EQ(result, lic::ShareResult::OK);

	EXPECT_CALL(*pLicWrap, AddSharedLicense(_)).Times(AtLeast(2));
	ON_CALL(*pLicWrap, GetSharedLicSum()).WillByDefault(ReturnRef(lic));
	uint64_t licID(0);
	slave.ReceiveLicenseShare(TransportHelperFake::tmp_cnt, licID);
	slave.RestoreSharedLicenses();

	//verify slave saved license to registry too
	VS_RegistryKey slavelic_key(false, SHARED_LIC_KEY_NAME);
	ASSERT_TRUE(slavelic_key.IsValid());
	EXPECT_TRUE(slavelic_key.HasValue(LICENSE_TAG));
}

TEST_F(SharedLicenseTest, LicenseCheck) {
	using ::testing::Return;
	using ::testing::ReturnRef;
	using ::testing::_;
	auto pLicWrap = static_cast<LicenseWrapMock*>(p_licWrap);
	ON_CALL(*pLicWrap, IsMaster()).WillByDefault(Return(true));
	ON_CALL(*pLicWrap, IsSlave()).WillByDefault(Return(true));
	SetAllowAnySlave(true);

	auto lic = MakeSimpleLic(10);
	slave.RequestLicense(lic, master_server);	// send request -> tmp_cnt
	ON_CALL(*pLicWrap, GetSharedLicSum()).WillByDefault(ReturnRef(lic));

	ON_CALL(*pLicWrap, ShareMyLicense(_)).WillByDefault(Return(lic));
	master.ProcessShareLicenseRequest(TransportHelperFake::tmp_cnt, slave_server);
	uint64_t licID(0);
	slave.ReceiveLicenseShare(TransportHelperFake::tmp_cnt, licID);

	slave.SendSharedLicenseCheck(master_server);
	master.ReceiveSharedLicenseCheck(TransportHelperFake::tmp_cnt, slave_server);

	auto check_res = lic::LicenseCheckStatus::not_allowed;
	TransportHelperFake::tmp_cnt.GetValueI32(RESULT_PARAM, check_res);
	EXPECT_EQ(check_res, lic::LicenseCheckStatus::allowed);

	EXPECT_CALL(*pLicWrap, ClearSharedLicense()).Times(0);
	VS_License to_free;
	slave.ReceiveSharedLicenseCheckResponse(TransportHelperFake::tmp_cnt, to_free);
}

TEST_F(SharedLicenseTest, LicWrapTest_RequestedLessThanAvailable) {
	SetMyUsedResourses(10);

	const int what_i_have = 100;
	const int what_must_share = 50;
	VS_License start = MakeSimpleLic(what_i_have);
	VS_LicensesWrap wr;
	wr.MergeLicense(start);

	VS_License to_share = MakeSimpleLic(what_must_share);
	auto was_shared = wr.ShareMyLicense(to_share);
	EXPECT_TRUE(was_shared.CompareCountableResources(to_share));

	auto my_sum = wr.GetMyLicSum();
	EXPECT_EQ(my_sum.m_onlineusers, what_i_have - what_must_share);
	EXPECT_EQ(my_sum.m_terminal_pro_users, what_i_have - what_must_share);
	EXPECT_EQ(my_sum.m_gateways, what_i_have - what_must_share);
	EXPECT_EQ(my_sum.m_max_guests, what_i_have - what_must_share);

	wr.ReturnBackSharedResourses(to_share);
	auto new_sum = wr.GetMyLicSum();
	EXPECT_EQ(new_sum.m_onlineusers, what_i_have);
	EXPECT_EQ(new_sum.m_terminal_pro_users, what_i_have);
	EXPECT_EQ(new_sum.m_gateways, what_i_have);
	EXPECT_EQ(new_sum.m_max_guests, what_i_have);
}

TEST_F(SharedLicenseTest, LicWrapTest_RequestedMoreThanAvailable) {
	const int what_i_use = 50;
	SetMyUsedResourses(what_i_use);

	const int what_i_have = 100;
	VS_License start = MakeSimpleLic(what_i_have);
	VS_LicensesWrap wr;
	wr.MergeLicense(start);

	const int expected_to_share = what_i_have - what_i_use;

	for (const auto what_must_share : {60, VS_License::TC_INFINITY})
	{
		VS_License to_share = MakeSimpleLic(what_must_share);
		auto was_shared = wr.ShareMyLicense(to_share);
		EXPECT_EQ(was_shared.m_onlineusers, expected_to_share);
		EXPECT_EQ(was_shared.m_terminal_pro_users, expected_to_share);
		EXPECT_EQ(was_shared.m_gateways, expected_to_share);
		EXPECT_EQ(was_shared.m_max_guests, expected_to_share);

		const int expected_to_have = what_i_have - expected_to_share;
		auto my_sum = wr.GetMyLicSum();
		EXPECT_EQ(my_sum.m_onlineusers, expected_to_have);
		EXPECT_EQ(my_sum.m_terminal_pro_users, expected_to_have);
		EXPECT_EQ(my_sum.m_gateways, expected_to_have);
		EXPECT_EQ(my_sum.m_max_guests, expected_to_have);

		wr.ReturnBackSharedResourses(was_shared);
	}
}

TEST_F(SharedLicenseTest, LicWrapTest_ReciveSharedLicenseAndClear) {
	const int what_i_have = 100;
	const int what_i_received = 10;
	VS_License start = MakeSimpleLic(what_i_have);
	VS_LicensesWrap wr;
	wr.MergeLicense(start);

	wr.AddSharedLicense(MakeSimpleLic(what_i_received));
	auto shared_lic = wr.GetSharedLicSum();
	EXPECT_EQ(shared_lic.m_onlineusers, what_i_received);
	EXPECT_EQ(shared_lic.m_terminal_pro_users, what_i_received);
	EXPECT_EQ(shared_lic.m_gateways, what_i_received);
	EXPECT_EQ(shared_lic.m_max_guests, what_i_received);

	auto lic_sum = wr.GetLicSum();
	EXPECT_EQ(lic_sum.m_onlineusers, what_i_received + what_i_have);
	EXPECT_EQ(lic_sum.m_terminal_pro_users, what_i_received + what_i_have);
	EXPECT_EQ(lic_sum.m_gateways, what_i_received + what_i_have);
	EXPECT_EQ(lic_sum.m_max_guests, what_i_received + what_i_have);

	wr.ClearSharedLicense();
	auto new_shared_lic = wr.GetSharedLicSum();
	EXPECT_EQ(new_shared_lic.m_onlineusers, 0);
	EXPECT_EQ(new_shared_lic.m_terminal_pro_users, 0);
	EXPECT_EQ(new_shared_lic.m_gateways, 0);
	EXPECT_EQ(new_shared_lic.m_max_guests, 0);

	auto new_lic_sum = wr.GetLicSum();
	EXPECT_EQ(new_lic_sum.m_onlineusers, what_i_have);
	EXPECT_EQ(new_lic_sum.m_terminal_pro_users, what_i_have);
	EXPECT_EQ(new_lic_sum.m_gateways, what_i_have);
	EXPECT_EQ(new_lic_sum.m_max_guests, what_i_have);
}

TEST_F(SharedLicenseTest, ReturnLicenseTimeout) {
	using ::testing::AllOf;
	using ::testing::Field;
	using ::testing::Return;
	using ::testing::ReturnRef;

	auto pLicWrap = static_cast<LicenseWrapMock*>(p_licWrap);
	ON_CALL(*pLicWrap, IsSlave()).WillByDefault(Return(true));

	// shared 10 items, but we use only 5 => must free 5
	VS_License shared = MakeSimpleLic(10);
	pLicWrap->AddSharedLicenseReal(shared);
	ON_CALL(*pLicWrap, GetSharedLicSum()).WillByDefault(ReturnRef(shared));
	SetMyUsedResourses(5);

	// 1. Try to return 5 licenses to master
	slave.ReturnLicenseOverhead("master");
	auto& returnLicReqs = GetLicenseOverhead();
	ASSERT_EQ(returnLicReqs.size(),1u);

	// 2. Verify that we really try to free 5 licenses. Set req time back => master is not answering
	auto& licReq = returnLicReqs.begin()->second;
	EXPECT_EQ(licReq.lic.m_onlineusers, 5);
	licReq.reqTime = std::chrono::steady_clock::now() - std::chrono::seconds(65);

	// 3. Expect request to be deleted and shared license added back again
	EXPECT_CALL(*pLicWrap, AddSharedLicense(AllOf(
		Field(&VS_License::m_onlineusers, 5),
		Field(&VS_License::m_terminal_pro_users, 5),
		Field(&VS_License::m_max_guests, 5),
		Field(&VS_License::m_gateways, 5)
	))).Times(1);
	slave.ObserveLicenseOverhead();
	EXPECT_TRUE(returnLicReqs.empty());
}

struct TestData {
	int sharedForMe = 0;
	int usedByMe = 0;
	int mustFree = 0;
	int expectedOverhead = 0;
};

const TestData data[] = {
	{5, 2, 4, 1},
	{5, 5, 5, 5},
	{0, 0, 0, 0},
	{2, 2, 1, 1},
	{5, 2, VS_License::TC_INFINITY, 2},
	{VS_License::TC_INFINITY, 100, VS_License::TC_INFINITY, 0},
	{100, 100, VS_License::TC_INFINITY, 100},
};

class OverheadTest : public SharedLicTestInitializer, public ::testing::TestWithParam<TestData>
{
	void SetUp() override {
		SharedLicTestInitializer::SetUpEnv();
	}
	void TearDown() override {
		SharedLicTestInitializer::TearDownEnv();
	}
};

TEST_P(OverheadTest, Calculate) {
	using ::testing::Return;
	using ::testing::ReturnRef;

	// 0. Prepare test data
	auto testData = GetParam();
	VS_License shared = MakeSimpleLic(testData.sharedForMe);

	auto pLicWrap = static_cast<LicenseWrapMock*>(p_licWrap);
	ON_CALL(*pLicWrap, IsSlave()).WillByDefault(Return(true));
	ON_CALL(*pLicWrap, IsMaster()).WillByDefault(Return(true));
	ON_CALL(*pLicWrap, GetSharedLicSum()).WillByDefault(ReturnRef(shared));
	pLicWrap->AddSharedLicenseReal(shared);

	SetMyUsedResourses(testData.usedByMe);
	VS_License toFree = MakeSimpleLic(testData.mustFree);

	// 1. Send command to free license by force. master->slave
	master.ReturnLicense("slave", toFree);

	// 2. Receive command in tmp_cnt. Slave executing command
	EXPECT_TRUE(lic::Sharer::GetLicense(TransportHelperFake::tmp_cnt, "", toFree));
	slave.ReturnSharedResoursesToMaster(toFree, "master");

	// 3. Here must be step where slave sends to master request to free license
	//    but we skip it for simplicity

	// 4. Master acknowledges the request (just copy slaves cnt here, due to step 3)
	VS_Container fromMaster = TransportHelperFake::tmp_cnt;
	fromMaster.AddValueI32(RESULT_PARAM, lic::ShareResult::OK);

	// 5. Recive ack from master and calculate overhead
	VS_License overhead;
	slave.ReceiveReturnedSharedResourcesResp(fromMaster, overhead);

	// 6. Verify results
	EXPECT_EQ(overhead.m_onlineusers, testData.expectedOverhead);
	EXPECT_EQ(overhead.m_terminal_pro_users, testData.expectedOverhead);
	EXPECT_EQ(overhead.m_gateways, testData.expectedOverhead);
	EXPECT_EQ(overhead.m_max_guests, testData.expectedOverhead);
}

INSTANTIATE_TEST_CASE_P(Overhead,
	OverheadTest,
	::testing::ValuesIn(data));

TEST(License, Serializing) {
	VS_License lic;
	lic.m_onlineusers = 10;
	lic.m_terminal_pro_users = 20;
	lic.m_gateways = 30;
	lic.m_max_guests = 40;
	lic.m_restrict |= (VS_License::ENTERPRISE_SLAVE | VS_License::ENTERPRISE_MASTER);
	lic.m_validafter = vs::chrono::round<std::chrono::seconds>(std::chrono::system_clock::now());
	lic.m_validuntil = lic.m_validafter + std::chrono::hours(24);
	lic.m_serverName = "ServerName";
	lic.m_hw_md5 = std::string(32, 'U').c_str();
	lic.m_error = 0;

	std::unique_ptr<uint8_t[]> serializedLic;
	size_t len(0);
	EXPECT_TRUE(lic.Serialize(serializedLic, len));
	EXPECT_GT(len, 0);

	VS_License lic1;
	EXPECT_TRUE(lic1.Deserialize(serializedLic.get(), len));
	EXPECT_EQ(lic.m_onlineusers, lic1.m_onlineusers);
	EXPECT_EQ(lic.m_terminal_pro_users, lic1.m_terminal_pro_users);
	EXPECT_EQ(lic.m_gateways, lic1.m_gateways);
	EXPECT_EQ(lic.m_max_guests, lic1.m_max_guests);
	EXPECT_EQ(lic.m_restrict, lic1.m_restrict);
	EXPECT_EQ(lic.m_validafter, lic1.m_validafter);
	EXPECT_EQ(lic.m_validuntil, lic1.m_validuntil);
	EXPECT_STREQ(lic.m_serverName.m_str, lic1.m_serverName.m_str);
	EXPECT_STREQ(lic.m_hw_md5.m_str, lic1.m_hw_md5.m_str);
	EXPECT_EQ(lic.m_error, lic1.m_error);
}

}
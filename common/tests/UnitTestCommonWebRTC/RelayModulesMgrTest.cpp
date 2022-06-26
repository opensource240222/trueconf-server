#include "TransceiverLib/VS_RelayModulesMgr.h"
#include "TransceiverLib/VS_RelayModule.h"

#include <gtest/gtest.h>

namespace test_relay_mod_mgr {

class DummyRelayModule : public VS_RelayModule
{
public:
	explicit DummyRelayModule(const char* name) : VS_RelayModule(name) {}
	bool ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase>&) override { return false; }
};

class RelayModulesMgr : public VS_RelayModulesMgr
{
public:
	using VS_RelayModulesMgr::GetModule;
};

struct RelayModulesMgrTest : public testing::Test
{
	RelayModulesMgr mgr;
	std::shared_ptr<VS_RelayModule> temp = std::make_shared<DummyRelayModule>("Mod");
	std::shared_ptr<VS_RelayModule> perm = std::make_shared<DummyRelayModule>("Mod");
};

TEST_F(RelayModulesMgrTest, Register)
{
	EXPECT_TRUE(mgr.RegisterModule(temp));
	EXPECT_EQ(temp, mgr.GetModule("Mod"));

	EXPECT_FALSE(mgr.RegisterModule(perm));
	EXPECT_TRUE(mgr.RegisterModule(perm, true));
}

TEST_F(RelayModulesMgrTest, Unregister)
{
	EXPECT_TRUE(mgr.RegisterModule(temp));
	EXPECT_EQ(temp, mgr.GetModule("Mod"));

	mgr.UnregisterModule(temp);
	EXPECT_EQ(nullptr, mgr.GetModule("Mod"));
}

TEST_F(RelayModulesMgrTest, Expired)
{
	EXPECT_TRUE(mgr.RegisterModule(temp));
	EXPECT_EQ(temp, mgr.GetModule("Mod"));

	temp = nullptr;
	EXPECT_EQ(nullptr, mgr.GetModule("Mod"));
}

TEST_F(RelayModulesMgrTest, TemporaryIsPreferred)
{
	EXPECT_TRUE(mgr.RegisterModule(temp));
	EXPECT_TRUE(mgr.RegisterModule(perm, true));
	EXPECT_EQ(temp, mgr.GetModule("Mod"));
}

TEST_F(RelayModulesMgrTest, UnregisterAllTemporaryModules)
{
	EXPECT_TRUE(mgr.RegisterModule(temp));
	EXPECT_TRUE(mgr.RegisterModule(perm, true));
	auto temp2 = std::make_shared<DummyRelayModule>("Mod2");
	EXPECT_TRUE(mgr.RegisterModule(temp2));

	mgr.UnregisterAllTemporaryModules();
	EXPECT_EQ(perm, mgr.GetModule("Mod"));
	EXPECT_EQ(nullptr, mgr.GetModule("Mod2"));
}

}

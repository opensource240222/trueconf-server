#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

#include "../../VCS/Services/VS_ConfMemStorage.h"
#include "../../common/ldap_core/common/VS_RegABStorage.h"
#include "std/cpplib/MakeShared.h"
#include "std-generic/sqlite/CppSQLite3.h"
#include "TrueGateway/clientcontrols/VS_TranscoderLogin.h"

struct VS_TestSimpleStorage : public VS_SimpleStorage {
	VS_TestSimpleStorage()
		: VS_SimpleStorage(vs::MakeShared<VS_TranscoderLogin>())
	{}
	bool FindUser(const vs_user_id &, VS_UserData &, bool) override { return false; }
	bool GetMissedCallMailTemplate(const std::chrono::system_clock::time_point , const char* , std::string& , const char * , std::string& , VS_SimpleStr &, VS_SimpleStr &, std::string& , std::string &) override{ return false; }
	bool GetInviteCallMailTemplate(const std::chrono::system_clock::time_point , const char *, std::string& , const char * , VS_SimpleStr &, VS_SimpleStr &, std::string &, std::string &) override { return false; }
	bool GetMissedNamedConfMailTemplate(const char* , std::string& , const char * , std::string& , VS_SimpleStr &, VS_SimpleStr &, std::string &, std::string &) override { return false; }
	bool GetParticipantLimit(const vs_user_id &, VS_ParticipantDescription::Type, int &, double &, double &) override { return false; }
	int LoginAsUser(const VS_SimpleStr& , const VS_SimpleStr& , const VS_SimpleStr& , VS_SimpleStr& , VS_StorageUserData& , VS_Container& , const VS_ClientType &) override { return 0; }
	bool FetchRights(const VS_StorageUserData &, VS_UserData::UserRights &) override  { return false; }
	bool GetRegGroupUsers(const std::string&, std::shared_ptr<VS_AbCommonMap>&) override { return false; }
	bool Test() override { return false; }
};

struct OfflineChatTest: public ::testing::Test {
	bool SetOfflineChat_4_3_8(const char* from_call_id, const char* to_call_id, const char* body_utf8, const char* from_dn) {
		if (!from_call_id || !to_call_id || !body_utf8 || !from_dn) return false;

		const char * SQLITE_TABLE = "OfflineChatMessages";
		try
		{
			CppSQLite3DB db;
			db.open(db_name);

			if (!db.tableExists(SQLITE_TABLE))
			{
				CppSQLite3Buffer bufSQL;
				bufSQL.format("create table %s(%Q varchar(256), %Q varchar(256), %Q varchar(256), %Q varchar(256), expires varchar(32), %Q varchar(32));",
					SQLITE_TABLE, FROM_PARAM, DISPLAYNAME_PARAM, TO_PARAM, MESSAGE_PARAM, "TimeColumn");

				db.execDML(bufSQL);
			}

			CppSQLite3Buffer bufSQL;
			bufSQL.format("insert into %s values(%Q, %Q, %Q, %Q, datetime('now', 'localtime', '%d days'), strftime('%%s','now'));",
				SQLITE_TABLE, from_call_id, from_dn, to_call_id, body_utf8, OFFLINE_MESS_EXPIRE_DAYS_INIT);

			db.execDML(bufSQL);
		}
		catch (CppSQLite3Exception&)
		{
			return false;
		}

		return true;
	}

	void SetUp() {
		boost::filesystem::remove(db_name);
	}

	void TearDown() {
		boost::filesystem::remove(db_name);
	}

	void VerifyOfflineChat(const VS_Container &received_message) {
		auto rcv_from = received_message.GetStrValueRef(FROM_PARAM);
		auto rcv_from_dn = received_message.GetStrValueRef(DISPLAYNAME_PARAM);
		auto rcv_to = received_message.GetStrValueRef(TO_PARAM);
		auto rcv_body_utf8 = received_message.GetStrValueRef(MESSAGE_PARAM);

		ASSERT_STREQ(rcv_from, from);
		ASSERT_STREQ(fromDisplayName, rcv_from_dn);
		ASSERT_STREQ(to, rcv_to);
		ASSERT_STREQ(message_body, rcv_body_utf8);
	}

	std::shared_ptr<VS_DBStorageInterface> reg_stor = std::make_shared<VS_TestSimpleStorage>();
	const char *from = "user1@127.0.0.1";
	const char *fromDisplayName = "User1DN";
	const char *to = "user2@127.0.0.1";
	const char *message_body = "Hello";
	const char* db_name = "OfflineChatMessages.sqlite";
};

TEST_F(OfflineChatTest, DISABLED_SaveRetrieveOfflineChat) {
	VS_Container cnt;
	cnt.AddValue(FROM_PARAM, from);
	cnt.AddValue(DISPLAYNAME_PARAM, fromDisplayName);
	cnt.AddValue(TO_PARAM, to);
	cnt.AddValue(MESSAGE_PARAM, message_body);

	ASSERT_TRUE(reg_stor->SetOfflineChatMessage(from, to, message_body, fromDisplayName, cnt));

	std::vector<VS_Container> messages;
	auto messages_num = reg_stor->GetOfflineChatMessages(to, messages);
	ASSERT_EQ(messages_num, messages.size());
	ASSERT_EQ(messages.size(), 1);

	VerifyOfflineChat(messages[0]);
}

// bug 42092. Due to difference in "OfflineChatMessages" table definition in server 4.3.8 and 4.3.9
// verify that we able to retrieve offline chat saved by TrueConf Server 4.3.8
TEST_F(OfflineChatTest, DISABLED_Save438Chat_RetrieveOfflineChat) {
	ASSERT_TRUE(SetOfflineChat_4_3_8(from, to, message_body, fromDisplayName));

	std::vector<VS_Container> messages;
	auto messages_num = reg_stor->GetOfflineChatMessages(to, messages);
	ASSERT_EQ(messages_num, messages.size());
	ASSERT_EQ(messages.size(), 1);

	VerifyOfflineChat(messages[0]);
}

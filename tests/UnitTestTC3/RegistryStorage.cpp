#include "tests/common/GMockOverride.h"
#include "../../VCS/Services/VS_RegistryStorage.h"
#include "../../common/ldap_core/common/VS_RegABStorage.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_UserData.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/MakeShared.h"
#include "../../ServerServices/VS_ReadLicense.h"
#include "../../common/std/CallLog/VS_DBCallLogPostgres.h"
#include "../../common/statuslib/VS_ExtendedStatus.h"
#include "TrueGateway/clientcontrols/VS_TranscoderLogin.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/filesystem.hpp>

#include "std-generic/compat/memory.h"

extern std::string g_tr_endpoint_name;

namespace regstorage_test {
	const std::string TEST_LOGIN = "test_login";
	const std::string TEST_DISPLAY_NAME = "test display name";
	const std::string TEST_FIRST_NAME = "test_first_name";
	const std::string TEST_LAST_NAME = "test_last_name";
	const std::string TEST_PASSWORD = "test_passwd";
	const std::string TEST_SHA1_PASSWORD = "sha1_passwd";
	const std::string TEST_H323_PASSWORD = "h323_passwd";
	const std::string TEST_COMPANY = "test_company";
	const std::string TEST_EMAIL = "test@mail.com";
	const std::string TEST_GROUP = "test_group";

	struct DB_CallLogMock : public callLog::Postgres {
		MOCK_METHOD5_OVERRIDE(GetCalls, int(VS_Container& cnt, int& entries, VS_AddressBook ab, const vs_user_id& owner, time_t last_deleted_call));

		MOCK_METHOD2_OVERRIDE(LogConferenceStart, bool(const VS_ConferenceDescription& conf, bool remote));
		MOCK_METHOD1_OVERRIDE(LogConferenceEnd, bool(const VS_ConferenceDescription& conf));
		MOCK_METHOD6_OVERRIDE(LogParticipantInvite, bool(const vs_conf_id& conf_id, const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,
			const std::chrono::system_clock::time_point time, VS_ParticipantDescription::Type type));
		MOCK_METHOD2_OVERRIDE(LogParticipantJoin, bool(const VS_ParticipantDescription& pd, const VS_SimpleStr& callid2));
		MOCK_METHOD4_OVERRIDE(LogParticipantReject, bool(const vs_conf_id& conf_id, const vs_user_id& user, const vs_user_id& invited_from, const VS_Reject_Cause cause));
		MOCK_METHOD1_OVERRIDE(LogParticipantLeave, bool(const VS_ParticipantDescription& pd));
	};

	struct RegistryStorageTest : public ::testing::Test {
		RegistryStorageTest() : our_endpoint(g_tr_endpoint_name){
			full_test_login += TEST_LOGIN;
			full_test_login += "@";
			full_test_login += our_endpoint;
			full_test_login.erase(full_test_login.find('#'));

			p_licWrap = new VS_LicensesWrap;
		}
		~RegistryStorageTest() {
			delete p_licWrap;
		}

		void InitTestRegStructure() {
			// prepare test registry structure
			AddSharedKeys();
			AddGroup(TEST_GROUP);

			AddUser(TEST_LOGIN, TEST_DISPLAY_NAME);
			AddUserToGroup(TEST_LOGIN, TEST_GROUP);

			AddUser("test_user2", TEST_DISPLAY_NAME);
			AddUserToGroup("test_user2", TEST_GROUP);
		}

		void ClearTestRegStructure() {
			std::string old_root(VS_RegistryKey::GetDefaultRoot());

			auto key_to_remove = old_root;
			auto pos = key_to_remove.find_last_of('\\');
			if (pos == std::string::npos) return;
			key_to_remove.erase(pos);

			VS_RegistryKey::SetDefaultRoot("");
			VS_RegistryKey root(false, key_to_remove, false, false);
			root.RemoveKey("Server");

			VS_RegistryKey::SetDefaultRoot(old_root);
		}


		virtual void SetUp() override
		{
			InitTestRegStructure();
			InitRegistryStorage();
			storage->ResetLicenseChecker([](VS_LicenseEvents) {return true; });
			transLogin = vs::MakeShared<VS_TranscoderLogin>();
			boost::system::error_code ec;
			boost::filesystem::remove(VS_SimpleStorage::AVATARS_DIRECTORY,ec);

			SetDBCallLog(vs::make_unique<DB_CallLogMock>());
		}

		virtual void TearDown() override
		{
			boost::system::error_code ec;
			boost::filesystem::remove(VS_SimpleStorage::AVATARS_DIRECTORY,ec);

			storage = nullptr;
			storage_callog = nullptr;
			ClearTestRegStructure();
		}

		bool use_groups = true;
		std::shared_ptr<VS_RegistryStorage> storage = nullptr;
		callLog::Postgres* storage_callog = nullptr;
		std::string our_endpoint;
		std::string full_test_login;
		std::shared_ptr<VS_TranscoderLogin> transLogin;

		void AddSharedKeys() {
			VS_RegistryKey cfg_root(false, CONFIGURATION_KEY, false, true);

			cfg_root.SetString("shared_secret2", VS_SimpleStorage::SHARED_KEY_2);
			cfg_root.SetString("shared_secret3", VS_SimpleStorage::SHARED_KEY_3);
		}

		void AddAutologinData(const string_view user_login) {
			std::string root = USERS_KEY; root += "\\"; root += user_login; root += "\\AutoLogins";
			VS_RegistryKey	user_key(false, root, false, true);
			user_key.SetString("app_id", "md5_hash");
		}

		void AddUserPhones(const VS_StorageUserData& user) {
			std::string phones_key_str = USERS_KEY;
			phones_key_str += "\\"; phones_key_str += user.m_realLogin.GetUser();
			phones_key_str += "\\"; phones_key_str += VS_SimpleStorage::USER_PHONES_TAG;
			phones_key_str += "\\"; phones_key_str += "0";
			char ctr('0');

			for (auto& item : user.m_phones)
			{
				VS_RegistryKey	phones_root(false, phones_key_str, false, true);
				phones_root.SetString(item.phone.m_str, USERPHONE_PARAM);
				phones_root.SetValue(&item.type, sizeof(item.type), VS_REG_INTEGER_VT, TYPE_PARAM);

				phones_key_str.pop_back();
				++ctr;
				phones_key_str.push_back(ctr);
			}
		}

		void AddUserToRegistry(const VS_StorageUserData& user) {
			std::string new_user = USERS_KEY; new_user += "\\"; new_user += user.m_realLogin.GetUser();
			VS_RegistryKey	user_key(false, new_user, false, true);

			user_key.SetString(user.m_FirstName.c_str(), FIRST_NAME_TAG);
			user_key.SetString(user.m_LastName.c_str(), LAST_NAME_TAG);
			user_key.SetString(user.m_displayName.c_str(), DISPLAY_NAME_TAG);
			user_key.SetValue(&user.m_status, sizeof(user.m_status), VS_REG_INTEGER_VT, STATUS_TAG);
			user_key.SetString(user.m_password.m_str, PASSWORD_TAG);
			user_key.SetString(user.m_HA1_password.m_str, HA1_PASSWORD_TAG);
			user_key.SetString(user.m_h323_password.m_str, H323_PASSWORD_TAG_OLD);
			user_key.SetString(user.m_Company.c_str(), COMPANY_TAG);
			user_key.SetValue(&user.m_type, sizeof(user.m_type), VS_REG_INTEGER_VT, TYPE_TAG);
			AddAutologinData(user.m_realLogin.GetUser());
			user_key.SetString(user.m_email.m_str, VS_SimpleStorage::EMAIL_TAG);
			AddUserPhones(user);
		}

		void AddGroup(const string_view group_name) {
			std::string group = GROUPS_KEY; group += "\\"; group += group_name;
			VS_RegistryKey groups_key(false, group, false, true);
			groups_key.SetString(std::string(group_name).c_str(), VS_SimpleStorage::GROUP_NAME_TAG);

			int32_t can_edit_ab(VS_GroupManager::GR_EDIT_GROUP_AB);
			groups_key.SetValue(&can_edit_ab, sizeof(can_edit_ab), VS_REG_INTEGER_VT, RIGHTS_TAG);
		}

		void AddUserToGroup(const string_view user_call_id, const string_view group_name) {
			std::string groups_users = GROUPS_KEY; groups_users += "\\"; groups_users += group_name; groups_users += "\\"; groups_users += USERS_KEY;
			VS_RegistryKey groups_users_key(false, groups_users, false, true);
			groups_users_key.SetString(std::string(user_call_id).c_str(), std::string(user_call_id).c_str());
		}

		VS_StorageUserData MakeSimpleUser(const string_view login, const string_view display_name) {
			VS_StorageUserData ud;
			ud.m_FirstName = TEST_FIRST_NAME;
			ud.m_LastName = TEST_LAST_NAME;
			ud.m_displayName = std::string(display_name);
			ud.m_status = VS_UserData::US_LOGIN;
			ud.m_password = TEST_PASSWORD.c_str();
			ud.m_HA1_password = TEST_SHA1_PASSWORD.c_str();
			ud.m_h323_password = TEST_H323_PASSWORD.c_str();
			ud.m_Company = TEST_COMPANY;
			ud.m_email = TEST_EMAIL.c_str();
			ud.m_type = VS_UserData::UT_PERSON;
			ud.m_realLogin = VS_RealUserLogin(login);
			ud.m_login = std::string(login).c_str();
			return ud;
		}

		void AddUser(const string_view login, const string_view display_name) {
			AddUserToRegistry(MakeSimpleUser(login, display_name));
		}

		void InitRegistryStorage() { storage = std::make_shared<VS_RegistryStorage>(new VS_RegABStorage(), use_groups, our_endpoint.c_str(), transLogin); };
		void SetDBCallLog(std::unique_ptr<callLog::Postgres> &&new_calllog) {
			storage->ResetDBCallLog(std::move(new_calllog));
			storage_callog = storage->GetCallLog();
		}
	};

	TEST_F(RegistryStorageTest, Initiialization) {
		auto reg_storage = std::make_shared<VS_RegistryStorage>(new VS_RegABStorage(), use_groups, our_endpoint.c_str(), transLogin);
		ASSERT_EQ(reg_storage->error_code, 0);
	}

	TEST_F(RegistryStorageTest, SearchUsers) {
		std::string query = "p.last_name LIKE N'"; query += TEST_LAST_NAME; query += "%s";

		VS_Container res;
		EXPECT_EQ(2,storage->SearchUsers(res, query, nullptr));	// two users with TEST_LAST_NAME were added in 'InitTestRegStructure'
	}

	TEST_F(RegistryStorageTest, GetDisplayName) {

		std::string res_dn;
		storage->GetDisplayName(TEST_LOGIN.c_str(), res_dn);
		EXPECT_STREQ(res_dn.c_str(), TEST_DISPLAY_NAME.c_str());
	}

	TEST_F(RegistryStorageTest, MakeLog) {
		using ::testing::_;
		ASSERT_NE(storage_callog, nullptr);

		EXPECT_CALL(*static_cast<DB_CallLogMock*>(storage_callog), LogParticipantLeave(_)).Times(1);
		VS_ParticipantDescription part;
		storage->LogParticipantLeave(part);

		auto pCallog = std::static_pointer_cast<VS_DBStorage_CallLogInterface>(storage);

		EXPECT_CALL(*static_cast<DB_CallLogMock*>(storage_callog), LogConferenceStart(_,_)).Times(1);
		VS_ConferenceDescription conf;
		pCallog->LogConferenceStart(conf);

		EXPECT_CALL(*static_cast<DB_CallLogMock*>(storage_callog), LogConferenceEnd(_)).Times(1);
		pCallog->LogConferenceEnd(conf);

		EXPECT_CALL(*static_cast<DB_CallLogMock*>(storage_callog), LogParticipantInvite(_, _, _, _, _, _)).Times(1);
		pCallog->LogParticipantInvite("conf_id","call_id","app_id","call_id2");

		EXPECT_CALL(*static_cast<DB_CallLogMock*>(storage_callog), LogParticipantJoin(_,_)).Times(1);
		pCallog->LogParticipantJoin(part);

		EXPECT_CALL(*static_cast<DB_CallLogMock*>(storage_callog), LogParticipantReject(_, _, _, _)).Times(1);
		pCallog->LogParticipantReject("conf_id","user", "invited_from", VS_Reject_Cause::CONFERENCE_IS_BUSY);

		EXPECT_CALL(*static_cast<DB_CallLogMock*>(storage_callog), LogParticipantLeave(_)).Times(1);
		pCallog->LogParticipantLeave(part);
	}

	TEST_F(RegistryStorageTest, UpdateStatus) {

		VS_ExtendedStatusStorage dummy;
		int status = 10;
		storage->SetUserStatus(TEST_LOGIN.c_str(), status, dummy, false, "");

		VS_StorageUserData ud;
		storage->FindUser(TEST_LOGIN.c_str(), ud);
		EXPECT_EQ(ud.m_online_status, status);
	}

	TEST_F(RegistryStorageTest, GetGroups) {
		int entries(0);
		VS_Container res, dummy;
		EXPECT_EQ(storage->FindUsers(res, entries, AB_GROUPS, TEST_LOGIN.c_str(), "", 0, dummy), SEARCH_DONE);
		EXPECT_EQ(entries, 1);	// one group was added to registry structure in "InitTestRegStructure" function
	}

	TEST_F(RegistryStorageTest, Calls) {
		using ::testing::_;
		using ::testing::Return;

		ASSERT_NE(storage_callog, nullptr);
		EXPECT_CALL(*static_cast<DB_CallLogMock*>(storage_callog), GetCalls(_, _, _, _, _)).Times(3);
		ON_CALL(*static_cast<DB_CallLogMock*>(storage_callog), GetCalls(_, _, _, _, _)).WillByDefault(Return(SEARCH_DONE));

		int entries(0);
		VS_Container res, dummy;
		EXPECT_EQ(storage->FindUsers(res, entries, AB_MISSED_CALLS, "owner", TEST_LOGIN, 0, dummy), SEARCH_DONE);
		EXPECT_EQ(storage->FindUsers(res, entries, AB_PLACED_CALLS, "owner", TEST_LOGIN, 0, dummy), SEARCH_DONE);
		EXPECT_EQ(storage->FindUsers(res, entries, AB_RECEIVED_CALLS, "owner", TEST_LOGIN, 0, dummy), SEARCH_DONE);
	}

	TEST_F(RegistryStorageTest, BanList) {
		VS_Container in, dummy;
		long hash(0);
		VS_SimpleStr add_call_id;
		std::string add_dn;
		const char banned[] = "test_user2";

		// 1. Add test_user2 to ban list of TEST_LOGIN
		in.AddValue(CALLID_PARAM, banned);
		in.AddValue(DISPLAYNAME_PARAM, TEST_DISPLAY_NAME);
		EXPECT_EQ(std::static_pointer_cast<VS_DBStorageInterface>(storage)->AddToAddressBook(AB_BAN_LIST, TEST_LOGIN.c_str(), in, hash, dummy, add_call_id, add_dn), 0);

		// 2. Get ban list for TEST_LOGIN
		int entries(0);
		VS_Container res;
		EXPECT_EQ(storage->FindUsers(res, entries, AB_BAN_LIST, TEST_LOGIN.c_str(), "", 0, dummy), SEARCH_DONE);
		EXPECT_EQ(entries, 1);

		// 3. Verify expectations
		auto pBanned = res.GetStrValueRef(CALLID_PARAM);
		ASSERT_NE(pBanned, nullptr);
		EXPECT_THAT(pBanned, ::testing::StartsWith(banned));

		auto pBannedDN = res.GetStrValueRef(DISPLAYNAME_PARAM);
		ASSERT_NE(pBannedDN, nullptr);
		EXPECT_STREQ(pBannedDN, TEST_DISPLAY_NAME.c_str());
	}

	TEST_F(RegistryStorageTest, AddressBook) {
		VS_Container in, dummy;
		long hash(0);
		VS_SimpleStr add_call_id;
		std::string add_dn;
		const string_view to_add = "test_user3";

		// 0. Add test_user3 to address book of TEST_LOGIN
		in.AddValue(CALLID_PARAM, to_add);
		in.AddValue(DISPLAYNAME_PARAM, TEST_DISPLAY_NAME);
		EXPECT_EQ(std::static_pointer_cast<VS_DBStorageInterface>(storage)->AddToAddressBook(AB_COMMON, TEST_LOGIN.c_str(), in, hash, dummy, add_call_id, add_dn), 0);

		// 1. Try to add alredy existed user added previously
		EXPECT_EQ(std::static_pointer_cast<VS_DBStorageInterface>(storage)->AddToAddressBook(AB_COMMON, TEST_LOGIN.c_str(), in, hash, dummy, add_call_id, add_dn), VSS_USER_EXISTS);

		// 2. Verify expectations
		VS_Container res;
		int entries(0);
		EXPECT_EQ(storage->FindUsers(res, entries, AB_COMMON, TEST_LOGIN.c_str(), "", 0, dummy), SEARCH_DONE);
		EXPECT_EQ(entries, 2);	// test_user2 was added externally i.e from web (emulate it in RegistryStorageTest contructor) + test_user3 was added in this test

		// 3. Use client hash
		int32_t client_hash(0);
		res.GetValueI32(HASH_PARAM, client_hash);
		EXPECT_EQ(storage->FindUsers(res, entries, AB_COMMON, TEST_LOGIN.c_str(), "", client_hash, dummy), SEARCH_NOT_MODIFIED);
	}

	TEST_F(RegistryStorageTest, FindPersonDetails) {
		VS_Container res, dummy;
		int entries(0);
		EXPECT_EQ(storage->FindUsers(res, entries, AB_PERSON_DETAILS, "owner", TEST_LOGIN, 0, dummy), SEARCH_DONE);
		EXPECT_EQ(entries, 1);
	}

	TEST_F(RegistryStorageTest, UserPhones) {
		VS_Container dummy;
		long hash(0);
		VS_SimpleStr add_call_id;
		std::string add_dn;

		char c = '1';
		for (size_t i = 1; i <= 3; ++i, ++c)
		{
			VS_Container in;
			std::string call_id = "call_id"; call_id += c;
			std::string phone = "+1234567890"; phone += c;
			VS_UserPhoneType type = (VS_UserPhoneType)i;

			in.AddValue(CALLID_PARAM, call_id);
			in.AddValue(USERPHONE_PARAM, phone);
			in.AddValue(TYPE_PARAM, type);
			EXPECT_EQ(std::static_pointer_cast<VS_DBStorageInterface>(storage)->AddToAddressBook(AB_PHONES, TEST_LOGIN.c_str(), in, hash, dummy, add_call_id, add_dn), 0);
		}

		VS_Container res;
		int entries(0);
		EXPECT_EQ(storage->FindUsers(res, entries, AB_PHONES, TEST_LOGIN.c_str(), "" , 0, dummy), SEARCH_DONE);
		EXPECT_EQ(entries, 3);
	}

	TEST_F(RegistryStorageTest, UserPicture) {

		VS_Container in, dummy;
		long hash(0);

		// 1. Store test buffer as .jpg picture
		uint8_t test_buff[] = {0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF};
		const char* av_type = "Image/jpeg";
		in.AddValue("avatar_type", av_type);
		in.AddValue("avatar", test_buff, sizeof(test_buff));
		EXPECT_EQ(std::static_pointer_cast<VS_DBStorageInterface>(storage)->UpdateAddressBook(AB_USER_PICTURE, "", TEST_LOGIN.c_str(), in, hash, dummy), 0);

		// 2. Load .jpg picture
		int entries(0);
		VS_Container res;
		EXPECT_EQ(storage->FindUsers(res, entries, AB_USER_PICTURE, "owner", TEST_LOGIN, 0, dummy), SEARCH_DONE);
		EXPECT_EQ(entries, 1);

		// 3. Compare initial and result values
		const char *res_av_type = res.GetStrValueRef("picturetype");
		EXPECT_STRCASEEQ(res_av_type, av_type);

		size_t res_pic_size(0);
		const uint8_t *picture = static_cast<const uint8_t*>(res.GetBinValueRef("picture", res_pic_size));
		EXPECT_EQ(res_pic_size, sizeof(test_buff));
		ASSERT_NE(picture, nullptr);
		EXPECT_EQ(memcmp(test_buff, picture, res_pic_size),0);

		// 4. Verify with client hash
		EXPECT_EQ(storage->FindUsers(res, entries, AB_USER_PICTURE, "owner", TEST_LOGIN, hash, dummy), SEARCH_NOT_MODIFIED);
	}

	TEST_F(RegistryStorageTest, FindUser) {

		VS_StorageUserData ud;
		EXPECT_TRUE(storage->FindUser(TEST_LOGIN.c_str(), ud));
		EXPECT_STREQ(ud.m_displayName.c_str(), TEST_DISPLAY_NAME.c_str());
		EXPECT_STREQ(ud.m_FirstName.c_str(), TEST_FIRST_NAME.c_str());
		EXPECT_STREQ(ud.m_LastName.c_str(), TEST_LAST_NAME.c_str());
		EXPECT_EQ(ud.m_status, VS_UserData::US_LOGIN);
		EXPECT_STREQ(ud.m_password.m_str, TEST_PASSWORD.c_str());
		EXPECT_STREQ(ud.m_HA1_password.m_str, TEST_SHA1_PASSWORD.c_str());
		EXPECT_STREQ(ud.m_h323_password.m_str, TEST_H323_PASSWORD.c_str());
		EXPECT_STREQ(ud.m_Company.c_str(), TEST_COMPANY.c_str());
		EXPECT_STREQ(ud.m_email.m_str, TEST_EMAIL.c_str());
		EXPECT_EQ(ud.m_type,VS_UserData::UT_PERSON);
		EXPECT_STREQ(ud.m_realLogin.GetUser().c_str(), TEST_LOGIN.c_str());
		EXPECT_STREQ(ud.m_login.m_str, full_test_login.c_str());

		ASSERT_GT(ud.m_groups.size(), static_cast<size_t>(0));
		EXPECT_STREQ(ud.m_groups[0].c_str(), TEST_GROUP.c_str());
	}

	TEST_F(RegistryStorageTest, TryFindWrongID) {
		VS_StorageUserData ud;
		EXPECT_FALSE(storage->FindUser(nullptr, ud));
		EXPECT_EQ(storage->error_code, VSS_USER_NOT_VALID);
	}

	TEST_F(RegistryStorageTest, UserNotFound) {
		VS_StorageUserData ud;
		EXPECT_FALSE(storage->FindUser("not_existed_user", ud));
		EXPECT_EQ(storage->error_code, VSS_USER_NOT_FOUND);
	}

	TEST_F(RegistryStorageTest, Login) {
		VS_SimpleStr app_server,auto_key;
		VS_StorageUserData user;
		VS_Container props;

		EXPECT_EQ(storage->LoginAsUser(TEST_LOGIN.c_str(), TEST_PASSWORD.c_str(), "appID", auto_key, user, props), USER_LOGGEDIN_OK);
		EXPECT_STREQ(user.m_name.m_str, full_test_login.c_str());

		// verify autologin
		ASSERT_TRUE(auto_key.Length() > 0);
		EXPECT_EQ(storage->LoginAsUser(TEST_LOGIN.c_str(), nullptr, "appID", auto_key, user, props), USER_LOGGEDIN_OK);
	}
}
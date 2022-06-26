#include <gtest/gtest.h>
#include "std/VS_RegistryPasswordEncryption.h"

TEST(RegistryPasswordEncryption, EncryptionAndDecryptionOfRegistryPassword)
{
	std::string test_user{ "some_user123" };

	std::string test_password{ "some_password" };
	std::string test_password1{ "" };
	std::string test_password2{ "321-13-21-31-23-312" };

	std::string encrypted_test_password = sec::EncryptRegistryPassword(test_user, test_password);
	std::string encrypted_test_password1 = sec::EncryptRegistryPassword(test_user, test_password1);
	std::string encrypted_test_password2 = sec::EncryptRegistryPassword(test_user, test_password2);

	EXPECT_STREQ(test_password.c_str(), sec::DecryptRegistryPassword(test_user, encrypted_test_password).c_str());
	EXPECT_STREQ(test_password1.c_str(), sec::DecryptRegistryPassword(test_user, encrypted_test_password1).c_str());
	EXPECT_STREQ(test_password2.c_str(), sec::DecryptRegistryPassword(test_user, encrypted_test_password2).c_str());
}

TEST(RegistryPasswordEncryption, DecryptionOfRegistryPassword)
{
	std::string test_user{ "some_user123" };

	std::string test_password{ "some_password" };
	std::string test_password1{ "" };
	std::string test_password2{ "321-13-21-31-23-312" };

	std::string test_password_encrypted{ "v2*1575033886*jTOPChn746zwcA31W1ndJw==" };
	std::string test_password1_encrypted{ "v2*1575033886*2q7X2At9DqOWvnMhSdPoqg==" };
	std::string test_password2_encrypted{ "v2*1575033887*XMiAzKi+Rh/DO5/05VlFbAi7WlJTx2nTiDjliBRDcbQ=" };

	EXPECT_STREQ(test_password.c_str(), sec::DecryptRegistryPassword(test_user, test_password_encrypted).c_str());
	EXPECT_STREQ(test_password1.c_str(), sec::DecryptRegistryPassword(test_user, test_password1_encrypted).c_str());
	EXPECT_STREQ(test_password2.c_str(), sec::DecryptRegistryPassword(test_user, test_password2_encrypted).c_str());
}

#include "VS_RegistryPasswordEncryption.h"

#include "std/cpplib/base64.h"
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <boost/algorithm/string.hpp>
#include <ctime>
#include <vector>

namespace {
        // Random string for H.323 and LDAP password AES CBC encode
	const char REGISTRY_PASSWORD_HARDCODED_STRING[] = "Eduard`s really cool string";

	std::string MakeAESKeyForRegistryPassword(string_view user_name, string_view timestamp)
	{
		std::string long_symmetric_key;
		long_symmetric_key.reserve(timestamp.size() + user_name.size() + std::strlen(REGISTRY_PASSWORD_HARDCODED_STRING));
		long_symmetric_key += timestamp;
		long_symmetric_key += user_name;
		long_symmetric_key += REGISTRY_PASSWORD_HARDCODED_STRING;

		std::string symmetric_key_hash; //SHA256 of long_symmetric_key
		symmetric_key_hash.resize(SHA256_DIGEST_LENGTH);
		(void)SHA256(reinterpret_cast<unsigned char *>(&long_symmetric_key[0]), long_symmetric_key.size(), reinterpret_cast<unsigned char *>(&symmetric_key_hash[0]));
		return symmetric_key_hash;
	}

	template<class Function, class T>
	std::string DoTransformation(Function fun, T* input, std::size_t input_size, std::size_t& output_size)
	{
		output_size = 0;
		fun(input, input_size, nullptr, output_size);
		if (output_size == 0)
			return {};
		std::string output;
		output.resize(output_size);
		if (!fun(input, input_size, reinterpret_cast<typename std::remove_const<T>::type*>(&output[0]), output_size))
			return {};
		output.resize(output_size);
		return output;
	}

	template <int (&InitFun)(EVP_CIPHER_CTX *ctx,
		        const EVP_CIPHER *cipher, ENGINE *impl,
		        const unsigned char *key,
		        const unsigned char *iv),
	          int (&UpdateFun)(EVP_CIPHER_CTX *ctx, unsigned char *out,
				int *outl, const unsigned char *in, int inl),
	          int (&FinalFun)(EVP_CIPHER_CTX *ctx, unsigned char *outm,
			    int *outl)>
	std::string DoEncryption(string_view password, string_view key)
	{
		std::string res;
		int encrypted_len = password.size() + /*EVP_MAX_BLOCK_LENGTH*/64;

		res.resize(encrypted_len);
		EVP_CIPHER_CTX *ctx;

		if (!(ctx = EVP_CIPHER_CTX_new()))
			return {};

		if (1 != InitFun(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char*>(key.data()), nullptr))
			return {};

		int tmp_len = 0;
		if (1 != UpdateFun(ctx, reinterpret_cast<unsigned char*>(&res[0]), &tmp_len, reinterpret_cast<const unsigned char*>(password.data()), password.size()))
			return {};
		encrypted_len = tmp_len;

		if (1 != FinalFun(ctx, reinterpret_cast<unsigned char*>(&res[encrypted_len]), &tmp_len))
			return {};
		encrypted_len += tmp_len;
		res.resize(encrypted_len);

		EVP_CIPHER_CTX_free(ctx);

		return res;
	}
} //anonymous namespace

namespace sec {
	std::string EncryptRegistryPassword(string_view user_name, string_view plaintext_password)
	{
		const std::string timestamp_str = std::to_string(std::time(nullptr));
		const std::string symmetric_key = MakeAESKeyForRegistryPassword(user_name, timestamp_str);

		std::string encoded_password = DoEncryption<EVP_EncryptInit_ex, EVP_EncryptUpdate, EVP_EncryptFinal_ex>(plaintext_password, symmetric_key);
		if (encoded_password.empty())
			return {};

		std::size_t base64ed_encoded_pass_size;
		std::string base64ed_password = DoTransformation(base64_encode, encoded_password.data(), encoded_password.size(), base64ed_encoded_pass_size);
		if (base64ed_password.empty())
			return {};

		std::string result;
		result.reserve(sizeof("v2**") - 1 + timestamp_str.size() + base64ed_encoded_pass_size);
		result += "v2*";
		result += timestamp_str;
		result += '*';
		result += base64ed_password;

		return result;
	}

	std::string DecryptRegistryPassword(string_view user_name, string_view password_from_registry)
	{
		std::vector<std::string> tokens;
		boost::split(tokens, password_from_registry, boost::is_any_of("*"));

		if (tokens.size() != 3 || tokens[0] != "v2")
			return {};
		const std::string symmetric_key = MakeAESKeyForRegistryPassword(user_name, tokens[1]);

		std::size_t unbase64ed_encoded_pass_size;
		std::string unbase64ed_password = DoTransformation(base64_decode, tokens[2].data(), tokens[2].size(), unbase64ed_encoded_pass_size);
		if (unbase64ed_password.empty())
			return {};

		return DoEncryption<EVP_DecryptInit_ex, EVP_DecryptUpdate, EVP_DecryptFinal_ex>(unbase64ed_password, symmetric_key);
	}
} //namespace sec

#if defined(_WIN32) // Not ported yet

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <chrono>

#include <gtest/gtest.h>

#include "SecureLib/VS_CertificateIssue.h"
#include "SecureLib/VS_SecureConstants.h"
#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "SecureLib/VS_CryptoInit.h"
#include "std-generic/cpplib/VS_Container.h"
#include "ca_cert.h"
#include "ca_privkey.h"

using namespace std::chrono;

namespace {

	// imports

	// typedefs
	typedef std::unique_ptr<VS_PKey> VS_PKeyPtr;
	typedef std::unique_ptr<VS_CertificateRequest> VS_CertificateRequestPtr;
	typedef std::unique_ptr<VS_CertAuthority> VS_CertAuthorityPtr;


	// unit test class
	class SubjectAltNameTest : public testing::Test {
	protected:
	};

	typedef struct _Entry {
		const char *name;
		const char *value;
	} Entry;

	typedef struct _SubjAltNameEntry {
		SubjectAltNameType type;
		const char *value;
	} SubjAltNameEntry;


	// cert data
#define COMMON_NAME "*.domain.local"

	static Entry COMMON_CERT_FIELDS[] =
	{
		{ "commonName", COMMON_NAME },
		{ "organizationName", "Networking Programming Department" },
		{ "countryName", "UA" },
		{ "surname", "User Surname" },
		{ "emailAddress", "info@trueconf.ua" }
	};

	// subject alt name values for request
	static SubjAltNameEntry REQ_SUBJ_ALT_NAMES[] =
	{
		{ SUBJ_ALT_NAME_DNS, COMMON_NAME },
		{ SUBJ_ALT_NAME_DNS, "domain.local" },
		{ SUBJ_ALT_NAME_DNS, "*.my.domain.local" },
		{ SUBJ_ALT_NAME_DNS, "my.domain.local" },
		{ SUBJ_ALT_NAME_IP, "10.0.13.1" }
	};

	// subject alt name values for certificate authority
	static SubjAltNameEntry CA_SUBJ_ALT_NAMES[] =
	{
		{ SUBJ_ALT_NAME_EMAIL, "sales@trueconf.com" },
		{ SUBJ_ALT_NAME_IP, "2001:db8:11a3:9d7:1f34:8a2e:7a0:765d" }
	};

	static VS_PKeyPtr make_key()
	{
		auto key = std::make_unique<VS_PKey>();

		if (!key->GenerateKeys(VS_DEFAULT_PKEY_LEN, alg_pk_RSA))
		{
			return NULL;
		}

		return key;
	}

	static VS_CertificateRequestPtr make_request(VS_PKeyPtr &key)
	{
		auto req = std::make_unique<VS_CertificateRequest>();


		if (!req->SetPKeys(key.get(), key.get()))
		{
			return nullptr;
		}

		// add common cert fields
		for (auto &v : COMMON_CERT_FIELDS)
		{
			if (!req->SetEntry(v.name, v.value))
				return nullptr;
		}

		for (auto &v : REQ_SUBJ_ALT_NAMES)
		{
			if (!req->SetSubjectAltName(v.type, v.value))
				return nullptr;
		}

		if (!req->SignRequest())
		{
			return nullptr;
		}

		return req;
	}

	static void dump_data(const void *data, size_t size, const char *path)
	{
		std::fstream out(path, std::ios::binary | std::ios::out);

		out.write((char *)data, size);

		out.close();
	}

	static void dump_data(const void *data, size_t size, FILE *out)
	{
		fwrite(data, 1, size, out);
	}

	static void print_subj_alt_names(SubjAltNameExtensionsSet &set)
	{
		std::cout << "\t\t!!! Subject Alternative Name values !!!!" << std::endl << std::endl;
		for (auto &v : set)
		{
			std::cout << "Type: " << std::to_string(v.first) << "\t value: " << v.second.c_str() << std::endl;
		}
	}

	TEST_F(SubjectAltNameTest, request_and_certificate_creation)
	{
		uint32_t size = 0;
		std::vector<uint8_t> data;
		SubjAltNameExtensionsSet exts;
		VS_Certificate ca_cert;

		auto key = make_key();
		ASSERT_NE(nullptr, key) << "Can\'t create key!" << std::endl;

		auto req = make_request(key);
		ASSERT_NE(nullptr, key) << "Can\'t create certificate request!" << std::endl;

		// check certificate request subject alternative names
		exts.clear();
		req->GetSubjectAltNames(exts);
		for (size_t i = 0; i < exts.size(); i++)
		{
			auto &v = exts[i];
			auto &expect = REQ_SUBJ_ALT_NAMES[i];
			ASSERT_EQ(v.first, expect.type);
			ASSERT_STREQ(v.second.c_str(), expect.value);
		}

		// dump data
		//cout << "\t\t!!! Certificate request !!!!" << endl << endl;
		req->SaveTo(nullptr, size, store_PEM_BUF);
		data.resize(size);
		req->SaveTo(reinterpret_cast<char *>(data.data()), size, store_PEM_BUF);
		//dump_data(&data[0], size, stdout);
		//dump_data(&data[0], size, "cert.req");
		std::cout << std::endl;

		exts.clear();
		req->GetSubjectAltNames(exts);
		//print_subj_alt_names(exts);

		// create certificate authority
		auto ca = std::make_unique<VS_CertAuthority>();

		ASSERT_TRUE(ca->SetCertRequest(req.get())) << "Can\'t set certificate request" << std::endl;

		// set CA certificate and key
		ASSERT_FALSE(!ca->SetCACertificate((const char *)ca_cert_data, sizeof(ca_cert_data), store_PEM_BUF) ||
			!ca->SetCAPrivateKey((const char *)ca_privkey_data,store_PEM_BUF))
			<< "Can't set CA private key or data." << std::endl;

		ASSERT_TRUE(ca->VerifyRequestSign()) << "Can't verify request sign!" << std::endl;

		// add additional fields
		for (auto &v : CA_SUBJ_ALT_NAMES)
		{
			ASSERT_TRUE(ca->SetSubjectAltName(v.type, v.value))
				<< "Can\'t set subject alternative name of type " << std::to_string(v.type) << " with value: "
				<< v.value << std::endl;
		}

		// set certificate expiration time
		system_clock::time_point not_before = system_clock::now(), not_after = system_clock::now() + 30 * 365 * hours(24);
		// assign file time

		ASSERT_TRUE(ca->SetExpirationTime(not_before, not_after)) << "Can\'t set expiration time" << std::endl;

		size = 0;
		ca->IssueCertificate(nullptr, size, store_PEM_BUF);
		ASSERT_TRUE(ca->IssueCertificate((data.resize(size + 1), reinterpret_cast<char *>(data.data())), size, store_PEM_BUF))
			<< "Can't create certificate" << std::endl;

		//cout << "\t\t!!!Generated certificate!!!" << endl << endl;

		//dump_data(&data[0], size, stdout);
		//dump_data(&data[0], size, "test.crt");

		std::cout << std::endl;

		// fetch extensions and print them
		{
			VS_Certificate cert;

			cert.SetCert(reinterpret_cast<const char *>(data.data()), size, store_PEM_BUF);

			exts.clear();
			cert.GetSubjectAltNames(exts);
			ASSERT_EQ(exts.size(), sizeof(REQ_SUBJ_ALT_NAMES) / sizeof(REQ_SUBJ_ALT_NAMES[0])
				+ sizeof(CA_SUBJ_ALT_NAMES) / sizeof(CA_SUBJ_ALT_NAMES[0]));

			for (auto i = 0; i < sizeof(REQ_SUBJ_ALT_NAMES) / sizeof(REQ_SUBJ_ALT_NAMES[0]);i++)
				ASSERT_TRUE(exts.end() !=std::find(exts.begin(), exts.end(), std::make_pair(REQ_SUBJ_ALT_NAMES[i].type, std::string(REQ_SUBJ_ALT_NAMES[i].value))));
			for (auto i = 0; i < sizeof(CA_SUBJ_ALT_NAMES) / sizeof(CA_SUBJ_ALT_NAMES[0]);i++)
				ASSERT_TRUE(exts.end() != std::find(exts.begin(), exts.end(), std::make_pair(CA_SUBJ_ALT_NAMES[i].type, std::string(CA_SUBJ_ALT_NAMES[i].value))));
			print_subj_alt_names(exts);
		}
	}
}

#endif

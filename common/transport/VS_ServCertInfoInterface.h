#pragma once
#include "std/cpplib/VS_SimpleStr.h"
#include <cstdint>

class VS_ServCertInfoInterface
{
public:
	enum class get_info_res
	{
		undef = -1,
		ok, // public key has been got (0)
		key_is_absent, // there isn't public key for the server (1);
		db_error, // public key may be exist, but  db some error occurred (2)
		auto_verify, // verify without checking of sign (3)
		auto_deny // ServerVerificationFailed without checking other conditions
	};
	virtual get_info_res GetPublicKey(const VS_SimpleStr& server_name,
		VS_SimpleStr &pub_key, uint32_t &vcs_ver) = 0;
	virtual get_info_res GetServerCertificate(const VS_SimpleStr &server_name,
		VS_SimpleStr &cert) = 0;
};
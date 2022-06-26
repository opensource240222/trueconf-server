#pragma once

#include "SecureLib/OpenSSLTypesWrapDefs.h"
#include "std-generic/cpplib/Box.h"

#include <vector>
#include <string>

namespace ntlm {

struct AUTH_MSG {
	std::vector<unsigned char> ntChallengeResponse;
	std::vector<unsigned char> lmChallengeResponse;
	std::string domainName;
	std::string userName;
	std::string workstation;
	std::vector<unsigned char> encryptedRandomSessionKey;
	uint32_t negotiateFlags;
};

struct AtributeValue {
	uint16_t id;
	std::vector<unsigned char> value;
};

struct CHALLENGE_MSG {
	std::string targetName;
	uint32_t negotiateFlags;
	unsigned char serverChallenge[8];
	std::vector<AtributeValue> targetInfo;
};

struct sessionKey {
	sessionKey();
	~sessionKey();
	sessionKey(const sessionKey&) = delete;
	sessionKey(sessionKey&&);
	sessionKey& operator=(sessionKey&&);
	sessionKey& operator=(const sessionKey&) = delete;

	uint32_t seq;
	std::vector<unsigned char> sealkey;
	std::vector<unsigned char> signkey;

	vs::RC4KeyPtr rc4Handle;
};
}
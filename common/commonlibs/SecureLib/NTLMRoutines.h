#pragma once

#include <string>
#include <vector>
#include <array>
#include <openssl/md5.h>
#include <openssl/md4.h>
#include <openssl/rc4.h>

#include "NTLMMessages.h"

namespace ntlm {

	// defined here https://msdn.microsoft.com/en-us/library/cc236700.aspx
	void ComputeResponse(
		  const std::string& user
		, const std::string& password
		, const std::string& domain
		, const unsigned char serverChallenge[8]
		, const std::vector<unsigned char> &temp
		, std::vector<unsigned char> &ntChallengeResponse
		, std::vector<unsigned char> &lmChallengeResponse
		, unsigned char outSessionBaseKey[MD5_DIGEST_LENGTH]);

	void LMOWFv1(const std::string & key, unsigned char responseKey[MD4_DIGEST_LENGTH]);
	void ComputeResponseV1(
		  const std::string& user
		, const std::string& password
		, const unsigned char serverChallenge[8]
		, const unsigned char clientChallenge[8]
		, const uint32_t negFlags
		, const bool NoLMResponseNTLMv1
		, std::vector<unsigned char> &ntChallengeResponse
		, std::vector<unsigned char> &lmChallengeResponse
		, unsigned char outSessionBaseKey[MD4_DIGEST_LENGTH]);

	bool CalculateNTLMAuth(
		const std::string& user
		, const std::string& password
		, const std::string& domain
		, const std::vector<unsigned char> &serverName	// server-naming context
		, const unsigned char serverchallenge[8]
		, const uint32_t negFlags
		, const uint64_t winTimestamp
		, bool useNTLMv2
		, std::vector<unsigned char> &ntChallengeResponse
		, std::vector<unsigned char> &lmChallengeResponse
		, unsigned char exportedSessionKey[MD5_DIGEST_LENGTH]
		, std::vector<unsigned char>& encryptedRandomSessionKey);

	// SIGNKEY and SEALKEY defined here https://msdn.microsoft.com/en-us/library/cc236711.aspx
	void SIGNKEY(uint32_t negFlags, const unsigned char exportedSessionKey[MD5_DIGEST_LENGTH], const std::string& mode, std::vector<unsigned char>& signKey);
	void SEALKEY(uint32_t negFlags, const unsigned char exportedSessionKey[MD5_DIGEST_LENGTH], const std::string& mode, std::vector<unsigned char>& sealKey);
	// defined here https://msdn.microsoft.com/en-us/library/cc236710.aspx
	void KXKEY(
		uint32_t negFlags
		, const unsigned char sessionBaseKey[MD5_DIGEST_LENGTH]
		, const std::vector<unsigned char> &lmChallengeResponse
		, const  unsigned char serverChallange[8]
		, const unsigned char LMOWF[16]
		, bool useNTLMv2
		, unsigned char kxKey[16]);

	bool AuthMessageEncode(const AUTH_MSG &m, std::vector<unsigned char> & outBuff, size_t &micOffset);
	bool DecodeChallengeMsg(const void* data, size_t len, CHALLENGE_MSG &res);

	void CalculateMD5(
		const void* data1, const size_t len1,
		unsigned char outHash[MD5_DIGEST_LENGTH]);
	bool RC4Encrypt(const unsigned char key[16], const unsigned char *data, size_t dataLen, std::vector<unsigned char>&outData);
	void EncodeTargetInfo(const std::vector<ntlm::AtributeValue>& targetInfo, std::vector<unsigned char> &resBuff);
	bool MakeSignature(const void* data, size_t len, ntlm::sessionKey& sendKey, uint32_t negFlags, std::array<unsigned char, 16u> &sign);
	void MakeClientChallengeStruct(const uint64_t winTimestamp, const unsigned char clientChallenge[8], const std::vector<unsigned char>& avPairs, std::vector<unsigned char>&OUT_struct);
	void ResetRC4Key(ntlm::sessionKey& sendKey, uint32_t negFlags);
}
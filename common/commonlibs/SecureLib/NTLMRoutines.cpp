#include "NTLMRoutines.h"
#include "NTLMFlags.h"
#include "NTLMDataHeaders.h"

#include "SecureLib/CRC32.h"
#include "SecureLib/OpenSSLCompat/tc_hmac.h"
#include "SecureLib/OpenSSLTypesWrapImpl.h"

#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/stdendian.h"
#include "std-generic/cpplib/utf8.h"

#include <openssl/des.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <algorithm>
#include <cassert>
#include <memory.h>

const char ntlmSSP[] = "NTLMSSP";
const unsigned NTLMRevisionCurrent = 0x0F;

void CalculateMD4(const void* data, const size_t len, unsigned char outHash[MD4_DIGEST_LENGTH]) {
	EVP_MD_CTX *m = EVP_MD_CTX_create();
	VS_SCOPE_EXIT{ EVP_MD_CTX_destroy(m); };

	EVP_DigestInit_ex(m, EVP_md4(), NULL);
	EVP_DigestUpdate(m, data, len);
	EVP_DigestFinal_ex(m, outHash, NULL);
}
namespace ntlm{

sessionKey::sessionKey()
	: seq{}
	, rc4Handle(std::make_unique<RC4_KEY>())
{}
sessionKey::sessionKey(sessionKey&&) = default;
sessionKey::~sessionKey() = default;
sessionKey& sessionKey::operator=(sessionKey&&) = default;

void CalculateMD5(
	const void* data1, const size_t len1,
	const void* data2, const size_t len2,
	unsigned char outHash[MD5_DIGEST_LENGTH])
{
	auto ctx = EVP_MD_CTX_create();
	EVP_DigestInit_ex(ctx, EVP_md5(), NULL);
	VS_SCOPE_EXIT{ EVP_MD_CTX_destroy(ctx); };

	EVP_DigestUpdate(ctx, data1, len1);
	EVP_DigestUpdate(ctx, data2, len2);
	EVP_DigestFinal_ex(ctx, outHash, NULL);
}

void CalculateMD5(
	const void* data1, const size_t len1,
	unsigned char outHash[MD5_DIGEST_LENGTH])
{
	auto ctx = EVP_MD_CTX_create();
	VS_SCOPE_EXIT{ EVP_MD_CTX_destroy(ctx); };

	EVP_DigestInit_ex(ctx, EVP_md5(), NULL);
	EVP_DigestUpdate(ctx, data1, len1);
	EVP_DigestFinal_ex(ctx, outHash, NULL);
}
}

void Extend56To64Bit(const unsigned char key56[7], unsigned char key64[8]) {
	key64[0] = key56[0];
	key64[1] = (key56[0] << 7) | (key56[1] >> 1);
	key64[2] = (key56[1] << 6) | (key56[2] >> 2);
	key64[3] = (key56[2] << 5) | (key56[3] >> 3);
	key64[4] = (key56[3] << 4) | (key56[4] >> 4);
	key64[5] = (key56[4] << 3) | (key56[5] >> 5);
	key64[6] = (key56[5] << 2) | (key56[6] >> 6);
	key64[7] = (key56[6] << 1);
}

bool DES(const unsigned char inpkey[7], const unsigned char data[8], unsigned char encData[8]) {
	unsigned char key[8];
	Extend56To64Bit(inpkey, key);

	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	EVP_CIPHER_CTX_init(ctx);
	VS_SCOPE_EXIT{ EVP_CIPHER_CTX_free(ctx); };

	if(EVP_CipherInit_ex(ctx, EVP_des_cbc(), NULL, key, NULL, DES_ENCRYPT) != 1)
		return false;
	if (EVP_Cipher(ctx, encData, data, 8) != 1)
		return false;
	return true;
}

void HMAC_MD5(
	const unsigned char* key, size_t keyLen,
	const void* data1, const size_t len1,
	unsigned char outHash[MD5_DIGEST_LENGTH])
{
	HMAC_CTX *c = HMAC_CTX_new();
	unsigned int hmaclen(0);

	HMAC_Init_ex(c, key, keyLen, EVP_md5(), NULL);
	VS_SCOPE_EXIT{ HMAC_CTX_free(c); };

	HMAC_Update(c, static_cast<const unsigned char*>(data1), len1);
	HMAC_Final(c, outHash, &hmaclen);
	assert(hmaclen == MD5_DIGEST_LENGTH);
}

void HMAC_MD5(
	const unsigned char* key, size_t keyLen,
	const void* data1, const size_t len1,
	const void* data2, const size_t len2,
	unsigned char outHash[MD5_DIGEST_LENGTH])
{
	HMAC_CTX *c = HMAC_CTX_new();
	unsigned int hmaclen(0);

	HMAC_Init_ex(c, key, keyLen, EVP_md5(), NULL);
	VS_SCOPE_EXIT{ HMAC_CTX_free(c); };

	HMAC_Update(c, static_cast<const unsigned char*>(data1), len1);
	HMAC_Update(c, static_cast<const unsigned char*>(data2), len2);
	HMAC_Final(c, outHash, &hmaclen);
	assert(hmaclen == MD5_DIGEST_LENGTH);
}

// defined here https://msdn.microsoft.com/en-us/library/cc236700.aspx
int NTOWFv2(const std::string & key, const std::string & username, const std::string & target, unsigned char responseKey[MD5_DIGEST_LENGTH])
{
	auto ucs2Key = vs::UTF8toUTF16Convert(key);
	auto ucs2User = vs::UTF8toUTF16Convert(username);
	std::transform(ucs2User.begin(), ucs2User.end(), ucs2User.begin(), ::towupper);
	auto ucs2Target = vs::UTF8toUTF16Convert(target);

	unsigned char md4Passwd[MD4_DIGEST_LENGTH] = {};
	CalculateMD4(ucs2Key.data(), ucs2Key.length() * 2, md4Passwd);

	HMAC_MD5(
		md4Passwd, MD4_DIGEST_LENGTH,
		ucs2User.data(), ucs2User.length() * 2,
		ucs2Target.data(), ucs2Target.length() * 2,
		responseKey);
	return 0;
}

// defined here https://msdn.microsoft.com/en-us/library/cc236699.aspx
void NTOWFv1(const std::string & key, unsigned char responseKey[MD4_DIGEST_LENGTH]) {
	auto ucs2Key = vs::UTF8toUTF16Convert(key);
	CalculateMD4(ucs2Key.data(), ucs2Key.length() * 2, responseKey);
}

void ntlm::LMOWFv1(const std::string & key, unsigned char responseKey[MD4_DIGEST_LENGTH]) {
	const char data[] = "KGS!@#$%";
	auto upperKey = vs::UTF8ToUpper(key);
	while (upperKey.length() < 14)	// must be padded to 14 bytes
		upperKey.push_back('\0');

	DES(reinterpret_cast<const unsigned char*>(upperKey.data()), reinterpret_cast<const unsigned char*>(data), responseKey);
	DES(reinterpret_cast<const unsigned char*>(upperKey.data() + 7), reinterpret_cast<const unsigned char*>(data), responseKey + 8);
}

// defined here https://msdn.microsoft.com/en-us/library/cc236717.aspx
void DESL(const unsigned char key[16], const unsigned char data[8], unsigned char res[24]) {
	unsigned char thirdKey[7] = { key[14], key[15], 0x00, 0x00, 0x00, 0x00,0x00 };
	DES(key, data, res);
	DES(key + 7, data, res + 8);
	DES(thirdKey, data, res + 16);
}

void ntlm::ComputeResponse(
	  const std::string& user
	, const std::string& password
	, const std::string& domain
	, const unsigned char serverChallenge[8]
	, const std::vector<unsigned char> &temp
	, std::vector<unsigned char> &ntChallengeResponse
	, std::vector<unsigned char> &lmChallengeResponse
	, unsigned char outSessionBaseKey[MD5_DIGEST_LENGTH])
{
	unsigned char responseNT[MD5_DIGEST_LENGTH] = {};
	unsigned char responseLM[MD5_DIGEST_LENGTH] = {};
	NTOWFv2(password, user, domain, responseNT);
	memcpy(responseLM, responseNT, MD5_DIGEST_LENGTH);

	unsigned char NTProofStr[MD5_DIGEST_LENGTH] = {};
	if (user.empty() && password.empty()) {
		lmChallengeResponse.push_back(0);
		HMAC_MD5(responseNT, MD5_DIGEST_LENGTH, NTProofStr, MD5_DIGEST_LENGTH, outSessionBaseKey);
		return;
	}

	HMAC_MD5(
		responseNT, MD5_DIGEST_LENGTH,
		serverChallenge, 8,
		temp.data(), temp.size(),
		NTProofStr);

	auto &clientChall = *reinterpret_cast<const CLIENT_CHALLENGE_HEADER*>(temp.data());

	ntChallengeResponse.reserve(MD5_DIGEST_LENGTH + temp.size());
	ntChallengeResponse.insert(ntChallengeResponse.end(), NTProofStr, NTProofStr + MD5_DIGEST_LENGTH);
	ntChallengeResponse.insert(ntChallengeResponse.end(), temp.begin(), temp.end());

	unsigned char lmhash[MD5_DIGEST_LENGTH] = {};
	HMAC_MD5(responseLM, MD5_DIGEST_LENGTH, serverChallenge, 8, clientChall.challengeFromClient, 8,	lmhash);

	lmChallengeResponse.reserve(MD5_DIGEST_LENGTH + 8);
	lmChallengeResponse.insert(lmChallengeResponse.end(), lmhash, lmhash + MD5_DIGEST_LENGTH);
	lmChallengeResponse.insert(lmChallengeResponse.end(), clientChall.challengeFromClient, clientChall.challengeFromClient + 8);

	HMAC_MD5(responseNT, MD5_DIGEST_LENGTH, NTProofStr, MD5_DIGEST_LENGTH, outSessionBaseKey);
}

void ntlm::ComputeResponseV1(
	  const std::string & user
	, const std::string & password
	, const unsigned char serverChallenge[8]
	, const unsigned char clientChallenge[8]
	, const uint32_t negFlags
	, const bool NoLMResponseNTLMv1
	, std::vector<unsigned char>& ntChallengeResponse
	, std::vector<unsigned char>& lmChallengeResponse
	, unsigned char outSessionBaseKey[MD4_DIGEST_LENGTH])
{
	unsigned char responseKeyNT[MD4_DIGEST_LENGTH] = {};
	unsigned char responseKeyLM[MD4_DIGEST_LENGTH] = {};

	NTOWFv1(password, responseKeyNT);
	LMOWFv1(password, responseKeyLM);
	CalculateMD4(responseKeyNT, sizeof(responseKeyNT), outSessionBaseKey);

	if (user.empty() && password.empty()) {
		ntChallengeResponse.clear();
		lmChallengeResponse.push_back(0);
		return;
	}

	ntChallengeResponse.resize(24);
	lmChallengeResponse.resize(24);
	if (negFlags & NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY) {
		// Set NtChallengeResponse to DESL(ResponseKeyNT, MD5(ConcatenationOf(CHALLENGE_MESSAGE.ServerChallenge, ClientChallenge))[0..7])
		unsigned char md5Hash[MD5_DIGEST_LENGTH] = {};
		CalculateMD5(serverChallenge, 8, clientChallenge, 8, md5Hash);
		DESL(responseKeyNT, md5Hash, ntChallengeResponse.data());

		// Set LmChallengeResponse to ConcatenationOf{ClientChallenge, Z(16)}
		memcpy(lmChallengeResponse.data(), clientChallenge, 8);
		memset(lmChallengeResponse.data() + 8, 0, lmChallengeResponse.size() - 8);

	}
	else {
		// Set NtChallengeResponse to DESL(ResponseKeyNT,CHALLENGE_MESSAGE.ServerChallenge)
		DESL(responseKeyNT, serverChallenge, ntChallengeResponse.data());
		if (NoLMResponseNTLMv1) {
			lmChallengeResponse = ntChallengeResponse;
		}
		else {
			// Set LmChallengeResponse to DESL(ResponseKeyLM, CHALLENGE_MESSAGE.ServerChallenge)
			DESL(responseKeyLM, serverChallenge, lmChallengeResponse.data());
		}
	}

}

bool ntlm::RC4Encrypt(const unsigned char key[16], const unsigned char *data, size_t dataLen, std::vector<unsigned char>&outData) {
	EVP_CIPHER_CTX *c = EVP_CIPHER_CTX_new();
	EVP_CIPHER_CTX_init(c);
	VS_SCOPE_EXIT{ EVP_CIPHER_CTX_free(c); };

	if(EVP_CipherInit_ex(c, EVP_rc4(), NULL, key, NULL, 1) != 1)
		return false;

	outData.resize(dataLen);
	if (!EVP_Cipher(c, outData.data(), data, outData.size()))
		return false;
	return true;
}

void ntlm::MakeClientChallengeStruct(const uint64_t winTimestamp, const unsigned char clientChallenge[8], const std::vector<unsigned char>& avPairs, std::vector<unsigned char>&OUT_struct) {
	OUT_struct.reserve(sizeof(ntlm::CLIENT_CHALLENGE_HEADER) + avPairs.size() + 4);
	OUT_struct.resize(sizeof(ntlm::CLIENT_CHALLENGE_HEADER));

	auto &challng = *reinterpret_cast<ntlm::CLIENT_CHALLENGE_HEADER*>(OUT_struct.data());
	challng.respType = challng.hiRespType = 0x1;
	memset(&challng.reserved, 0, sizeof(challng.reserved));
	challng.timeStamp = htole64(winTimestamp);
	memcpy(challng.challengeFromClient, clientChallenge, sizeof(challng.challengeFromClient));
	memset(&challng.reserved1, 0, sizeof(challng.reserved1));
	OUT_struct.insert(OUT_struct.end(), avPairs.begin(), avPairs.end());
	OUT_struct.insert(OUT_struct.end(), 4, 0);
}

// defined here https://msdn.microsoft.com/en-us/library/cc236676.aspx
bool ntlm::CalculateNTLMAuth(
	const std::string & user
	, const std::string & password
	, const std::string & domain
	, const std::vector<unsigned char> &serverName
	, const unsigned char serverchallenge[8]
	, const uint32_t negFlags
	, const uint64_t winTimestamp
	, bool useNTLMv2
	, std::vector<unsigned char> &ntChallengeResponse
	, std::vector<unsigned char> &lmChallengeResponse
	, unsigned char exportedSessionKey[MD5_DIGEST_LENGTH]
	, std::vector<unsigned char>& encryptedRandomSessionKey)
{
	unsigned char keyEX[MD5_DIGEST_LENGTH];
	unsigned char sessionBaseKey[MD5_DIGEST_LENGTH] = {};
	unsigned char clientChallenge[8] = {};
	if (RAND_bytes(clientChallenge, sizeof(clientChallenge)) != 1)
		return false;

	std::vector<unsigned char> clientChallengeMSG;
	MakeClientChallengeStruct(winTimestamp, clientChallenge, serverName, clientChallengeMSG);

	if (useNTLMv2) {
		ComputeResponse(
			user, password, domain,
			serverchallenge, clientChallengeMSG, ntChallengeResponse, lmChallengeResponse, sessionBaseKey);
	}
	else {
		ComputeResponseV1(
			user, password, serverchallenge, clientChallenge, negFlags, false
			, ntChallengeResponse, lmChallengeResponse, sessionBaseKey);
	}

	unsigned char LMWOF[16] = {};
	if (!useNTLMv2)
		LMOWFv1(password, LMWOF);
	KXKEY(negFlags, sessionBaseKey, lmChallengeResponse, serverchallenge, LMWOF, useNTLMv2, keyEX);
	if (negFlags & NTLMSSP_NEGOTIATE_KEY_EXCH) {
		if (RAND_bytes(exportedSessionKey, MD5_DIGEST_LENGTH) != 1)
			return false;

		if(!RC4Encrypt(keyEX, exportedSessionKey, MD5_DIGEST_LENGTH, encryptedRandomSessionKey))
			return false;
	}
	else {
		memcpy(exportedSessionKey, keyEX, 16);
		encryptedRandomSessionKey.clear();
	}
	return true;
}

void ntlm::SIGNKEY(uint32_t negFlags, const unsigned char exportedSessionKey[MD5_DIGEST_LENGTH], const std::string & mode, std::vector<unsigned char>& signKey)
{
	if (!(negFlags & NTLMSSP_NEGOTIATE_SIGN) || !(negFlags & NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY)){
		signKey.clear();
		return;
	}

	signKey.resize(MD5_DIGEST_LENGTH);
	if (mode == "Client") {
		const char clientServerMagic[] = "session key to client-to-server signing key magic constant";
		CalculateMD5(exportedSessionKey, MD5_DIGEST_LENGTH, clientServerMagic, sizeof(clientServerMagic), signKey.data());
	}
	else {
		const char serverClientMagic[] = "session key to server-to-client signing key magic constant";
		CalculateMD5(exportedSessionKey, MD5_DIGEST_LENGTH, serverClientMagic, sizeof(serverClientMagic), signKey.data());
	}
}

size_t GetSealKeySize(uint32_t negFlags) {
	if (NTLMSSP_NEGOTIATE_128 & negFlags)
		return 16;
	else if (NTLMSSP_NEGOTIATE_56 & negFlags)
		return 7;
	else
		return 5;
}

void ntlm::SEALKEY(uint32_t negFlags, const unsigned char exportedSessionKey[MD5_DIGEST_LENGTH], const std::string & mode, std::vector<unsigned char>& sealKey)
{

	if (negFlags & NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY) {
		const size_t sealKeySize = GetSealKeySize(negFlags);
		sealKey.resize(MD5_DIGEST_LENGTH);
		if (mode == "Client") {
			const char clientServerMagic[] = "session key to client-to-server sealing key magic constant";
			CalculateMD5(exportedSessionKey, sealKeySize, clientServerMagic, sizeof(clientServerMagic), sealKey.data());
		}
		else {
			const char serverClientMagic[] = "session key to server-to-client sealing key magic constant";
			CalculateMD5(exportedSessionKey, sealKeySize, serverClientMagic, sizeof(serverClientMagic), sealKey.data());
		}
	}
	else if ((negFlags & NTLMSSP_NEGOTIATE_LM_KEY) || (negFlags & NTLMSSP_NEGOTIATE_DATAGRAM ) && (NTLMRevisionCurrent >= NTLMSSP_REVISION_W2K3)) {
		if (NTLMSSP_NEGOTIATE_56 & negFlags) {
			const size_t sealKeySize = 7;
			sealKey.resize(sealKeySize + 1);
			memcpy(sealKey.data(), exportedSessionKey, sealKeySize);
			sealKey[sealKeySize] = 0xA0;
		}
		else {
			const size_t sealKeySize = 5;
			sealKey.resize(sealKeySize + 3);
			memcpy(sealKey.data(), exportedSessionKey, sealKeySize);
			sealKey[sealKeySize] = 0xE5;
			sealKey[sealKeySize + 1] = 0x38;
			sealKey[sealKeySize + 2] = 0xB0;
		}
	}
	else {
		sealKey.insert(sealKey.end(), exportedSessionKey, exportedSessionKey + MD5_DIGEST_LENGTH);
	}
}

void ntlm::KXKEY(
	uint32_t negFlags
	, const unsigned char sessionBaseKey[MD5_DIGEST_LENGTH]
	, const std::vector<unsigned char>& lmChallengeResponse
	, const unsigned char serverChallange[8]
	, const unsigned char LMOWF[16]
	, bool useNTLMv2
	, unsigned char kxKey[16])
{
	// If NTLM v2 is used, KeyExchangeKey MUST be set to the given 128-bit SessionBaseKey value.
	if (useNTLMv2) {
		memcpy(kxKey, sessionBaseKey, MD5_DIGEST_LENGTH);
		return;
	}

	// If NTLM v1 is used and extended session security is not negotiated
	if (!(NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY & negFlags)) {
		if (NTLMSSP_NEGOTIATE_LM_KEY & negFlags) {
			unsigned char k[7] = { LMOWF[7], 0xBD, 0xBD ,0xBD ,0xBD ,0xBD ,0xBD };
			DES(LMOWF, lmChallengeResponse.data(), kxKey);
			DES(k, lmChallengeResponse.data(), kxKey + 8);
		}
		else {
			if (NTLMSSP_REQUEST_NON_NT_SESSION_KEY & negFlags) {
				memcpy(kxKey, LMOWF, 8);
				memset(kxKey + 8, 0, 8);
			}
			else
				memcpy(kxKey, sessionBaseKey, MD5_DIGEST_LENGTH);
		}
	}
	else { // If NTLM v1 is used and extended session security is negotiated
		HMAC_MD5(sessionBaseKey, 16, serverChallange, 8, lmChallengeResponse.data(), 8, kxKey);
	}
}

size_t FillResponseFields(ntlm::InfoFields& f, size_t currOffset, size_t payloadSize) {
	f.responseMaxLen = f.responseLen = htole16(static_cast<uint16_t>(payloadSize));
	f.responseBufferOffset = htole32(currOffset);
	return currOffset + payloadSize;
}

size_t FillResponseFields(ntlm::InfoFields& f, size_t currOffset, const std::string &payload, uint32_t negotiateFlags, std::vector<unsigned char>& outData) {
	if (NTLMSSP_NEGOTIATE_UNICODE & negotiateFlags) {
		auto ucs2payload = vs::UTF8toUTF16Convert(payload);
		auto pData = (unsigned char*)ucs2payload.data();
		outData.insert(outData.end(), pData, pData + ucs2payload.size() * 2);
	}
	else if (NTLM_NEGOTIATE_OEM & negotiateFlags) {
		// here we MUST convert utf-8 strings to OEM character set encoding
		outData.insert(outData.end(), payload.begin(), payload.end());	// but for now just copy
	}
	else {
		// must not be here
		assert(false);
	}
	f.responseMaxLen = f.responseLen = htole16(static_cast<uint16_t>(outData.size()));
	f.responseBufferOffset = htole32(currOffset);
	return currOffset + outData.size();
}

bool ntlm::AuthMessageEncode(const AUTH_MSG & m, std::vector<unsigned char>& outBuff, size_t &micOffset)
{
	outBuff.resize(sizeof(AUTHENTICATE_MESSAGE_HEADER));
	AUTHENTICATE_MESSAGE_HEADER &head = *reinterpret_cast<AUTHENTICATE_MESSAGE_HEADER *>(outBuff.data());
	memcpy(head.signature, ntlmSSP, 8);
	head.messageType = htole32(3);
	micOffset = sizeof(AUTHENTICATE_MESSAGE_HEADER) - sizeof(AUTHENTICATE_MESSAGE_HEADER::MIC);

	size_t payloadOffset = sizeof(head);
	std::vector<unsigned char> domain;
	payloadOffset = FillResponseFields(head.domainName, payloadOffset, m.domainName, m.negotiateFlags, domain);
	std::vector<unsigned char> user;
	payloadOffset = FillResponseFields(head.userName, payloadOffset, m.userName, m.negotiateFlags, user);
	std::vector<unsigned char> workstation;
	payloadOffset = FillResponseFields(head.workstation, payloadOffset, m.workstation, m.negotiateFlags, workstation);
	payloadOffset = FillResponseFields(head.lmChallengeResponse, payloadOffset, m.lmChallengeResponse.size());
	payloadOffset = FillResponseFields(head.ntChallengeResponse, payloadOffset, m.ntChallengeResponse.size());
	payloadOffset = FillResponseFields(head.encryptedRandomSessionKey, payloadOffset, m.encryptedRandomSessionKey.size());
	head.negotiateFlags = htole32(m.negotiateFlags);
	memset(head.MIC, 0, sizeof(head.MIC));
	if (NTLMSSP_NEGOTIATE_VERSION & m.negotiateFlags)
		head.version = 0xF0000001DB10106;

	outBuff.reserve(sizeof(head) + m.lmChallengeResponse.size() + m.ntChallengeResponse.size() + domain.size() + user.size() + workstation.size() + m.encryptedRandomSessionKey.size());
	outBuff.insert(outBuff.end(), domain.begin(), domain.end());
	outBuff.insert(outBuff.end(), user.begin(), user.end());
	outBuff.insert(outBuff.end(), workstation.begin(), workstation.end());
	outBuff.insert(outBuff.end(), m.lmChallengeResponse.begin(), m.lmChallengeResponse.end());
	outBuff.insert(outBuff.end(), m.ntChallengeResponse.begin(), m.ntChallengeResponse.end());
	outBuff.insert(outBuff.end(), m.encryptedRandomSessionKey.begin(), m.encryptedRandomSessionKey.end());
	return true;
}

std::string DecodeTargetName(const void* data, size_t dataLen, uint32_t negFlags) {
	if (NTLMSSP_NEGOTIATE_UNICODE & negFlags) {
		assert(dataLen % 2 == 0);	// expect ucs2
		if (dataLen % 2 != 0)
			return {};
		return vs::UTF16toUTF8Convert(u16string_view(static_cast<const char16_t*>(data), dataLen / 2));
	}

	if (NTLM_NEGOTIATE_OEM & negFlags) {
		// here we MUST convert OEM strings to utf-8 character set encoding
		return std::string(static_cast<const char*>(data), static_cast<const char*>(data) + dataLen);	// but for now just copy
	}
	return {};
}

void ntlm::EncodeTargetInfo(const std::vector<ntlm::AtributeValue>& targetInfo, std::vector<unsigned char> &resBuff) {
	for (auto& av : targetInfo)
	{
		AV_PAIR p;
		p.avId = htole16(av.id);
		p.avLen = htole16(static_cast<uint16_t>(av.value.size()));
		auto pData = reinterpret_cast<unsigned char*>(&p);
		resBuff.insert(resBuff.end(), pData, pData + sizeof(AV_PAIR));
		resBuff.insert(resBuff.end(), av.value.begin(), av.value.end());
	}

	AV_PAIR end;
	end.avId = htole16(MsvAvEOL);
	end.avLen = 0;
	auto pData = reinterpret_cast<unsigned char*>(&end);
	resBuff.insert(resBuff.end(), pData, pData + sizeof(AV_PAIR));
}

void DecodeTargetInfo(const void* data, size_t dataLen, std::vector<ntlm::AtributeValue>& resValues) {
	if (!data || dataLen < sizeof(ntlm::AV_PAIR))
		return;

	auto pData = static_cast<const unsigned char*>(data);
	auto value = static_cast<const ntlm::AV_PAIR*>(data);
	while (value->avId != 0) {
		if (value->avLen != 0) {
			ntlm::AtributeValue res;
			res.id = le16toh(value->avId);
			auto pValue = pData + sizeof(ntlm::AV_PAIR);
			res.value.insert(res.value.end(), pValue, pValue + le16toh(value->avLen));
			resValues.emplace_back(res);
		}
		pData += sizeof(ntlm::AV_PAIR) + le16toh(value->avLen);
		value = reinterpret_cast<const  ntlm::AV_PAIR*>(pData);
	};
}

bool ntlm::DecodeChallengeMsg(const void * data, size_t len, CHALLENGE_MSG & res)
{
	if (!data || len < sizeof(CHALLENGE_MESSAGE_HEADER))
		return false;

	const auto &head = *reinterpret_cast<const CHALLENGE_MESSAGE_HEADER*>(data);
	if (memcmp(head.signature, ntlmSSP, sizeof(ntlmSSP)) != 0)
		return false;
	if (le32toh(head.msgType) != 2)
		return false;

	res.negotiateFlags = le32toh(head.negFlags);
	memcpy(res.serverChallenge, head.serverChallenge, sizeof(head.serverChallenge));
	if (le16toh(head.tagetName.responseLen) != 0) {
		const auto pTargetName = static_cast<const uint8_t*>(data) + le32toh(head.tagetName.responseBufferOffset);
		res.targetName = DecodeTargetName(pTargetName, le16toh(head.tagetName.responseLen), res.negotiateFlags);
	}
	if (le16toh(head.targetInfo.responseLen) != 0) {
		const auto pTargetInfo = static_cast<const uint8_t*>(data) + le32toh(head.targetInfo.responseBufferOffset);
		DecodeTargetInfo(pTargetInfo, le16toh(head.targetInfo.responseLen), res.targetInfo);
	}
	return true;
}


const unsigned int appProvidedSeqNum = 100;
// defined here https://msdn.microsoft.com/en-us/library/cc422953.aspx
bool SignMsg(const void* data, size_t len, RC4_KEY &rc4Handle, uint32_t negFlags, uint32_t seqNum, std::array<unsigned char, 16u> &sign) {
	if (NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY & negFlags)
		return false;
	if (!(NTLMSSP_NEGOTIATE_SIGN & negFlags) && !(NTLMSSP_NEGOTIATE_SEAL & negFlags))
		return false;
	if (len == 0)
		return false;

	unsigned char randBytes[4] = {};
	if (RAND_bytes(randBytes, sizeof(randBytes)) != 1)
		return false;
	auto crc32Sum = htole32(sec::crc32(data, len));

	auto &s = *reinterpret_cast<ntlm::SSP_MESSAGE_SIGNATURE*>(sign.data());
	s.version = htole32(0x00000001);
	RC4(&rc4Handle, sizeof(randBytes), randBytes, s.randomPad);
	RC4(&rc4Handle, sizeof(crc32Sum), reinterpret_cast<unsigned char*>(&crc32Sum), reinterpret_cast<unsigned char*>(&s.checksum));
	unsigned char null[4] = {};
	RC4(&rc4Handle, 4, null, reinterpret_cast<unsigned char*>(&s.seqN));

	if (NTLMSSP_NEGOTIATE_DATAGRAM & negFlags)
		s.seqN ^= htole32(appProvidedSeqNum);
	else
		s.seqN ^= htole32(seqNum);
	memset(s.randomPad, 0, sizeof(s.randomPad));
	return true;
}

void encode_le_uint32(uint32_t n, unsigned char *p)
{
	p[0] = (n >> 0) & 0xFF;
	p[1] = (n >> 8) & 0xFF;
	p[2] = (n >> 16) & 0xFF;
	p[3] = (n >> 24) & 0xFF;
}

bool SignMsgEx(const void* data, size_t len, const std::vector<unsigned char> &signkey, RC4_KEY &rc4Handle, uint32_t negFlags, uint32_t seqNum, std::array<unsigned char, 16u> &sign) {
	if (!(NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY & negFlags))
		return false;
	if (!(NTLMSSP_NEGOTIATE_SIGN & negFlags) && !(NTLMSSP_NEGOTIATE_SEAL & negFlags))
		return false;
	if (signkey.empty() || len == 0)
		return false;

	if (negFlags & NTLMSSP_NEGOTIATE_DATAGRAM) // In datagram mode seqnum explicitly provided by application
		seqNum = appProvidedSeqNum;

	unsigned char seqNumStr[4];
	encode_le_uint32(seqNum, seqNumStr);

	unsigned char hash[MD5_DIGEST_LENGTH] = {};
	HMAC_MD5(signkey.data(), signkey.size(), seqNumStr, sizeof(seqNumStr), data, len, hash);

	auto &s = *reinterpret_cast<ntlm::SSP_MESSAGE_SIGNATURE_Ex*>(sign.data());
	s.version = htole32(0x00000001);
	if (NTLMSSP_NEGOTIATE_KEY_EXCH & negFlags)
		RC4(&rc4Handle, sizeof(s.checksum), hash, s.checksum);
	else
		memcpy(s.checksum, hash, sizeof(s.checksum));
	s.seqN = htole32(seqNum);
	return true;
}

bool ntlm::MakeSignature(const void * data, size_t len, ntlm::sessionKey& sendKey, uint32_t negFlags, std::array<unsigned char, 16u>& sign)
{
		if ((NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY & negFlags))
		return SignMsgEx(data, len, sendKey.signkey, *sendKey.rc4Handle.get(), negFlags, sendKey.seq++, sign);
	return SignMsg(data, len, *sendKey.rc4Handle.get(), negFlags, sendKey.seq++, sign);
}

void ntlm::ResetRC4Key(ntlm::sessionKey& sendKey, uint32_t negFlags) {
	if (NTLMSSP_NEGOTIATE_DATAGRAM & negFlags) {
		// reinit rc4 handle like was told us to do here https://msdn.microsoft.com/en-us/library/cc236701.aspx
		// only needed by NTLM2 session security (when NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY is specified)
		unsigned char newKey[MD5_DIGEST_LENGTH] = {};
		unsigned char seqNumStr[4] = { appProvidedSeqNum, 0x00,0x00,0x00 };
		CalculateMD5(sendKey.sealkey.data(), sendKey.sealkey.size(), seqNumStr, sizeof(seqNumStr), newKey);
		RC4_set_key(sendKey.rc4Handle.get().get(), MD5_DIGEST_LENGTH, newKey);
	}
}

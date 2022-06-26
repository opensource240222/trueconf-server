#include "NTLMv2Client.h"
#include "SecureLib/OpenSSLTypesWrapImpl.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/TimeUtils.h"
#include "std-generic/debuglog/VS_Debug.h"
#include "std-generic/cpplib/utf8.h"
#include "std-generic/cpplib/stdendian.h"

#include "NTLMFlags.h"
#include "NTLMRoutines.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include "OpenSSLCompat/tc_hmac.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <array>

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

bool sec::NTLMv2Client::SignAuthMessage(const void * inBuf, size_t inLen, unsigned char ExportedSessionKey[16], std::array<unsigned char, 16>& outSignature)
{
	HMAC_CTX *c = HMAC_CTX_new();
	unsigned int hmaclen(0);

	HMAC_Init_ex(c, ExportedSessionKey, 16, EVP_md5(), NULL);
	VS_SCOPE_EXIT{ HMAC_CTX_free(c); };

	HMAC_Update(c, static_cast<const unsigned char*>(inBuf), inLen);
	HMAC_Final(c, outSignature.data(), &hmaclen);
	assert(hmaclen == 16u);
	return true;
}

void sec::NTLMv2Client::SaveCredetinals(const std::string & userName, const std::string & domain, const std::string & passwd)
{
	m_userName = userName;
	m_domain = domain;
	m_passwd = passwd;
}

void AddIndicationOfMIC(std::vector<ntlm::AtributeValue> &serverCtx) {
	auto it = std::find_if(serverCtx.begin(), serverCtx.end(), [](const ntlm::AtributeValue& v) {return v.id == MsvAvFlags; });
	if (it != serverCtx.end()) {
		assert(it->value.size() > 0);
		if (it->value.size() > 0)
			it->value[0] |= 0x2;
	}
	else {
		ntlm::AtributeValue av;
		av.id = MsvAvFlags;
		av.value = std::vector<unsigned char>(4, 0);
		av.value[0] |= 0x2;
		serverCtx.emplace_back(av);
	}
}

void AddChannelBindings(std::vector<ntlm::AtributeValue> &serverCtx) {
	auto it = std::find_if(serverCtx.begin(), serverCtx.end(), [](const ntlm::AtributeValue& v) {return v.id == MsvChannelBindings; });
	ntlm::AtributeValue av;
	av.value = std::vector<unsigned char>(MD5_DIGEST_LENGTH, 0);
	av.id = MsvChannelBindings;
	if (it != serverCtx.end())
		ntlm::CalculateMD5(it->value.data(), it->value.size(), av.value.data());
	serverCtx.emplace_back(std::move(av));
}

void AddClientSuppliedTargetName(std::vector<ntlm::AtributeValue> &serverCtx, const std::string& name) {
	auto ucs2ServiceName = vs::UTF8toUTF16Convert(name);
	ntlm::AtributeValue av;
	av.id = MsvAvTargetName;
	auto pData = reinterpret_cast<const unsigned char *>(ucs2ServiceName.data());
	av.value.insert(av.value.end(), pData, pData + ucs2ServiceName.length() * 2);
	serverCtx.emplace_back(std::move(av));
}

void EditServerContext(std::vector<ntlm::AtributeValue> &serverCtx, uint64_t &out_winTimestamp) {
	auto it = std::find_if(serverCtx.begin(), serverCtx.end(), [](const ntlm::AtributeValue& v) {return v.id == MsvAvTimestamp; });
	if (it != serverCtx.end()) {
		memcpy(&out_winTimestamp, it->value.data(), it->value.size());
		out_winTimestamp = le64toh(out_winTimestamp);
		AddIndicationOfMIC(serverCtx);
	}
	AddChannelBindings(serverCtx);
	AddClientSuppliedTargetName(serverCtx, "tc_service");
}

bool sec::NTLMv2Client::ProcessChallengeMessage(const void * inBuf, size_t inLen, uint32_t flags, bool useNTLMv2, std::vector<unsigned char>& outAuthenticateMsg)
{
	if (!inBuf || inLen == 0)
		return false;

	ntlm::CHALLENGE_MSG type2 = {};	// CHALLENGE MSG
	ntlm::AUTH_MSG type3 = {};	// AUTHENTICATE MSG

	if (!ntlm::DecodeChallengeMsg(inBuf, inLen, type2)) {
		dstream4 << "NTLMv2Client::ProcessChallengeMessage Error:failed to decode challange!";
		return false;
	}

	// rule from here https://msdn.microsoft.com/en-us/library/cc236650.aspx
	if (NTLMSSP_NEGOTIATE_DATAGRAM & flags)
		flags |= NTLMSSP_NEGOTIATE_KEY_EXCH;

	flags = type2.negotiateFlags | flags;
	type3.userName = m_userName;
	type3.domainName = m_domain;
	type3.workstation = "workstation";

	// add what we support, remove what we don't support
	if(flags & NTLMSSP_NEGOTIATE_UNICODE)
		flags &= ~NTLM_NEGOTIATE_OEM;
	flags &= ~NTLMSSP_NEGOTIATE_LM_KEY;
	flags &= ~NTLMSSP_NEGOTIATE_56;
	flags |= NTLMSSP_REQUEST_TARGET;

	type3.negotiateFlags = flags;


	uint64_t winTimestamp(0);
	EditServerContext(type2.targetInfo, winTimestamp);
	std::vector<unsigned char> rawTargetInfo;
	EncodeTargetInfo(type2.targetInfo, rawTargetInfo);

	bool timestampSended = winTimestamp != 0;
	if (!timestampSended)
		winTimestamp = tu::UnixSecondsToWindowsTicks(std::chrono::system_clock::now());

	unsigned char exportedSessionKey[MD5_DIGEST_LENGTH];
	if (!ntlm::CalculateNTLMAuth(
		m_userName, m_passwd, m_domain, rawTargetInfo, type2.serverChallenge, flags, winTimestamp, useNTLMv2,
		type3.ntChallengeResponse, type3.lmChallengeResponse, exportedSessionKey, type3.encryptedRandomSessionKey))
	{
		dstream4 << "NTLMv2Client::ProcessChallengeMessage Error:failed to calculate session key!";
		return false;
	}

	if (!(NTLMSSP_NEGOTIATE_LM_KEY & flags))
		type3.lmChallengeResponse.clear();

	if(timestampSended && useNTLMv2)
		type3.lmChallengeResponse = std::vector<unsigned char>(24,0);

	ntlm::SIGNKEY(flags, exportedSessionKey, "Client", m_sendKey.signkey);
	ntlm::SIGNKEY(flags, exportedSessionKey, "Server", m_recvKey.signkey);
	ntlm::SEALKEY(flags, exportedSessionKey, "Client", m_sendKey.sealkey);
	ntlm::SEALKEY(flags, exportedSessionKey, "Server", m_recvKey.sealkey);
	RC4_set_key(m_sendKey.rc4Handle.get().get(), m_sendKey.sealkey.size(), m_sendKey.sealkey.data());
	RC4_set_key(m_recvKey.rc4Handle.get().get(), m_recvKey.sealkey.size(), m_recvKey.sealkey.data());

	size_t micOffset(0);
	if(!ntlm::AuthMessageEncode(type3, outAuthenticateMsg, micOffset))
	{
		dstream4 << "NTLMv2Client::ProcessChallengeMessage Error:failed to encode authenticate message!";
		return false;
	}

	if (timestampSended) {	// when MsvAvTimestamp present, the client SHOULD provide a MIC
		std::array<unsigned char, 16> signature = {};
		std::vector<unsigned char> concat((unsigned char*)inBuf, (unsigned char*)inBuf + inLen);	// CHALLENGE_MESSAGE
		concat.insert(concat.end(), outAuthenticateMsg.begin(), outAuthenticateMsg.end());			// ConcatenationOf(CHALLENGE_MESSAGE, AUTHENTICATE_MESSAGE)

		if (!SignAuthMessage(concat.data(), concat.size(), exportedSessionKey, signature)) {		// HMAC_MD5(ExportedSessionKey, ConcatenationOf(CHALLENGE_MESSAGE, AUTHENTICATE_MESSAGE))
			dstream4 << "NTLMv2Client::ProcessChallengeMessage Error: failed to make MIC!";
			return false;
		}

		memcpy(outAuthenticateMsg.data() + micOffset, signature.data(), signature.size());
	}
	m_negotiatedFlags = flags;
	return !outAuthenticateMsg.empty();
}

bool sec::NTLMv2Client::SignMessage(const void * inBuf, size_t inLen, std::array<unsigned char, 16>& outSignature)
{
	if (!inBuf || inLen == 0)
		return false;

    if (m_negotiatedFlags & NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY) //key reseting required only in NTLM2 ses. security (unlike what MSDN docs says)
	    ntlm::ResetRC4Key(m_sendKey, m_negotiatedFlags);
	return ntlm::MakeSignature(inBuf, inLen, m_sendKey, m_negotiatedFlags, outSignature);
}

bool sec::NTLMv2Client::SealMessage(const void * inBuf, size_t inLen, std::vector<unsigned char>& encrypted, std::array<unsigned char, 16>& outSignature)
{
	if (!inBuf || inLen == 0)
		return false;

	encrypted.resize(inLen);
	ntlm::ResetRC4Key(m_sendKey, m_negotiatedFlags);
	RC4(m_sendKey.rc4Handle.get().get(), inLen, static_cast<const unsigned char*>(inBuf), encrypted.data());
	return ntlm::MakeSignature(inBuf, inLen, m_sendKey, m_negotiatedFlags, outSignature);
}

#pragma once

#include <array>
#include <string>
#include <vector>

#include "NTLMMessages.h"

/* secure */
namespace sec {
	/* class represent client side of NTLM handshake
		1. Client -> NEGOTIATE_MESSAGE ->    Server	// this part is absent due to datagram mode in sip
		2. Client <- CHALLENGE_MESSAGE <-    Server
		3. Client -> AUTHENTICATE_MESSAGE -> Server
	*/
class NTLMv2Client{
	std::string m_userName;
	std::string m_domain;
	std::string m_passwd;
	uint32_t m_negotiatedFlags = 0;

	ntlm::sessionKey m_sendKey = {};
	ntlm::sessionKey m_recvKey = {};

	/* Sign procedure from here https://msdn.microsoft.com/en-us/library/cc236676.aspx */
	bool SignAuthMessage(const void *inBuf, size_t inLen, unsigned char ExportedSessionKey[16], std::array<unsigned char, 16> &outSignature);
public:
	void SaveCredetinals(const std::string& userName, const std::string& domain, const std::string& passwd);

	/* processing CHALLENGE_MESSAGE from server https://msdn.microsoft.com/en-us/library/cc236642.aspx
	   and making AUTHENTICATE_MESSAGE for server https://msdn.microsoft.com/en-us/library/cc236643.aspx
	*/
	bool ProcessChallengeMessage(const void *inBuf, size_t inLen, uint32_t flags, bool useNTLMv2, std::vector<unsigned char> &outAuthenticateMsg);
	bool SignMessage(const void *inBuf, size_t inLen, std::array<unsigned char, 16> &outSignature);
	bool SealMessage(const void *inBuf, size_t inLen, std::vector<unsigned char> &encrypted, std::array<unsigned char, 16> &outSignature);
};
}
#include "VS_SIPSeqSSPI.h"
#include "SecureLib/NTLMFlags.h"

VS_SIPSeqSSPI::VS_SIPSeqSSPI(eType type) : m_type(type) {
}

VS_SIPSeqSSPI::~VS_SIPSeqSSPI() {
}

bool VS_SIPSeqSSPI::AcquireCred(const char *user, const char *pass, const char *domain) {
	if (!user || !pass || !domain)
		return false;
	m_ntlmClient.SaveCredetinals(user, domain, pass);
	return true;
}

VS_SIPSeqSSPI::eStatus VS_SIPSeqSSPI::InitContext(const void *in_buf, size_t in_len, std::vector<unsigned char> &out_buf) {
	if (!in_buf || in_len == 0) {
		m_inited = true;
		return ContinueNeeded;
	}

	int32_t flags = 0
		| NTLMSSP_NEGOTIATE_DATAGRAM
		| NTLMSSP_NEGOTIATE_SIGN
		| NTLMSSP_NEGOTIATE_NTLM
		| NTLMSSP_NEGOTIATE_SEAL;
	m_inited = m_ntlmClient.ProcessChallengeMessage(in_buf, in_len, flags, true, out_buf);
	if(!m_inited)
		return Error;

	return Ok;
}

bool VS_SIPSeqSSPI::MakeSignature(const void *in_buf, size_t in_len, std::array<unsigned char, 16> &out_buf) {
	return m_ntlmClient.SignMessage(in_buf, in_len, out_buf);
}

bool VS_SIPSeqSSPI::EncryptMessage(const void * in_buf, size_t in_len, std::vector<unsigned char>& encrypted, std::array<unsigned char, 16>& signature)
{
	return m_ntlmClient.SealMessage(in_buf, in_len, encrypted, signature);
}

bool VS_SIPSeqSSPI::ContexInited() const
{
	return m_inited;
}
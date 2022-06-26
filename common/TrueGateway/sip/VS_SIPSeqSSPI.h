#pragma once

#include <memory>
#include <vector>

#include "SecureLib/NTLMv2Client.h"

class VS_SIPSeqSSPI {
public:
	enum eType { NTLM };
	enum eStatus { Ok, ContinueNeeded, Error };
	explicit VS_SIPSeqSSPI(eType type);
	~VS_SIPSeqSSPI();

	eType type() const { return m_type; }

	bool AcquireCred(const char *user, const char *pass, const char *domain);
	eStatus InitContext(const void *in_buf, size_t in_len, std::vector<unsigned char> &out_buf);
	bool MakeSignature(const void *in_buf, size_t in_len, std::array<unsigned char, 16> &out_buf);
	bool EncryptMessage(const void *in_buf, size_t in_len, std::vector<unsigned char> &encrypted, std::array<unsigned char, 16> &signature);
	bool ContexInited() const;
private:
	eType m_type;
	bool m_inited = false;
	sec::NTLMv2Client m_ntlmClient;
};
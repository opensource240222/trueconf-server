#pragma once

#include "VS_SIPAuthScheme.h"

#include <boost/regex.hpp>

class VS_SIPAuthDigest: public VS_SIPAuthScheme
{
public:
	VS_SIPAuthDigest();
	~VS_SIPAuthDigest();

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	TSIPErrorCodes Init(VS_RTSP_ParserInfo* call) override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	eSIP_AUTH_SCHEME scheme() const override;

	static std::string GenerateNonceValue();

private:
	bool StrToHex(std::string aStr, std::size_t &aRet) const;
};
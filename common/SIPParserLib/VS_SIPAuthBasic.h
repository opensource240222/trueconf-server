#pragma once

#include "VS_SIPAuthScheme.h"

class VS_SIPAuthBasic: public VS_SIPAuthScheme
{
public:
	VS_SIPAuthBasic();
	~VS_SIPAuthBasic();

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;
	TSIPErrorCodes Init(VS_RTSP_ParserInfo* call) override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	eSIP_AUTH_SCHEME scheme() const override;
};
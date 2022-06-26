#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <memory>
#include <string>

class VS_RTSP_UserAgent : public VS_BaseField
{
public:
	VS_RTSP_UserAgent();
	std::string GetUAString() const {return m_sUserAgent;};
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;
	TSIPErrorCodes Init(VS_RTSP_ParserInfo * call) override;
	void Clean() noexcept override;
private:
	std::string m_sUserAgent;
};

std::unique_ptr<VS_BaseField> VS_RTSP_UserAgent_Instanse();
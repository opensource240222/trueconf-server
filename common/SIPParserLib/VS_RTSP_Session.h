#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <chrono>
#include <memory>
#include <string>

class VS_RTSP_Session : public VS_BaseField
{
public:
	VS_RTSP_Session();
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;
	std::string GetSessionId() const {return m_sSessionId;}
	TSIPErrorCodes Init(VS_RTSP_ParserInfo* call) override;
	std::chrono::seconds GetTimeout() const { return m_timeout; };

private:
	std::string m_sSessionId;
	std::chrono::seconds m_timeout; //https://tools.ietf.org/html/rfc2326 [12.37 Session]
};
std::unique_ptr<VS_BaseField> VS_RTSP_Session_Instance();
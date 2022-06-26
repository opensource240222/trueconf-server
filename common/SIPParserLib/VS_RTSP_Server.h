#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <memory>
#include <string>

class VS_RTSP_Server : public VS_BaseField
{
public:
	TSIPErrorCodes Encode (VS_SIPBuffer &aBuffer) const override;
	TSIPErrorCodes Decode (VS_SIPBuffer &aBuffer) override;

	const std::string &Get(void) const;
private:
	std::string m_server;
};
std::unique_ptr<VS_BaseField> VS_RTSP_Server_Instance();
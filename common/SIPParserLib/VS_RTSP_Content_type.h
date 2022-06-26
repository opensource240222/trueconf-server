#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <memory>
#include <string>

class VS_RTSP_Content_type : public VS_BaseField
{
public:
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;
	TSIPErrorCodes Init(VS_RTSP_ParserInfo * info) override;
	const std::string &GetContentType() const;

private:
	std::string m_sContentType;
};
std::unique_ptr<VS_BaseField> VS_RTSP_Content_type_Instance();
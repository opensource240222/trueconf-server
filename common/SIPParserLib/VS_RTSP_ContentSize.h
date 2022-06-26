#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <memory>

class VS_RTSP_ContentLength : public VS_BaseField
{
public:
	VS_RTSP_ContentLength();
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Init (VS_RTSP_ParserInfo * info) override;
	std::size_t GetContentLength() const;

private:
	std::size_t m_ContentLength;
};
std::unique_ptr<VS_BaseField> VS_RTSP_ContentLength_Instance();
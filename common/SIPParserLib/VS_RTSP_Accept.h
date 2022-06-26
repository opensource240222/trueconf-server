#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <memory>
#include <string>

class VS_RTSP_Accept : public VS_BaseField
{
public:
	std::string GetAcceptString() const {return m_sAccept;};
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;
	TSIPErrorCodes Init(VS_RTSP_ParserInfo * parsingInfo) override;
private:
	std::string m_sAccept;

};

std::unique_ptr<VS_BaseField> VS_RTSP_Accept_Instanse();
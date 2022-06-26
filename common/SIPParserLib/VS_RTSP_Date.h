#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <memory>
#include <string>

class VS_RTSP_Date : public VS_BaseField
{
public:
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;
	const std::string &GetDate() const {return m_date;}
private:
	std::string m_date;
};
std::unique_ptr<VS_BaseField> VS_RTSP_Date_Instance();
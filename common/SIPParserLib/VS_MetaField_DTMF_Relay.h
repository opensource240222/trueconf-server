#pragma once

#include "../SIPParserBase/VS_BaseField.h"

class VS_MetaField_DTMF_Relay: public VS_BaseField
{
public:
	explicit VS_MetaField_DTMF_Relay(const char dtmf_digit);
	~VS_MetaField_DTMF_Relay();

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;

private:
	char m_dtmf_digit;
};
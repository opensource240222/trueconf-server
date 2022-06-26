#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <memory>

class VS_RTSP_CSeq : public VS_BaseField
{
public:
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;
	int GetValue() const {return m_CSeqValue;}
	TSIPErrorCodes Init(VS_RTSP_ParserInfo * info) override;
	VS_RTSP_CSeq();
private:
	int m_CSeqValue;
};
std::unique_ptr <VS_BaseField> VS_RTSP_CSeq_Instance();
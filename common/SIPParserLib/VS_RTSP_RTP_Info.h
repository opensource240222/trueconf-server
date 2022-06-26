#pragma once

#include "../SIPParserBase/VS_BaseField.h"
#include "VS_RTSP_Const.h"

class VS_RTSP_RTP_Info : public VS_BaseField
{
public:
	VS_RTSP_RTP_Info();
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;
private:
	sRTPInfo m_rtpInfo;
};
VS_BaseField* VS_RTP_Info_Instance();
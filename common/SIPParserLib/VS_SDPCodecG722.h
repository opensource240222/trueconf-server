#pragma once

#include "VS_SDPCodec.h"

class VS_SDPCodecG722: public VS_SDPCodec
{
public:
	VS_SDPCodecG722();
	~VS_SDPCodecG722();

	TSIPErrorCodes Decode(VS_SIPBuffer &_buffer)override;
	TSIPErrorCodes Encode(VS_SIPBuffer &_buffer) const override;
	std::unique_ptr<VS_SDPCodec> Clone() const override;
};
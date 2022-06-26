#pragma once

#include "VS_SDPCodec.h"

class VS_SDPCodecMPA : public VS_SDPCodec
{
public:
	VS_SDPCodecMPA();
	~VS_SDPCodecMPA();

	TSIPErrorCodes Decode(VS_SIPBuffer &_buffer)override;
	TSIPErrorCodes Encode(VS_SIPBuffer &_buffer) const override;
	std::unique_ptr<VS_SDPCodec> Clone() const override;
};
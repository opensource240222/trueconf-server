#pragma once
#include "VS_SDPCodec.h"

class VS_SDPCodecOpus : public VS_SDPCodec {
public:
	VS_SDPCodecOpus();
	VS_SDPCodecOpus(const VS_SDPCodecOpus&);
	virtual ~VS_SDPCodecOpus();

	TSIPErrorCodes Decode(VS_SIPBuffer &_buffer)override;
	TSIPErrorCodes Encode(VS_SIPBuffer &_buffer) const override;
	std::unique_ptr<VS_SDPCodec> Clone() const override;
};
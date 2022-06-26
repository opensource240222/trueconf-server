#pragma once

#include "VS_SDPCodec.h"

class VS_SDPCodecXH264UC : public VS_SDPCodec {
public:
	VS_SDPCodecXH264UC();
	VS_SDPCodecXH264UC(const VS_SDPCodecXH264UC&);
	~VS_SDPCodecXH264UC();

	TSIPErrorCodes Decode(VS_SIPBuffer &_buffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &_buffer) const override;
	std::unique_ptr<VS_SDPCodec> Clone() const override;

	void FillRcvVideoMode(VS_GatewayVideoMode &mode) const override;
	void FillSndVideoMode(VS_GatewayVideoMode &mode) const override;
};
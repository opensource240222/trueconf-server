#pragma once
#include "VS_SDPCodec.h"

class VS_SDPTelEvent : public VS_SDPCodec {
public:
	VS_SDPTelEvent();
	VS_SDPTelEvent(const VS_SDPTelEvent&);
    virtual ~VS_SDPTelEvent();

	TSIPErrorCodes Decode(VS_SIPBuffer &_buffer) override;
    TSIPErrorCodes Encode(VS_SIPBuffer &_buffer) const override;

	std::unique_ptr<VS_SDPCodec> Clone() const override;
};
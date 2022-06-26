#pragma once

#include "VS_SDPCodec.h"

class VS_SDPCodecSiren14 : public VS_SDPCodec
{
public:
	explicit VS_SDPCodecSiren14( unsigned long bitrate );
	VS_SDPCodecSiren14(const VS_SDPCodecSiren14&);
	~VS_SDPCodecSiren14();
	TSIPErrorCodes Encode(VS_SIPBuffer &_buffer) const override;
	TSIPErrorCodes Decode(VS_SIPBuffer &_buffer)override;
	std::unique_ptr<VS_SDPCodec> Clone() const override;

	VS_SDPCodecSiren14& operator=(const VS_SDPCodecSiren14&) = delete;

private:
	unsigned long m_bitrate;
};

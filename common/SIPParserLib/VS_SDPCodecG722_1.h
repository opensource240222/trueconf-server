#pragma once

#include "VS_SDPCodec.h"

#include <boost/regex.hpp>

class VS_SDPCodecG722_1: public VS_SDPCodec
{
public:
	explicit VS_SDPCodecG722_1(unsigned long bitrate = 24000);
	VS_SDPCodecG722_1(const VS_SDPCodecG722_1&);
	~VS_SDPCodecG722_1();

	VS_SDPCodecG722_1& operator=(const VS_SDPCodecG722_1&) = delete;

	std::unique_ptr<VS_SDPCodec> Clone() const override;

	TSIPErrorCodes Decode(VS_SIPBuffer &_buffer)override;
	TSIPErrorCodes Encode(VS_SIPBuffer &_buffer) const override;


private:
	unsigned long m_bitrate;
};
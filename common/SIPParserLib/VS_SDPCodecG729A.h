#pragma once

#include "VS_SDPCodec.h"

#include <boost/regex.hpp>

class VS_SDPCodecG729A: public VS_SDPCodec
{
public:
	const static boost::regex e;

	VS_SDPCodecG729A();
	VS_SDPCodecG729A(const VS_SDPCodecG729A&);
	~VS_SDPCodecG729A();

	VS_SDPCodecG729A& operator=(const VS_SDPCodecG729A&) = delete;

	std::unique_ptr<VS_SDPCodec> Clone() const override;

	TSIPErrorCodes Decode(VS_SIPBuffer &_buffer)override;
	TSIPErrorCodes Encode(VS_SIPBuffer &_buffer) const override;

private:
	bool FindParam_AnnexB(string_view _in);

private:
	bool					m_AnnexB;
};
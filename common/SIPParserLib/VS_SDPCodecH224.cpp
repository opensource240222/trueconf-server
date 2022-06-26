#include "VS_SDPCodecH224.h"
#include "std-generic/compat/memory.h"

VS_SDPCodecH224::VS_SDPCodecH224()
{
	SetNumChannels(1);
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
}

TSIPErrorCodes VS_SDPCodecH224::Decode(VS_SIPBuffer &_buffer)
{
	return TSIPErrorCodes::e_UNKNOWN;
}

TSIPErrorCodes VS_SDPCodecH224::Encode(VS_SIPBuffer &_buffer) const
{
	if (!IsValid())
		return GetLastClassError();
	return VS_SDPCodec::Encode(_buffer);
}

std::unique_ptr<VS_SDPCodec> VS_SDPCodecH224::Clone() const
{
	return vs::make_unique<VS_SDPCodecH224>(*this);
}
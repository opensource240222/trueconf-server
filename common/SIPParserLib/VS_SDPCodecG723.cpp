#include "VS_SDPCodecG723.h"
#include "std-generic/compat/memory.h"

VS_SDPCodecG723::VS_SDPCodecG723()
{
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
}

VS_SDPCodecG723::~VS_SDPCodecG723()
{

}

TSIPErrorCodes VS_SDPCodecG723::Decode(VS_SIPBuffer &)
{
	return TSIPErrorCodes::e_UNKNOWN;
}

TSIPErrorCodes VS_SDPCodecG723::Encode(VS_SIPBuffer &_buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();
	return VS_SDPCodec::Encode(_buffer);
}

std::unique_ptr<VS_SDPCodec> VS_SDPCodecG723::Clone() const
{
	return vs::make_unique<VS_SDPCodecG723>(*this);
}
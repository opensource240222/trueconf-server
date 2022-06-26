#include "VS_SDPCodecG711U.h"
#include "std-generic/compat/memory.h"

VS_SDPCodecG711U::VS_SDPCodecG711U()
{
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
}

VS_SDPCodecG711U::~VS_SDPCodecG711U()
{

}

TSIPErrorCodes VS_SDPCodecG711U::Decode(VS_SIPBuffer &_buffer)
{

	return TSIPErrorCodes::e_UNKNOWN;
}

TSIPErrorCodes VS_SDPCodecG711U::Encode(VS_SIPBuffer &_buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();
	return VS_SDPCodec::Encode(_buffer);
}

std::unique_ptr<VS_SDPCodec> VS_SDPCodecG711U::Clone() const
{
	return vs::make_unique<VS_SDPCodecG711U>(*this);
}
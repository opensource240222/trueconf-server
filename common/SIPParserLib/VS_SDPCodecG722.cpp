#include "VS_SDPCodecG722.h"
#include "std-generic/compat/memory.h"

VS_SDPCodecG722::VS_SDPCodecG722()
{
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
}

VS_SDPCodecG722::~VS_SDPCodecG722()
{

}

TSIPErrorCodes VS_SDPCodecG722::Decode(VS_SIPBuffer &_buffer)
{
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPCodecG722::Encode(VS_SIPBuffer &_buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();
	return VS_SDPCodec::Encode(_buffer);
}

std::unique_ptr<VS_SDPCodec> VS_SDPCodecG722::Clone() const
{
	return vs::make_unique<VS_SDPCodecG722>(*this);
}
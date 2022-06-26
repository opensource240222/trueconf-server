#include "VS_SDPCodecG728.h"
#include "std-generic/compat/memory.h"

VS_SDPCodecG728::VS_SDPCodecG728()
{
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
}

VS_SDPCodecG728::~VS_SDPCodecG728()
{

}

TSIPErrorCodes VS_SDPCodecG728::Decode(VS_SIPBuffer &)
{

	return TSIPErrorCodes::e_UNKNOWN;
}

TSIPErrorCodes VS_SDPCodecG728::Encode(VS_SIPBuffer &_buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();
	return VS_SDPCodec::Encode(_buffer);
}

std::unique_ptr<VS_SDPCodec> VS_SDPCodecG728::Clone() const
{
	return vs::make_unique<VS_SDPCodecG728>(*this);
}
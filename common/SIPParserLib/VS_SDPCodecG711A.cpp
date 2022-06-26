#include "VS_SDPCodecG711A.h"
#include "std-generic/compat/memory.h"

VS_SDPCodecG711A::VS_SDPCodecG711A()
{
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
}

VS_SDPCodecG711A::~VS_SDPCodecG711A()
{

}

TSIPErrorCodes VS_SDPCodecG711A::Decode(VS_SIPBuffer &_buffer)
{

	return TSIPErrorCodes::e_UNKNOWN;
}

TSIPErrorCodes VS_SDPCodecG711A::Encode(VS_SIPBuffer &_buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	const TSIPErrorCodes err = VS_SDPCodec::Encode(_buffer);
	if ( err != TSIPErrorCodes::e_ok )
	{
		return err;
	}

	return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_SDPCodec> VS_SDPCodecG711A::Clone() const
{
	return vs::make_unique<VS_SDPCodecG711A>(*this);
}
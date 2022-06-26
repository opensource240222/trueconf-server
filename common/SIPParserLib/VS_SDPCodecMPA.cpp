#include "VS_SDPCodecMPA.h"
#include "std-generic/compat/memory.h"

VS_SDPCodecMPA::VS_SDPCodecMPA()
{
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
}

VS_SDPCodecMPA::~VS_SDPCodecMPA()
{
}

TSIPErrorCodes VS_SDPCodecMPA::Decode(VS_SIPBuffer &_buffer)
{
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPCodecMPA::Encode(VS_SIPBuffer &_buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();
	return VS_SDPCodec::Encode(_buffer);
}

std::unique_ptr<VS_SDPCodec> VS_SDPCodecMPA::Clone() const
{
	return vs::make_unique<VS_SDPCodecMPA>(*this);
}

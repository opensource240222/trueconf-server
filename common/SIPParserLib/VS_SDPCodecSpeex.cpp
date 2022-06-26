#include "VS_SDPCodecSpeex.h"
#include "std-generic/compat/memory.h"

VS_SDPCodecSpeex::VS_SDPCodecSpeex()
{
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
}

VS_SDPCodecSpeex::~VS_SDPCodecSpeex()
{

}

TSIPErrorCodes VS_SDPCodecSpeex::Decode(VS_SIPBuffer &_buffer)
{
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPCodecSpeex::Encode(VS_SIPBuffer &_buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	const TSIPErrorCodes err = VS_SDPCodec::Encode(_buffer);
	if ( err != TSIPErrorCodes::e_ok )
	{
		return err;
	}

	_buffer.AddData("a=ptime:80");
	_buffer.AddData("\r\n");


	return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_SDPCodec> VS_SDPCodecSpeex::Clone() const
{
	return vs::make_unique<VS_SDPCodecSpeex>(*this);
}
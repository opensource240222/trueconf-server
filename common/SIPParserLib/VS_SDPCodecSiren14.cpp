#include "VS_SDPCodecSiren14.h"
#include "../tools/Server/CommonTypes.h"
#include "std-generic/compat/memory.h"

VS_SDPCodecSiren14::VS_SDPCodecSiren14(unsigned long bitrate) : m_bitrate(bitrate)
{
}

VS_SDPCodecSiren14::VS_SDPCodecSiren14(const VS_SDPCodecSiren14& src):VS_SDPCodec(src)
{
	this->m_bitrate = src.m_bitrate;
}

VS_SDPCodecSiren14::~VS_SDPCodecSiren14()
{

}

TSIPErrorCodes VS_SDPCodecSiren14::Encode(VS_SIPBuffer &_buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	TSIPErrorCodes err = VS_SDPCodec::Encode(_buffer);
	if ( err != TSIPErrorCodes::e_ok )
	{
		return err;
	}

	char buff[30 +
		std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 +
		std::numeric_limits<unsigned long>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
	snprintf(buff, sizeof(buff), "a=fmtp:%d bitrate=%lu\r\n", m_pt, m_bitrate);
	_buffer.AddData(buff, strlen(buff));

	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPCodecSiren14::Decode(VS_SIPBuffer &_buffer)
{
	std::unique_ptr<char[]> data;
	TSIPErrorCodes err = _buffer.GetAllDataAllocConst(data);
	if (TSIPErrorCodes::e_ok != err )
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	m_bitrate = 0;
	if(!FindParam_bitrate(data.get(), m_bitrate))
	{
		SetValid(false);
		SetError(err);
		return err;
	}
	if (m_bitrate == 0) m_bitrate = 48000;

	this->SetCodecType( m_bitrate == 24000 ? e_rcvSIREN14_24 :
						m_bitrate == 32000 ? e_rcvSIREN14_32 :
											 e_rcvSIREN14_48);
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_SDPCodec> VS_SDPCodecSiren14::Clone() const
{
	return vs::make_unique<VS_SDPCodecSiren14>(*this);
}
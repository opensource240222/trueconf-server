#include "VS_SDPCodecG722_1.h"
#include "../tools/Server/CommonTypes.h"
#include "std-generic/compat/memory.h"

VS_SDPCodecG722_1::VS_SDPCodecG722_1(unsigned long bitrate): m_bitrate(bitrate)
{

}

VS_SDPCodecG722_1::~VS_SDPCodecG722_1()
{

}

VS_SDPCodecG722_1::VS_SDPCodecG722_1(const VS_SDPCodecG722_1& src) : VS_SDPCodec(src), m_bitrate(src.m_bitrate)
{}


TSIPErrorCodes VS_SDPCodecG722_1::Decode(VS_SIPBuffer &_buffer)
{
	std::unique_ptr<char[]> data;
	TSIPErrorCodes err = _buffer.GetAllDataAllocConst(data);

	if (TSIPErrorCodes::e_ok != err || !FindParam_bitrate(data.get(), m_bitrate))
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	// update codec type (we support only bitrate=24kbps and 32kbps for send g.722.1)
	this->SetCodecType((m_bitrate==24000)? e_rcvG722124: (m_bitrate==32000)? e_rcvG722132 : e_rcvNone);

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPCodecG722_1::Encode(VS_SIPBuffer &_buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	const TSIPErrorCodes err = VS_SDPCodec::Encode(_buffer);
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

std::unique_ptr<VS_SDPCodec> VS_SDPCodecG722_1::Clone() const
{
	return vs::make_unique<VS_SDPCodecG722_1>(*this);
}
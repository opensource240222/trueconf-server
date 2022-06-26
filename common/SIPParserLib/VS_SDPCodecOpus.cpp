#include "VS_SDPCodecOpus.h"
#include <sstream>
#include "std-generic/compat/memory.h"

VS_SDPCodecOpus::VS_SDPCodecOpus() {

}

VS_SDPCodecOpus::VS_SDPCodecOpus(const VS_SDPCodecOpus& src):VS_SDPCodec(src)
{

}

VS_SDPCodecOpus::~VS_SDPCodecOpus() {

}

TSIPErrorCodes VS_SDPCodecOpus::Decode(VS_SIPBuffer &_buffer) {

	std::unique_ptr<char[]> data;
	TSIPErrorCodes err = _buffer.GetAllDataAllocConst(data);
	if (TSIPErrorCodes::e_ok != err) {
		SetValid(false);
		SetError(err);
		return err;
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPCodecOpus::Encode(VS_SIPBuffer &_buffer) const {
	if (!IsValid())
		return GetLastClassError();

	char buf[64];
	sprintf(buf, "a=rtpmap:%i OPUS/48000/2\r\n", m_pt);
	_buffer.AddData(buf, strlen(buf));
	sprintf(buf, "a=fmtp:%i maxplaybackrate=16000\r\n", m_pt);
	_buffer.AddData(buf, strlen(buf));

	return TSIPErrorCodes::e_ok;
}


std::unique_ptr<VS_SDPCodec> VS_SDPCodecOpus::Clone() const
{
	return vs::make_unique<VS_SDPCodecOpus>(*this);
}
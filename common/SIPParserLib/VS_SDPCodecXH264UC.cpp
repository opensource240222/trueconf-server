#include "VS_SDPCodecXH264UC.h"

#include "../tools/Server/CommonTypes.h"
#include "std-generic/compat/memory.h"

VS_SDPCodecXH264UC::VS_SDPCodecXH264UC() {

}

VS_SDPCodecXH264UC::VS_SDPCodecXH264UC(const VS_SDPCodecXH264UC&src):VS_SDPCodec(src)
{

}

VS_SDPCodecXH264UC::~VS_SDPCodecXH264UC() {

}

TSIPErrorCodes VS_SDPCodecXH264UC::Decode(VS_SIPBuffer &) {
	// ignore for now
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPCodecXH264UC::Encode(VS_SIPBuffer &_buffer) const {
	if (!IsValid())
		return GetLastClassError();

	const TSIPErrorCodes err = VS_SDPCodec::Encode(_buffer);
	if (err != TSIPErrorCodes::e_ok) {
		return err;
	}

	_buffer.AddData("a=fmtp:");
	_buffer.AddData(std::to_string(this->GetPT()));
	_buffer.AddData(" packetization-mode=1;mst-mode=NI-TC\r\n");

	return TSIPErrorCodes::e_ok;
}

void VS_SDPCodecXH264UC::FillRcvVideoMode(VS_GatewayVideoMode &mode) const {
	mode.Mode = 31;
}

void VS_SDPCodecXH264UC::FillSndVideoMode(VS_GatewayVideoMode &mode) const {
	mode.Mode = 31;
}

std::unique_ptr<VS_SDPCodec> VS_SDPCodecXH264UC::Clone() const
{
	return vs::make_unique<VS_SDPCodecXH264UC>(*this);
}
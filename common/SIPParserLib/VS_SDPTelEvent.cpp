#include "VS_SDPTelEvent.h"
#include "std-generic/compat/memory.h"
#include <sstream>

VS_SDPTelEvent::VS_SDPTelEvent() {

}

VS_SDPTelEvent::VS_SDPTelEvent(const VS_SDPTelEvent& src):VS_SDPCodec(src)
{

}

VS_SDPTelEvent::~VS_SDPTelEvent() {

}

TSIPErrorCodes VS_SDPTelEvent::Decode(VS_SIPBuffer &_buffer) {
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

TSIPErrorCodes VS_SDPTelEvent::Encode(VS_SIPBuffer &_buffer) const {
    if (!IsValid())
        return GetLastClassError();

    char buf[64];
    sprintf(buf, "a=rtpmap:%i telephone-event/8000\r\n", m_pt);
    _buffer.AddData(buf, strlen(buf));

    return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_SDPCodec> VS_SDPTelEvent::Clone() const
{
	return vs::make_unique<VS_SDPTelEvent>(*this);
}

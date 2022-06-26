
#include "trueconf_video_decoder_factory.h"
#include "trueconf_video_decoder.h"
#include "trueconf_callback_defines.h"

#include "api/video_codecs/sdp_video_format.h"
#include "media/base/mediaconstants.h"

using namespace tc_webrtc_api;

TrueConfVideoDecoderFactory::TrueConfVideoDecoderFactory(TrueConfVideoDecoderCallback *callback)
	: m_callback(callback)
{
	m_sdpFormats.push_back(webrtc::SdpVideoFormat(cricket::kVp8CodecName));
	{
		/// h.264 sdp
		std::vector <rtc::Optional<std::string>> sdp;
		sdp.push_back({"42001f"}); /// baseline
		sdp.push_back({"42e01f"}); /// constrained baseline
		sdp.push_back({"4d001f"}); /// main
		sdp.push_back({"64001f"}); /// high
		//sdp.push_back({"640c"}); /// constrained high
        for (const auto& it : sdp) {
			webrtc::SdpVideoFormat h264Sdp(cricket::kH264CodecName,
											{ {cricket::kH264FmtpProfileLevelId, *it},
											{ cricket::kH264FmtpLevelAsymmetryAllowed, "1" },
											{ cricket::kH264FmtpPacketizationMode, "1" } } );
			m_sdpFormats.push_back(h264Sdp);
		}
	}
}

TrueConfVideoDecoderFactory::~TrueConfVideoDecoderFactory()
{

}

webrtc::VideoCodecType TrueConfVideoDecoderFactory::GetCodecType(const std::string & payloadName) const
{
	if (STR_CASE_CMP(payloadName.c_str(), cricket::kVp8CodecName) == 0) {
		return webrtc::kVideoCodecVP8;
	}
	else if (STR_CASE_CMP(payloadName.c_str(), cricket::kH264CodecName) == 0) {
		return webrtc::kVideoCodecH264;
	}
	return webrtc::kVideoCodecUnknown;
}

std::unique_ptr<webrtc::VideoDecoder> TrueConfVideoDecoderFactory::CreateVideoDecoder(const webrtc::SdpVideoFormat& format)
{
	std::unique_ptr<webrtc::VideoDecoder> decoder;
	auto type = GetCodecType(format.name);
    if (type != webrtc::kVideoCodecUnknown) {
        std::string stream_id;
        auto it = format.parameters.find("stream_id");
		if (it != format.parameters.end()) {
			stream_id = it->second;
		}
        auto dec = std::make_unique<TrueConfVideoDecoder>(type, stream_id);
		dec->fireReceiveEncodedFrame.connect(m_callback, &TrueConfVideoDecoderCallback::OnReceiveVideoFrame);
		dec->fireUpdatePayload.connect(m_callback, &TrueConfVideoDecoderCallback::OnUpdateReceiveVideoPayload);
		m_callback->fireReceiveRequestKeyFrame.connect(dec.get(), &TrueConfVideoDecoder::OnKeyFrameRequest);
		decoder = std::move(dec);
	}
	return decoder;
}

std::vector<webrtc::SdpVideoFormat> TrueConfVideoDecoderFactory::GetSupportedFormats() const
{
	return m_sdpFormats;
}

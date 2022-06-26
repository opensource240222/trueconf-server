
#include "trueconf_video_encoder_factory.h"
#include "trueconf_video_encoder.h"
#include "trueconf_callback_defines.h"

#include "api/video_codecs/sdp_video_format.h"
#include "media/base/mediaconstants.h"
#include "media/base/h264_profile_level_id.h"

using namespace tc_webrtc_api;

TrueConfVideoEncoderFactory::TrueConfVideoEncoderFactory(TrueConfVideoEncoderCallback *callback)
	: m_callback(callback)
{
	m_sdpFormats.push_back(webrtc::SdpVideoFormat(cricket::kVp8CodecName));
	{
		/// h.264 sdp
		const rtc::Optional<std::string> profile_string = {"42001f"}; /// baseline
		//const rtc::Optional<std::string> profile_string = "42e0"; /// constrained baseline
		//const rtc::Optional<std::string> profile_string = "4d00"; /// main
		//const rtc::Optional<std::string> profile_string = "640c"; /// constrained high
		//const rtc::Optional<std::string> profile_string = "6400"; /// high
		webrtc::SdpVideoFormat h264Sdp(cricket::kH264CodecName,
										{ {cricket::kH264FmtpProfileLevelId, *profile_string},
										{ cricket::kH264FmtpLevelAsymmetryAllowed, "1" },
										{ cricket::kH264FmtpPacketizationMode, "1" } } );
		m_sdpFormats.push_back(h264Sdp);
	}
}

TrueConfVideoEncoderFactory::~TrueConfVideoEncoderFactory()
{

}

std::vector<webrtc::SdpVideoFormat> TrueConfVideoEncoderFactory::GetSupportedFormats() const
{
	return m_sdpFormats;
}

webrtc::VideoEncoderFactory::CodecInfo TrueConfVideoEncoderFactory::QueryVideoEncoder(const webrtc::SdpVideoFormat& format) const
{
	webrtc::VideoEncoderFactory::CodecInfo info;
	info.has_internal_source = false;
	info.is_hardware_accelerated = false;
	return info;
}

bool TrueConfVideoEncoderFactory::IsFormatSupported(const webrtc::SdpVideoFormat& format) const
{
	for (const auto& it : m_sdpFormats) {
		if (STR_CASE_CMP(it.name.c_str(), format.name.c_str()) != 0) {
			continue;
		}
		if (STR_CASE_CMP(format.name.c_str(), cricket::kH264CodecName) != 0 || webrtc::H264::IsSameH264Profile(it.parameters, format.parameters)) {
			return true;
		}
	}
	return false;
}

webrtc::VideoCodecType TrueConfVideoEncoderFactory::GetCodecType(const std::string & payloadName) const
{
	if (STR_CASE_CMP(payloadName.c_str(), cricket::kVp8CodecName) == 0) {
		return webrtc::kVideoCodecVP8;
	}
	else if (STR_CASE_CMP(payloadName.c_str(), cricket::kH264CodecName) == 0) {
		return webrtc::kVideoCodecH264;
	}
	return webrtc::kVideoCodecUnknown;
}

std::unique_ptr<webrtc::VideoEncoder> TrueConfVideoEncoderFactory::CreateVideoEncoder(const webrtc::SdpVideoFormat& format)
{
	std::unique_ptr<webrtc::VideoEncoder> encoder;
	if (IsFormatSupported(format)) {
		auto type = GetCodecType(format.name);
		auto enc = std::make_unique<TrueConfVideoEncoder>(type);
		enc->fireSetRates.connect(m_callback, &TrueConfVideoEncoderCallback::OnSetRates);
		enc->fireRequestKeyFrame.connect(m_callback, &TrueConfVideoEncoderCallback::OnRequestKeyFrame);
		enc->fireUpdatePayload.connect(m_callback, &TrueConfVideoEncoderCallback::OnUpdateSendVideoPayload);
		m_callback->fireReceiveRequestBitrate.connect(enc.get(), &TrueConfVideoEncoder::OnUpdateTargetReceiveBitrate);
		encoder = std::move(enc);
	}
	return encoder;
}



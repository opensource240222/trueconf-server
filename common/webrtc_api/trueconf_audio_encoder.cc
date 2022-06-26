
#include "trueconf_audio_encoder.h"

#include "rtc_base/logging.h"

using namespace tc_webrtc_api;

webrtc::AudioEncoder::CodecType GetAudioCodecType(const char *plname)
{
	if (STR_CASE_CMP(plname, "ISAC") == 0) {
		return webrtc::AudioEncoder::CodecType::kIsac;
	}
	else if (STR_CASE_CMP(plname, "g722") == 0) {
		return webrtc::AudioEncoder::CodecType::kG722;
	}
	else if (STR_CASE_CMP(plname, "opus") == 0) {
		return webrtc::AudioEncoder::CodecType::kOpus;
	}
	else if (STR_CASE_CMP(plname, "PCMA") == 0) {
		return webrtc::AudioEncoder::CodecType::kPcmA;
	}
	else if (STR_CASE_CMP(plname, "PCMU") == 0) {
		return webrtc::AudioEncoder::CodecType::kPcmU;
	}
	return webrtc::AudioEncoder::CodecType::kOther;
}

TrueConfAudioEncoder::TrueConfAudioEncoder(const webrtc::AudioCodecInfo& info, int payloadType, const std::string & payloadName)
{
	m_codecInfo = info;
	m_codecPayload = payloadType;
	m_codecType = GetAudioCodecType(payloadName.c_str());

	RTC_LOG_V(rtc::LoggingSeverity::LS_INFO) << "[TrueConf] TrueConfAudioEncoder : plname = "
		<< payloadName.c_str()
		<< ", pltype = "
		<< m_codecPayload
		<< ", sr = "
		<< m_codecInfo.sample_rate_hz
		<< ", nch = "
		<< m_codecInfo.num_channels
		<< ", rate = "
		<< m_codecInfo.default_bitrate_bps;

}

TrueConfAudioEncoder::~TrueConfAudioEncoder()
{

}

int TrueConfAudioEncoder::SampleRateHz() const
{
	return m_codecInfo.sample_rate_hz;
}

size_t TrueConfAudioEncoder::NumChannels() const
{
	return m_codecInfo.num_channels;
}

int TrueConfAudioEncoder::RtpTimestampRateHz() const
{
	return (m_codecType == webrtc::AudioEncoder::CodecType::kG722) ? m_codecInfo.sample_rate_hz / 2 : m_codecInfo.sample_rate_hz;
}

size_t TrueConfAudioEncoder::Num10MsFramesInNextPacket() const
{
	return 0;
}

size_t TrueConfAudioEncoder::Max10MsFramesInAPacket() const
{
	return 0;
}

int TrueConfAudioEncoder::GetTargetBitrate() const
{
	return m_codecInfo.default_bitrate_bps;
}

void TrueConfAudioEncoder::Reset()
{

}

webrtc::AudioEncoder::EncodedInfo TrueConfAudioEncoder::EncodeImpl(uint32_t rtp_timestamp, rtc::ArrayView<const int16_t> audio, rtc::Buffer* encoded)
{
	webrtc::AudioEncoder::EncodedInfo info;
	info.encoded_bytes = audio.data()[0];
	info.encoded_timestamp = rtp_timestamp;
	info.encoder_type = m_codecType;
	info.payload_type = m_codecPayload;
	info.speech = true;
	info.send_even_if_empty = false;
	encoded->AppendData((uint8_t*)(audio.data() + 1), info.encoded_bytes);
	return info;
}
#include "trueconf_audio_decoder.h"
#include "trueconf_callback_defines.h"

#include "rtc_base/logging.h"

using namespace tc_webrtc_api;

TrueConfAudioDecoder::TrueConfAudioDecoder(const std::string & payload_name, int sample_rate, int num_channels, TrueConfAudioDecoderCallback *callback)
{
	m_sampleRateHz = sample_rate;
	m_payloadName = payload_name;
	if (STR_CASE_CMP(payload_name.c_str(), "opus") != 0 && STR_CASE_CMP(payload_name.c_str(), "speex") != 0) {
		for (auto & c : m_payloadName) c = toupper(c);
	}
	if (STR_CASE_CMP(m_payloadName.c_str(), "G722") == 0) {
		m_sampleRateHz *= 2;
	}
	m_numChannels = num_channels;

	RTC_LOG_V(rtc::LoggingSeverity::LS_INFO) << "[TrueConf] TrueConfAudioDecoder : plname = "
		<< m_payloadName.c_str()
		<< ", sr = "
		<< m_sampleRateHz
		<< ", nch = "
		<< num_channels;

	callback->OnUpdateReceiveAudioPayload(m_payloadName, m_sampleRateHz);
	fireReceiveEncodedFrame.connect(callback, &TrueConfAudioDecoderCallback::OnReceiveAudioFrame);
}

TrueConfAudioDecoder::~TrueConfAudioDecoder()
{
	Reset();
}

void TrueConfAudioDecoder::Reset()
{

}

int TrueConfAudioDecoder::SampleRateHz() const
{
	return m_sampleRateHz;
}

size_t TrueConfAudioDecoder::Channels() const
{
	return m_numChannels;
}

bool TrueConfAudioDecoder::HasDecodePlc() const
{
	return false;
}

int TrueConfAudioDecoder::DecodeInternal(const uint8_t* encoded,
										size_t encoded_len,
										int sample_rate_hz,
										int16_t* decoded,
										SpeechType* speech_type)
{
	/// fire delivery frame
	return -1;
}

int TrueConfAudioDecoder::IncomingPacket(const uint8_t* payload,
										size_t payload_len,
										uint16_t rtp_sequence_number,
										uint32_t rtp_timestamp,
										uint32_t arrival_timestamp)
{
	fireReceiveEncodedFrame(payload, static_cast<uint32_t>(payload_len), rtp_timestamp);
	return 0;
}

#ifndef WEBRTC_API_TRUECONF_AUDIODECODER_H_
#define WEBRTC_API_TRUECONF_AUDIODECODER_H_

#include "api/audio_codecs/audio_decoder.h"
#include "rtc_base/sigslot.h"
#include "common_types.h"

namespace tc_webrtc_api
{
	class TrueConfAudioDecoderCallback;
}

namespace tc_webrtc_api
{

	class TrueConfAudioDecoder : public webrtc::AudioDecoder
	{

	public:

		TrueConfAudioDecoder(const std::string & payload_nam, int sample_rate, int num_channels, TrueConfAudioDecoderCallback *callback);
		~TrueConfAudioDecoder() final;
		void Reset() override;
		int SampleRateHz() const override;
		size_t Channels() const override;
		bool HasDecodePlc() const override;
		int IncomingPacket(const uint8_t* payload,
							size_t payload_len,
							uint16_t rtp_sequence_number,
							uint32_t rtp_timestamp,
							uint32_t arrival_timestamp) override;

	protected:

		int DecodeInternal(const uint8_t* encoded,
							size_t encoded_len,
							int sample_rate_hz,
							int16_t* decoded,
							SpeechType* speech_type) override;

	protected:

		sigslot::signal3<const uint8_t*, uint32_t, uint32_t, sigslot::multi_threaded_local> fireReceiveEncodedFrame;

	private:

		int m_sampleRateHz;
		std::string m_payloadName;
		int m_numChannels;

	};

}

#endif /* WEBRTC_API_TRUECONF_AUDIODECODER_H_ */

#ifndef WEBRTC_API_TRUECONF_AUDIOENCODERFACTORY_H_
#define WEBRTC_API_TRUECONF_AUDIOENCODERFACTORY_H_

#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/audio_codecs/audio_codec_pair_id.h"

#include <vector>

namespace tc_webrtc_api
{
	class TrueConfAudioEncoderCallback;
}

namespace tc_webrtc_api
{

	class TrueConfAudioEncoderFactory : public webrtc::AudioEncoderFactory
	{

	public:

		TrueConfAudioEncoderFactory(TrueConfAudioEncoderCallback *callback);
		~TrueConfAudioEncoderFactory() override;
		std::vector<webrtc::AudioCodecSpec> GetSupportedEncoders() override;
		rtc::Optional<webrtc::AudioCodecInfo> QueryAudioEncoder(const webrtc::SdpAudioFormat& format) override;
		std::unique_ptr<webrtc::AudioEncoder> MakeAudioEncoder(int payload_type, const webrtc::SdpAudioFormat& format, rtc::Optional<webrtc::AudioCodecPairId> codec_pair_id) override;

	private:

		TrueConfAudioEncoderCallback *m_callback;
		std::vector<webrtc::AudioCodecSpec> m_supportedCodecs;
		std::vector<webrtc::SdpAudioFormat> m_sdpFormats;

	};

}

#endif /* WEBRTC_API_TRUECONF_AUDIOENCODERFACTORY_H_ */
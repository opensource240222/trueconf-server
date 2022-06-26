
#ifndef WEBRTC_API_TRUECONF_AUDIODECODERFACTORY_H_
#define WEBRTC_API_TRUECONF_AUDIODECODERFACTORY_H_

#include "api/audio_codecs/audio_decoder_factory.h"

namespace tc_webrtc_api
{
	class TrueConfAudioDecoderCallback;
}

namespace tc_webrtc_api
{

	class TrueConfAudioDecoderFactory : public webrtc::AudioDecoderFactory
	{

	public:

		TrueConfAudioDecoderFactory(TrueConfAudioDecoderCallback *callback);
		~TrueConfAudioDecoderFactory() override;
		std::vector<webrtc::AudioCodecSpec> GetSupportedDecoders() override;
		bool IsSupportedDecoder(const webrtc::SdpAudioFormat& format) override;
		std::unique_ptr<webrtc::AudioDecoder> MakeAudioDecoder(const webrtc::SdpAudioFormat& format, rtc::Optional<webrtc::AudioCodecPairId> codec_pair_id) override;

	private:

		TrueConfAudioDecoderCallback *m_callback;
		std::vector<webrtc::AudioCodecSpec> m_supportedSpecs;

	};

}

#endif /* WEBRTC_API_TRUECONF_AUDIODECODERFACTORY_H_ */
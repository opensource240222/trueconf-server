
#ifndef WEBRTC_API_TRUECONF_VIDEOENCODERFACTORY_H_
#define WEBRTC_API_TRUECONF_VIDEOENCODERFACTORY_H_

#include "api/video_codecs/video_decoder_factory.h"
#include "common_types.h"

namespace tc_webrtc_api
{
	class TrueConfVideoDecoderCallback;
}

namespace tc_webrtc_api
{

	class TrueConfVideoDecoderFactory : public webrtc::VideoDecoderFactory
	{

	public:

		TrueConfVideoDecoderFactory(TrueConfVideoDecoderCallback *callback);
		~TrueConfVideoDecoderFactory() final;
		std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;
		std::unique_ptr<webrtc::VideoDecoder> CreateVideoDecoder(const webrtc::SdpVideoFormat& format) override;

	private:

		webrtc::VideoCodecType GetCodecType(const std::string & payloadName) const;

	private:

		std::vector<webrtc::SdpVideoFormat> m_sdpFormats;
		TrueConfVideoDecoderCallback *m_callback;

	};

}

#endif /* WEBRTC_API_TRUECONF_VIDEOENCODERFACTORY_H_ */
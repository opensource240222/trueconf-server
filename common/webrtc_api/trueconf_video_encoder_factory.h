
#ifndef WEBRTC_API_TRUECONF_VIDEODECODERFACTORY_H_
#define WEBRTC_API_TRUECONF_VIDEODECODERFACTORY_H_

#include "api/video_codecs/video_encoder_factory.h"
#include "common_types.h"

#include <vector>

namespace tc_webrtc_api
{
	class TrueConfVideoEncoderCallback;
}

namespace tc_webrtc_api
{

	class TrueConfVideoEncoderFactory : public webrtc::VideoEncoderFactory
	{

	public:

		TrueConfVideoEncoderFactory(TrueConfVideoEncoderCallback *callback);
		~TrueConfVideoEncoderFactory() final;
		std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;
		webrtc::VideoEncoderFactory::CodecInfo QueryVideoEncoder(const webrtc::SdpVideoFormat& format) const override;
		std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(const webrtc::SdpVideoFormat& format) override;

	private:

		bool IsFormatSupported(const webrtc::SdpVideoFormat& format) const;
		webrtc::VideoCodecType GetCodecType(const std::string & payloadName) const;

	private:

		std::vector<webrtc::SdpVideoFormat> m_sdpFormats;
		TrueConfVideoEncoderCallback *m_callback;

	};

}

#endif /* WEBRTC_API_TRUECONF_VIDEODECODERFACTORY_H_ */
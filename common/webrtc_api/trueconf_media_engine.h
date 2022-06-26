
#ifndef WEBRTC_API_TRUECONF_MEDIAENGINE_H_
#define WEBRTC_API_TRUECONF_MEDIAENGINE_H_

#include <string>
#include <mutex>
#include "trueconf_callback_defines.h"

namespace webrtc {
	class AudioDeviceModule;
	class AudioEncoderFactory;
	class AudioDecoderFactory;
	class VideoEncoderFactory;
	class VideoDecoderFactory;
}

namespace tc_webrtc_api
{
	class TrueConfVideoCapturer;
	class TrueConfVideoEncoderFactory;
	class TrueConfVideoDecoderFactory;
}

namespace tc_webrtc_api
{

	class TrueConfMediaEngine : public TrueConfVideoEncoderCallback,
								public TrueConfVideoDecoderCallback,
								public TrueConfVideoCapturerCallback,
								public TrueConfAudioEncoderCallback,
								public TrueConfAudioDecoderCallback,
								public TrueConfAudioDeviceModuleCallback
	{
	public:
		TrueConfMediaEngine(const char *peerName);
		~TrueConfMediaEngine() final;
		TrueConfVideoCapturer* CreateVideoCapturer(bool screenCapturer);
		std::unique_ptr<webrtc::VideoEncoderFactory> CreateVideoEncoderFactory();
		std::unique_ptr<webrtc::VideoDecoderFactory> CreateVideoDecoderFactory();
		rtc::scoped_refptr<webrtc::AudioDeviceModule> CreateAudioDeviceModule();
		rtc::scoped_refptr<webrtc::AudioEncoderFactory> CreateAudioEncoderFactory();
		rtc::scoped_refptr<webrtc::AudioDecoderFactory> CreateAudioDecoderFactory();
	public: /// trueconf relay controls
		/// video module
		void DeliverVideoFrame(const uint8_t *frame, int cmpsize, int width, int height, bool key, int64_t timestamp);
		void BitrateRequest(uint32_t bitrate);
		void KeyFrameRequest();
		/// audio module
		void DeliverAudioFrame(const uint8_t* audio, int len, int samples);
	public: /// peerconnection controls
		sigslot::signal0<sigslot::multi_threaded_local> firePeerConnectionDead;
		sigslot::signal0<sigslot::multi_threaded_local> fireSendRequestKeyFrame;
		sigslot::signal3<const char*, uint32_t, uint32_t, sigslot::multi_threaded_local> fireSetRates;
		sigslot::signal5<const char*, const char*, uint8_t, uint16_t, uint16_t, sigslot::multi_threaded_local> fireUpdateSendVideoPayload;
		sigslot::signal6<const char*, const char*, const char*, uint8_t, uint16_t, uint16_t, sigslot::multi_threaded_local> fireUpdateReceiveVideoPayload;
		sigslot::signal6<const char*, const char*, const uint8_t*, int32_t, bool, uint32_t, sigslot::multi_threaded_local> fireReceiveVideoFrame;
		sigslot::signal2<const char*, int, sigslot::multi_threaded_local> fireUpdateSendAudioPayload;
		sigslot::signal2<const char*, int, sigslot::multi_threaded_local> fireUpdateReceiveAudioPayload;
		sigslot::signal4<const char*, const uint8_t*, int32_t, uint32_t, sigslot::multi_threaded_local> fireReceiveAudioFrame;
	protected: /// TrueConfVideoEncoderCallback
		void OnRequestKeyFrame() override;
		void OnSetRates(uint32_t bitrate, uint32_t framerate) override;
		void OnUpdateSendVideoPayload(std::string plName, uint8_t plType, uint16_t width, uint16_t height) override;
	protected: /// TrueConfVideoDecoderCallback
		void OnReceiveVideoFrame(const std::string &streamid, const uint8_t *frame, int32_t size, bool key, uint32_t timestamp) override;
		void OnUpdateReceiveVideoPayload(const std::string &streamid, std::string plName, uint8_t plType, uint16_t width, uint16_t height) override;
	protected: /// TrueConfAudioEncoderCallback
		void OnUpdateSendAudioPayload(std::string plName, int plFrequency, int channels) override;
	protected: /// TrueConfAudioDecoderCallback
		void OnReceiveAudioFrame(const uint8_t *frame, uint32_t size, uint32_t timestamp) override;
		void OnUpdateReceiveAudioPayload(std::string plName, int plFrequency) override;
	protected: /// TrueConfVideoCapturerCallback
		void OnPeerConnectionDead() override;
	private:
		std::recursive_mutex m_lockSimulcastDecoder;
		std::string m_peerName;
		int m_plAudioSamplerate;
		int m_numChannels;
	};

}

#endif /* WEBRTC_API_TRUECONF_MEDIAENGINE_H_ */

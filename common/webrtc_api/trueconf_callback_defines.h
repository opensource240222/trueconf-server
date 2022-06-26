
#ifndef WEBRTC_API_TRUECONF_CALLBACKDEFINES_H_
#define WEBRTC_API_TRUECONF_CALLBACKDEFINES_H_

#include "rtc_base/scoped_ref_ptr.h"
#include "rtc_base/sigslot.h"

namespace webrtc {
	class VideoFrame;
}

namespace tc_webrtc_api
{

	class TrueConfVideoCapturerCallback : public sigslot::has_slots<>
	{
	public:
        TrueConfVideoCapturerCallback() {};
		virtual ~TrueConfVideoCapturerCallback() {};
		sigslot::signal1<const webrtc::VideoFrame&, sigslot::multi_threaded_local> fireDeliverVideoFrame;
		virtual void OnPeerConnectionDead() = 0;
	};

	class TrueConfVideoEncoderCallback : public sigslot::has_slots<>
	{
	public:
		TrueConfVideoEncoderCallback() {};
		virtual ~TrueConfVideoEncoderCallback() {};
		virtual void OnRequestKeyFrame() = 0;
		virtual void OnSetRates(uint32_t bitrate, uint32_t framerate) = 0;
		virtual void OnUpdateSendVideoPayload(std::string plName, uint8_t plType, uint16_t width, uint16_t height) = 0;
		sigslot::signal1<uint32_t, sigslot::multi_threaded_local> fireReceiveRequestBitrate;
	};

	class TrueConfVideoDecoderCallback : public sigslot::has_slots<>
	{
	public:
        TrueConfVideoDecoderCallback() {};
		virtual ~TrueConfVideoDecoderCallback() {};
		virtual void OnReceiveVideoFrame(const std::string &streamid, const uint8_t *frame, int32_t size, bool key, uint32_t timestamp) = 0;
		virtual void OnUpdateReceiveVideoPayload(const std::string &streamid, std::string plName, uint8_t plType, uint16_t width, uint16_t height) = 0;
		sigslot::signal0<sigslot::multi_threaded_local> fireReceiveRequestKeyFrame;
	};

	class TrueConfAudioDeviceModuleCallback
	{
	public:
        TrueConfAudioDeviceModuleCallback() {};
		virtual ~TrueConfAudioDeviceModuleCallback() {};
		sigslot::signal5<const uint8_t*, int, int, int, int, sigslot::multi_threaded_local> fireDeliverAudioFrame;
	};

	class TrueConfAudioEncoderCallback : public sigslot::has_slots<>
	{
	public:
        TrueConfAudioEncoderCallback() {};
		virtual ~TrueConfAudioEncoderCallback() {};
		virtual void OnUpdateSendAudioPayload(std::string plName, int plFrequency, int channels) = 0;
	};

	class TrueConfAudioDecoderCallback : public sigslot::has_slots<>
	{
	public:
        TrueConfAudioDecoderCallback() {};
		virtual ~TrueConfAudioDecoderCallback() {};
		virtual void OnReceiveAudioFrame(const uint8_t *frame, uint32_t size, uint32_t timestamp) = 0;
		virtual void OnUpdateReceiveAudioPayload(std::string plName, int plFrequency) = 0;
	};

}

#endif /* WEBRTC_API_TRUECONF_CALLBACKDEFINES_H_ */
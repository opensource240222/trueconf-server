
#ifndef WEBRTC_API_TRUECONF_VIDEOCAPTURER_H_
#define WEBRTC_API_TRUECONF_VIDEOCAPTURER_H_

#include "media/base/videocapturer.h"

namespace tc_webrtc_api
{
	class TrueConfVideoCapturerCallback;
}

namespace tc_webrtc_api
{

	class TrueConfVideoCapturer : public cricket::VideoCapturer
	{
	public:
		explicit TrueConfVideoCapturer() = delete;
		TrueConfVideoCapturer(TrueConfVideoCapturerCallback *callback, bool screenCapturer);
		~TrueConfVideoCapturer() override;
		cricket::CaptureState Start(const cricket::VideoFormat& capture_format) override;
		void Stop() override;
		bool IsRunning() override;
		bool IsScreencast() const override;
	public:
		void OnDeliverVideoFrame(const webrtc::VideoFrame &frame);
	protected:
		sigslot::signal0<sigslot::multi_threaded_local> firePeerDead;
	protected:
		bool GetPreferredFourccs(std::vector<uint32_t>* fourccs) override;
		void OnSinkWantsChanged(const rtc::VideoSinkWants& wants) override;
	private:
		bool m_runningCapturer;
		bool m_screenCapturer;
		TrueConfVideoCapturerCallback *m_callback;
	};

}

#endif /* WEBRTC_API_TRUECONF_VIDEOCAPTURER_H_ */
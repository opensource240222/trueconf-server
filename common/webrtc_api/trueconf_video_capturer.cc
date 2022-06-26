
#include "trueconf_video_capturer.h"
#include "trueconf_video_frame_buffer.h"
#include "trueconf_callback_defines.h"

using namespace tc_webrtc_api;

TrueConfVideoCapturer::TrueConfVideoCapturer(TrueConfVideoCapturerCallback *callback, bool screenCapturer)
	:	m_runningCapturer(false),
		m_screenCapturer(screenCapturer),
		m_callback(callback)
{
	std::vector<cricket::VideoFormat> formats;
	formats.push_back(cricket::VideoFormat(1920, 1080, cricket::VideoFormat::FpsToInterval(15), cricket::FOURCC_I420));
	formats.push_back(cricket::VideoFormat(1280,  720, cricket::VideoFormat::FpsToInterval(15), cricket::FOURCC_I420));
	formats.push_back(cricket::VideoFormat( 640,  360, cricket::VideoFormat::FpsToInterval(15), cricket::FOURCC_I420));
	formats.push_back(cricket::VideoFormat( 320,  180, cricket::VideoFormat::FpsToInterval(15), cricket::FOURCC_I420));
	SetSupportedFormats(formats);
	SetId("TrueConf Video Capturer");
	set_enable_video_adapter(false);
	firePeerDead.connect(m_callback, &TrueConfVideoCapturerCallback::OnPeerConnectionDead);
}

TrueConfVideoCapturer::~TrueConfVideoCapturer()
{
	firePeerDead();
}

bool TrueConfVideoCapturer::GetPreferredFourccs(std::vector<uint32_t>* fourccs)
{
	if (!fourccs) {
		return false;
	}
	fourccs->clear();
	fourccs->push_back(cricket::FOURCC_I420);
	return true;
}

cricket::CaptureState TrueConfVideoCapturer::Start(const cricket::VideoFormat& capture_format)
{
	SetCaptureFormat(&capture_format);
	SetCaptureState(cricket::CS_RUNNING);
	m_callback->fireDeliverVideoFrame.connect(this, &TrueConfVideoCapturer::OnDeliverVideoFrame);
	m_runningCapturer = true;
	return cricket::CS_STARTING;
}

void TrueConfVideoCapturer::Stop()
{
	m_callback->fireDeliverVideoFrame.disconnect(this);
	m_runningCapturer = false;
	SetCaptureFormat(nullptr);
	SetCaptureState(cricket::CS_STOPPED);
}

bool TrueConfVideoCapturer::IsRunning()
{
	return m_runningCapturer;
}

bool TrueConfVideoCapturer::IsScreencast() const
{
	return m_screenCapturer;
}

void TrueConfVideoCapturer::OnSinkWantsChanged(const rtc::VideoSinkWants& wants)
{

}

void TrueConfVideoCapturer::OnDeliverVideoFrame(const webrtc::VideoFrame &frame)
{
	OnFrame(frame, frame.width(), frame.height());
}

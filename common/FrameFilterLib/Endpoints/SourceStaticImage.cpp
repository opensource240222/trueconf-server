#include "FrameFilterLib/Endpoints/SourceStaticImage.h"
#include "std/cpplib/VS_MediaFormat.h"
#include "std/cpplib/VS_Singleton.h"
#include "std/cpplib/VS_ThreadPool.h"

namespace ffl {
	std::shared_ptr<SourceStaticImage> SourceStaticImage::Create()
	{
		return std::make_shared<SourceStaticImage>();
	}

	SourceStaticImage::SourceStaticImage()
		: m_active(false)
		, m_frame_period(std::chrono::milliseconds(100))
		, m_send_cnt(0)
	{
		SetName("image source");
		// Set 64x64 black square as default image
		const unsigned w = 64;
		const unsigned h = 64;
		m_prepared_frame = vs::SharedBuffer(w*h*3/2);
		std::memset(m_prepared_frame.data(), 0, w*h);
		std::memset(m_prepared_frame.data<char>()+w*h, 128, w*h/2);
		SetFormat(FilterFormat::MakeVideo(FOURCC_I420, w, h));
	}

	bool SourceStaticImage::SetImage(const void* imageI420, size_t size, unsigned w, unsigned h)
	{
		if (w == 0 || h == 0 || !imageI420 || size < w*h*3/2)
			return false;

		std::lock_guard<std::mutex> lock(m_mutex);
		m_prepared_frame = vs::SharedBuffer(w*h*3/2);
		std::memcpy(m_prepared_frame.data(), imageI420, m_prepared_frame.size());
		SetFormat(FilterFormat::MakeVideo(FOURCC_I420, w, h));
		return true;
	}

	void SourceStaticImage::SetFPS(unsigned int fps)
	{
		if (fps == 0)
			fps = 10;

		std::lock_guard<std::mutex> lock(m_mutex);
		m_frame_period = std::chrono::microseconds(1000000/fps);
		if (m_active)
			ScheduleNextSend();
	}

	void SourceStaticImage::Pause()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_active = false;
	}

	void SourceStaticImage::Resume()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_active = true;
		ScheduleNextSend();
	}

	void SourceStaticImage::NotifySourceUnused()
	{
		Pause();
	}

	void SourceStaticImage::SendFrame(unsigned int send_id)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		if (send_id != m_send_cnt)
			return;
		if (!m_active)
			return;
		auto now = std::chrono::steady_clock::now();
		if (now < m_last_frame_time + m_frame_period)
			return;
		m_last_frame_time = now;
		ScheduleNextSend();
		auto frame = m_prepared_frame;
		auto interval = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(m_frame_period).count());
		std::lock_guard<std::mutex> send_lock(m_send_mutex);
		lock.unlock();
		SendFrameToSubscribers(std::move(frame), FrameMetadata::MakeVideo(interval, true));
	}

	void SourceStaticImage::ScheduleNextSend()
	{
		VS_Singleton<VS_ThreadPool>::Instance().Post(std::bind(&SourceStaticImage::SendFrame, shared_from_this(), ++m_send_cnt), m_last_frame_time + m_frame_period);
	}
}

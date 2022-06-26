#pragma once

#include "FrameFilterLib/Base/AbstractSource.h"

#include <chrono>
#include <mutex>

namespace ffl {
	class SourceStaticImage : public AbstractSource
	{
	public:
		static std::shared_ptr<SourceStaticImage> Create();

		SourceStaticImage();

		bool SetImage(const void* imageI420, size_t size, unsigned w, unsigned h);
		void SetFPS(unsigned int fps);

		void Pause();
		void Resume();

	protected:
		void NotifySourceUnused() override;

	private:
		void SendFrame(unsigned int send_id);
		void ScheduleNextSend();

		std::shared_ptr<SourceStaticImage> shared_from_this()
		{
			return std::shared_ptr<SourceStaticImage>(AbstractSource::shared_from_this(), this);
		}

		std::shared_ptr<SourceStaticImage const> shared_from_this() const
		{
			return std::shared_ptr<SourceStaticImage const>(AbstractSource::shared_from_this(), this);
		}

	private:
		std::mutex m_mutex;
		bool m_active;
		vs::SharedBuffer m_prepared_frame;
		std::chrono::steady_clock::time_point m_last_frame_time;
		std::chrono::steady_clock::duration m_frame_period;
		unsigned m_send_cnt;
		std::mutex m_send_mutex; // guards just call to SendFrameToSubscribers
	};
}

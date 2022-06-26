#pragma once

#include "Buffer.h"

#include <boost/circular_buffer.hpp>

namespace stream {

class DefaultBuffer : public Buffer
{
public:
	explicit DefaultBuffer(unsigned max_frames = 200);

	bool Init(string_view conf_name, string_view part_name) override;
	void Destroy(string_view conf_name, string_view part_name) override;
	Status PutFrame(uint32_t tick_count, Track track, std::unique_ptr<char[]>&& buffer, size_t size) override;
	Status GetFrame(uint32_t& tick_count, Track& track, std::unique_ptr<char[]>& buffer, size_t& size) override;
	unsigned GetFrameCount() const override;
	unsigned GetFrameCount(Track track) const override;
	bool Ready() const override;
	uint32_t GetQueueBytes() const override;

private:
	struct frame_info
	{
		uint32_t tick_count;
		Track track;
		std::unique_ptr<char[]> buffer;
		size_t size;
	};
	boost::circular_buffer<frame_info> m_queue;
	unsigned m_frames_by_track[256];
	uint32_t m_max_frames = 0;
	uint32_t m_queue_bytes = 0;
	bool m_ready = false;
	bool m_skip_video = false;
};

}

#include "DefaultBuffer.h"
#include "../Protocol.h"
#include "../VS_StreamsDefinitions.h"

#include <cstring>

namespace stream {

DefaultBuffer::DefaultBuffer(unsigned max_frames)
	: m_queue(std::min(std::max(max_frames, 1u), 655u)) //TODO: C++17: std::clamp
{
	m_max_frames = max_frames;
	std::memset(m_frames_by_track, 0, sizeof(m_frames_by_track));
}

bool DefaultBuffer::Init(string_view conf_name, string_view part_name)
{
	m_conf_name = std::string(conf_name);
	m_part_name = std::string(part_name);
	return true;
}

void DefaultBuffer::Destroy(string_view /*conf_name*/, string_view /*part_name*/)
{
	std::memset(m_frames_by_track, 0, sizeof(m_frames_by_track));
	m_queue.clear();
}

auto DefaultBuffer::PutFrame(uint32_t tick_count, Track track, std::unique_ptr<char[]>&& buffer, size_t size) -> Status
{
	if (size > VS_STREAM_MAX_SIZE_FRAME) {
		return Status::fatal;
	}
	if (!m_skip_video && m_queue.full()) {
		m_queue.erase(
			std::remove_if(m_queue.begin(), m_queue.end(), [this] (const frame_info & frame)
			{
				if (frame.track != Track::video) {
					return false;
				}
				m_queue_bytes -= frame.size;
				--m_frames_by_track[id(frame.track)];
				return true;
			}),
			m_queue.end()
		);
		m_skip_video = true;
	}
	if (m_skip_video) {
		m_skip_video = (m_queue.size() * 100 / m_max_frames > 75);
	}
	if (m_skip_video && track == Track::video) {
		return Status::non_fatal;
	}
	if (m_queue.full()) {
		auto &front = m_queue.front();
		--m_frames_by_track[id(front.track)];
		m_queue_bytes -= front.size;
	}
	m_queue.push_back(frame_info{ tick_count, track, std::move(buffer), size });
	++m_frames_by_track[id(track)];
	m_queue_bytes += size;
	return Status::success;
}

auto DefaultBuffer::GetFrame(uint32_t& tick_count, Track& track, std::unique_ptr<char[]>& buffer, size_t& size) -> Status
{
	if (m_queue.empty())
	{
		tick_count = 0;
		track = {};
		buffer.reset();
		size = 0;
		return Status::non_fatal;
	}

	auto& front = m_queue.front();
	tick_count = front.tick_count;
	track = front.track;
	buffer = std::move(front.buffer);
	size = front.size;
	--m_frames_by_track[id(front.track)];
	m_queue_bytes -= size;
	m_queue.pop_front();
	m_ready = true;
	return Status::success;
}

unsigned DefaultBuffer::GetFrameCount() const
{
	return m_queue.size();
}

unsigned DefaultBuffer::GetFrameCount(Track track) const
{
	return m_frames_by_track[id(track)];
}

bool DefaultBuffer::Ready() const
{
	return m_ready;
}

uint32_t DefaultBuffer::GetQueueBytes() const
{
	return m_queue_bytes;
}

}

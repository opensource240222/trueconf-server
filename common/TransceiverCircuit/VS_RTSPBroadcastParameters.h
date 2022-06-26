#pragma once

class VS_RTSPBroadcastParameters
{
public:
	explicit VS_RTSPBroadcastParameters(unsigned frame_duration_us);
	void Update(unsigned frame_duration_us);

	unsigned frame_timeout_us;
	unsigned frame_timeout_max_video_us;
	unsigned frame_timeout_max_audio_us;
	unsigned fb_max_size_bytes;
	unsigned fb_max_size_us;
	unsigned replicator_safety_timeout_us;
	unsigned send_retry_timeout_us;
	unsigned send_retry_interval_us;
	unsigned key_frame_delay_us;
};
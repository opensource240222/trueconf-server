#pragma once

#include <string>

class VS_RTPModuleParameters
{
public:
	VS_RTPModuleParameters();
	void Update();
	void UpdateSlideshow();

	std::string log_dir;
	std::string language;
	std::string slide_upload_url;
	std::string background_image_path;
	unsigned slide_show_duration_s;
	unsigned gconf_width;
	unsigned gconf_height;
	unsigned in_min_keyframe_interval_s;
	unsigned in_max_keyframe_interval_s;
	unsigned slide_interval_us;
	unsigned slide_dimension;
	unsigned slide_quality;
	unsigned ffl_trace_flags;
	bool force_transcoding;
	bool in_video_transcoding;
	bool use_uniform_transmit;
	bool content_as_slides;
};

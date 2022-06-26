#include "VS_RTSPBroadcastParameters.h"

#include "../std/cpplib/VS_RegistryKey.h"
#include "../std/cpplib/VS_RegistryConst.h"
#include "../std/debuglog/VS_Debug.h"

#include <algorithm>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

VS_RTSPBroadcastParameters::VS_RTSPBroadcastParameters(unsigned frame_duration_us)
	: frame_timeout_us(100000)
	, frame_timeout_max_video_us(200000)
	, frame_timeout_max_audio_us(120000)
	, fb_max_size_bytes(4*1024*1024)
	, fb_max_size_us(1000000)
	, replicator_safety_timeout_us(10000)
	, send_retry_timeout_us(10000)
	, send_retry_interval_us(5000)
	, key_frame_delay_us(500000)
{
	Update(frame_duration_us);
}

void VS_RTSPBroadcastParameters::Update(unsigned frame_duration_us)
{
	VS_RegistryKey conf_key(false, CONFIGURATION_KEY);
	conf_key.GetValue(&frame_timeout_us, sizeof(frame_timeout_us), VS_REG_INTEGER_VT, "RTSP Frame Timeout");
	conf_key.GetValue(&frame_timeout_max_video_us, sizeof(frame_timeout_max_video_us), VS_REG_INTEGER_VT, "RTSP Frame Timeout Max Video");
	conf_key.GetValue(&frame_timeout_max_audio_us, sizeof(frame_timeout_max_audio_us), VS_REG_INTEGER_VT, "RTSP Frame Timeout Max Audio");

	frame_timeout_us = std::max({frame_timeout_us, 10000u, frame_duration_us});
	frame_timeout_max_audio_us = std::max({frame_timeout_max_audio_us, frame_timeout_us});
	frame_timeout_max_video_us = std::max({frame_timeout_max_video_us, frame_timeout_us});

	fb_max_size_us = std::max({fb_max_size_us, std::max({frame_timeout_max_video_us, frame_timeout_max_audio_us}) + frame_duration_us});
	replicator_safety_timeout_us = std::max({replicator_safety_timeout_us, frame_timeout_max_video_us, frame_timeout_max_audio_us});
	send_retry_timeout_us = std::max({send_retry_timeout_us, frame_duration_us});

	dstream4 << "RTPSParameters: Update:\n"
		<< "\tframe_timeout_us=" << frame_timeout_us << '\n'
		<< "\tframe_timeout_max_video_us=" << frame_timeout_us << '\n'
		<< "\tframe_timeout_max_audio_us=" << frame_timeout_us << '\n'
		<< "\tfb_max_size_bytes=" << fb_max_size_bytes << '\n'
		<< "\tfb_max_size_us=" << fb_max_size_us << '\n'
		<< "\treplicator_safety_timeout_us=" << replicator_safety_timeout_us << '\n'
		<< "\tsend_retry_timeout_us=" << send_retry_timeout_us << '\n'
		<< "\tsend_retry_interval_us=" << send_retry_interval_us << '\n'
		<< "\tkey_frame_delay_us=" << key_frame_delay_us << '\n'
		;
}
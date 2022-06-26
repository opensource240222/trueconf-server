#pragma once

#include <chrono>

struct TConferenceStatistics {
	int size_of_stat = 0;
	int participant_time = 0;
	int broadcast_time = 0;
	int video_w = 0;
	int video_h = 0;
	int loss_rcv_packets = 0;
	int avg_send_bitrate = 0;
	int avg_rcv_bitrate = 0;
	int avg_cpu_load = 0;
	int avg_jitter = 0;
	double avg_send_fps = 0.0;
	std::chrono::system_clock::time_point start_part_gmt {};
	std::chrono::system_clock::time_point end_part_gmt {};
};
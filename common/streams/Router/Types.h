#pragma once

#include "../fwd.h"
#include "std-generic/cpplib/macro_utils.h"

#include <string>

namespace stream
{

	struct ParticipantFrameSizeInfo
	{
		VS_FORWARDING_CTOR2(ParticipantFrameSizeInfo, participant_name_from, mb) {}
		std::string participant_name_from;
		int32_t mb;
	};

	struct ParticipantLoadInfo
	{
		VS_FORWARDING_CTOR3(ParticipantLoadInfo, conference_name, participant_name, load) {}
		std::string conference_name;
		std::string participant_name;
		int32_t load;
	};

	enum eBandwidthState : uint32_t
	{
		undef = 0x01,
		idle = 0x02,
		limit = 0x04,
		detect = 0x08,
		freeze = 0x10,
	};

	inline const char* bandwidth_state_to_string(eBandwidthState x)
	{
		switch (x) {
			case eBandwidthState::undef:  return "undef";
			case eBandwidthState::idle:   return "idle";
			case eBandwidthState::limit:  return "limit";
			case eBandwidthState::detect: return "detect";
			case eBandwidthState::freeze: return "freeze";
		}
		return "unknown";
	}

	struct ParticipantBandwidthInfo
	{
		uint32_t physicalBandwidth = (uint32_t) (-1);
		uint32_t upperPhysicalBandwidth = (uint32_t) (-1);
		uint32_t lowerPhysicalBandwidth = (uint32_t) (-1);
		uint32_t calculateBandwidth = (uint32_t) (-1);
		int32_t restrictBitrate = 0;
		int32_t loadBandwidth = 0;
		double upperStep = 0.0;
		uint64_t overflowTime = 0;
		uint64_t changeBandwidthTime = 0;
		uint32_t numPeriodicTicks = 0;
		uint32_t numDetectTicks = 0;
		uint32_t queueBytes = 0;
		uint32_t queueLenght = 0;
		uint32_t receivedBytes = 0;
		uint32_t state = eBandwidthState::undef;
		std::string loggedParticipant;
	};

}

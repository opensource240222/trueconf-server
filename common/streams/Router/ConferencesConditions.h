#pragma once

#include "../fwd.h"

#include <cstddef>

class VS_MediaFormat;

namespace stream {

class ConferencesConditions
{
public:
	virtual void CreateConference(const char* conf_name) = 0;
	virtual void RemoveConference(const char* conf_name, const ConferenceStatistics& cs) = 0;
	virtual void AddParticipant(const char* conf_name, const char* part_name) = 0;
	virtual void RemoveParticipant(const char* conf_name, const char* part_name, const ParticipantStatistics& ps) = 0;
	virtual int RestrictBitrate(const char* conf_name, const char* part_name, int bitrate) = 0;
	virtual int RestrictBitrateSVC(const char* conf_name, const char* part_name, int v_bitrate, int bitrate, int old_bitrate) = 0;
	virtual void RequestKeyFrame(const char* conf_name, const char* part_name) = 0;
	virtual void SetParticipantCaps(const char* conf_name, const char* part_name, const void* caps_buf, size_t buf_sz) = 0;
};

}

#pragma once
#include "std/cpplib/VS_Protocol.h"

class VS_ConfControlInterface
{
public:
	virtual void StartConference(const char *conf_name) = 0;
	virtual void StartConference(const char *conf_name, const char *owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type) = 0;
	virtual void StopConference(const char *conf_name) = 0;
	virtual void ParticipantConnect(const char *conf_name, const char *part_name) = 0;
	virtual void ParticipantDisconnect(const char *conf_name, const char *part_name) = 0;
	virtual void SetParticipantCaps(const char *conf_name, const char *part_name, const void *caps_buf, const unsigned long buf_sz) = 0;
	virtual void RestrictBitrateSVC(const char *conferenceName, const char *participantName, long v_bitrate, long bitrate, long old_bitrate) = 0;
	virtual void RequestKeyFrame(const char *conferenceName, const char *participantName) = 0;
};
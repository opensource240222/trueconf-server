#pragma once
#ifndef _VS_DBSTORAGE_CALLLOGINTERFACE_H
#define _VS_DBSTORAGE_CALLLOGINTERFACE_H

#include "../cpplib/VS_Protocol.h"
#include "../cpplib/VS_SimpleStr.h"
#include "VS_ParticipantDescription.h"
#include <vector>

class VS_ConferenceDescription;

struct VS_ParticipantDeviceParams {
	const char* conference_id;
	const char* user_instance;
	const char* action;
	const char* device_id;
	const char* device_type;
	const char* device_name;
	bool		is_active;
	bool*		pmute;
	int32_t*	pvolume;
	VS_ParticipantDeviceParams() : conference_id(0), user_instance(0), action(0), device_id(0), device_type(0), device_name(0), is_active(false), pmute(0), pvolume(0) {}

};

class VS_DBStorage_CallLogInterface
{
public:
	//logging
	virtual bool LogConferenceStart(const VS_ConferenceDescription& conf, bool remote = false) = 0;
	virtual bool LogConferenceEnd(const VS_ConferenceDescription& conf) = 0;
	virtual bool LogParticipantInvite(const vs_conf_id& conf_id, const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,
		const std::chrono::system_clock::time_point time = std::chrono::system_clock::time_point(), VS_ParticipantDescription::Type type = VS_ParticipantDescription::PRIVATE_HOST) = 0;
	virtual bool LogParticipantJoin(const VS_ParticipantDescription& pd, const VS_SimpleStr& callid2 = NULL) = 0;
	virtual bool LogParticipantReject(const vs_conf_id& conf_id, const vs_user_id& user, const vs_user_id& invited_from, const VS_Reject_Cause cause) = 0;
	virtual bool LogParticipantLeave(const VS_ParticipantDescription& pd) = 0;
	virtual bool LogParticipantStatistics(const VS_ParticipantDescription& pd) = 0;
	virtual bool LogConferenceRecordStart(const vs_conf_id& /*conf_id*/, const char* /*filename*/, std::chrono::system_clock::time_point /*started_at*/) { return false; }
	virtual bool LogConferenceRecordStop(const vs_conf_id& /*conf_id*/, std::chrono::system_clock::time_point /*stopped_at*/, uint64_t /*file_size*/) { return false; }
	virtual bool LogSystemParams(std::vector<int>& /*params*/) { return false; }
	virtual bool LogParticipantDevices(VS_ParticipantDeviceParams& /*params*/) { return false; }
	virtual bool LogParticipantRole(VS_ParticipantDescription& /*params*/) { return false; }
	virtual long SetRegID(const char* /*call_id*/, const char* /*reg_id*/, VS_RegID_Register_Type /*type*/) { return -1; }

	virtual ~VS_DBStorage_CallLogInterface(){}
};

#endif
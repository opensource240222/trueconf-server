#pragma once
#ifndef VS_DBOBJECS_CALL_LOG_H
#define VS_DBOBJECS_CALL_LOG_H

#include <string>

#include "VS_DBStorageInterface.h"
#include "VS_DBNull.h"
#include "tlb_import/msado26.tlh"

class VS_DBObjects_CallLog : public VS_DBStorage_CallLogInterface, public VS_DBNULL_OBJ
{
	const int reconnect_max = 2;
	const int reconnect_timeout = 10000;
	ADODB::_ConnectionPtr db1;
	ADODB::_CommandPtr log_conf_start, log_conf_end, log_part_join, log_part_leave, log_part_invite, log_part_stats;
	bool m_inited;

public:
	VS_DBObjects_CallLog();
	~VS_DBObjects_CallLog();

	bool Init(const VS_SimpleStr& conn_str1, const VS_SimpleStr& user1, const VS_SimpleStr& password1);

	// implementation of VS_DBStorage_CallLogInterface
	bool LogConferenceStart(const VS_ConferenceDescription& conf, bool remote = false) override;
	bool LogConferenceEnd(const VS_ConferenceDescription& conf) override;
	bool LogParticipantInvite(const vs_conf_id& conf_id, const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,
		const std::chrono::system_clock::time_point time = std::chrono::system_clock::time_point(), VS_ParticipantDescription::Type type = VS_ParticipantDescription::PRIVATE_HOST) override;
	bool LogParticipantJoin(const VS_ParticipantDescription& pd, const VS_SimpleStr& call_id2 = NULL) override;
	bool LogParticipantReject(const vs_conf_id& conf_id, const vs_user_id& user, const vs_user_id& invited_from, const VS_Reject_Cause cause) override;
	bool LogParticipantLeave(const VS_ParticipantDescription& pd) override;
	bool LogParticipantStatistics(const VS_ParticipantDescription& pd) override;

	void SplitConfID(const vs_conf_id& conf_id, int& conference, VS_SimpleStr& broker_id);
};

#endif /* VS_DBOBJECS_CALL_LOG_H */
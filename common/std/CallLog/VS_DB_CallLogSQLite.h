#pragma once

#include <ctime>
#include <cstring>
#include <string>
#include <memory>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <cstdlib>

#include "VS_DBStorage_CallLogInterface.h"

class VS_ConferenceDescription;
class VS_ParticipantDescription;
class VS_Container;
class CppSQLite3DB;
class CppSQLite3Query;
struct TConferenceStatistics;

class VS_DB_CallLogSQLite : public VS_DBStorage_CallLogInterface{
public:
	VS_DB_CallLogSQLite() {}
	virtual ~VS_DB_CallLogSQLite() {}

	virtual bool LogConferenceStart(const VS_ConferenceDescription& conf, bool remote = false /*Unused*/) override;
	virtual bool LogConferenceEnd(const VS_ConferenceDescription& conf) override;
	virtual bool LogParticipantInvite(const vs_conf_id& conf_id, const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,
		const std::chrono::system_clock::time_point time, VS_ParticipantDescription::Type type = VS_ParticipantDescription::PRIVATE_HOST) override;
	virtual bool LogParticipantJoin(const VS_ParticipantDescription& pd, const VS_SimpleStr& callid2 = NULL) override;
	virtual bool LogParticipantReject(const vs_conf_id& conf_id, const vs_user_id& user, const vs_user_id& invited_from, const VS_Reject_Cause cause) override;
	virtual bool LogParticipantLeave(const VS_ParticipantDescription& pd) override;
	virtual bool LogParticipantStatistics(const VS_ParticipantDescription& pd) override;

	bool LogParticipantStatistics(VS_Container& cnt, const TConferenceStatistics* stat, const std::string& displayName);

   /*
	* prevMonthCount - date where you will search, 0 - current date, N - (current date - N monthes)
	*/
	virtual int GetCalls(VS_Container& cnt, int& entries, VS_AddressBook ab, const vs_user_id& owner, int_fast64_t last_deleted_call, int prevMonthCount = 0) const;

protected:
	static const char m_SQLITE_TABLE_CALLS[];
	static const char m_SQLITE_CONF_TABLE_CALLS[];
	static const char m_CALL_ID2_PARAM[];
	static const char m_DB_CONFERENCE_PARAM[];
	static const char m_DB_SERVER_PARAM[];
	static const char m_DISPLAY_NAME_PARAM[];
	static const char m_CALL_ID_PARAM[];
	static const char m_APP_ID_PARAM[];
	static const char m_DB_TIME_PARAM[];
	static const char m_PART_LEAVE_DB_TIME_PARAM[];
	static const char m_PART_JOIN_DB_TIME_PARAM[];
	static const char m_DB_TYPE_PARAM[];
	static const char m_PART_LEAVE_REASON_PARAM[];
	static const char m_PART_BYTES_SENT_PARAM[];
	static const char m_PART_BYTES_RECEIVED_PARAM[];
	static const char m_PART_RECON_SND_PARAM[];
	static const char m_PART_RECON_RCV_PARAM[];
	static const char m_PART_ID_PARAM[];
	static const char m_PART_TIME_PARAM[];
	static const char m_BROADCAST_TIME_PARAM[];
	static const char m_VIDEO_WIDTH_PARAM[];
	static const char m_VIDEO_HIGHT_PARAM[];
	static const char m_LOSS_RECEIVE_PARAM[];
	static const char m_AVG_CPU_LOAD_PARAM[];
	static const char m_AVG_JITTER_PARAM[];
	static const char m_AVG_SEND_FPS_PARAM[];

	static const char m_DB_OWNER_PARAM[];
	static const char m_DB_SUBTYPE_PARAM[];
	static const char m_DB_CONF_START_TIME_PARAM[];
	static const char m_DB_CONF_END_TIME_PARAM[];
	static const char m_DB_MAX_USERS_PARAM[];
	static const char m_DB_PUBLIC_PARAM[];
	static const char m_DB_TOPIC_PARAM[];
	static const char m_CONF_TERM_REASON_PARAM[];
	static const char m_CONF_RECORD_FILE_PARAM[];
	static const char m_CONF_GUEST_FLAGS_PARAM[];

private:
	bool Calls_InitSQLiteDB(CppSQLite3DB& db, int prevMonthCount = 0, bool do_create = true) const;
	bool HaveMonthLog(int count) const;
	boost::optional<long> fetch_long(CppSQLite3Query & q, const char * name);
};
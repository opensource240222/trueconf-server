#pragma once

#include "VS_DBStorage_CallLogInterface.h"
#include "std-generic/cpplib/Box.h"

struct TConferenceStatistics;
namespace cppdb { class session; }

namespace callLog {

	class Postgres : public VS_DBStorage_CallLogInterface {
	public:
		Postgres();
		~Postgres();
		virtual bool LogConferenceStart(const VS_ConferenceDescription& conf, bool remote = false /*Unused*/) override;
		virtual bool LogConferenceEnd(const VS_ConferenceDescription& conf) override;
		virtual bool LogParticipantInvite(const vs_conf_id& conf_id, const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,
			const std::chrono::system_clock::time_point time, VS_ParticipantDescription::Type type = VS_ParticipantDescription::PRIVATE_HOST) override;
		virtual bool LogParticipantJoin(const VS_ParticipantDescription& pd, const VS_SimpleStr& callid2 = NULL) override;
		virtual bool LogParticipantReject(const vs_conf_id& conf_id, const vs_user_id& user, const vs_user_id& invited_from, const VS_Reject_Cause cause) override;
		virtual bool LogParticipantLeave(const VS_ParticipantDescription& pd) override;
		virtual bool LogParticipantStatistics(const VS_ParticipantDescription& pd) override;
		virtual bool LogParticipantStatistics(VS_Container& cnt, const TConferenceStatistics* stat, const std::string& displayName);
		virtual bool LogConferenceRecordStart(const vs_conf_id& conf_id, const char* filename, std::chrono::system_clock::time_point started_at) override;
		virtual bool LogConferenceRecordStop(const vs_conf_id& conf_id, std::chrono::system_clock::time_point stopped_at, uint64_t file_size) override;
		virtual bool LogSystemParams(std::vector<int> &params) override;
		virtual bool LogParticipantDevices(VS_ParticipantDeviceParams& params) override;
		virtual bool LogParticipantRole(VS_ParticipantDescription& params) override;
		virtual long SetRegID(const char* call_id, const char* reg_id, VS_RegID_Register_Type type) override;
		virtual int GetCalls(VS_Container& cnt, int& entries, VS_AddressBook ab, const vs_user_id& owner, time_t last_deleted_call);

		bool Init(const std::string& db_conn_str);
		vs_conf_id CreateNewConfID();

	private:
		cppdb::session OpenSession();
	private:
		struct cppdb_pool_ptr_tag;
		vs::Box<cppdb_pool_ptr_tag, sizeof(void*)> m_pool;
	};

}

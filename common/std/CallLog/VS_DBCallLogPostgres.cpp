#if defined(HAVE_CPPDB)

#include "VS_DBCallLogPostgres.h"
#include "std-generic/cpplib/VS_CppDBLibs.h"
#include "../../std/debuglog/VS_Debug.h"
#include "VS_ConferenceDescription.h"
#include "../../std/statistics/TConferenceStatistics.h"
#include "std-generic/clib/vs_time.h"
#include "std-generic/cpplib/VS_CppDBIncludes.h"
#include "std-generic/cpplib/VS_RemoveTranscoderID.h"

#include <ctime>

#define DEBUG_CURRENT_MODULE VS_DM_LOGSERVICE

#if defined(_WIN32)
#	define timezone _timezone
#endif

namespace
{
	bool to_gmtime(std::chrono::system_clock::time_point tp, std::tm& tm)
	{
		auto as_time_t = std::chrono::system_clock::to_time_t(tp);
		return gmtime_r(&as_time_t, &tm) != nullptr;
	}
}

namespace callLog {

	bool FetchResult(cppdb::result &res, const string_view function_name) {
		int32_t ret(-1);
		if (!res.next() || !res.fetch(0, ret)) {
			dstream3 << "Error\t Postgres::" << function_name << " can't fetch result!\n";
			return false;
		}

		if (ret != 0) {
			dstream3 << "Postgres::" << function_name << " returned '" << ret << "'!\n";
			return false;
		}

		return ret == 0;
	}

	struct Postgres::cppdb_pool_ptr_tag : vs::BoxTag<cppdb::pool::pointer> {};

	Postgres::Postgres() = default;
	Postgres::~Postgres() = default;

	bool Postgres::LogConferenceStart(const VS_ConferenceDescription & conf, bool remote) try
	{
		dstream3 << "$LogConferenceStart\n";

		if (!conf.m_name || !conf.m_owner) {
			dstream3 << "Error\t Postgres::LogConferenceStart not enough parameters for logging!\n";
			return false;
		}

		auto session = OpenSession();
		if (!session.is_open()) return false;

		bool is_public = conf.m_public;
		int guest_flags = 0;
		VS_ConferenceDescription::GetMultyConfInfo(conf.m_call_id, is_public, guest_flags);

		cppdb::null_tag_type appid_tag = !conf.m_appID ? cppdb::null_value : cppdb::not_null_value;
		cppdb::null_tag_type named_conf_id = cppdb::null_value;
		if(conf.m_call_id) named_conf_id = (conf.m_call_id == conf.m_name) ? cppdb::null_value : cppdb::not_null_value;
		cppdb::null_tag_type topic = conf.m_topic.empty() ? cppdb::null_value : cppdb::not_null_value;
		std::tm tm{};
		if (!to_gmtime(conf.m_logStarted, tm))
			return false;

		cppdb::result res = session << "select * from stat.log_conference_start(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
			<< conf.m_name.m_str << conf.m_owner.m_str << conf.m_type << conf.m_SubType
			<< cppdb::use(conf.m_appID.m_str, appid_tag) << is_public << cppdb::use(conf.m_call_id.m_str, named_conf_id)
			<< conf.m_MaxParticipants << cppdb::use(conf.m_topic, topic) << guest_flags << tm;

		return FetchResult(res, "LogConferenceStart");
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: LogConferenceStart failed: " << e.what();
		return false;
	}

	bool Postgres::LogConferenceEnd(const VS_ConferenceDescription & conf) try
	{
		dstream3 << "$LogConferenceEnd '" << (conf.m_name? conf.m_name.m_str : "empty") << "'\n";

		if (!conf.m_name || !conf.m_owner) {
			dstream3 << "Error\t Postgres::LogConferenceEnd not enough parameters for logging!\n";
			return false;
		}

		auto session = OpenSession();
		if (!session.is_open()) return false;

		cppdb::result res = session << "select * from stat.log_conference_end(?, ?)" << conf.m_name.m_str << conf.m_logCause;
		return FetchResult(res, "LogConferenceEnd");
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: LogConferenceEnd failed: " << e.what();
		return false;
	}

	bool Postgres::LogParticipantInvite(const vs_conf_id & conf_id, const VS_SimpleStr & call_id1,
		const VS_SimpleStr & app_id, const VS_SimpleStr & call_id2, const std::chrono::system_clock::time_point time,
		VS_ParticipantDescription::Type type) try
	{
		if (!conf_id || !call_id1 || !call_id2) {
			dstream3 << "Error\tPostgres::LogParticipantInvite not enough parameters for logging!\n";
			return false;
		}

		dstream3 << "$LogParticipantInvite '" << conf_id.m_str << "'\n";

		auto session = OpenSession();
		if (!session.is_open()) return false;

		cppdb::result res = session << "select * from stat.log_participant_invite(?, ?, ?)" << conf_id.m_str << call_id1.m_str << call_id2.m_str;
		return FetchResult(res, "LogParticipantInvite");
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: LogParticipantInvite failed: " << e.what();
		return false;
	}

	bool Postgres::LogParticipantJoin(const VS_ParticipantDescription & pd, const VS_SimpleStr & callid2) try
	{
		dstream3 << "$LogParticipantJoin\n";

		if (!pd.m_conf_id || !pd.m_user_id) {
			dstream3 << "Error\tPostgres::LogParticipantJoin not enough parameters for logging!\n";
			return false;
		}

		auto session = OpenSession();
		if (!session.is_open()) return false;

		auto call_id = VS_RemoveTranscoderID_sv(SimpleStrToStringView(pd.m_user_id));

		cppdb::result res = session << "select * from stat.log_participant_join(?, ?, ?, ?, ?, ?)"
			<< pd.m_conf_id.m_str << std::string(call_id) << pd.m_type  << pd.m_displayName
			<< cppdb::use(pd.m_appID.m_str, !pd.m_appID ? cppdb::null_value : cppdb::not_null_value)
			<< pd.m_user_id.m_str;
		return FetchResult(res, "LogParticipantJoin");
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: LogParticipantJoin failed: " << e.what();
		return false;
	}

	bool Postgres::LogParticipantReject(const vs_conf_id & conf_id, const vs_user_id & user, const vs_user_id& invited_from, const VS_Reject_Cause cause) try
	{
		if (!conf_id || !user) {
			dstream3 << "Error\tPostgres::LogParticipantReject not enough parameters for logging!\n";
			return false;
		}

		dstream3 << "$LogParticipantReject\n";

		auto session = OpenSession();
		if (!session.is_open()) return false;

		cppdb::result res = session << "select * from stat.log_participant_reject(?, ?, ?)" << conf_id.m_str << invited_from.m_str << user.m_str;
		return FetchResult(res, "LogParticipantReject");
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: LogParticipantReject failed: " << e.what();
		return false;
	}

	bool Postgres::LogParticipantLeave(const VS_ParticipantDescription & pd) try
	{
		if (!pd.m_conf_id || !pd.m_user_id) {
			dstream3 << "Error\tPostgres::LogParticipantLeave not enough parameters for logging!\n";
			return false;
		}
		dstream3 << "$LogParticipantLeave\n";

		auto session = OpenSession();
		if (!session.is_open()) return false;

		auto call_id = VS_RemoveTranscoderID_sv(SimpleStrToStringView(pd.m_user_id));

		cppdb::result res = session << "select * from stat.log_participant_leave(?, ?, ?, ?, ?, ?, ?, ?)"
			<< pd.m_conf_id.m_str << std::string(call_id) << pd.m_cause
			<< pd.m_bytesSnd << pd.m_bytesRcv << pd.m_reconSnd << pd.m_reconRcv
			<< pd.m_user_id.m_str;
		return FetchResult(res, "LogParticipantLeave");
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: LogParticipantLeave failed: " << e.what();
		return false;
	}

	bool Postgres::LogParticipantStatistics(const VS_ParticipantDescription & pd) try
	{
		if (!pd.m_conf_id || !pd.m_user_id) {
			dstream3 << "Error\tPostgres::LogParticipantStatistics not enough parameters for logging!\n";
			return false;
		}

		dstream3 << "$LogParticipantStatistics participant\n";

		auto session = OpenSession();
		if (!session.is_open()) return false;

		cppdb::result res = session << "select * from stat.log_participant_statistics(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
			<< pd.m_conf_id.m_str << pd.m_user_id.m_str << pd.m_bytesSnd << pd.m_bytesRcv << pd.m_reconSnd << pd.m_reconSnd
			<< cppdb::null << cppdb::null << cppdb::null << cppdb::null << cppdb::null << cppdb::null << cppdb::null << cppdb::null;	// no conf statistic here
		return FetchResult(res, "LogParticipantStatistics participant");
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: LogParticipantStatistics participant failed: " << e.what();
		return false;
	}

	bool Postgres::LogParticipantStatistics(VS_Container& cnt, const TConferenceStatistics* stat, const std::string& displayName) try
	{
		const char* conf_id = cnt.GetStrValueRef(CONFERENCE_PARAM);
		const char* call_id = cnt.GetStrValueRef(CALLID_PARAM);

		if (!stat || !conf_id || !call_id) {
			dstream3 << "Error\tPostgres::LogParticipantStatistics not enough parameters for logging!\n";
			return false;
		}

		dstream3 << "$LogParticipantStatistics conference\n";

		auto session = OpenSession();
		if (!session.is_open()) return false;

		cppdb::result res = session << "select * from stat.log_participant_statistics(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
			<< conf_id << call_id
			<< cppdb::null << cppdb::null << cppdb::null << cppdb::null	// no conf participant here
			<< stat->participant_time << stat->broadcast_time << stat->video_w << stat->video_h << stat->loss_rcv_packets
			<< stat->avg_cpu_load << stat->avg_jitter << stat->avg_send_fps;
		return FetchResult(res, "LogParticipantStatistics conference");
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: LogParticipantStatistics failed: " << e.what();
		return false;
	}

	bool Postgres::LogConferenceRecordStart(const vs_conf_id& conf_id, const char* filename, std::chrono::system_clock::time_point started_at) try
	{
		if (!conf_id || !filename) {
			dstream3 << "Error\tPostgres::LogConferenceRecordStart not enough parameters for logging!\n";
			return false;
		}

		dstream3 << "$LogConferenceRecordStart\n";

		cppdb::null_tag_type has_started_at_tag = cppdb::null_value;
		std::tm start_time_as_tm{};

		if (started_at != decltype(started_at){}) {
			has_started_at_tag = cppdb::not_null_value;
			if (!to_gmtime(started_at, start_time_as_tm)) {
				dstream3 << "Error\tPostgres::LogConferenceRecordStop error while converting record start time to UTC!\n";
				return false;
			}
		}

		auto session = OpenSession();
		if (!session.is_open()) return false;

		cppdb::result res = session << "select * from stat.start_recording(?, ?, ?)" << conf_id.m_str << filename << cppdb::use(start_time_as_tm, has_started_at_tag);
		return FetchResult(res, "LogConferenceRecordStart");
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: LogConferenceRecordStart failed: " << e.what();
		return false;
	}

	bool Postgres::LogConferenceRecordStop(const vs_conf_id& conf_id, std::chrono::system_clock::time_point stopped_at, uint64_t file_size) try
	{
		if (!conf_id) {
			dstream3 << "Error\tPostgres::LogConferenceRecordStop not enough parameters for logging!\n";
			return false;
		}

		dstream3 << "$LogConferenceRecordStop\n";

		auto session = OpenSession();
		if (!session.is_open()) return false;

		cppdb::null_tag_type has_stopped_at_tag = cppdb::null_value;
		std::tm stop_time_as_tm{};

		if (stopped_at != decltype(stopped_at){}) {
			has_stopped_at_tag = cppdb::not_null_value;
			if (!to_gmtime(stopped_at, stop_time_as_tm))
			{
				dstream3 << "Error\tPostgres::LogConferenceRecordStop error while converting record stop time to UTC!\n";
				return false;
			}
		}

		cppdb::result res = session << "select * from stat.stop_recording(?, ?, ?)" << conf_id.m_str << cppdb::use(stop_time_as_tm, has_stopped_at_tag) << file_size;
		return FetchResult(res, "LogConferenceRecordStop");
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: LogConferenceRecordStop failed: " << e.what();
		return false;
	}

	bool Postgres::LogSystemParams(std::vector<int> &p) try
	{
		if (p.size() != 13)
			return false;

		auto session = OpenSession();
		if (!session.is_open()) return false;

		//log.add_stat(_cpu_load              int,
		//	_endpoints             int,
		//	_transport_bitrate_in  int,
		//	_transport_bitrate_out int,
		//	_streams               int,
		//	_streams_bitrate_in    int,
		//	_streams_bitrate_out   int,
		//	_online_users          int,
		//	_conferences           int,
		//	_participants          int,
		//	_guests                int,
		//	_gateways              int,
		//	_terminals             int)

		cppdb::result res = session << "select * from log.add_stat(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
			<< p[0] << p[1] << p[2] << p[3] << p[4] << p[5] << p[6] << p[7] << p[8] << p[9] << p[10] << p[11] << p[12];
		return FetchResult(res, "LogSystemParams");
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: LogSystemParams failed: " << e.what();
		return false;
	}

	bool Postgres::LogParticipantDevices(VS_ParticipantDeviceParams& params) try
	{
		if (!params.conference_id || !params.user_instance || !params.action || !params.device_id || !params.device_type)
			return false;

		cppdb::null_tag_type mt = params.pmute ? cppdb::not_null_value : cppdb::null_value;
		cppdb::null_tag_type vt = params.pvolume ? cppdb::not_null_value : cppdb::null_value;
		cppdb::null_tag_type dnt = params.device_name ? cppdb::not_null_value : cppdb::null_value;
		bool mute = params.pmute ? *params.pmute : false;
		int32_t volume = params.pvolume ? *params.pvolume : 0;

		auto session = OpenSession();
		if (!session.is_open()) return false;

		auto call_id = VS_RemoveTranscoderID_sv(params.user_instance);

		cppdb::result res = session << "select * from stat.log_participant_device(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
			<< params.conference_id
			<< std::string(call_id)
			<< params.user_instance
			<< params.action
			<< params.device_id
			<< params.device_type
			<< cppdb::use(params.device_name, dnt)
			<< params.is_active
			<< true
			<< cppdb::use(mute, mt)
			<< cppdb::use(volume, vt);
		return FetchResult(res, "LogParticipantDevices");
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: LogParticipantDevices failed: " << e.what();
		return false;
	}

	bool Postgres::LogParticipantRole(VS_ParticipantDescription& pd) try
	{
		if (pd.m_conf_id.IsEmpty() || pd.m_user_id.IsEmpty())
			return false;

		auto session = OpenSession();
		if (!session.is_open()) return false;
		int32_t onpodium = pd.m_brStatus&BS_SND_PAUSED ? 0 : 1;

		auto call_id = VS_RemoveTranscoderID_sv(SimpleStrToStringView(pd.m_user_id));

		cppdb::result res = session << "select * from stat.log_participant_role(?, ?, ?, ?, ?)"
			<< pd.m_conf_id.m_str
			<< std::string(call_id)
			<< pd.m_user_id.m_str
			<< pd.m_role
			<< onpodium;
		return FetchResult(res, "LogParticipantRole");
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: LogParticipantRole failed: " << e.what();
		return false;
	}

	long Postgres::SetRegID(const char* call_id, const char* reg_id, VS_RegID_Register_Type type) try
	{
		if (!reg_id || !*reg_id)
			return -1;
		if (!call_id)
			call_id = "";

		auto session = OpenSession();
		if (!session.is_open())
			return 1;

		cppdb::result res;
		if (VS_RegID_Register_Type::REGID_REGISTER == type)
			res = session << "select * from push.add_token(?, ?)" << reg_id << call_id;
		else
			res = session << "select * from push.delete_token(?)" << reg_id;
		return FetchResult(res, "SetRegID") ? 0 : -1;
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: SetRegID failed: " << e.what();
		return 1;
	}


	int Postgres::GetCalls(VS_Container & cnt, int & entries, VS_AddressBook ab, const vs_user_id & owner, time_t last_deleted_call) try
	{
		if (!owner) {
			dstream3 << "Error\tPostgres::GetCalls not enough parameters for logging!\n";
			return SEARCH_FAILED;
		}

		auto session = OpenSession();
		if (!session.is_open()) return SEARCH_FAILED;
		cppdb::result res;

		switch (ab) {
		case AB_MISSED_CALLS:
		{
			dstream3 << "$GetMissedCalls\n";
			res = session << "select * from stat.get_missed_calls(?)" << owner.m_str;
			break;
		}
		case AB_RECEIVED_CALLS:
		{
			dstream3 << "$GetAnsweredCalls\n";
			res = session << "select * from stat.get_answered_calls(?)" << owner.m_str;
			break;
		}
		case AB_PLACED_CALLS:
		{
			dstream3 << "$GetPlacedCalls\n";
			res = session << "select * from stat.get_outgoing_calls(?)" << owner.m_str;
			break;
		}
		default:
			return SEARCH_FAILED;
		}

		while (res.next()) {
			std::string call_id;
			std::tm utc_call_time;	// expect utc time zone in database
			if (!res.fetch(0, call_id) || !res.fetch(1, utc_call_time)) continue;

			if (cnt.AddValue(USERNAME_PARAM, call_id) && cnt.AddValue(TIME_PARAM, utc_call_time))
				++entries;
		}

		return SEARCH_DONE;
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: GetCalls failed: " << e.what();
		return SEARCH_FAILED;
	}

	bool Postgres::Init(const std::string& db_conn_str) try
	{
		m_pool = cppdb::pool::create(db_conn_str);
		auto session = OpenSession();
		if (!session.is_open())	return false;	// verify that we can successfully open session
		// clean conferences
		cppdb::result res = session << "select * from stat.end_inactive_conferences()";
		bool bres = FetchResult(res, "end_inactive_conferences");
		return true;
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: Init failed: " << e.what();
		return false;
	}

	cppdb::session Postgres::OpenSession() try
	{
		return m_pool.get()->open();
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: OpenSession failed : " << e.what();
		return {};
	}

	vs_conf_id Postgres::CreateNewConfID() try
	{
		auto session = OpenSession();
		if (!session.is_open()) return 0;

		cppdb::result res = session << "select * from stat.generate_conference_id()";
		std::string str_conf_id;

		if (!res.next())
		{
			return 0;
		}

		res.fetch(str_conf_id);
		return vs_conf_id(str_conf_id.c_str());
	}
	catch (const cppdb::cppdb_error& e)
	{
		dstream3 << "Postgres: CreateNewConfID failed : " << e.what();
		return {};
	}
}

#endif
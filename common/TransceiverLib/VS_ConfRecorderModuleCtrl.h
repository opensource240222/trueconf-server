#pragma once
#include "VS_RelayModule.h"
#include <boost/filesystem.hpp>
#include <boost/signals2/signal.hpp>
#include "../std/cpplib/VS_ConferenceID.h"
#include "std-generic/asio_fwd.h"

#include <boost/asio/steady_timer.hpp>

class VS_ConfRecorderModuleCtrl : public VS_RelayModule
{
public:
	typedef boost::signals2::signal<void(const vs_conf_id&, const std::string&, std::chrono::system_clock::time_point)> signalRecordStartInfo;
	typedef boost::signals2::signal<void(const vs_conf_id&, std::chrono::system_clock::time_point, uint64_t)> signalRecordStopInfo;

	VS_ConfRecorderModuleCtrl(boost::asio::io_service& ios);
	virtual ~VS_ConfRecorderModuleCtrl(){};

	boost::signals2::connection ConnectRecordStartInfo(signalRecordStartInfo::slot_type&& slot);
	boost::signals2::connection ConnectRecordStopInfo(signalRecordStopInfo::slot_type&& slot);

	bool StartRecordConference(const char *conf_name);
	bool StopRecordConference(const char *conf_name);
	bool PauseRecordConference(const char *conf_name);
	bool ResumeRecordConference(const char *conf_name);

	const static std::string	default_recording_path;
	const static uintmax_t		min_allowed_bytes;
	static bool PrepareRecordingDirectory(boost::filesystem::path &path);

private:
	boost::asio::steady_timer m_deadline_timer;
	void OnEraseTimer(const boost::system::error_code& error);

	signalRecordStartInfo fireRecordStartInfo;
	signalRecordStopInfo fireRecordStopInfo;
	virtual bool ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess);
};

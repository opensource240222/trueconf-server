#include "VS_ConfRecorderModuleCtrl.h"
#include "VS_ConfRecordRelayMsg.h"
#include "std/debuglog/VS_Debug.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/range/iterator_range.hpp>

#include <atomic>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

const std::string VS_ConfRecorderModuleCtrl::default_recording_path = "Recordings";
const uintmax_t  VS_ConfRecorderModuleCtrl::min_allowed_bytes = 10ll * 1024 * 1024 * 1024;//10Gb
const uint32_t ERASE_TIMEOUT_DEFAULT = 60 * 60; // 1 hour
const uint32_t ERASE_MAX_TIME_DEFAULT = 60;
std::atomic<std::chrono::seconds> ERASE_TIMEOUT(static_cast<std::chrono::seconds>(ERASE_TIMEOUT_DEFAULT));
std::atomic<std::chrono::seconds> ERASE_MAX_TIME(static_cast<std::chrono::seconds>(ERASE_MAX_TIME_DEFAULT));

bool VS_ConfRecorderModuleCtrl::PrepareRecordingDirectory(boost::filesystem::path &path)
{
	VS_RegistryKey cfg(false, CONFIGURATION_KEY);
	char path_buff[512] = { 0 };
	const char *pPath = path_buff;
	if (!cfg.GetValue(path_buff, sizeof(path_buff), VS_REG_STRING_VT, "Recordings Path"))
		pPath = VS_ConfRecorderModuleCtrl::default_recording_path.c_str();

	boost::system::error_code ec;
	boost::filesystem::create_directories(pPath, ec);
	if (ec) {
		dstream0 << "PrepareRecordingDirectory: Error creating '" << pPath << "': " << ec.message();
		return false;
	}

	path = pPath;

	return true;
}

VS_ConfRecorderModuleCtrl::VS_ConfRecorderModuleCtrl(boost::asio::io_service& ios) :
	VS_RelayModule(VS_ConfRecordRelayMsg::module_name), m_deadline_timer(ios)
{
	boost::system::error_code ec;
	m_deadline_timer.expires_from_now(std::chrono::seconds(10), ec);
	if (!ec)
		m_deadline_timer.async_wait(boost::bind(&VS_ConfRecorderModuleCtrl::OnEraseTimer, this, _1));
	else
		dstream4 << "Failed to start timer on VS_ConfRecorderModuleCtrl::OnEraseTimer";
}

bool VS_ConfRecorderModuleCtrl::StartRecordConference(const char *conf_name)
{
	boost::filesystem::path path;
	if (!PrepareRecordingDirectory(path))
	{
		dprint0("StartRecordConference: Can't prepare directory for Recording\n");
		return false;
	}
	boost::system::error_code er;
	boost::filesystem::space_info si = boost::filesystem::space(path, er);
	if (er)
	{
		dprint0("StartRecordConference: get space error. Error code = %d\n", er.value());
		return false;
	}
	if (si.available < VS_ConfRecorderModuleCtrl::min_allowed_bytes)
	{
		dstream0 << "StartRecordConference: warning: disc space is low. Available is " << si.available << " bytes\n";
	}
	auto mess = boost::make_shared<VS_ConfRecordRelayMsg>();
	mess->MakeStartRecordConf(conf_name, "");
	return SendMsg(mess, conf_name);
}

bool VS_ConfRecorderModuleCtrl::StopRecordConference(const char *conf_name)
{
	auto mess = boost::make_shared<VS_ConfRecordRelayMsg>();
	mess->MakeStopRecordConf(conf_name, "");
	return SendMsg(mess, conf_name);
}

bool VS_ConfRecorderModuleCtrl::PauseRecordConference(const char * conf_name)
{
	auto mess = boost::make_shared<VS_ConfRecordRelayMsg>();
	mess->MakePauseRecordConf(conf_name, "");
	return SendMsg(mess, conf_name);
}

bool VS_ConfRecorderModuleCtrl::ResumeRecordConference(const char * conf_name)
{
	auto mess = boost::make_shared<VS_ConfRecordRelayMsg>();
	mess->MakeResumeRecordConf(conf_name, "");
	return SendMsg(mess, conf_name);
}

bool VS_ConfRecorderModuleCtrl::ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess)
{
	auto conf_write_mess = boost::make_shared<VS_ConfRecordRelayMsg>();
	if (conf_write_mess->SetMessage(mess->GetMess()))
	{
		switch (conf_write_mess->GetConfWriteMessType())
		{
		case VS_ConfRecordRelayMsg::e_OnStartRecordConference:
			{
				auto conf_id = conf_write_mess->GetConferenceName();
				auto filename = conf_write_mess->GetRecordFilename();
				auto started_at = conf_write_mess->GetRecordStartTime();
				fireRecordStartInfo(conf_id, filename, started_at);
				break;
			}
		case VS_ConfRecordRelayMsg::e_OnStopRecordConference:
			{
				auto conf_id = conf_write_mess->GetConferenceName();
				auto stopped_at = conf_write_mess->GetRecordStopTime();
				auto file_size = conf_write_mess->GetRecordFileSize();
				fireRecordStopInfo(conf_id, stopped_at, file_size);
				break;
			}
		default:break;
		}
	}
	return true;
}

void VS_ConfRecorderModuleCtrl::OnEraseTimer(const boost::system::error_code& error)
{
	auto start = std::chrono::steady_clock::now();
	dstream4 << "VS_ConfRecorderModuleCtrl::OnEraseTimer";
	if (error)
		return;	// canceled
	VS_RegistryKey cfg(false, CONFIGURATION_KEY);
	{
		uint32_t seconds(0);
		cfg.GetValue(&seconds, sizeof(seconds), VS_REG_INTEGER_VT, "Recordings Erase Timeout");
		ERASE_TIMEOUT = std::chrono::seconds(seconds ? seconds : ERASE_TIMEOUT_DEFAULT);
		seconds = 0;
		cfg.GetValue(&seconds, sizeof(seconds), VS_REG_INTEGER_VT, "Recordings Erase MaxTime");
		ERASE_MAX_TIME = std::chrono::seconds(seconds ? seconds : ERASE_MAX_TIME_DEFAULT);
	}
	boost::system::error_code ec;
	m_deadline_timer.expires_from_now(ERASE_TIMEOUT.load(), ec);
	m_deadline_timer.async_wait(boost::bind(&VS_ConfRecorderModuleCtrl::OnEraseTimer, this, _1));
	uint32_t days(0);
	cfg.GetValue(&days, sizeof(days), VS_REG_INTEGER_VT, "Recordings Expire Days");
	if (days == 0)
		return;
	boost::filesystem::path path;
	if (!PrepareRecordingDirectory(path))
		return;
	if (!boost::filesystem::is_directory(path, ec) || ec)
		return;
	auto dir = boost::make_iterator_range(boost::filesystem::directory_iterator(path, ec), {});
	if (ec)
		return;
	for (auto& entry : dir)
	{
		if (std::chrono::steady_clock::now() - start > ERASE_MAX_TIME.load())
			return;
		auto ext = entry.path().extension().wstring();
		if (!boost::iequals(ext, L".avi") &&
			!boost::iequals(ext, L".mkv") &&
			!boost::iequals(ext, L".mp4"))
			continue;
		auto t = boost::filesystem::last_write_time(entry.path(), ec);
		if (ec)
			continue;
		auto n = time(nullptr);
		if (t == (time_t)(-1) || n == (time_t)(-1))
			continue;
		if (n < t || (n - t < days * 24 * 60 * 60))
			continue;
		dstream3 << "Erase recording file: " << entry;
		boost::filesystem::remove(entry, ec);
		if (ec)
			dstream3 << "failed to erase file: " << entry;
	}
}

boost::signals2::connection VS_ConfRecorderModuleCtrl::ConnectRecordStartInfo(signalRecordStartInfo::slot_type&& slot)
{
	return fireRecordStartInfo.connect(slot);
}

boost::signals2::connection VS_ConfRecorderModuleCtrl::ConnectRecordStopInfo(signalRecordStopInfo::slot_type&& slot)
{
	return fireRecordStopInfo.connect(slot);
}

#undef DEBUG_CURRENT_MODULE

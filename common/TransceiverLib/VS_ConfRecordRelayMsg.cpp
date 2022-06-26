#include "VS_ConfRecordRelayMsg.h"

const char VS_ConfRecordRelayMsg::module_name[] = "ConferenceWriterModule";

VS_ConfRecordRelayMsg::VS_ConfRecordRelayMsg() : VS_EnvelopeRelayMessage(module_name)
{}

VS_ConfRecordRelayMsg::~VS_ConfRecordRelayMsg()
{}

bool VS_ConfRecordRelayMsg::MakeStartRecordConf(const char *conference_name, const char *part_name)
{
	return MakeMsgImpl(conference_name,part_name,e_StartRecordConference);
}
bool VS_ConfRecordRelayMsg::MakeStopRecordConf(const char *conference_name, const char *part_name)
{
	return MakeMsgImpl(conference_name,part_name,e_StopRecordConference);
}

bool VS_ConfRecordRelayMsg::MakeOnStartRecordConf(const char* conference_name, const char* part_name,
	const char* record_filename, std::chrono::system_clock::time_point started_at)
{
	return MakeMsgImpl(conference_name, part_name, e_OnStartRecordConference, record_filename, started_at);
}

bool VS_ConfRecordRelayMsg::MakeOnStopRecordConf(const char* conference_name, const char* part_name,
	std::chrono::system_clock::time_point stopped_at, uint64_t file_size)
{
	return MakeMsgImpl(conference_name, part_name, e_OnStopRecordConference, stopped_at, file_size);
}

bool VS_ConfRecordRelayMsg::MakePauseRecordConf(const char * conference_name, const char * part_name)
{
	return MakeMsgImpl(conference_name, part_name, e_PauseRecordConference);
}

bool VS_ConfRecordRelayMsg::MakeResumeRecordConf(const char * conference_name, const char * part_name)
{
	return MakeMsgImpl(conference_name, part_name, e_ResumeRecordConference);
}

bool VS_ConfRecordRelayMsg::MakeMsgImpl(const char* conference_name, const char* part_name,
	const VS_ConfRecordRelayMsg::MessageType type, const char* record_filename,
	std::chrono::system_clock::time_point started_at)
{
	return
		MakeMsgBase(conference_name, part_name, type) &&
		SetParam("RecordFilename", record_filename) &&
		SetParam("RecordStartedAt", started_at) &&
		Make();
}

bool VS_ConfRecordRelayMsg::MakeMsgImpl(const char* conference_name, const char* part_name,
	const VS_ConfRecordRelayMsg::MessageType type, std::chrono::system_clock::time_point stopped_at, uint64_t file_size)
{
	return
		MakeMsgBase(conference_name, part_name, type) &&
		SetParamI64("RecordFileSize", file_size) &&
		SetParam("RecordStoppedAt", stopped_at) &&
		Make();
}

bool VS_ConfRecordRelayMsg::MakeMsgImpl(const char *conference_name, const char *part_name, const VS_ConfRecordRelayMsg::MessageType type)
{
	return
		MakeMsgBase(conference_name, part_name, type) &&
		Make();
}

bool VS_ConfRecordRelayMsg::MakeMsgBase(const char *conference_name, const char *part_name, const VS_ConfRecordRelayMsg::MessageType type)
{
	ClearContainer();
	return
		SetParam("Type", (int32_t)type) &&
		SetParam("ConfName", conference_name) &&
		SetParam("PartName", part_name);
}

uint32_t VS_ConfRecordRelayMsg::GetConfWriteMessType() const
{
	int32_t type;
	if(GetParam("Type",type))
		return (uint32_t)type;
	return 0;
}
const char *VS_ConfRecordRelayMsg::GetConferenceName() const
{
	return GetStrValRef("ConfName");
}
const char *VS_ConfRecordRelayMsg::GetPartName() const
{
	return GetStrValRef("PartName");
}
const char *VS_ConfRecordRelayMsg::GetRecordFilename() const
{
	return GetStrValRef("RecordFilename");
}

uint64_t VS_ConfRecordRelayMsg::GetRecordFileSize() const
{
	uint64_t file_size;
	if (GetParamI64("RecordFileSize", file_size))
		return file_size;
	return 0;
}

std::chrono::system_clock::time_point VS_ConfRecordRelayMsg::GetRecordStartTime() const
{
	std::chrono::system_clock::time_point started_at;
	if (GetParam("RecordStartedAt", started_at))
		return started_at;
	return {};
}

std::chrono::system_clock::time_point VS_ConfRecordRelayMsg::GetRecordStopTime() const
{
	std::chrono::system_clock::time_point stopped_at;
	if (GetParam("RecordStoppedAt", stopped_at))
		return stopped_at;
	return {};
}

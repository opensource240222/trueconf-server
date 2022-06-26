#pragma once
#include "VS_EnvelopeRelayMessage.h"

class VS_ConfRecordRelayMsg : public VS_EnvelopeRelayMessage
{
public:
	enum MessageType
	{
		e_StartRecordConference,
		e_StopRecordConference,
		e_OnStartRecordConference,
		e_OnStopRecordConference,
		e_PauseRecordConference,
		e_ResumeRecordConference
	};
	const static char module_name[];
	VS_ConfRecordRelayMsg();
	virtual ~VS_ConfRecordRelayMsg();

	uint32_t GetConfWriteMessType() const;

	bool MakeStartRecordConf(const char *conference_name, const char *part_name);
	bool MakeStopRecordConf(const char *conference_name, const char *part_name);
	bool MakeOnStartRecordConf(const char *conference_name, const char *part_name, const char *record_filename, std::chrono::system_clock::time_point started_at);
	bool MakeOnStopRecordConf(const char *conference_name, const char *part_name, std::chrono::system_clock::time_point stopped_at, uint64_t file_size);
	bool MakePauseRecordConf(const char *conference_name, const char *part_name);
	bool MakeResumeRecordConf(const char *conference_name, const char *part_name);

	const char *GetConferenceName() const;
	const char *GetPartName() const;
	const char *GetRecordFilename() const;
	uint64_t GetRecordFileSize() const;
	std::chrono::system_clock::time_point GetRecordStartTime() const;
	std::chrono::system_clock::time_point GetRecordStopTime() const;

private:
	bool MakeMsgImpl(const char *conference_name, const char *part_name, const VS_ConfRecordRelayMsg::MessageType type);
	bool MakeMsgImpl(const char *conference_name, const char *part_name, const VS_ConfRecordRelayMsg::MessageType type, const char* record_filename, std::chrono::system_clock::time_point started_at);
	bool MakeMsgImpl(const char *conference_name, const char *part_name, const VS_ConfRecordRelayMsg::MessageType type, std::chrono::system_clock::time_point stopped_at, uint64_t file_size);
	bool MakeMsgBase(const char *conference_name, const char *part_name, const VS_ConfRecordRelayMsg::MessageType type);
};
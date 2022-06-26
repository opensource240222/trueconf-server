#pragma once

#include "VS_EnvelopeRelayMessage.h"

class VS_RTSPBroadcastRelayMessage : public VS_EnvelopeRelayMessage
{
public:
	enum MessageType
	{
		e_InvalidMessageType = 0,
		e_StartRTSPBroadcast,
		e_StopRTSPBroadcast,
		e_AnnounceRTSPBroadcast,
		e_AnnounceStatusReport,
	};

	static const char module_name[];

	VS_RTSPBroadcastRelayMessage();
	virtual ~VS_RTSPBroadcastRelayMessage();

	uint32_t GetMessageType() const;

	bool MakeStartRTSPBroadcast(string_view conf_name, string_view url, string_view description_utf8, string_view enabled_codecs, string_view helper_program);
	bool MakeStopRTSPBroadcast(string_view conf_name);
	bool MakeAnnounceRTSPBroadcast(string_view conf_name, string_view announce_id, string_view url, string_view username, string_view password, bool rtp_over_tcp, string_view enabled_codecs, unsigned keepalive_timeout, unsigned retries, unsigned retry_delay);
	bool MakeAnnounceStatusReport(string_view conf_name, string_view announce_id, bool is_active, string_view reason);

	string_view GetConferenceName() const;
	const char* GetURL() const;
	const char* GetDescription() const;
	string_view GetEnabledCodecs() const;
	string_view GetAnnounceID() const;
	const char* GetUsername() const;
	const char* GetPassword() const;
	bool GetRTPOverTCP() const;
	unsigned GetKepaliveTimeout() const;
	unsigned GetRetries() const;
	unsigned GetRetryDelay() const;
	bool GetIsActive() const;
	string_view GetReason() const;
	const char* GetHelperProgram() const;
};

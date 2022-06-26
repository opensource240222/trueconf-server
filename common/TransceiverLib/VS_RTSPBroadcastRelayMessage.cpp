#include "VS_RTSPBroadcastRelayMessage.h"

const char VS_RTSPBroadcastRelayMessage::module_name[] = "ConferenceRTSPBroadcaster";

VS_RTSPBroadcastRelayMessage::VS_RTSPBroadcastRelayMessage() : VS_EnvelopeRelayMessage(module_name)
{
}

VS_RTSPBroadcastRelayMessage::~VS_RTSPBroadcastRelayMessage()
{
}

bool VS_RTSPBroadcastRelayMessage::MakeStartRTSPBroadcast(string_view conf_name, string_view url, string_view description_utf8, string_view enabled_codecs, string_view helper_program)
{
	ClearContainer();
	if (!SetParam("Type", (int32_t)e_StartRTSPBroadcast)
	 || !SetParam("ConfName", conf_name)
	 || !SetParam("URL", url)
	 || !SetParam("Description", description_utf8)
	 || !SetParam("Codecs", enabled_codecs)
	 || !SetParam("RTSP Helper Program", helper_program)
	)
		return false;
	return Make();
}

bool VS_RTSPBroadcastRelayMessage::MakeStopRTSPBroadcast(string_view conf_name)
{
	ClearContainer();
	if (!SetParam("Type", (int32_t)e_StopRTSPBroadcast)
	 || !SetParam("ConfName", conf_name)
	)
		return false;
	return Make();
}

bool VS_RTSPBroadcastRelayMessage::MakeAnnounceRTSPBroadcast(string_view conf_name, string_view announce_id, string_view url, string_view username, string_view password, bool rtp_over_tcp, string_view enabled_codecs, unsigned keepalive_timeout, unsigned retries, unsigned retry_delay)
{
	ClearContainer();
	if (!SetParam("Type", (int32_t)e_AnnounceRTSPBroadcast)
	 || !SetParam("ConfName", conf_name)
	 || !SetParam("AnnounceID", announce_id)
	 || !SetParam("URL", url)
	 || !SetParam("Username", username)
	 || !SetParam("Password", password)
	 || !SetParam("RTPOverTCP", rtp_over_tcp)
	 || !SetParam("Codecs", enabled_codecs)
	 || !SetParam("KeepaliveTimeout", (int32_t)keepalive_timeout)
	 || !SetParam("Retries", (int32_t)retries)
	 || !SetParam("RetryDelay", (int32_t)retry_delay)
	)
		return false;
	return Make();
}

bool VS_RTSPBroadcastRelayMessage::MakeAnnounceStatusReport(string_view conf_name, string_view announce_id, bool is_active, string_view reason)
{
	ClearContainer();
	if (!SetParam("Type", (int32_t)e_AnnounceStatusReport)
		|| !SetParam("ConfName", conf_name)
		|| !SetParam("AnnounceID", announce_id)
		|| !SetParam("IsActive", is_active)
		|| !SetParam("Reason", reason)
		)
		return false;
	return Make();
}

uint32_t VS_RTSPBroadcastRelayMessage::GetMessageType() const
{
	int32_t type;
	if (GetParam("Type", type))
		return (uint32_t)type;
	return e_InvalidMessageType;
}

string_view VS_RTSPBroadcastRelayMessage::GetConferenceName() const
{
	return GetStrValueView("ConfName");
}

const char* VS_RTSPBroadcastRelayMessage::GetURL() const
{
	return GetStrValRef("URL");
}

const char* VS_RTSPBroadcastRelayMessage::GetDescription() const
{
	return GetStrValRef("Description");
}

string_view VS_RTSPBroadcastRelayMessage::GetEnabledCodecs() const
{
	return GetStrValueView("Codecs");
}

string_view VS_RTSPBroadcastRelayMessage::GetAnnounceID() const
{
	return GetStrValueView("AnnounceID");
}

const char* VS_RTSPBroadcastRelayMessage::GetUsername() const
{
	return GetStrValRef("Username");
}

const char* VS_RTSPBroadcastRelayMessage::GetPassword() const
{
	return GetStrValRef("Password");
}

bool VS_RTSPBroadcastRelayMessage::GetRTPOverTCP() const
{
	bool value(false);
	GetParam("RTPOverTCP", value);
	return value;
}

unsigned VS_RTSPBroadcastRelayMessage::GetKepaliveTimeout() const
{
	int32_t value(0);
	GetParam("KeepaliveTimeout", value);
	return value;
}

unsigned VS_RTSPBroadcastRelayMessage::GetRetries() const
{
	int32_t value(0);
	GetParam("Retries", value);
	return value;
}

unsigned VS_RTSPBroadcastRelayMessage::GetRetryDelay() const
{
	int32_t value(0);
	GetParam("RetryDelay", value);
	return value;
}

bool VS_RTSPBroadcastRelayMessage::GetIsActive() const
{
	bool value(false);
	GetParam("IsActive", value);
	return value;
}

string_view VS_RTSPBroadcastRelayMessage::GetReason() const
{
	return GetStrValueView("Reason");
}

const char* VS_RTSPBroadcastRelayMessage::GetHelperProgram() const
{
	return GetStrValRef("RTSP Helper Program");
}
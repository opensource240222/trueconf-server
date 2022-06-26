#pragma once

#include "../interfaces/VS_ParserInterface.h"
#include "SIPParserLib/VS_RTSP_ParserInfo.h"
#include "SIPParserLib/VS_RTSP_Request.h"
#include "SIPParserLib/VS_RTSP_Response.h"
#include "tools/Server/CommonTypes.h"

#include <chrono>

class VS_RTSPParser : public VS_ParserInterface
{
	static std::string GetUAStringFromResponse(VS_RTSP_Response &rsp);
private:
	bool Send_SetupAudio(const VS_RTSP_Response &resp);
	bool Send_SetupVideo(const VS_RTSP_Response &resp);
	bool Send_SetupMedia(const VS_RTSP_Response &resp, VS_MediaChannelInfo* channel);
	bool Send_Play(const VS_RTSP_Response &resp);
	bool Send_Teardown();
	bool Send_Options();
	bool Send_GetParameter();
	bool Send_Describe();
	bool FillMediaChannelsBySDP(VS_SDPMetaField *sdp);
	void Reject();
	void ProcessNextAction(const VS_RTSP_Response &rsp, const void *buf, std::size_t sz);
	bool EnableAuthorization(VS_RTSP_Response &rsp);
	void RetryCurrentAction(const VS_RTSP_Response &rsp, const void *buf, std::size_t sz);
	void UpdateKeepAlivePeriod(const VS_RTSP_Response &rsp);
	VS_MediaChannelInfo* AudioChannel();
	VS_MediaChannelInfo* VideoChannel();
	bool IsComplete(const void *data, std::size_t sz) const;
private:
	typedef vs::fast_recursive_mutex mutex_t;
private:

	std::vector<VS_MediaChannelInfo> m_media_channels;

	std::string m_from;
	std::string m_to;
	std::string m_input_buffer;

	mutex_t m_lock_request_for_send;
	VS_RTSP_Request m_request_for_send; // thre is no queue. there is only 1 message to send.

	mutex_t m_lock_info;
	VS_RTSP_ParserInfo m_info;

	VS_RTSP_Request m_sdp;
	VS_CallConfig m_config;

	std::chrono::steady_clock::time_point m_start_request_time;
	std::chrono::steady_clock::time_point m_last_message_time;
	std::chrono::steady_clock::time_point m_start_logout_time;
	std::chrono::steady_clock::duration m_keep_alive_period;

	enum VS_State
	{
		state_null,
		state_options,
		state_describe,
		state_wait_for_media_channels,
		state_setup_video,
		state_setup_audio,
		state_play,
		state_playing,
		state_teardown
	};
	VS_State m_state;

	bool m_about_to_destroy;

	bool m_useRemoteTransceiver;
public:

	int SetRecvBuf(const void *buf,  std::size_t sz, const VS_ChannelID channelId, const net::address &remoteAddr,
		net::port remotePort, const net::address &localAddr, net::port localPort) override;
	int GetBufForSend(void* /*buf*/, std::size_t &/*sz*/, const VS_ChannelID /*channelId*/,
		const net::address & /*remoteAddr*/, net::port /*remotePort*/, const net::address & /*localAddr*/, net::port /*localPort*/) override;

	std::string NewDialogID(string_view sipTo, string_view dtmf, const VS_CallConfig &config, string_view myName) override;
	bool InviteMethod(string_view dialogId, string_view fromId, string_view toId, const VS_ConferenceInfo &cfgInfo,
		string_view dnFromUTF8 = {}, bool newSession = true, bool forceCreate = false) override;
	void Hangup(string_view dialogId) override;
	acs::Response Protocol(const void* buf, std::size_t sz) override;
	void Timeout() override;
	void Shutdown() override;
	bool SetMediaChannels(string_view dialogId, const std::vector<VS_MediaChannelInfo>& channels, const std::string& existingConfID, std::int32_t bandwRcv) override;
	bool FillMediaChannels(string_view dialogId, std::vector<VS_MediaChannelInfo>& channels) override;

	bool InviteReplay(string_view dialogId, VS_CallConfirmCode confirmCode, bool isGroupConf, string_view confName, string_view to_displayName) override
	{ return true; }

	void LoggedOutAsUser(string_view dialogId) override;

	void FastUpdatePicture(string_view dialogId) override
	{ return; }

	void Chat(string_view dialogId, const std::string &from, const std::string &to, const std::string &dn, const char *mes) override
	{ return; }

	void SetPeerCSAddress(string_view /*dialogId*/, const net::Endpoint &/*ep*/) override
	{ return; }

	void SetUserToDialogIdCallback(std::function<void(string_view login, string_view dialogId)>) override {}

	~VS_RTSPParser(void);

	bool IsTrunkFull() override
	{ return true; }

	VS_ChannelID GetDefaultChannelID() override
	{ return e_RTSP; }

	bool NeedPermanentConnection(){ return false; };

	VS_CallConfig::eSignalingProtocol MySignallingProtocol() override
	{	return VS_CallConfig::RTSP; };

protected:
	VS_RTSPParser();
	static void PostConstruct(std::shared_ptr<VS_RTSPParser>& /*p*/) {}
};

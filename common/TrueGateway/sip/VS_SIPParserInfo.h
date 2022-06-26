#pragma once

#include "SIPParserLib/VS_BFCPSession_fwd.h"
#include "TrueGateway/interfaces/VS_ParserInfo.h"
#include "net/Endpoint.h"

#include <boost/signals2/signal.hpp>

#include <vector>
#include <queue>
#include <chrono>
#include "SIPParserBase/VS_Const.h"
#include "SIPParserLib/VS_TimerExtention.h"
#include "std-generic/compat/set.h"
#include "std-generic/compat/map.h"
#include "std-generic/compat/functional.h"
#include "FakeClient/VS_ConferenceInfo.h"

class VS_SIPRequest;
class VS_SIPField_Via;
class VS_SIPField_Contact;
class VS_SIPField_RecordRoute;
class VS_SIPURI;
class VS_SIPAuthScheme;
class VS_SDPCodec;
class VS_SDPField_MediaStream;
class VS_AcsLog;
class VS_SignalChannel;

class VS_SIPParserInfo : public VS_ParserInfo
{
private:
	net::Endpoint m_my_cs_address;

public:
	VS_SIPParserInfo(const std::string& server_user_agent);
	VS_SIPParserInfo(const VS_SIPParserInfo &) = delete;
	~VS_SIPParserInfo();

	typedef boost::signals2::signal<void(string_view)> DieSignalType;

	boost::signals2::connection ConnectToDie(const DieSignalType::slot_type& slot)
	{
		return m_signal_Die.connect(slot);
	}

	bool IsRequest() const;
	void IsRequest(bool b);

	eStartLineType GetMessageType() const;							// Из файла VS_Const.h -> eStartLineType
	void SetMessageType(eStartLineType messageType);

	int GetMySequenceNumber() const;
	int IncreaseMySequenceNumber();

	int GetSIPSequenceNumber() const;
	void SetSIPSequenceNumber(int seq);

	int GetSIPProtocol() const;										// SIP/2.0

	int GetResponseCode() const;
	void SetResponseCode(int code);

	void ResponseStr(std::string str);
	const std::string& ResponseStr() const;

	void SIPDialogID(std::string id);
	const std::string& SIPDialogID() const;

	VS_SIPField_Via* GetSIPViaCurrent();
	const VS_SIPField_Via *GetViaTop();
	void ResetIndexSIPVia();
	std::size_t GetSIPViaSize() const;
	bool SetSIPVia(const VS_SIPField_Via* via);
	void ClearSIPVia();
	VS_SIPField_Contact* GetSIPContact() const;
	bool SetSIPContact(const VS_SIPField_Contact* contact);
	void ClearSIPContact();
	void FillUriSetForEstablishedDialog(const VS_SIPURI *contact, const std::vector<VS_SIPField_RecordRoute*> &routeSet, bool isClientUA);
	std::size_t GetSIPRouteSetSize() const;
	const VS_SIPURI* GetNextSIPRouteFromSet();
	void ResetSIPRouteIndex();

	void MyBranch(std::string branch);
	std::string MyBranch() const;

	void AlterMyBranch();

	void SetDisplayNameMy(std::string name);
	const std::string& GetDisplayNameMy() const;
	void SetDisplayNameSip(std::string name);
	const std::string& GetDisplayNameSip() const;

	bool SetTagMy(std::string tag);
	const std::string& GetTagMy() const;

	void SetTagSip(std::string tag);
	const std::string& GetTagSip() const;

	void SetEpidMy(std::string id);
	const std::string& GetEpidMy() const;

	void SetEpidSip(std::string id);
	const std::string& GetEpidSip() const;

	void SipInstance(std::string str);
	const std::string& SipInstance() const;

	eContentType GetContentType() const;
	void SetContentType(const eContentType type);

	void SetLocalBandwidth(unsigned int bandwidth);
	unsigned int GetLocalBandwidth() const;
	void SetRemoteBandwidth(unsigned int bandwidth);
	unsigned int GetRemoteBandwidth() const;

	uint64_t GetSDPSessionId() const;
	unsigned int GetSDPSessionVersion() const;
	void IncreaseSDPSessionVersion();

	std::vector<VS_SDPField_MediaStream*>& MediaStreams();
	VS_SDPField_MediaStream* MediaStream(size_t index, const bool create = false);

	bool WaitDTMFAcknowledge() const;
	void WaitDTMFAcknowledge(bool);
	bool HasDTMFAcknowledge() const;
	void HasDTMFAcknowledge(bool);
	void SetDTMFRequestTime(const std::chrono::steady_clock::time_point time);
	std::chrono::steady_clock::time_point GetLastDTMFRequestTime() const;
	bool HasDTMFAnswerTimeout() const;
	void SetLastDTMFBarnch(std::string branch);
	const std::string &GetLastDTMFBranch() const;

	void SetDTMFPauseTime(std::chrono::steady_clock::duration time);
	std::chrono::steady_clock::duration GetDTMFPauseTime() const;
	void AddDTMFPauseTime(std::chrono::steady_clock::duration time);
	bool DTMFPausePassed() const;

	//TODO: split to many methods
	void SetMyCsAddress(const net::Endpoint &addr);
	const net::Endpoint &GetMyCsAddress() const;

	void SetMyMediaAddress(net::address addr);
	const net::address& GetMyMediaAddress() const;

	void SetViaHost(std::string host);
	const std::string& GetViaHost() const;

	void SetContactHost(std::string host);
	const std::string& GetContactHost() const;

////////////////////////
	std::shared_ptr<VS_SIPAuthScheme> GetAuthScheme() const;
	void SetAuthScheme(const std::shared_ptr<VS_SIPAuthScheme>& scheme);
	bool IsAuthInInvite() const;
	void AuthInInvite(bool isAuth);
	bool VerifyNTLM_SA_lifetime() const;
	bool IsNTLMContext() const;

////////////////////////
		//void AddMyCodec(const SDPMediaType media_type, const int codec);
		//int GetMyCodec(const SDPMediaType media_type);
		//void ResetMyCodecIndex(const SDPMediaType media_type);

////////////////////////

	std::chrono::steady_clock::time_point GetCreateCtxTick() const;
	std::chrono::steady_clock::time_point GetRingingStartTick() const;
	void SetRingingStartTick(const std::chrono::steady_clock::time_point tick);
	std::chrono::steady_clock::time_point GetByeTick() const;
	void SetByeTick(const std::chrono::steady_clock::time_point tick);
	bool IsByeSent() const;
	void ByeIsSent();
	std::chrono::steady_clock::time_point GetRegisterTick() const;
	void SetRegisterTick(std::chrono::steady_clock::time_point tick);
	void SetRegCtxDialogID(const std::string& id);
	std::string GetRegCtxDialogID() const;
	std::chrono::steady_clock::time_point GetOptionsTick() const;
    void SetOptionsTick(std::chrono::steady_clock::time_point tick);
	bool IsKeepAliveSendNeeded();
	bool IsKeepAliveEnabled() const;
	void EnableKeepAlive();
	void SetKeepAliveInterval(std::chrono::steady_clock::duration interval); // in ms
	bool IsSessionTimerEnabled() const;
	void DisableSessionTimer();
	void EnableSessionTimer();
	bool IsSessionTimerUsed() const;
	void UseSessionTimer();
	void UnuseSessionTimer();
	bool IsSessionTimerInRequire() const;
	void AddSessionTimerToRequireHeader();
	void RemoveSessionTimerFromRequireHeader();
	bool IsWeRefresh() const;
	void IsWeRefresh(bool b);

	void EnableBFCP();
	void DisableBFCP();
	bool IsBFCPEnabled() const;
	unsigned int GetBFCPSupportedRoles() const;
	void SetBFCPSupportedRoles(unsigned int roles);
	net::port GetBFCPLocalPort() const;
	boost::shared_ptr<bfcp::ClientSession> GetBFCPClientSession() const;
	void SetBFCPClientSession(const boost::shared_ptr<bfcp::ClientSession>& session);
	boost::shared_ptr<bfcp::ServerSession> GetBFCPServerSession() const;
	void SetBFCPServerSession(const boost::shared_ptr<bfcp::ServerSession>& session);
	std::shared_ptr<VS_SignalChannel> GetBFCPChannel() const;
	void SetBFCPChannel(const std::shared_ptr<VS_SignalChannel>& channel);
	bool GetSlideshowState() const;
	void SetSlideshowState(bool active);

	std::shared_ptr<VS_SIPRequest> GetLastInvite() const;
	void SetLastInvite(const std::shared_ptr<VS_SIPRequest>& invite);
	enum class SMCContinuation // SMC == SetMediaChannels
	{
		Nothing,
		SendINVITE,
		SendResponse,
		//SendACK, // We are not sending empty invites, so we won't get an offer in 2xx response and have to wait for media channels to send an ACK
	};
	SMCContinuation GetSMCContinuation() const;
	void SetSMCContinuation(SMCContinuation mode);
	std::chrono::steady_clock::time_point GetSMCTick() const;
	void SetSMCTick(std::chrono::steady_clock::time_point tick);

	bool IsAnswered() const;
	void IsAnswered(const bool isAnswered);
	void SetAnswered(const std::chrono::steady_clock::time_point tick);
	std::chrono::steady_clock::time_point GetAnswered() const;

	bool IsDialogEstablished() const;
	void DialogEstablished(bool val = true);

	bool IsDirection_ToSIP() const;
	void SetDirection(const bool IsToSIP);

	void AddDTMF(const char digit);
	char GetDTMF();
	void SetDTMF_Treshold(std::uint32_t treshold);
	std::uint32_t GetDTMF_Treshold() const;

	std::vector<std::shared_ptr<const VS_SDPCodec>> GetLocalAudioCodecs() const;
	std::vector<std::shared_ptr<const VS_SDPCodec>> GetLocalVideoCodecs() const;
	std::vector<std::shared_ptr<const VS_SDPCodec>> GetLocalDataCodecs() const;

	void SetMyExternalCsAddress(std::string host);
	const std::string &GetMyExternalCsAddress() const;		// Returns External CS Address in string format.

	void NoMoreFUP();
	bool IsNoMoreFUP() const;

	void LimitH264Level(int level);

	void SetPasswordTranscoder(std::string password);		// password hash starts with "$4*", to login transcoder as tc_user
	const std::string &GetPasswordTranscoder() const;

	void SetListenPort(net::port port);
	net::port GetListenPort() const;

	void SetInviteAfterRegister(std::shared_ptr<VS_SIPParserInfo>);
	std::shared_ptr<VS_SIPParserInfo> IsInviteAfterRegister() const;

	bool IsRegisterContext() const;
	void IsRegisterContext(bool val);

	bool SetConfig(VS_CallConfig cfg) override;

	bool IsCompactHeaderAllowed() const;
	bool UseSingleBestCodec() const;
	const std::string &GetInviteAfterOptions() const;
	void SetInviteAfterOptions(std::string val);
	void NeedUpdateOptionsBranch(bool val);
	bool DoINeedUpdateOptionsBranch() const;

	bool NeedsOptionsBeforeInvite(void) const;

	bool IsGroupConf() const override;
	void SetGroupConf(bool b) override;

	bool IsPublicConf() const;
	void SetPublicConf(bool b);

	bool CanIChooseCodec() const;
	bool CanIChooseCodec(bool val);

	bool NoRtpmapForAudioStaticPayload() const;
	bool NoRtpmapForVideoStaticPayload() const;

	bool ICEEnabled() const;
	bool SRTPEnabled() const;
	bool HaveAuthenticatedTLSConnection() const;

	unsigned GetCnum() const;
	unsigned GetIncrCnum() const;

	void SetSRTPKey(std::string s);
	const std::string &GetSRTPKey() const;

	const std::string& IceUfrag() const;
	const std::string& IcePwd() const;

	const std::string& GetContactGruu() const;
	void SetContactGruu(std::string s);

	void AddPendingInviteUser(std::string fromId);
	std::vector<std::string> PopPendingInvites();
	size_t GetPendingInvitesSize() const;

	void AddPendingReqInviteUser(std::string fromId);

	bool GetPendingReqInvite(string_view user, std::string &fullId);

	void AddPengingMessage(string_view msg, eContentType ct = CONTENTTYPE_TEXT_PLAIN);
	std::vector<std::tuple<std::string, eContentType>> PopPendingMessages();

	void SetConfTopic(const std::string &s);
	const std::string &GetConfTopic() const;

	void SetConfID(const std::string &s);
	const std::string &GetConfID() const;

	const VS_ConferenceInfo& GetConfInfo() const;

	std::pair<std::uint32_t, std::uint32_t> GetSsrcRangeAudio() const;
	std::pair<std::uint32_t, std::uint32_t> GetSsrcRangeVideo() const;

	TimerExtention &GetTimerExtention();
	bool InCall() const;

	// For retry-after header: sip -> tc
	void RetryAfterTime(std::chrono::steady_clock::time_point p);
	std::chrono::steady_clock::time_point RetryAfterTime() const;
	void NeedRetryAfter(bool b);
	bool NeedRetryAfter() const;

	// For retry-after header: tc - > sip
	std::chrono::steady_clock::duration GetRetryAfterValue() const;
	void SetRetryAfterValue(std::chrono::steady_clock::duration value);

	class RedirectCache
	{
	public:
		RedirectCache();
		bool InsertNewAddress(string_view address);
		bool GetAddressToCall(std::string &outAddress);
		bool MarkAsCalled(string_view address);
		bool HaveNotCalledAddresses() const;
	private:
		template<class K, class V>
		using map_t = vs::map<K, V, vs::less<>>;

		map_t<std::string /* uri_to_call */, bool /* was called */ > m_cache;
		std::uint16_t m_not_called_addresses;
	};

	void InsertCallIdsToRedirect(const std::vector<std::string> &ids);
	bool HaveAddressesToRedirect() const;
	bool GetAddressToRedirect(std::string &outAddress);
	bool NeedAddressRedirection() const;
	void NeedAddressRedirection(bool val);

	void ClearCodecs(void);
	void ReadyToDie(const bool val);
	bool ReadyToDie() const;

	void CreatedByChatBot(const bool val);
	bool CreatedByChatBot() const;

	void MsgAliveTick(const std::chrono::steady_clock::time_point);
	std::chrono::steady_clock::time_point MsgAliveTick() const;
	bool ActiveMsgCtx() const;
	bool RTFSupported() const;
	void SetAcceptedTextTypesRemote(const std::vector<std::string>& types);
	bool IsTel(void) const;
	void SetIsTel(bool is_tel);
	const std::string& GetServerUserAgent() const;

protected:
	VS_SIPField_Via* GetSIPVia(std::size_t index);

private:
	bool							m_ready_to_die;
	bool							m_is_request;						// Request | Response
	eStartLineType					m_message_type;						// TYPE_INVITE, TYPE_ACK, TYPE_BYE, ...

	int								m_my_sequence_number;				// CSeq: 351 REGISTER
	int								m_sip_sequence_number;				// Only for VS_SIPResponse

	int								m_response_code;
	std::string						m_response_str;
	std::string						m_DialogID;

	std::string						m_my_display_name;
	std::string						m_my_tag;

	std::string						m_sip_display_name;
	std::string 					m_sip_tag;

	std::string						m_my_epid;
	std::string						m_sip_epid;
	std::string						m_sip_instance;

	eContentType					m_content_type;
	unsigned int					m_local_bandwidth;
	unsigned int					m_remote_bandwidth;

	std::uint64_t					m_sdp_session_id;
	unsigned int					m_sdp_session_version;

	std::vector<VS_SDPField_MediaStream*>	m_media_streams;
	std::vector<std::string>				m_accepted_text_types_remote;

	std::vector<VS_SIPField_Via*>			m_sip_via;
	std::size_t								m_sip_via_index;

	struct
	{
		std::vector<const VS_SIPURI*>			set;
		size_t									index;
	} m_sip_route;

	VS_SIPField_Contact*					m_sip_contact;

	std::string								m_my_branch_const_part;
	int										m_my_branch_variable_part;
	bool									m_my_transaction_in_progress;

//////////////////////////////////////////////////////////////////////////

	net::address					m_my_media_address;
	std::string						m_my_external_cs_host;	 // External CS Address in string format.

//////////////////////////////////////////////////////////////////////////
	std::shared_ptr<VS_SIPAuthScheme>	m_auth_scheme;
	bool							m_auth_in_invite;

	//std::vector<int>				m_my_audio_codec_table;
	//unsigned int					m_audio_table_index;

	//std::vector<int>				m_my_video_codec_table;
	//unsigned int					m_video_table_index;

	std::chrono::steady_clock::time_point					m_create_ctx_tick;
	std::chrono::steady_clock::time_point					m_ringing_start_tick;
	std::chrono::steady_clock::time_point					m_bye_tick;
	std::chrono::steady_clock::time_point					m_answered_tick;
	std::chrono::steady_clock::time_point					m_register_tick;		// time of last success registration of SIP terminal on our server, or last re-registration
	std::chrono::steady_clock::time_point					m_keep_alive_tick;
	std::chrono::steady_clock::time_point                   m_options_tick;
	std::chrono::steady_clock::time_point					m_msg_ctx_activity_tick;	// to verify that auth message ctx is in use and delete it if not
	std::chrono::steady_clock::duration						m_keep_alive_interval;
	bool							m_keep_alive_enabled;
	bool							m_session_timer_enabled = true;
	bool							m_use_session_timer = false;
	bool							m_is_we_refresh = false;
	bool							m_session_timer_in_require = true;
	std::shared_ptr<VS_SIPRequest>	m_last_invite;
	SMCContinuation					m_smc_mode;
	std::chrono::steady_clock::time_point					m_smc_tick;
	bool							m_dialog_established;
	bool							m_answered_ack;
	bool							m_direction_to_sip;
	bool							m_bye_is_sent;
	bool							m_wait_dtmf_acknoledge;
	bool							m_has_dtmf_acknoledge;
	std::chrono::steady_clock::time_point	m_last_dtmf_request_time;
	std::string						m_last_dtmf_branch;

	std::chrono::steady_clock::duration	m_dtmf_pause_time;

	std::queue<char>				m_dtmf_digits;
	std::uint32_t					m_dtmf_treshold;
	bool							m_no_more_fup_this_call;	// indicates whether Fast Update Picture further can be sent during current call
	bool							m_bfcp_is_enabled;			//Enabled bfcp;
	unsigned int					m_bfcp_supported_roles;   // eSDP_FLOORCTRL_ROLE

	boost::shared_ptr<bfcp::ClientSession> m_bfcp_client_session;
	boost::shared_ptr<bfcp::ServerSession> m_bfcp_server_session;
	std::shared_ptr<VS_SignalChannel> m_bfcp_channel;
	bool							m_slideshow_active;

	// For retry-after header: sip -> tc
	std::chrono::steady_clock::time_point m_time_to_retryafter;
	bool m_need_to_retryafter;

	// For retry-after header: tc - > sip
	std::chrono::steady_clock::duration m_retry_after_value;

	std::string						m_via_host;
	std::string					    m_contact_host;

	void AddRouteToSet(const VS_SIPURI *route);
	void ClearSIPRouteSet();

	void FillCodecsFromString(string_view codecs);
	void AddCodec(string_view codec);
	std::vector<std::shared_ptr<const VS_SDPCodec>>		m_lac, m_lvc;		// local audio/video codecs
	std::vector<std::shared_ptr<const VS_SDPCodec>>		m_ldc;				// local fecc codecs

	std::string						m_password_transcoder;

	net::port						m_listen_port;

	std::string						m_srtp_key, m_ice_ufrag, m_ice_pwd;
	std::string						m_contact_gruu;
public:
	std::shared_ptr<VS_RegCtx>		secureCtx = nullptr;
private:
	std::pair<uint32_t, uint32_t>	m_ssrc_range_audio, m_ssrc_range_video;

	std::vector<std::string>		m_pending_invites;
	vs::set<std::string>			m_pending_req_invites;
	std::vector<std::tuple<std::string, eContentType>> m_pending_messages;	// used as temp storage for message until invite will be answered

	VS_ConferenceInfo m_currConfInfo;

	std::shared_ptr<VS_SIPParserInfo>		m_IsInviteAfterRegister;
	bool m_update_options_branch;

	std::string m_invite_after_options; // branch of options request


	bool m_can_i_choose_codec;
	TimerExtention m_timerExt;

	DieSignalType m_signal_Die;
	RedirectCache m_redirect_cache;
	bool m_need_call_to_redirected;

	bool m_created_by_chat_bot;
	bool m_is_reg_ctx;	// is register ctx on remote server
	bool m_is_tel;

	std::string m_regCtxDialogID;
	const std::string m_server_user_agent;
};

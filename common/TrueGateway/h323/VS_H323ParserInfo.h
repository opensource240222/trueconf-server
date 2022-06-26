#pragma once

#include "../interfaces/VS_ParserInfo.h"
#include "tools/Server/CommonTypes.h"
#include "tools/Server/vs_messageQueue.h"
#include "tools/H323Gateway/Lib/src/VS_H235_Authenticator.h"
#include "tools/H323Gateway/Lib/src/VS_H235SecurityCapability.h"
#include "tools/Server/VS_MediaChannelInfo.h"

#include <boost/signals2/signal.hpp>

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "std-generic/compat/set.h"

struct VS_AsnObjectId;
class VS_ConnectionUDP;
class VS_SignalChannel;

enum class SDPMediaType : int;

#include <chrono>

///////////////////////////////////////////////////////////////////////////////
static constexpr unsigned int H264_CODEC_PARAMETER_PROFILE(41);
static constexpr unsigned int H264_CODEC_PARAMETER_LEVEL(42);
static constexpr unsigned int H264_CODEC_PARAMETER_CUSTOMMBPS = 3;
static constexpr unsigned int H264_CODEC_PARAMETER_CUSTOMFS(4);
static constexpr unsigned int H264_CODEC_PARAMETER_CU(42);
static constexpr bool OPTIONAL_DATA_EXIST(false);
static constexpr std::chrono::milliseconds MSD_WAIT_TIMEOUT(500);
static constexpr std::chrono::milliseconds MSDA_WAIT_TIMEOUT(5000);
static constexpr int MSD_COUNT_LIMIT(10);
///////////////////////////////////////////////////////////////////////////////

enum HangupMode {
	e_none = 0,
	e_my_hangup,
	e_not_my_hangup
};

enum CallDirection {
	e_in = 0,
	e_out
};

struct VS_H245Data
{
	int m_audioSet = 0;
	std::uint32_t m_treshold = 0;
	unsigned int  m_videoMaxBitrate = 0;
	/// Ожидание реквеста на открытие
	unsigned long m_audioWait = 0; //TODO:FIXME(unused variable)
	unsigned long m_videoWait = 0; //TODO:FIXME(unused variable)
	int m_H323ApplicationType = 0; // Type of a terminal
	unsigned int m_reg_H264_level = 0;
	unsigned int m_reg_H264_CustomMaxMBPS = 0;

	///reserved
	char H245Flags[6];
	static const char h245_req_recv = 0x02;
	static const char h245_rsp_send = 0x04;
	static const char h245_req_send = 0x08;
	static const char h245_rsp_recv = 0x10;
	static const char h245_rsp_rejected = 0x20;
	/// notation of bits
	/// 0 - operation has been finished
	/// 1 - other side send message
	/// 2 - i have answered to this^ message
	/// 3 - i have sent message to other side
	/// 4 - other side has answered to my^ message
	///////////////////////////////////////////////////////////////////////////////
	///OLC
	std::uint32_t m_audioNumberLCSender = 0;
	std::uint32_t m_audioNumberLCReciver = 0;
	std::uint32_t m_videoNumberLCSender = 0;
	std::uint32_t m_videoNumberLCReciver = 0;
	std::uint32_t m_slidesNumberLCSender = 0;
	std::uint32_t m_slidesNumberLCReciver = 0;
	std::uint32_t m_dataNumberLCSender = 0;
	std::uint32_t m_dataNumberLCReciver = 0;

	std::uint32_t	m_my_msd_num = 0;
	std::uint32_t	m_my_msd_type = 0;

	std::uint32_t	m_their_msd_num = 0;
	std::uint32_t	m_their_msd_type = 0;

	std::uint32_t	m_msd_type = 0;		// Master or Slave or AutoDetect
	std::uint32_t	m_msd_mode = 0;		// WaitPeer

	std::uint32_t m_SequenceNumber = 0;
};

struct VS_H239TokenInfo
{
	VS_H239TokenInfo();

	bool owned;
	unsigned symmetry_breaking_of_request; // Must be 0 iff our request is still unanswerred
	std::chrono::steady_clock::time_point last_indication_time;
};

enum MSDState {
	MSDIdle = 0,
	MSDOutgoingAwaitingResponse,
	MSDIncomingAwaitingResponse,
	numMSDStates
};

class VS_H323ParserInfo: public VS_ParserInfo
{
	struct InternalLogicalChannel
	{
		VS_H245LogicalChannelInfo	*lc_info;
		VS_ConnectionUDP			*rtp;
		VS_ConnectionUDP			*rtcp;
		/////VS_ConnectionUDP			*rtcpReceiver;
	};

	///////////////////////////////////////////////////////////////////////////////
	///Информация о кодеках. Должна быть настраиваема.
	//Video
	VS_H245VideoCapability* vc[vc_number] = { 0 };
	VS_H245VideoCapability vc_default;
	bool m_vc_default_set;
	VS_H245VideoCapability vsc_default;
	bool m_vsc_default_set;

	int videoCapabilityRcv;
	//Audio
	VS_H245AudioCapability* ac[ac_number] = { 0 };
	unsigned ac_default;
	int audioCapabilityRcv;

	unsigned m_peer_h245_version;
	bool m_h239_capability_present;
	bool m_h239_enabled;
	VS_H239TokenInfo m_presentation_token;
	bool m_slideshow_active;

	// h224
	VS_H245DataApplicationCapability* dc[dc_number] = { 0 };
	VS_H245DataApplicationCapability dc_default;
	bool m_dc_default_set;
	bool m_h224_capability_present;

	bool m_is_recv_audio_ready;
	bool m_is_recv_video_ready;


	/*unsigned long					m_my_cs_address_ip;
	unsigned short					m_my_cs_address_port;
	eConnectionType					m_my_cs_address_conn_type;*/

	net::address m_my_cs_address;
	net::address m_my_external_cs_address;
	net::port m_my_cs_port;

	std::string						m_DialogID;
	// 16 bytes encoded by VS_H323Parser::EncodeDialogID()
	std::string						m_ConferenceID;
	std::string						m_CallIdentifier;

	bool							m_keep_alive_enabled;

	std::string						m_my_display_name;

	// Dialed digit of this server.
	std::string 					m_my_dialed_digit;

	// DialedDigit
	std::string						m_digit_from;
	std::string						m_digit_to;

	// H323-ID
	std::string						m_alias_from;
	std::string						m_alias_to;

	net::address m_peer_cs_address;
	net::port m_peer_cs_port;

	std::string						m_peer_display_name;
	std::shared_ptr<VS_SignalChannel> m_h245_channel;
	VS_TearMessageQueue m_h245_input_queue;

	H323_User_Type					m_user_type;

	std::unordered_map<std::uint32_t, VS_H245LogicalChannelInfo>	 m_LogicalChannelsMap;
	std::unordered_set<std::uint32_t>			m_pending_lcs;

	bool							m_isHangupStarted;
	bool							m_is_in_dialog;

	bool							m_set_modes_done;
	std::chrono::steady_clock::time_point					m_start_tick;
	std::chrono::steady_clock::time_point					m_tcs_wait_tick;
	int								m_msd_counter;
	std::chrono::steady_clock::time_point					m_msd_wait_tick;
	std::chrono::steady_clock::time_point					m_msd_timer_tick; //T106

	MSDState						m_msd_state;

	CallDirection					m_direction;
	bool							m_IsGroupConf = false;
	unsigned int					m_crv;

	//VS_SimpleStr					m_digit_from; //кто звонит
	//unsigned char					m_call_identifier[16];

	VS_H245Data						m_H245Params;

	std::vector<int> m_ac_precedence;

	HangupMode	m_hangup_mode;

	// true: this is call between terminals, registred as TreuConf users in VCS in gatekeeper mode.
	// false: this is simple call to/from h323-treminal.
	bool m_gk_registred_call;

	uint16_t						m_base_LC_number;
	std::string						m_dtmf;

	static unsigned int GetVideoIndex( const int value );
	static unsigned int GetDataIndex( const int value );
	static unsigned int GetCodecIndex( const int value );

	bool CodecInit(string_view enabledCodecs, bool conventionalSirenTCE, bool enableH263plus2);
public:

	static unsigned int GetAudioIndex( const int value );
	static int GetCodecID(const unsigned int index);
	bool InitH245Params(const std::string &enabledCodecs, size_t maxBr, bool ConventionalSirenTCE = false, bool EnableH263plus2 = false);

	VS_MediaChannelInfo audio_channel;
	VS_MediaChannelInfo video_channel;
	VS_MediaChannelInfo slides_channel;
	VS_MediaChannelInfo data_channel;

	bool IsOurGenericChannel(const std::uint32_t lcNumber) const;
	// h235
	VS_H235Authenticator	h235_auth;
	std::unordered_map<unsigned /*channel number*/, VS_H235SecurityCapability> channels_h235caps;
	void SetH235Enabled(const bool v);

	bool UseNAT() const;

	typedef boost::signals2::signal<void(string_view)> DieSignalType;
	boost::signals2::connection ConnectToDie(const DieSignalType::slot_type& slot)
	{
		return m_signal_Die.connect(slot);
	}

	enum MSDMode
	{
		MSDWait = 0,		// Await incomming MSD (don't send request)
		MSDStart,			// We start MSD immediatly
		MSDSkip				// Don't check MSD at all (go to OLC)
	};

	/// Слежение за состоянием обмена. Флаги.
	enum VS_CapabilityExchangeStages
	{
		e_msd=0, ///master-slave
		e_tcs, ///terminal capability
		e_olc_video, ///open logical channels video
		e_olc_audio, ///open logical channels audio
		e_olc_slides, ///open logical channels slides
		e_olc_data,
	};

	VS_H323ParserInfo();
	~VS_H323ParserInfo(void);

	void SetMyCsAddress(const net::address& address, net::port port);
	const net::address& GetMyLocalCsAddress() const;
	const net::address& GetMyCsAddress() const;
	net::port GetMyCsPort() const;
	void SetMyExternalCsAddress(const net::address& address);
	const net::address& GetMyExternalCsAddress() const;

	const std::string &GetDialogID() const;
	void SetDialogID(std::string id);

	// Returns and sets conference id, encoded by VS_H323Parser::EncodeDialogID() method.
	const std::string &GetConferenceID() const;
	void SetConferenceID(std::string confId);

	// Returns and sets CallIdentifier.guid field from h225 packet, encoded by VS_H323Parser::EncodeDialogID() method.
	const std::string &GetCallIdentifier() const;
	void SetCallIdentifier(std::string callId);

	bool IsKeepAliveEnabled() const;
	void EnableKeepAlive();

	void SetPeerCsAddress(const net::address& address, net::port port);
	const net::address& GetPeerCsAddress() const;
	net::port GetPeerCsPort() const;

	const std::shared_ptr<VS_SignalChannel> &GetH245Channel() const;
	void SetH245Channel(std::shared_ptr<VS_SignalChannel> channel);
	VS_TearMessageQueue& GetH245InputQueue();

	bool IsH323UserTypeAutoDetect() const;

	void SetDisplayNameMy(std::string name);
	const std::string &GetDisplayNameMy() const;

	void SetH323UserType(const H323_User_Type user_type);
	H323_User_Type GetH323UserType() const;

	VS_MediaChannelInfo* GetMediaChannel(VS_H245LogicalChannelInfoDataType data_type);

	bool CreateH245LogicalChannel(VS_H245LogicalChannelInfoDataType datType, std::uint32_t forwardLogicalChannelNumber,
		bool isSender);
	bool GetH245LogicalChannel(std::uint32_t forwardLogicalChannelNumber,
		VS_H245LogicalChannelInfo &info) const;
	bool GetH245LogicalChannel(VS_H245LogicalChannelInfoDataType dataType, bool isSender,
		VS_H245LogicalChannelInfo &info) const;
	bool SetH245LogicalChannel(std::uint32_t forwardLogicalChannelNumber,
		const VS_H245LogicalChannelInfo &info);
	bool CloseH245LogicalChannel(std::uint32_t forwardLogicalChannelNumber);
	const std::unordered_set<std::uint32_t> &GetPendingLCList() const;
	bool GetLCPending(std::uint32_t number) const;
	void SetLCPending(std::uint32_t number, bool isPending);

	bool IsHangupStarted() const;
	void SetHangupStarted(bool value);

	CallDirection GetCallDirection() const;
	void SetCallDirection(CallDirection direction);

	bool IsGroupConf() const override;
	void SetGroupConf(bool isGroupConf) override;

	unsigned int GetCRV() const;
	void SetCRV(const unsigned int crv);

	VS_H245Data* GetH245Params();

	VS_H245VideoCapability** GetVideoCapability();
	VS_H245AudioCapability** GetAudioCapability();
	VS_H245DataApplicationCapability** GetDataCapability();

	void SetPeerH245Version(unsigned version);

	unsigned GetPeerH245Version() const;

	void SetH239CapabilityPresent(bool value);

	bool IsH239CapabilityPresent() const;

	void SetH239Enabled(bool value);

	bool IsH239Enabled() const;

	VS_H239TokenInfo* GetH239PresentationToken();

	bool GetSlideshowState() const;

	void SetSlideshowState(bool active);

	bool IsH224CapabilityPresent() const;

	void SetH224CapabilityPresent(bool value);

	void SetRecvAudioReady(bool value);

	void SetRecvVideoReady(bool value);

	bool IsRecvAudioReady() const;

	bool IsRecvVideoReady() const;

	unsigned GetACDefault() const;

	void SetACDefault(unsigned value);

	VS_H245VideoCapability* GetVCDefault();

	void SetVCDefault(VS_H245VideoCapability& cap);

	bool HasVCDefault() const;

	VS_H245VideoCapability* GetVSCDefault();

	void SetVSCDefault(VS_H245VideoCapability& cap);

	bool HasVSCDefault() const;

	VS_H245DataApplicationCapability* GetDCDefault();

	void SetDCDefault(VS_H245DataApplicationCapability& cap);

	bool HasDCDefault() const;

	bool IsReady(VS_CapabilityExchangeStages stage) const;
	bool IsReady() const;
	bool IsInDialog() const;
	void SetInDialog(bool inDialog);

	VS_GatewayVideoMode* GetRecvVideoMode(int codec_type);
	VS_GatewayAudioMode* GetRecvAudioMode(int codec_type);
	VS_GatewayVideoMode* GetRecvSlidesVideoMode(int codec_type);
	VS_GatewayDataMode* GetRecvDataMode(VS_H323DataCodec codec_type);
	void SetHangupMode(HangupMode mode);
	HangupMode GetHangupMode() const;

	void SetDisplayNamePeer(std::string name);
	const std::string& GetDisplayNamePeer() const;

	// Sets or takes dialed digit of this server for registration request.
	void SetMyDialedDigit(std::string digit);
	const std::string &GetMyDialedDigit() const;

	// Sets or takes H323-ID of caller (from)
	void SetSrcAlias(std::string alias);
	const std::string &GetSrcAlias() const;

	// Sets or takes H323-ID of callee (to)
	void SetDstAlias(std::string alias);
	const std::string &GetDstAlias() const;

	// Sets or takes DialedDigit of caller (from)
	void SetSrcDigit(std::string digit);
	const std::string &GetSrcDigit() const;

	// Sets or takes DialedDigit of callee (to)
	void SetDstDigit(std::string digit);
	const std::string &GetDstDigit() const;

	void SetOLCStartTick(std::chrono::steady_clock::time_point tick);
	bool IsOLCTimeout(std::chrono::steady_clock::time_point now) const;

	void SetTCSWaitingTick(std::chrono::steady_clock::time_point tick);
	bool IsTCSSendNeeded(std::chrono::steady_clock::time_point now) const;

	bool GenNewMSDNums();
	void StartWaitingMSD();
	bool IsMSDSendNeeded() const;
	void StopWaitingMSD();

	void StartMSDTimer();
	bool IsMSDTimerExpired() const;
	bool IsMSDTimerStarted() const;
	void StopMSDTimer();

	MSDState MsdState() const;
	void SetMsdState(MSDState state);

	void SetModesDone(bool isDone);
	bool IsSetModesDone() const;

	// If context is marked as "gatekeeper call" that means
	// this is the call between terminals, registred in VSC as TrueConf users.
	// So, VSC is a gatekeeper.
	void MarkAsGatekeeperCall();
	bool IsGatekeeperCall() const;
	void SetH264Level(int level);

	const std::vector<int> &GetAudioCodecPrecedenceList(void) const;

	static int GetACPayloadType(const int codecId);
	static int GetACDynamicPayloadType(const int codecId);
	static bool FillACObjectId(const int codecId, VS_AsnObjectId *oid);

	bool IsMaster() const;

	void SetMSDCounterForTesting(int i);
	void SetBaseLCNumber(uint16_t base = 777); // 777 - the value which was used on older versions of TCS

	void SetDTMF(const std::string &dtmf);
	const std::string &GetDTMF(void) const;
private:
	DieSignalType m_signal_Die;
};

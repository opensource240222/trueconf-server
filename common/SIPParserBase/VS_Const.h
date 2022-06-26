#pragma once

#define BRANCH_MAGIC_NUMBER "z9hG4bK"

const static unsigned DISCARD_PROTOCOL_PORT = 9;

extern const char DEFAULT_ENABLED_CODECS[];

extern const char CONFIGURATION_INVITE[]; // = "Invite";
extern const char CONFIGURATION_UPDATE[]; // = "Update";
extern const char CONFIGURATION_ACK[]; // = "Ack";
extern const char CONFIGURATION_BYE[]; // = "Bye";
extern const char CONFIGURATION_INFO[]; // = "Info";
extern const char CONFIGURATION_CANCEL[]; // = "Cancel";
extern const char CONFIGURATION_OPTIONS[]; // = "Options";
extern const char CONFIGURATION_REGISTER[]; // = "Register";
extern const char CONFIGURATION_NOTIFY[]; // Notify;
extern const char CONFIGURATION_SUBSCRIBE[]; // = "Subscribe";
extern const char CONFIGURATION_PUBLISH[]; // = "Publish";
extern const char CONFIGURATION_MESSAGE[]; // = "Message";
extern const char CONFIGURATION_TRY_LOGIN[];// = "TryLogin";
extern const char CONFIGURATION_SIP[]; // = SIP;

enum eConnectionType : int
{
	CONNECTIONTYPE_INVALID = 0,
	CONNECTIONTYPE_TCP,
	CONNECTIONTYPE_UDP,
	CONNECTIONTYPE_BOTH,
	CONNECTIONTYPE_TLS
};

enum eMessageType
{
	MESSAGE_TYPE_INVALID = -1,
	MESSAGE_TYPE_REQUEST,
	MESSAGE_TYPE_RESPONSE
};

enum eSIPProto
{
	SIPPROTO_INVALID = -1,
	SIPPROTO_SIP20
};

enum eContentType : int
{
	CONTENTTYPE_INVALID = -1,
	CONTENTTYPE_SDP,
	CONTENTTYPE_MEDIACONTROL_XML,
	CONTENTTYPE_PIDF_XML,
	CONTENTTYPE_DTMF_RELAY,
	CONTENTTYPE_BFCP,
	CONTENTTYPE_TEXT_PLAIN,
	CONTENTTYPE_TEXT_RTF,
	CONTENTTYPE_MULTIPART,
	CONTENTTYPE_UNKNOWN,
};

enum eStartLineType : int
{
	TYPE_INVALID = -1,
	TYPE_INVITE,
	TYPE_ACK,
	TYPE_BYE,
	TYPE_CANCEL,
	TYPE_REGISTER,
	TYPE_INFO,
	TYPE_SUBSCRIBE,
	TYPE_NOTIFY,
	TYPE_UPDATE,
	TYPE_OPTIONS,
	TYPE_PUBLISH,
	TYPE_MESSAGE
};

enum eAddrType
{
	SIPURI_INVALID = 0,
	SIPURI_SIP,
	SIPURI_SIPS,
    SIPURI_MAILTO,
	SIPURI_TEL
};

enum eSIP_AAA_QOP : int
{
    SIP_AAA_QOP_INVALID = 0,
	SIP_AAA_QOP_AUTH,
	SIP_AAA_QOP_AUTH_INT
};

enum eSIP_AAA_ALGORITHM : int
{
    SIP_AAA_ALGORITHM_INVALID = 0,
	SIP_AAA_ALGORITHM_MD5,
	SIP_AAA_ALGORITHM_MD5_SESS
};

enum eSIP_AUTH_SCHEME
{
	SIP_AUTHSCHEME_INVALID = 0,
	SIP_AUTHSCHEME_BASIC,
	SIP_AUTHSCHEME_DIGEST,
	SIP_AUTHSCHEME_NTLM,
	SIP_AUTHSCHEME_KERBEROS,
	SIP_AUTHSCHEME_TLS_DSK
};

enum eSIP_Events
{
	SIP_EVENT_INVALID = 0,
	SIP_EVENT_PRESENCE
};

enum eSIP_UserAgents
{
	SIP_USER_AGENT_INVALID = 0,
	SIP_USER_AGENT_VISICRON_GATEWAY,
	SIP_USER_AGENT_POLYCOM,
	SIP_USER_AGENT_SJPHONE,
	SIP_USER_AGENT_RTC				// (Example: Windows Messenger)
};

enum eSIP_ExtensionPack
{
	SIP_EXTENSION_INVALID = 0,
	SIP_EXTENSION_TIMER = 1,
	SIP_EXTENSION_100REL = 2,
	SIP_EXTENSION_REPLACES = 4,
	SIP_EXTENSION_PATH = 8,
	SIP_EXTENSION_GRUU = 16,
};

// SDP
enum eSDPNetType
{
	SDP_NETTYPE_INVALID = -1,
	SDP_NETTYPE_IN
};

enum eSDPProto
{
	SDP_PROTO_INVALID = -1,
	SDP_PROTO_IP4,
	SDP_PROTO_IP6
};

enum eSDPPayloadType
{
	SDP_PT_INVALID = -1,
	SDP_PT_G711U = 0,
	SDP_PT_G711A = 8,
	SDP_PT_GSM = 3,
	SDP_PT_G723 = 4,
	SDP_PT_G722_64k = 9,
	SDP_PT_MPA = 14,
	SDP_PT_G728 = 15,
	SDP_PT_G729A = 18,
	SDP_PT_H261 = 31,
	SDP_PT_H263 = 34,

	SDP_PT_DYNAMIC_H264 = 100,
	SDP_PT_DYNAMIC_H263plus = 101,
	SDP_PT_DYNAMIC_H263plus2 = 102,
	SDP_PT_DYNAMIC_G722124 = 103,
	SDP_PT_DYNAMIC_G722132 = 104,

	SDP_PT_DYNAMIC_SPEEX_8k = 105,
	SDP_PT_DYNAMIC_SPEEX_16k = 106,
	SDP_PT_DYNAMIC_SPEEX_32k = 107,

	SDP_PT_DYNAMIC_ISAC_32k = 108,
	SDP_PT_DYNAMIC_OPUS = 109,
	SDP_PT_DYNAMIC_TEL_EVENT = 110,

	SDP_PT_DYNAMIC_H224 = 111,

	SDP_PT_DYNAMIC_SIREN14_24k = 113,
	SDP_PT_DYNAMIC_SIREN14_32k = 114,
	SDP_PT_DYNAMIC_SIREN14_48k = 115,
	SDP_PT_DYNAMIC_XH264UC = 122,
};

enum class SDPMediaType : int
{
	invalid = -1,
	audio,
	video,
	application,
	application_bfcp,
	application_fecc,
	message,
};

enum eSDP_RTPPROTO
{
	SDP_RTPPROTO_INVALID = 0,
	SDP_RTPPROTO_RTP_AVP,
	SDP_RTPPROTO_RTP_SAVP,
	SDP_RTPPROTO_UDP,
	SDP_PROTO_TCP_BFCP, // for bfcp. see rfc4583 section 11
	SDP_PROTO_TCP_TLS_BFCP,
	SDP_PROTO_UDP_BFCP, // draft-ietf-bfcpbis-rfc4582bis-15
	SDP_PROTO_UDP_TLS_BFCP,
	SDP_PROTO_SIP,
};

enum eSDP_ContentType : int // rfc4796#section-5, included only ones we care about
{
	SDP_CONTENT_INVALID = 0,
	SDP_CONTENT_UNKNOWN,
	SDP_CONTENT_MAIN,
	SDP_CONTENT_SLIDES,
};

enum eSDP_Setup // rfc4145#section-4
{
	SDP_SETUP_INVALID = 0,
	SDP_SETUP_ACTIVE,
	SDP_SETUP_PASSIVE,
	SDP_SETUP_ACTPASS,
	SDP_SETUP_HOLDCONN,
};
enum eSDP_Connection // rfc4145#section-5
{
	SDP_CONNECTION_INVALID = 0,
	SDP_CONNECTION_NEW,
	SDP_CONNECTION_EXISTING
};

enum eSDP_FLOORCTRL_ROLE
{
	SDP_FLOORCTRL_ROLE_INVALID = 0,
	SDP_FLOORCTRL_ROLE_C_ONLY = 0x1,
	SDP_FLOORCTRL_ROLE_S_ONLY = 0x2,
	SDP_FLOORCTRL_ROLE_C_S = 0x4
};

enum eSDP_MediaChannelDirection
{
	SDP_MEDIACHANNELDIRECTION_INVALID = -1,
	SDP_MEDIACHANNELDIRECTION_SENDRECV,
	SDP_MEDIACHANNELDIRECTION_SEND,
	SDP_MEDIACHANNELDIRECTION_RECV,
	SDP_MEDIACHANNELDIRECTION_INACTIVE
};

enum eSDP_Bandwidth
{
	SDP_BANDWIDTH_INVALID = 0,
	SDP_BANDWIDTH_CT,											// Conference Total
	SDP_BANDWIDTH_AS,											// Application Specific
	SDP_BANDWIDTH_TIAS											// Transport Independet Application Specific
};

// STUN
enum eSTUNField
{
	STUN_FIELD_INVALID = 0x0000,
	STUN_FIELD_ADDRESS_MAPPED = 0x0001,
	STUN_FIELD_ADDRESS_RESPONSE = 0x0002,
	STUN_FIELD_CHANGE_REQUEST = 0x0003,
	STUN_FIELD_ADDRESS_SOURCE = 0x0004,
	STUN_FIELD_ADDRESS_CHANGED = 0x0005,
	STUN_FIELD_USERNAME = 0x0006,
	STUN_FIELD_PASSWORD = 0x0007,
	STUN_FIELD_MESSAGE_INTEGRITY = 0x0008,
	STUN_FIELD_ERROR_CODE = 0x0009,
	STUN_FIELD_UNKNOWN_ATTRIBUTES = 0x000a,
	STUN_FIELD_ADDRESS_REFLECTED_FROM = 0x000b,
};

enum eSTUN_ChangeMask
{
	STUN_MASK_CHANGE_NO = 0x00,
	STUN_MASK_CHANGE_PORT = 0x02,
	STUN_MASK_CHANGE_IP = 0x04
};

enum eSTUN_MESSAGE_TYPE
{
	STUN_MESSAGE_TYPE_INVALID = 0x0000,
	STUN_MESSAGE_TYPE_BINDING_REQUEST = 0x0001,
	STUN_MESSAGE_TYPE_BINDING_RESPONSE = 0x0101,
	STUN_MESSAGE_TYPE_BINDING_ERROR_RESPONSE = 0x0111,
	STUN_MESSAGE_TYPE_SHARED_SECRET_REQUEST = 0x0002,
	STUN_MESSAGE_TYPE_SHARED_SECRET_RESPONSE = 0x0102,
    STUN_MESSAGE_TYPE_SHARED_SECRET_ERROR_RESPONSE = 0x0112
};

enum eSTUN_TEST
{
	STUN_TEST_INVALID = -1,
	STUN_TEST_I = 1,
	STUN_TEST_II = 2,
	STUN_TEST_III = 3
};

enum eSTUN_NAT
{
	STUN_NAT_INVALID = -1,
	STUN_NAT_UDP_BLOCKED = 0,
	STUN_NAT_SYMMETRIC_UDP_FIREWALL,
	STUN_NAT_OPEN_INTERNET,
	STUN_NAT_FULL_CONE,
	STUN_NAT_SYMMETRIC,
	STUN_NAT_RESTRICTED_IP,
	STUN_NAT_RESTRICTED_PORT
};

enum class REFRESHER : unsigned int
{
	REFRESHER_INVALID = 0,
	REFRESHER_UAC,
	REFRESHER_UAS
};

enum class TSIPErrorCodes : short
{
	e_null = 0,
	e_ok = 1,
	e_noMemory,
	e_InputParam,
	e_buffer,
	e_EndOfBuffer,
	e_ObjectFactory,
	e_match,
	e_header,
	e_Content,
	e_UnknownPT,
	e_NoSupportedCodecs,
	e_BufferFoundContent,
	e_UnknownContent,
	e_UNKNOWN,
	e_badObjectState = -1,
	e_bad = -2,
};

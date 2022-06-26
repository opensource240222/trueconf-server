#pragma once

#include "../../SIPParserBase/VS_Const.h"
#include "../../std/cpplib/VS_Protocol.h"
#include "std-generic/cpplib/string_view.h"
#include "../H323Gateway/Lib/src/VS_H235SecurityCapability.h"

#include <cstdint>
#include <functional>
#include <vector>
#include <chrono>
#include "std-generic/attributes.h"
#include <boost/asio/ip/udp.hpp>

class VS_Container;

//TODO:FIXME
static const unsigned int	CONFERENCEID_LENGTH						= 16;

static const unsigned int	T35_COUNTRY_CODE_USA					= 0xB5;
static const unsigned int	T35_COUNTRY_CODE_RUSSIA					= 0xB8;
static const unsigned int	T35_MANUFACTURER_CODE_CISCO				= 0x0012;
static const unsigned int	T35_MANUFACTURER_CODE_POLYCOM			= 0x2331;
static const unsigned int	T35_MANUFACTURER_CODE_NETMEETING		= 0x534C;

#ifdef _SVKS_M_BUILD_
static const unsigned char H323Gateway_Product[] = "H.323 Terminal";
static const unsigned char H323Gateway_Version[] = "1.0";
#else
static const unsigned char H323Gateway_Product[] = "TrueConf Gateway";
static const unsigned char H323Gateway_Version[] = "4.3";
#endif

namespace video_presets
{
	enum class mode : int32_t
	{
		sd = 0,
		hq = 1,
		hd,
		fhd,
		max
	};
	constexpr int32_t id(mode x) { return static_cast<int32_t>(x); }

	struct preset
	{
		int32_t width;
		int32_t height;
		int32_t bitrate;
		int32_t nlayers;
	};

	static constexpr preset video_info[id(mode::max)] = {
		{ 360,  180,  200, 1},
		{ 640,  360,  420, 2},
		{1280,  720, 1024, 3},
		{1920, 1080, 2048, 4}
	};
	constexpr int32_t bitrate(mode x) { return video_info[id(x)].bitrate; }
	constexpr int32_t width(mode x) { return video_info[id(x)].width; }
	constexpr int32_t height(mode x) { return video_info[id(x)].height; }
	constexpr int32_t layers(mode x) { return video_info[id(x)].nlayers; }

	static const uint32_t max_bitrate = 4096;
	static const uint32_t max_bitrate_groupconf = 1024;
	static const uint32_t default_gateway_bandwidth = bitrate(mode::fhd);
	static const int32_t max_h264_level = 40;
	static const int32_t max_h264_level_groupconf = 32;
}

enum H323_User_Type
{
	H323UT_NONE,
	H323UT_MSNET,
	H323UT_POLICAMVV,
	H323UT_TANDBERG,
	H323UT_MCU,
	H323UT_CISCO
};
enum VS_ChannelID : int
{
	e_noChannelID = 0,
	e_RAS,					// unicast		LocalIP:1719
	e_RAS_DISCOVERY,		// multicast	224.0.1.41:1718
	e_H225,
	e_H245,
	e_H225_and_H245,
	e_SIP_CS,
	e_SIP_Register,
	e_SIP_Transmitter,		// Channel between REG & CS
	e_BFCP,
	e_RTSP,
};

static const unsigned int c_fakeMPEG4ES = 0x01000000;

enum VS_H323AudioCodec		// MAX NUMBER = (0xFF - 1)
{
	e_rcvNone		 = 0x00,
	e_rcvG729a		 = 0x01,
	e_rcvG711Alaw64k = 0x02,
    e_rcvG711Ulaw64k = 0x04,
	e_rcvG723		 = 0x08,
    e_rcvG728		 = 0x10,
	e_rcvG722_64k	 = 0x20,
	e_rcvSPEEX_8kHz	 = 0x40,		// not used
	e_rcvSPEEX_16kHz = 0x80,
	e_rcvSPEEX_32kHz = 0x100,		// not used
	e_rcvG722124	 = 0x200,
	e_rcvG722132	 = 0x400,
	e_rcvISAC_16kHz	 = 0x800,
	e_rcvSIREN14_24  = 0x1000,
	e_rcvSIREN14_48  = 0x2000,
	e_rcvSIREN14_32  = 0x4000,
	e_rcvOPUS		 = 0x8000,
	e_rcvAAC		 = 0x10000,
	e_rcvMPA		 = 0x20000,
	e_rcvTelEvent	 = 0x40000,
};

enum class VS_H323DataCodec : int {
	dataNone		 = 0x00,
	FECC			 = 0x01,
};

/// !!! Siren 14 supported extended modes !!!
// See ITU's "Low-complexity coding at 24 and 32 kbit/s for
// hands-free operation in systems with low frame
// loss", Annex A (table A.6, p. 23 at the time of writing:
// "Generic Capability parameter table for extended modes of G.722.1,
// describing the supported modes").
//
// http://www.itu.int/rec/T-REC-G.722.1-200505-I/en
// You could combine this modes using logical OR operation.
enum VS_SIREN14_supportedExtendedModes {
	e_SIREN14_24K = 64, // bit 2 - 24kbit/s
	e_SIREN14_32K = 32, // bit 3 - 32kbit/s
	e_SIREN14_48K = 16, // bit 4 - 48kbit/s
};

// Object IDs for generic audio codecs.
static constexpr unsigned int id_G7221[] = { 0, 0, 7, 7221, 1, 0 };
static constexpr unsigned int id_SIREN14[] = { 0, 0, 7, 7221, 1, 1, 0 };

static constexpr unsigned int vc_number = 5;
static constexpr unsigned int ac_number = 15;//5;
static constexpr unsigned int dc_number = 2;

enum VS_H323VideoCodec		// MAX NUMBER = (0xFF - 1)
{
	e_videoNone		 = 0x00,
    e_videoH261      = 0x01,
	e_videoH263      = 0x02,
	e_videoH263plus  = 0x04,
	e_videoH263plus2 = 0x08,
	e_videoH264      = 0x10,

	e_videoVPX		 = 0x20,
	e_videoVPXHD	 = 0x40,

	// ...

	e_videoCyclone	 = 0x80,

	e_videoXH264UC	 = 0x100,
};

struct VS_H323BeginH245Modes
{
	VS_H323BeginH245Modes()
	{
		rcvAudioPayloadType=0;
		rcvVideoPayloadType=0;
		rcvVideoMode=0;
		sndVideoMode=0;
		rcvVideoBitrate=0;
		sndVideoBitrate=0;
		rcvAudioCodecType=0;
		sndAudioCodecType=0;
	    rcvVideoCodecType=0;
	    sndVideoCodecType=0;
	    rcvVideoCodecAnexInfo=0;
	    sndVideoCodecAnexInfo=0;
	}
	int rcvAudioPayloadType;
	int rcvVideoPayloadType;
	int rcvVideoMode; ///Cif, Qcif ...
	int sndVideoMode;			// for H.264 equls level
	int rcvVideoBitrate;
	int sndVideoBitrate;
	int rcvAudioCodecType;
	int sndAudioCodecType;
	int rcvVideoCodecType; /// H.263,H.263+,H.264 ...
	int sndVideoCodecType;
	/////////////////////////////////////////////////
	/// Encoding Rules
	enum VideoAnexes
	{
		e_anexF = 0x01, /// bit 0x01 - Anex F
		e_anexI = 0x02, /// bit 0x02 - Anex I
		e_anexJ = 0x04, /// bit 0x04 - Anex J
		e_anexL = 0x08, /// bit 0x08 - Anex L
		e_anexT = 0x10, /// bit 0x10 - Anex T
		e_anexN = 0x20, /// bit 0x20 - Anex N
		e_anexW = 0x40 /// bit 0x40 - Anex W
	};
	int rcvVideoCodecAnexInfo; /// H.263+(AnexF,I,T)
	int sndVideoCodecAnexInfo;

	static void GetH264ResolutionByLevel(int level, std::uint32_t &width, std::uint32_t &height)
	{
		// 16x9
		if (level >= 31)
		{
			width = 1280;
			height = 720;
		}
		else if (level>=30) {
			width = 768;
			height = 448;
		}else if (level>=22){
			width	= 640;
			height	= 360;
		}else if (level>=21){
			width	= 480;
			height	= 264;
		}else{
			width	= 352;
			height	= 288;
		}
	}

	// assumed that macroblock size is 16 x 16
	static void GetH264ResolutionByMaxFs(size_t maxFs, std::uint32_t &width, std::uint32_t &height)
	{
		if (((width = 1280)/16) * ((height = 720)/16) <= maxFs)
			return;
		if (((width = 1024)/16) * ((height = 576)/16) <= maxFs)
			return;
		if (((width = 864)/16) * ((height = 480)/16) <= maxFs)
			return;
		if (((width = 704)/16) * ((height = 576)/16) <= maxFs)
			return;
		if (((width = 800)/16) * ((height = 448)/16) <= maxFs)
			return;
		if (((width = 768)/16) * ((height = 448)/16) <= maxFs)
			return;
		if (((width = 640)/16) * ((height = 360)/16) <= maxFs)
			return;
		if (((width = 480)/16) * ((height = 264)/16) <= maxFs)
			return;
//320, 180, 16, 9, 8, 2, {}
		/*if (((width = 768)/16) * ((height = 448)/16) <= maxFs)
			return;*/
		width = 352; height = 288;
	}

	static bool CheckH264ParamsAvailabilityByMaxMbps(size_t maxMbps, size_t width, size_t height, size_t fps)
	{
		size_t mbWidth = width / 16;
		if (width % 16)
			width++;
		size_t mbHeight = height / 16;
		if (height % 16)
			height++;

		if (mbWidth * mbHeight * fps <= maxMbps)
			return true;

		return false;
	}

};

struct VS_MPEG4ESConfiguration // RFC3640 4.1
{
	VS_MPEG4ESConfiguration()
		: size_length(0)
		, index_length(0)
		, index_delta_length(0)
		, constant_size(0)
	{
	}
	bool Serialize(VS_Container& cnt, string_view name) const;
	bool Deserialize(const VS_Container& cnt, const char* name);

	friend bool operator==(const VS_MPEG4ESConfiguration& lhs, const VS_MPEG4ESConfiguration& rhs);
	friend bool operator!=(const VS_MPEG4ESConfiguration& lhs, const VS_MPEG4ESConfiguration& rhs);

	uint8_t size_length;
	uint8_t index_length;
	uint8_t index_delta_length;
//	uint8_t cts_delta_length;
//	uint8_t dts_delta_length;
//	uint8_t ras_indication;

	uint16_t constant_size;
//	uint32_t constant_duration;
//	uint32_t max_displacement;
	std::vector<uint8_t> config;
};

struct VS_GatewayMediaMode{
	VS_H235SecurityCapability sec_cap;
	int PayloadType;
	bool InitializeNAT;
	uint32_t ClockRate;

	VS_GatewayMediaMode(int _PayloadType, bool _InitializeNAT) :PayloadType(_PayloadType),InitializeNAT(_InitializeNAT), ClockRate(0){}

	bool Serialize(VS_Container& cnt, string_view name) const;
	bool Deserialize(const VS_Container& cnt, const char* name);

	bool operator==(const VS_GatewayMediaMode& other) const
	{
		return sec_cap == other.sec_cap
			&& PayloadType == other.PayloadType
			&& InitializeNAT == other.InitializeNAT
			&& ClockRate == other.ClockRate;
	}
};

struct VS_GatewayVideoMode : VS_GatewayMediaMode
{
	static const int MAX_NAL_UNIT_SIZE = 1000;

	VS_GatewayVideoMode() : VS_GatewayMediaMode(0,false), Mode(0), Bitrate(0), CodecType(0), CodecAnexInfo(0), MaxFs(0), MaxMbps(0),
		sizeOfSPS(0), sizeOfPPS(0), IsFIRSupported(false), IsMixerCIFMode(false),
		preferred_width(0), preferred_height(0), gconf_to_term_width(0), gconf_to_term_height(0)
	{	}
	bool Serialize(VS_Container& cnt, string_view name) const;
	bool Deserialize(const VS_Container& cnt, const char* name);

	bool operator==(const VS_GatewayVideoMode& other)const
	{
		return VS_GatewayMediaMode::operator==(other)
			&& Mode == other.Mode
			&& Bitrate == other.Bitrate
			&& CodecType == other.CodecType
			&& CodecAnexInfo == other.CodecAnexInfo
			&& MaxFs == other.MaxFs
			&& MaxMbps == other.MaxMbps
			&& sizeOfSPS == other.sizeOfSPS
			&& sizeOfPPS == other.sizeOfPPS
			&& IsFIRSupported == other.IsFIRSupported
			&& IsMixerCIFMode == other.IsMixerCIFMode
			&& preferred_width == other.preferred_width
			&& preferred_height == other.preferred_height
			&& gconf_to_term_width == other.gconf_to_term_width
			&& gconf_to_term_height == other.gconf_to_term_height;
	}

	enum Anexes {
		e_anexF = 0x01, /// bit 0x01 - Anex F
		e_anexI = 0x02, /// bit 0x02 - Anex I
		e_anexJ = 0x04, /// bit 0x04 - Anex J
		e_anexL = 0x08, /// bit 0x08 - Anex L
		e_anexT = 0x10, /// bit 0x10 - Anex T
		e_anexN = 0x20, /// bit 0x20 - Anex N
		e_anexW = 0x40  /// bit 0x40 - Anex W
	};
	int Mode;			///Cif, Qcif ...
	std::uint32_t  Bitrate;
	int CodecType;		/// H.263,H.263+,H.264 ...
	int CodecAnexInfo;	/// H.263+(AnexF,I,T)
	int MaxFs;			/// H.264
	int MaxMbps;		/// H.264

	char SequenceParameterSet[MAX_NAL_UNIT_SIZE]; // Raw H.264 NAL unit
	char PictureParameterSet[MAX_NAL_UNIT_SIZE];  // Raw H.264 NAL unit

	int sizeOfSPS;
	int sizeOfPPS;

	bool IsFIRSupported;

	bool IsMixerCIFMode;		// use only standard CIF, CIF4, etc resolutions
	unsigned long preferred_width;
	unsigned long preferred_height;
	unsigned long gconf_to_term_width;
	unsigned long gconf_to_term_height;
};

struct VS_GatewayAudioMode : VS_GatewayMediaMode
{
	VS_GatewayAudioMode() :VS_GatewayMediaMode(SDP_PT_INVALID, false), SwapBytes(false), CodecType(e_rcvNone)
	{
	}
	bool Serialize(VS_Container& cnt, string_view name) const;
	bool Deserialize(const VS_Container& cnt, const char* name);

	bool operator==(const VS_GatewayAudioMode& other)const
	{
		return VS_GatewayMediaMode::operator==(other)
			&& mpeg4es == other.mpeg4es
			&& SwapBytes == other.SwapBytes
			&& CodecType == other.CodecType;
	}

	VS_MPEG4ESConfiguration mpeg4es;
	bool SwapBytes;
	int CodecType;
};

struct VS_GatewayDataMode : VS_GatewayMediaMode
{
	VS_GatewayDataMode():
		VS_GatewayMediaMode(SDP_PT_INVALID,false)
		, CodecType(VS_H323DataCodec::dataNone)
		, ExtendedCodec(false)
		, BitRate(-1)
	{
	}
	bool Serialize(VS_Container& cnt, string_view name) const;
	bool Deserialize(const VS_Container& cnt, const char* name);

	bool operator==(const VS_GatewayDataMode& other)const
	{
		return VS_GatewayMediaMode::operator==(other)
			&& CodecType == other.CodecType
			&& BitRate == other.BitRate
			&& ExtendedCodec == other.ExtendedCodec;
	}

	VS_H323DataCodec CodecType;
	bool ExtendedCodec;
	unsigned int BitRate;
};

enum VS_CallConfirmCode :int
{
	e_call_none		= 0,
	e_call_ok	= 1,
	e_call_busy	= 2,
	e_call_rejected = 3
};

struct VS_GatewayConferenceStat
{
	int		AudioCodec;		// VS_H323AudioCodec
	int		AudioBitrate;	// kbit
	int		AudioFreq;		// kHz

	int		VideoCodec;		// VS_H323VideoCodec
	int		VideoBitrate;	// kbit
	float	VideoFps;		// fps
	int		VideoWidth;		// pixels
	int		VideoHeight;	// pixels

	int		CurrentBitrate;	// total, kbit
	VS_GatewayConferenceStat() {
		AudioCodec = 0;
		AudioBitrate = 0;
		AudioFreq = 0;
		VideoCodec = 0;
		VideoBitrate = 0;
		VideoFps = 0;
		VideoWidth = 0;
		VideoHeight = 0;
		CurrentBitrate = 0;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// GetAppData protocol
const char H323_APP_DATA_TYPE[] = "H323_APP_DATA_TYPE";
const char H323_APP_DATA_CONTENT[]="H323_APP_DATA_CONTENT";
const char H323_APP_DATA_SOME_LONG_1[] = "H323_APP_DATA_SOME_LONG1";
const char H323_APP_DATA_SOME_LONG_2[] = "H323_APP_DATA_SOME_LONG2";
const char H323_APP_DATA_SOME_LONG_3[] = "H323_APP_DATA_SOME_LONG3";
const char H323_APP_DATA_SOME_LONG_4[] = "H323_APP_DATA_SOME_LONG4";
const char H323_APP_DATA_SOME_LONG_5[] = "H323_APP_DATA_SOME_LONG4";
const char H323_APP_DATA_SOME_LONG_6[] = "H323_APP_DATA_SOME_LONG4";
const char H323_APP_DATA_SOME_LONG_7[] = "H323_APP_DATA_SOME_LONG4";

enum VS_H323AppDataType {
	e_fastUpdatePicture,
	e_fastUpdateGob,
	e_freezePicture,
	e_sendSyncEveryGob,
	e_restrictBitRate,
	e_noRestrictions,
	e_rcvAudioChannelInactivate,
	e_sndAudioChannelInactivate,
	e_rcvAudioChannelActivate,
	e_sndAudioChannelActivate,
	e_rcvVideoChannelInactivate,
	e_sndVideoChannelInactivate,
	e_rcvVideoChannelActivate,
	e_sndVideoChannelActivate,
	e_terminalAccept,
	e_beginModes,
	e_rcvMode
};
///////////////////////////////////////////////////////////////////////////

enum VS_H245LogicalChannelInfoDataType
{
	e_video,
	e_audio,
	e_slides,
	e_data,
};

struct VS_H245LogicalChannelInfo
{
	typedef boost::asio::ip::udp::endpoint udp_endpoint;

	VS_H245LogicalChannelInfo()
		: m_forwardLogicalChannelNumber(0)
		, m_isSender(false)
	{}

	VS_H245LogicalChannelInfo(bool isSender, std::uint32_t number)
		: m_forwardLogicalChannelNumber(number)
		, m_isSender(isSender)
	{}

	udp_endpoint m_our_rtp_address;
	udp_endpoint m_our_rtcp_address;
	udp_endpoint m_remote_rtp_address;
	udp_endpoint m_remote_rtcp_address;
	std::uint32_t m_forwardLogicalChannelNumber;
	std::uint32_t m_sessionID;
	VS_H245LogicalChannelInfoDataType m_dataType;
	bool m_isSender;
};

struct SlideInfo {
	int w = 0, h = 0;
	size_t size;
	std::string img_type, about;
	int slide_n = 0, slide_count = 0;

	bool Serialize(VS_Container& cnt) const;
	bool Deserialize(const VS_Container& cnt);
};

typedef std::tuple<const char* /*fileName*/, const char* /*magnet*/, const char* /*url*/, const char* /*about*/> FileTransferInfo;

namespace vs
{
	const uint32_t default_audio_clock_rate = 8000;
	const uint32_t default_video_clock_rate = 90000;
	const uint32_t default_data_clock_rate = 4800;

	int GetCodecByEncodingName(string_view name, const std::uint32_t clockRate);
	bool GetCodecInfo(SDPMediaType mt, int codec, std::uint32_t &clockRate, eSDPPayloadType &PT, const char*& encodingName);
	bool GetCodecInfoByName(string_view name, SDPMediaType &mt, int &codec,
		uint32_t &clockRate, eSDPPayloadType &PT, const char*& encodingName);

	int GetCodecByStaticPayloadType(const SDPMediaType mediaType, const eSDPPayloadType pt);
	int GetCodecClockRateByCodecType(const SDPMediaType mt, int codec);
}
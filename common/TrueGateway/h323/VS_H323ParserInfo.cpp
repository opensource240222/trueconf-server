#include "VS_H323ParserInfo.h"
#include "VS_H323ExternalGatekeeper.h"
#include "tools/H323Gateway/Lib/VS_H323CapabilityGenerator.h"
#include "std/debuglog/VS_Debug.h"
#include <boost/algorithm/string.hpp>

#include <random>


#define DEBUG_CURRENT_MODULE VS_DM_H323PARSER


namespace
{
	const std::chrono::milliseconds WAIT_OLC_TIMEOUT(2 * 1000);
	const std::chrono::milliseconds TCS_WAIT_TIMEOUT(500);	// milliseconds

	std::mt19937							rand_generator(std::random_device().operator()());
	std::uniform_int_distribution<uint32_t>	rand_msd_distr(0, (1 << 24) - 1);
	std::uniform_int_distribution<uint32_t>	rand_olc_distr(800, 65534); // The number 800 is used to avoid conflicts with older versions of TCS, which use 777 as base number for H.323 logical channels.


	// audio codecs list
	const struct
	{
		const char *name;
		const int id; // internal code ID
		const int pt; // static (!) payload type
		const int dynpt; // dynamic (!) payload type
	}  AUDIO_CODECS[] =
	{
		{ "G722124", e_rcvG722124, SDP_PT_INVALID, SDP_PT_DYNAMIC_G722124 },
		{ "G722132", e_rcvG722132, SDP_PT_INVALID, SDP_PT_DYNAMIC_G722132 },
		{ "SIREN14_24", e_rcvSIREN14_24, SDP_PT_INVALID, SDP_PT_DYNAMIC_SIREN14_24k },
		{ "SIREN14_32", e_rcvSIREN14_32, SDP_PT_INVALID, SDP_PT_DYNAMIC_SIREN14_32k },
		{ "SIREN14_48", e_rcvSIREN14_48, SDP_PT_INVALID, SDP_PT_DYNAMIC_SIREN14_48k },
		{ "G722_64k", e_rcvG722_64k, SDP_PT_G722_64k, SDP_PT_INVALID },
		{ "G728", e_rcvG728, SDP_PT_G728, SDP_PT_INVALID },
		{ "G729a", e_rcvG729a, SDP_PT_G729A, SDP_PT_INVALID },
		{ "G723", e_rcvG723, SDP_PT_G723, SDP_PT_INVALID },
		{ "G711Alaw64k", e_rcvG711Alaw64k, SDP_PT_G711A, SDP_PT_INVALID },
		{ "G711Ulaw64k", e_rcvG711Ulaw64k, SDP_PT_G711U, SDP_PT_INVALID }
	};

	const struct
	{
		const int codec_id;
		const unsigned int *oid;
		const size_t oid_len;
	} AC_OIDS[] =
	{
		{ e_rcvG722124, id_G7221, sizeof(id_G7221) / sizeof(id_G7221[0]) },
		{ e_rcvG722132, id_G7221, sizeof(id_G7221) / sizeof(id_G7221[0]) },
		{ e_rcvSIREN14_24, id_SIREN14, sizeof(id_SIREN14) / sizeof(id_SIREN14[0]) },
		{ e_rcvSIREN14_32, id_SIREN14, sizeof(id_SIREN14) / sizeof(id_SIREN14[0]) },
		{ e_rcvSIREN14_48, id_SIREN14, sizeof(id_SIREN14) / sizeof(id_SIREN14[0]) }
	};

	inline size_t get_ac_precedence_list(string_view codecsList, std::vector<int> &out)
	{
		size_t init_size = 0;
		for (auto it = boost::make_find_iterator(codecsList, boost::algorithm::token_finder([](char x) { return x != ' '; },
			boost::algorithm::token_compress_on)); !it.eof(); ++it, ++init_size)
		{
			for (auto &codec : AUDIO_CODECS)
			{
				if (codec.name == string_view{ it->begin(), static_cast<std::size_t>(std::distance(it->begin(), it->end())) })
				{
					out.push_back(codec.id);
				}
			}
		}
		return out.size() - init_size;
	}

	inline int get_ac_payload_type(const int codecId)
	{
		for (auto &e : AUDIO_CODECS)
		{
			if (codecId == e.id)
			{
				return e.pt;
			}
		}

		return SDP_PT_INVALID;
	}

	inline int get_ac_dynamic_payload_type(const int codecId)
	{
		for (auto &e : AUDIO_CODECS)
		{
			if (codecId == e.id)
			{
				return e.dynpt;
			}
		}

		return SDP_PT_INVALID;
	}

	inline int get_ac_id(string_view codecName)
	{
		for (auto &e : AUDIO_CODECS)
		{
			if (codecName == e.name)
			{
				return e.id;
			}
		}

		return e_rcvNone;
	}

	inline bool fill_ac_oid(const int codecId, VS_AsnObjectId *oid)
	{
		for (auto &e : AC_OIDS)
		{
			if (codecId == e.codec_id)
			{
				memcpy(oid->value, e.oid, e.oid_len * sizeof(e.oid[0]));
				oid->size = e.oid_len;
				oid->filled = true;
				return true;
			}
		}

		oid->size = 0;
		oid->filled = false;

		return false;
	}
}


VS_H239TokenInfo::VS_H239TokenInfo()
	: owned(false)
	, symmetry_breaking_of_request(0)
{
}

VS_H323ParserInfo::VS_H323ParserInfo()
	: m_vc_default_set(false)
	, m_vsc_default_set(false)
	, videoCapabilityRcv(0)
	, ac_default(0)
	, audioCapabilityRcv(0)
	, m_peer_h245_version(16/*last version at the moment*/)
	, m_h239_capability_present(false)
	, m_h239_enabled(true)
	, m_slideshow_active(false)
	, m_dc_default_set(false)
	, m_h224_capability_present(false)
	, m_is_recv_audio_ready(false)
	, m_is_recv_video_ready(false)
	, m_my_cs_port(0)
	, m_keep_alive_enabled(false)
	, m_peer_cs_port(0)
	, m_user_type(H323UT_NONE)
	, m_isHangupStarted(false)
	, m_is_in_dialog(false)
	, m_set_modes_done(false)
	, m_msd_counter(0)
	, m_msd_state(MSDIdle)
	, m_direction(e_out)
	, m_crv(0)
	, m_hangup_mode(e_none)
	, m_gk_registred_call(false)
	, m_base_LC_number(0)
	, audio_channel(0)
	, video_channel(1)
	, slides_channel(2)
	, data_channel(3)
{
	std::fill_n(m_H245Params.H245Flags, sizeof(m_H245Params.H245Flags),0);
	audio_channel.type = SDPMediaType::audio;
	audio_channel.content = SDP_CONTENT_MAIN;
	audio_channel.direction = SDP_MEDIACHANNELDIRECTION_SENDRECV;

	video_channel.type = SDPMediaType::video;
	video_channel.content = SDP_CONTENT_MAIN;
	video_channel.direction = SDP_MEDIACHANNELDIRECTION_SENDRECV;

	slides_channel.type = SDPMediaType::video;
	slides_channel.content = SDP_CONTENT_SLIDES;
	slides_channel.direction = SDP_MEDIACHANNELDIRECTION_SENDRECV;

	data_channel.type = SDPMediaType::application_fecc;
	data_channel.direction = SDP_MEDIACHANNELDIRECTION_SENDRECV;

}

VS_H323ParserInfo::~VS_H323ParserInfo(void)
{
	VS_H323ExternalGatekeeper::Instance().RemoveConferenceID(m_DialogID);
	// Send RAS DRQ to the gatekeeper.
	VS_H323ExternalGatekeeper::Instance().Disengage(m_ConferenceID, m_CallIdentifier, m_direction == e_in);

	m_signal_Die(m_DialogID);

	for (auto p : vc)
		delete p;
	for (auto p : ac)
		delete p;
	for (auto p: dc)
		delete p;
}

void VS_H323ParserInfo::SetMyCsAddress(const net::address& address, net::port port)
{
	m_my_cs_address = address;
	m_my_cs_port = port;
}

const net::address& VS_H323ParserInfo::GetMyLocalCsAddress() const
{
	return m_my_cs_address;
}

net::port VS_H323ParserInfo::GetMyCsPort() const
{
	return m_my_cs_port;
}

bool VS_H323ParserInfo::InitH245Params(const std::string &enabledCodecs, size_t maxBr, bool ConventionalSirenTCE, bool EnableH263plus2)
{
	m_H245Params.m_audioSet = 0;

	//// TODO reading params from registry
	//VS_TerminalAbstractFactory* factory = VS_TerminalAbstractFactory::Instance();
	//if ( !factory || !factory->GetRegistryRoot() )
	//	return false;
	//VS_RegistryKey key(false, factory->GetRegistryRoot());
	//long ltype = 0;

	//if (key.GetValue(&ltype,4,VS_REG_INTEGER_VT,"Treshold")>0)
	//{
	//	m_treshold = ltype;
	//} else
	//{
	m_H245Params.m_treshold = 160;
	//}

	//unsigned int max_out_bw = 0;
	//if ( key.GetValue(&max_out_bw, 4, VS_REG_INTEGER_VT, "Gateway Bandwidth") )
	//{
	//	m_videoMaxBitrate = max_out_bw;
	//} else
	//{
	m_H245Params.m_videoMaxBitrate = maxBr ? maxBr : 1500;
	//}

	//if (key.GetValue(&ltype,4,VS_REG_INTEGER_VT,"Audio Type")>0)
	//{
	//	m_audioSet = ltype;
	//}

	m_H245Params.m_audioWait = 0;
	m_H245Params.m_videoWait = 0;

	m_H245Params.m_H323ApplicationType = GetH323UserType();

	if (m_H245Params.m_H323ApplicationType==H323UT_NONE ||
		m_H245Params.m_H323ApplicationType > H323UT_CISCO)
	{
		///by the default terminal type is POLYCOM
		m_H245Params.m_H323ApplicationType = H323UT_POLICAMVV;
	}

	if (IsGroupConf())
	{
		m_H245Params.m_reg_H264_level = 30;
		m_H245Params.m_reg_H264_CustomMaxMBPS = 216;
	} else 
	{
		m_H245Params.m_reg_H264_level = 42;
		//key.GetValue(&m_reg_H264_level,4,VS_REG_INTEGER_VT,"H.264 Level");
		m_H245Params.m_reg_H264_CustomMaxMBPS = 523;
		//key.GetValue(&m_reg_H264_CustomMaxMBPS,4,VS_REG_INTEGER_VT,"H.264 CustomMaxMBPS");
	}

	if (!CodecInit(enabledCodecs, ConventionalSirenTCE, EnableH263plus2))
		return false;
	m_H245Params.H245Flags[e_tcs] = 0;
	m_H245Params.H245Flags[e_msd] = 0;
	m_H245Params.H245Flags[e_olc_video] = 0;
	m_H245Params.H245Flags[e_olc_audio] = 0;
	m_H245Params.H245Flags[e_olc_slides] = 0;
	m_H245Params.H245Flags[e_olc_data] = 0;

	//long tmp = 777;
	std::uint32_t tmp;
	if (m_base_LC_number > 0)
	{
		tmp = m_base_LC_number;
	}
	else
	{
		tmp = rand_olc_distr(rand_generator);
	}
	//key.GetValue(&tmp,4,VS_REG_INTEGER_VT,"H.245 Audio Logical Channel");
	m_H245Params.m_audioNumberLCSender = tmp; ///Io aaeau.
	m_H245Params.m_audioNumberLCReciver = 0;

	//key.GetValue(&tmp,4,VS_REG_INTEGER_VT,"H.245 Video Logical Channel");
	m_H245Params.m_videoNumberLCSender = ++tmp;///Io aaeau.
	m_H245Params.m_videoNumberLCReciver = 0;

	m_H245Params.m_slidesNumberLCSender = ++tmp;
	m_H245Params.m_slidesNumberLCReciver = 0;

	m_H245Params.m_dataNumberLCSender = ++tmp;
	m_H245Params.m_dataNumberLCReciver = 0;

	//m_H245Params.m_my_msd_num = 0;	// in standart statusDeterminationNumber is a random number in the range 0 ... 2^24 - 1
	m_H245Params.m_my_msd_num = rand_msd_distr(rand_generator);
	assert(m_H245Params.m_my_msd_num < 1 << 24);
	m_H245Params.m_my_msd_type = 50;

	m_H245Params.m_their_msd_num = 0;
	m_H245Params.m_their_msd_type = 0;

	m_H245Params.m_msd_type = 0;		// 0-AutoDetect,1-Master,2-Slave
	//key.GetValue(&m_msd_type,4,VS_REG_INTEGER_VT,"MSD Type");

	m_H245Params.m_msd_mode = 0;		// 0-Wait MSD,1-Start MSD
	//key.GetValue(&m_msd_mode,4,VS_REG_INTEGER_VT,"MSD Mode");

	//m_Logger->TPrintf("%s Start with: MSD Type(%d), MSD Mode (%d)", m_LogIdent, m_msd_type, m_msd_mode);

	m_H245Params.m_SequenceNumber = 0;
	return true;
}

bool VS_H323ParserInfo::IsOurGenericChannel(const std::uint32_t lcNumber) const
{
	if (lcNumber == m_H245Params.m_audioNumberLCSender ||
		lcNumber == m_H245Params.m_videoNumberLCSender ||
		lcNumber == m_H245Params.m_slidesNumberLCSender||
		lcNumber == m_H245Params.m_dataNumberLCSender)
	{
		return true;
	}
	return false;
}

void VS_H323ParserInfo::SetH235Enabled(const bool v)
{
	h235_auth.SetSecurityEnabled(v);
}

bool VS_H323ParserInfo::UseNAT() const
{
	return !m_my_external_cs_address.is_unspecified();
}

bool VS_H323ParserInfo::CodecInit(string_view enabledCodecs, bool conventionalSirenTCE, bool enableH263plus2)
{
	const VS_AsnObjectId oid_G7221(id_G7221, sizeof(id_G7221) / sizeof(id_G7221[0]));
	const VS_AsnObjectId oid_SIREN14(id_SIREN14, sizeof(id_SIREN14) / sizeof(id_SIREN14[0]));
	videoCapabilityRcv=0;
	audioCapabilityRcv=0;

	ac_default= ac_number-1;

	// get codecs precedence list
	get_ac_precedence_list(enabledCodecs, m_ac_precedence);
	//		vc_default= vc_number-1;
	/////////////////////////////////

	for (auto& p: vc)
		p = new VS_H245VideoCapability;
	for (auto& p: ac)
		p = new VS_H245AudioCapability;
	for (auto& p : dc)
		p = new VS_H245DataApplicationCapability;
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//		int indexer = 0;
	//

	///////////////
	///G722.1.24
	if (enabledCodecs.find("G722124") != string_view::npos)
	{
		const unsigned int g7221_24_number = GetAudioIndex(e_rcvG722124); //(++indexer);
		if (g7221_24_number < ac_number)
		{
			dprint1("\n\t g7221_24_number = %d", g7221_24_number);
			bool isFlagOk = true;

			const auto max_bit_rate = 24000;
			VS_H245GenericCapability * result = 0;
			VS_H323_GenericCapabilityGenerator gen(oid_G7221);
			if (!gen.SetMaxBitRate(max_bit_rate))
			{
				dprint0("\n\t G.722.1 is not set properly - 2.");
				isFlagOk = false;
			}
			else
			{
				if (!gen.AddStandartIntParametr(true,
					VS_H245ParameterValue::e_unsignedMin, 1))
				{
					dprint0("\n\t G.722.1 is not set properly - 2.");
					isFlagOk = false;
				}
				else  result = gen.Generate();
			}
			if (isFlagOk && result)
			{

				ac[g7221_24_number]->choice = result;
				ac[g7221_24_number]->tag = VS_H245AudioCapability::e_genericAudioCapability;
				ac[g7221_24_number]->filled = true;
				//m_Logger->Printf("\n\t ---------------------------------------");
				//m_Logger->Printf("\n\t ---------My generic Capability---------");
				//m_Logger->Printf("\n\t ---------------------------------------");
				//m_Logger->Printf("\n\t HERE WAS CAPABILITY PARAMETERS!!!!     ");
				//m_Logger->Printf("\n\t ---------------------------------------");

				VS_GatewayAudioMode m;
				m.CodecType = e_rcvG722124;
				m.PayloadType = SDP_PT_DYNAMIC_G722124;
				m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG722124);
				audio_channel.rcv_modes_audio.push_back(m);
			}
		}
	}
	///////////////
	///G722.1.32
	if (enabledCodecs.find("G722132") != string_view::npos)
	{
		const unsigned int g7221_32_number = GetAudioIndex(e_rcvG722132); //(++indexer);
		if (g7221_32_number < ac_number)
		{
			dprint1("\n\t g7221_32_number = %d", g7221_32_number);
			bool isFlagOk = true;

			unsigned int maxBitRate = 32000;
			VS_H245GenericCapability * result = 0;
			VS_H323_GenericCapabilityGenerator gen(oid_G7221);
			if (!gen.SetMaxBitRate(maxBitRate))
			{
				dprint0("\n\t G.722.1 is not set properly - 2.");
				isFlagOk = false;
			}
			else
			{
				if (!gen.AddStandartIntParametr(true,
					VS_H245ParameterValue::e_unsignedMin, 1))
				{
					dprint0("\n\t G.722.1 is not set properly - 2.");
					isFlagOk = false;
				}
				else  result = gen.Generate();
			}
			if (isFlagOk && result)
			{

				ac[g7221_32_number]->choice = result;
				ac[g7221_32_number]->tag = VS_H245AudioCapability::e_genericAudioCapability;
				ac[g7221_32_number]->filled = true;
				//m_Logger->Printf("\n\t ---------------------------------------");
				//m_Logger->Printf("\n\t ---------My generic Capability---------");
				//m_Logger->Printf("\n\t ---------------------------------------");
				//m_Logger->Printf("\n\t   HERE HAS BEEN PARAMETERS!!!          ");

				//m_Logger->Printf("\n\t ---------------------------------------");

				VS_GatewayAudioMode m;
				m.CodecType = e_rcvG722132;
				m.PayloadType = SDP_PT_DYNAMIC_G722132;
				m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG722132);
				audio_channel.rcv_modes_audio.push_back(m);
			}
		}
	}

	if (conventionalSirenTCE)
	{
		///////////////
		/// SIREN14_24
		if (enabledCodecs.find("SIREN14_24") != string_view::npos)
		{
			const unsigned int siren14_24_number = GetAudioIndex(e_rcvSIREN14_24);
			if (siren14_24_number < ac_number)
			{
				dprint1("\n\t siren14_24_number = %d", siren14_24_number);

				bool isOk = true;
				VS_H245GenericCapability *result = 0;

				const auto max_bit_rate = 240;
				VS_H323_GenericCapabilityGenerator gen(oid_SIREN14);
				if (!gen.SetMaxBitRate(max_bit_rate))
				{
					dprint0("\n\t SIREN14_24 is not set properly - max.");
					isOk = false;
				}
				else
				{
					if (!gen.AddStandartIntParametr(true,
						VS_H245ParameterValue::e_unsignedMin, 1))
					{
						dprint0("\n\t SIREN14_24 is not set properly - min.");
						isOk = false;
					}
					if (!gen.AddStandartIntParametr(true,
						VS_H245ParameterValue::e_booleanArray, e_SIREN14_24K))
					{
						dprint0("\n\t SIREN14_24 is not set properly - min.");
						isOk = false;
					}
					else  result = gen.Generate();
				}

				if (isOk && result)
				{
					ac[siren14_24_number]->choice = result;
					ac[siren14_24_number]->tag = VS_H245AudioCapability::e_genericAudioCapability;
					ac[siren14_24_number]->filled = true;

					VS_GatewayAudioMode m;
					m.CodecType = e_rcvSIREN14_24;
					m.PayloadType = SDP_PT_DYNAMIC_SIREN14_24k;
					m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvSIREN14_24);
					audio_channel.rcv_modes_audio.push_back(m);
				}
			}
		}

		///////////////
		/// SIREN14_32
		if (enabledCodecs.find("SIREN14_32") != std::string::npos)
		{
			const unsigned int siren14_32_number = GetAudioIndex(e_rcvSIREN14_32);
			if (siren14_32_number < ac_number)
			{
				dprint1("\n\t siren14_32_number = %d", siren14_32_number);

				bool isOk = true;
				VS_H245GenericCapability *result = 0;

				unsigned int maxBitRate = 320;
				VS_H323_GenericCapabilityGenerator gen(oid_SIREN14);
				if (!gen.SetMaxBitRate(maxBitRate))
				{
					dprint0("\n\t SIREN14_32 is not set properly - max.");
					isOk = false;
				}
				else
				{
					if (!gen.AddStandartIntParametr(true,
						VS_H245ParameterValue::e_unsignedMin, 1))
					{
						dprint0("\n\t SIREN14_32 is not set properly - min.");
						isOk = false;
					}
					if (!gen.AddStandartIntParametr(true,
						VS_H245ParameterValue::e_booleanArray, e_SIREN14_32K))
					{
						dprint0("\n\t SIREN14_32 is not set properly - min.");
						isOk = false;
					}
					else  result = gen.Generate();
				}

				if (isOk && result)
				{
					ac[siren14_32_number]->choice = result;
					ac[siren14_32_number]->tag = VS_H245AudioCapability::e_genericAudioCapability;
					ac[siren14_32_number]->filled = true;

					VS_GatewayAudioMode m;
					m.CodecType = e_rcvSIREN14_32;
					m.PayloadType = SDP_PT_DYNAMIC_SIREN14_32k;
					m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvSIREN14_32);
					audio_channel.rcv_modes_audio.push_back(m);
				}
			}
		}

		///////////////
		/// SIREN14_48
		if (enabledCodecs.find("SIREN14_48") != std::string::npos)
		{
			const unsigned int siren14_48_number = GetAudioIndex(e_rcvSIREN14_48);
			if (siren14_48_number < ac_number)
			{
				dprint1("\n\t siren14_48_number = %d", siren14_48_number);

				bool isOk = true;
				VS_H245GenericCapability *result = 0;

				unsigned int maxBitRate = 480;
				VS_H323_GenericCapabilityGenerator gen(oid_SIREN14);
				if (!gen.SetMaxBitRate(maxBitRate))
				{
					dprint0("\n\t SIREN14_48 is not set properly - max.");
					isOk = false;
				}
				else
				{
					if (!gen.AddStandartIntParametr(true,
						VS_H245ParameterValue::e_unsignedMin, 1))
					{
						dprint0("\n\t SIREN14_48 is not set properly - min.");
						isOk = false;
					}
					if (!gen.AddStandartIntParametr(true,
						VS_H245ParameterValue::e_booleanArray, e_SIREN14_48K))
					{
						dprint0("\n\t SIREN14_48 is not set properly - min.");
						isOk = false;
					}
					else  result = gen.Generate();
				}

				if (isOk && result)
				{
					ac[siren14_48_number]->choice = result;
					ac[siren14_48_number]->tag = VS_H245AudioCapability::e_genericAudioCapability;
					ac[siren14_48_number]->filled = true;

					VS_GatewayAudioMode m;
					m.CodecType = e_rcvSIREN14_48;
					m.PayloadType = SDP_PT_DYNAMIC_SIREN14_48k;
					m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvSIREN14_48);
					audio_channel.rcv_modes_audio.push_back(m);
				}
			}
		}
	}
	else if
	   (enabledCodecs.find("SIREN14_24") != std::string::npos ||
		enabledCodecs.find("SIREN14_32") != std::string::npos ||
		enabledCodecs.find("SIREN14_48") != std::string::npos)
	{
		const char *codec_names[] = { "SIREN14_24", "SIREN14_32", "SIREN14_48"};
		int bitfield = 0;
		unsigned int max_bitrate = 0;
		int id = e_rcvNone;
		bool ok = false;
		int codec_index = 0;

		for (auto &c : codec_names)
		{
			if (enabledCodecs.find(c) == std::string::npos)
				continue;

			id = get_ac_id(c);
			codec_index = GetAudioIndex(id);
			switch (id)
			{
			case e_rcvSIREN14_24:
				max_bitrate = 240;
				bitfield |= e_SIREN14_24K;
				break;
			case e_rcvSIREN14_32:
				max_bitrate = 320;
				bitfield |= e_SIREN14_32K;
				break;
			case e_rcvSIREN14_48:
				max_bitrate = 480;
				bitfield |= e_SIREN14_48K;
				break;
			default:
				continue;
				break;
			}

			// Fill Gateway Mode
			{
				VS_GatewayAudioMode m;
				m.CodecType = id;
				m.PayloadType = get_ac_dynamic_payload_type(id);
				m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, id);
				audio_channel.rcv_modes_audio.push_back(m);
			}
			ok = true;
		}

		// TerminalCapabilityEntry Generation
		if (ok)
		{
			bool isOk = true;
			VS_H323_GenericCapabilityGenerator gen(oid_SIREN14);
			VS_H245GenericCapability *result = NULL;

			if (!gen.SetMaxBitRate(max_bitrate))
			{
				dprint0("\n\t SIREN14 is not set properly - max.");
				isOk = false;
			}
			else
			{
				if (!gen.AddStandartIntParametr(true,
					VS_H245ParameterValue::e_unsignedMin, 1))
				{
					dprint0("\n\t SIREN14 is not set properly - min.");
					isOk = false;
				}
				if (!gen.AddStandartIntParametr(true,
					VS_H245ParameterValue::e_booleanArray, bitfield))
				{
					dprint0("\n\t SIREN14 is not set properly - min.");
					isOk = false;
				}
				else  result = gen.Generate();
			}

			if (isOk && result)
			{
				ac[codec_index]->choice = result;
				ac[codec_index]->tag = VS_H245AudioCapability::e_genericAudioCapability;
				ac[codec_index]->filled = true;
			}
		}
	}

	///////////////
	///	g722-64k
	if (enabledCodecs.find("G722_64k") != std::string::npos)
	{
		const unsigned int g722_64k_number = GetAudioIndex(e_rcvG722_64k); //(++indexer);

		if (g722_64k_number < ac_number)
		{
			dprint1("\n\t g722_64k_number = %d", g722_64k_number);
			TemplInteger<1, 256> * g722_64k = new TemplInteger < 1, 256 > ;
			g722_64k->value = 20;///Kostya recomended value
			g722_64k->filled = true;
			///
			ac[g722_64k_number]->choice = g722_64k;
			ac[g722_64k_number]->tag = VS_H245AudioCapability::e_g722_64k;
			ac[g722_64k_number]->filled = true;
			g722_64k = 0;

			VS_GatewayAudioMode m;
			m.CodecType = e_rcvG722_64k;
			m.PayloadType = SDP_PT_G722_64k;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG722_64k);
			audio_channel.rcv_modes_audio.push_back(m);
		}
	}

	///////////////
	///G728
	if (enabledCodecs.find("G728") != std::string::npos)
	{
		const unsigned int g728_number = GetAudioIndex(e_rcvG728); //(++indexer);

		if (g728_number < ac_number)
		{
			dprint1("\n\t g728_number = %d", g728_number);
			TemplInteger<1, 256> * g728 = new TemplInteger < 1, 256 > ;
			g728->value = 20;///Kostya recomended value
			g728->filled = true;
			///
			ac[g728_number]->choice = g728;
			ac[g728_number]->tag = VS_H245AudioCapability::e_g728;
			ac[g728_number]->filled = true;
			g728 = 0;

			VS_GatewayAudioMode m;
			m.CodecType = e_rcvG728;
			m.PayloadType = SDP_PT_G728;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG728);
			audio_channel.rcv_modes_audio.push_back(m);
		}
	}

	//
	//////////////
	///G729a
	if (enabledCodecs.find("G729a") != std::string::npos)
	{
		const unsigned int g729a_number = GetAudioIndex(e_rcvG729a);//(++indexer);
		if (g729a_number < ac_number)
		{
			dprint1("\n\t g729a_number = %d", g729a_number);
			TemplInteger<1, 256> * g729a = new TemplInteger < 1, 256 > ;

			g729a->value = 20;///Kostya recomended value
			g729a->filled = true;
			///
			ac[g729a_number]->choice = g729a;
			ac[g729a_number]->tag = VS_H245AudioCapability::e_g729AnnexA;
			ac[g729a_number]->filled = true;
			g729a = 0;

			VS_GatewayAudioMode m;
			m.CodecType = e_rcvG729a;
			m.PayloadType = SDP_PT_G729A;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG729a);
			audio_channel.rcv_modes_audio.push_back(m);
		}
	}

	// Audio
	///G7231
	if (enabledCodecs.find("G723") != std::string::npos)
	{
		const unsigned int g723_number = GetAudioIndex(e_rcvG723);//(++indexer);
		if (g723_number < ac_number)
		{
			dprint1("\n\t g723_number = %d", g723_number);
			VS_H245AudioCapability_G7231 * g7231 = new VS_H245AudioCapability_G7231;
			//
			g7231->silenceSuppression.value = false;
			g7231->silenceSuppression.filled = true;
			//
			g7231->maxAl_sduAudioFrames.value = 1;//12;///was 12
			g7231->maxAl_sduAudioFrames.filled = true;
			//
			g7231->filled = true;
			//
			ac[g723_number]->tag = VS_H245AudioCapability::e_g7231;
			ac[g723_number]->filled = true;
			ac[g723_number]->choice = g7231;
			g7231 = 0;

			VS_GatewayAudioMode m;
			m.CodecType = e_rcvG723;
			m.PayloadType = SDP_PT_G723;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG723);
			audio_channel.rcv_modes_audio.push_back(m);
		}
	}
	////////////////
	///G711Alaw64k
	if (enabledCodecs.find("G711Alaw64k") != std::string::npos)
	{
		const unsigned int g711a_number = GetAudioIndex(e_rcvG711Alaw64k);//(++indexer);
		if (g711a_number < ac_number)
		{
			dprint1("\n\t g711a_number = %d", g711a_number);
			TemplInteger<1, 256> * g711Alaw64k = new TemplInteger < 1, 256 > ;
			g711Alaw64k->value = 20;///Kostya recomended value
			g711Alaw64k->filled = true;
			//
			ac[g711a_number]->choice = g711Alaw64k;
			ac[g711a_number]->tag = VS_H245AudioCapability::e_g711Alaw64k;
			ac[g711a_number]->filled = true;
			g711Alaw64k = 0;

			VS_GatewayAudioMode m;
			m.CodecType = e_rcvG711Alaw64k;
			m.PayloadType = SDP_PT_G711A;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG711Alaw64k);
			audio_channel.rcv_modes_audio.push_back(m);
		}
	}
	////////////////
	///G711Ulaw64k
	if (enabledCodecs.find("G711Ulaw64k") != std::string::npos)
	{
		const unsigned int g711u_number = GetAudioIndex(e_rcvG711Ulaw64k);//(++indexer);
		if (g711u_number < ac_number)
		{
			dprint1("\n\t g711u_number = %d", g711u_number);
			TemplInteger<1, 256> * g711Ulaw64k = new TemplInteger < 1, 256 > ;
			g711Ulaw64k->value = 20;///Kostya recomended value
			g711Ulaw64k->filled = true;

			ac[g711u_number]->choice = g711Ulaw64k;
			ac[g711u_number]->tag = VS_H245AudioCapability::e_g711Ulaw64k;
			ac[g711u_number]->filled = true;
			g711Ulaw64k = 0;

			VS_GatewayAudioMode m;
			m.CodecType = e_rcvG711Ulaw64k;
			m.PayloadType = SDP_PT_G711U;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG711Ulaw64k);
			audio_channel.rcv_modes_audio.push_back(m);
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Video
	////////////////////////
	///H.263++
	if (enableH263plus2 && enabledCodecs.find("H263plus2") != string_view::npos)
	{
		const unsigned int h263pp_number = GetVideoIndex(e_videoH263plus2);
		if (h263pp_number < vc_number)
		{
			dprint1("\n\t h263pp_number = %d", h263pp_number);
			VS_H245H263VideoCapability* h263pp = new VS_H245H263VideoCapability;

			h263pp->advancedPrediction.value = true;///Anex F
			h263pp->arithmeticCoding.value = false;
			h263pp->bppMaxKb.value = 0;
			h263pp->cif16MPI.value = 0;
			h263pp->cif4MPI.value = 2;
			h263pp->cifMPI.value = 1;//(i==0)?1:0;
			h263pp->errorCompensation.value = false;
			h263pp->hrd_B.value = 0;
			h263pp->maxBitRate.value = m_H245Params.m_videoMaxBitrate * 10;
			h263pp->pbFrames.value = false;
			h263pp->qcifMPI.value = 1;///(i==1)?1:0;;
			h263pp->slowCif16MPI.value = 0;
			h263pp->slowCif4MPI.value = 0;
			h263pp->slowCifMPI.value = 0;
			h263pp->slowQcifMPI.value = 0;
			h263pp->slowSqcifMPI.value = 0;
			h263pp->sqcifMPI.value = 0;//1;//(i==2)?1:0;;
			h263pp->temporalSpatialTradeOffCapability.value = false;
			h263pp->unrestrictedVector.value = false;
			//
			h263pp->advancedPrediction.filled = true;
			h263pp->arithmeticCoding.filled = true;
			h263pp->bppMaxKb.filled = true;
			h263pp->cif16MPI.filled = false;//was true
			h263pp->cif4MPI.filled = true;
			h263pp->cifMPI.filled = true;
			h263pp->errorCompensation.filled = true;
			h263pp->hrd_B.filled = true;
			h263pp->maxBitRate.filled = true;
			h263pp->pbFrames.filled = true;
			h263pp->qcifMPI.filled = true;
			h263pp->slowCif16MPI.filled = false;//true;
			h263pp->slowCif4MPI.filled = false;//true;
			h263pp->slowCifMPI.filled = false;//true;
			h263pp->slowQcifMPI.filled = false;//true;
			h263pp->slowSqcifMPI.filled = false;//true;
			h263pp->sqcifMPI.filled = false;//true;
			h263pp->temporalSpatialTradeOffCapability.filled = true;
			h263pp->unrestrictedVector.filled = true;
			h263pp->enhancementLayerInfo.filled = false;
			//////////////////////////
			///
			h263pp->h263Options.advancedIntraCodingMode.filled = true;
			h263pp->h263Options.advancedIntraCodingMode.value = false; ///Anex I

			h263pp->h263Options.deblockingFilterMode.filled = true;
			h263pp->h263Options.deblockingFilterMode.value = false;///Anex J

			h263pp->h263Options.improvedPBFramesMode.filled = true;

			h263pp->h263Options.unlimitedMotionVectors.filled = true;

			////////Anex L begin//////////
			h263pp->h263Options.fullPictureFreeze.filled = true;
			h263pp->h263Options.partialPictureFreezeAndRelease.filled = true;
			h263pp->h263Options.resizingPartPicFreezeAndRelease.filled = true;
			h263pp->h263Options.fullPictureSnapshot.filled = true;
			h263pp->h263Options.partialPictureSnapshot.filled = true;
			h263pp->h263Options.videoSegmentTagging.filled = true;
			h263pp->h263Options.progressiveRefinement.filled = true;
			////////Anex L end//////////
			h263pp->h263Options.dynamicPictureResizingByFour.filled = true;
			h263pp->h263Options.dynamicPictureResizingSixteenthPel.filled = true;
			h263pp->h263Options.dynamicWarpingHalfPel.filled = true;
			h263pp->h263Options.dynamicWarpingSixteenthPel.filled = true;

			h263pp->h263Options.independentSegmentDecoding.filled = true;

			h263pp->h263Options.slicesInOrder_NonRect.filled = true;
			h263pp->h263Options.slicesInOrder_Rect.filled = true;
			h263pp->h263Options.slicesNoOrder_NonRect.filled = true;
			h263pp->h263Options.slicesNoOrder_Rect.filled = true;

			h263pp->h263Options.alternateInterVLCMode.filled = true;

			h263pp->h263Options.modifiedQuantizationMode.filled = true;
			h263pp->h263Options.modifiedQuantizationMode.value = true; ///Anex T

			h263pp->h263Options.reducedResolutionUpdate.filled = true;

			h263pp->h263Options.transparencyParameters.filled = false;//o
			h263pp->h263Options.separateVideoBackChannel.filled = true;//!0 non optional
			h263pp->h263Options.refPictureSelection.filled = false;//o
			h263pp->h263Options.customPictureClockFrequency.filled = false;//o
			h263pp->h263Options.customPictureFormat.filled = false;//o
			h263pp->h263Options.modeCombos.filled = false;//o
			///
			//////////////////////////
			h263pp->h263Options.filled = true;
			//////////////////////////
			// H.263++ options
			h263pp->h263Options.h263Version3Options.filled = true;
			h263pp->h263Options.h263Version3Options.dataPartitionedSlices.value = false;
			h263pp->h263Options.h263Version3Options.dataPartitionedSlices.filled = true;
			h263pp->h263Options.h263Version3Options.fixedPointIDCT0.value = false;
			h263pp->h263Options.h263Version3Options.fixedPointIDCT0.filled = true;
			h263pp->h263Options.h263Version3Options.interlacedFields.value = false;
			h263pp->h263Options.h263Version3Options.interlacedFields.filled = true;
			h263pp->h263Options.h263Version3Options.currentPictureHeaderRepetition.value = false;
			h263pp->h263Options.h263Version3Options.currentPictureHeaderRepetition.filled = true;
			h263pp->h263Options.h263Version3Options.previousPictureHeaderRepetition.value = false;
			h263pp->h263Options.h263Version3Options.previousPictureHeaderRepetition.filled = true;
			h263pp->h263Options.h263Version3Options.nextPictureHeaderRepetition.value = false;
			h263pp->h263Options.h263Version3Options.nextPictureHeaderRepetition.filled = true;
			h263pp->h263Options.h263Version3Options.pictureNumber.value = false;
			h263pp->h263Options.h263Version3Options.pictureNumber.filled = true;
			h263pp->h263Options.h263Version3Options.spareReferencePictures.value = false;
			h263pp->h263Options.h263Version3Options.spareReferencePictures.filled = true;
			h263pp->h263Options.videoBadMBsCap.filled = false;
			//
			h263pp->filled = true;
			//
			vc[h263pp_number]->choice = h263pp;
			vc[h263pp_number]->tag = VS_H245VideoCapability::e_h263VideoCapability;
			vc[h263pp_number]->filled = true;

			VS_GatewayVideoMode m;
			m.CodecType = e_videoH263plus2;
			m.PayloadType = SDP_PT_INVALID;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::video, e_videoH263plus2);
			m.Mode = /*(h263pp->sqcifMPI.value!=0)	* 0x01|*/
				(h263pp->qcifMPI.value != 0) * 0x02 |
				(h263pp->cifMPI.value != 0) * 0x04 |
				(h263pp->cif4MPI.value != 0) * 0x08 |
				(h263pp->cif16MPI.value != 0) * 0x10;
			m.Bitrate = h263pp->maxBitRate.value * 100;
			m.CodecAnexInfo = 0;
			video_channel.rcv_modes_video.push_back(m);
			slides_channel.rcv_modes_video.push_back(m);

			//
			h263pp = 0;
			//
		}
	}
	////////////////////////

	////////////////////////
	///H.263+
	if (enabledCodecs.find("H263plus") != std::string::npos)
	{
		const unsigned int h263p_number = GetVideoIndex(e_videoH263plus);
		if (h263p_number < vc_number)
		{
			dprint1("\n\t h263p_number = %d", h263p_number);
			VS_H245H263VideoCapability* h263p = new VS_H245H263VideoCapability;

			h263p->advancedPrediction.value = true;///Anex F
			h263p->arithmeticCoding.value = false;
			h263p->bppMaxKb.value = 0;
			h263p->cif16MPI.value = 0;
			h263p->cif4MPI.value = 2;
			h263p->cifMPI.value = 1;//(i==0)?1:0;
			h263p->errorCompensation.value = false;
			h263p->hrd_B.value = 0;
			h263p->maxBitRate.value = m_H245Params.m_videoMaxBitrate * 10;
			h263p->pbFrames.value = false;
			h263p->qcifMPI.value = 1;///(i==1)?1:0;;
			h263p->slowCif16MPI.value = 0;
			h263p->slowCif4MPI.value = 0;
			h263p->slowCifMPI.value = 0;
			h263p->slowQcifMPI.value = 0;
			h263p->slowSqcifMPI.value = 0;
			h263p->sqcifMPI.value = 0;//1;//(i==2)?1:0;;
			h263p->temporalSpatialTradeOffCapability.value = false;
			h263p->unrestrictedVector.value = false;
			//
			h263p->advancedPrediction.filled = true;
			h263p->arithmeticCoding.filled = true;
			h263p->bppMaxKb.filled = true;
			h263p->cif16MPI.filled = false;//was true
			h263p->cif4MPI.filled = true;
			h263p->cifMPI.filled = true;
			h263p->errorCompensation.filled = true;
			h263p->hrd_B.filled = true;
			h263p->maxBitRate.filled = true;
			h263p->pbFrames.filled = true;
			h263p->qcifMPI.filled = true;
			h263p->slowCif16MPI.filled = false;//true;
			h263p->slowCif4MPI.filled = false;//true;
			h263p->slowCifMPI.filled = false;//true;
			h263p->slowQcifMPI.filled = false;//true;
			h263p->slowSqcifMPI.filled = false;//true;
			h263p->sqcifMPI.filled = false;//true;
			h263p->temporalSpatialTradeOffCapability.filled = true;
			h263p->unrestrictedVector.filled = true;
			h263p->enhancementLayerInfo.filled = false;
			//////////////////////////
			///
			h263p->h263Options.advancedIntraCodingMode.filled = true;
			h263p->h263Options.advancedIntraCodingMode.value = false; ///Anex I

			h263p->h263Options.deblockingFilterMode.filled = true;
			h263p->h263Options.deblockingFilterMode.value = false;///Anex J

			h263p->h263Options.improvedPBFramesMode.filled = true;

			h263p->h263Options.unlimitedMotionVectors.filled = true;

			////////Anex L begin//////////
			h263p->h263Options.fullPictureFreeze.filled = true;
			h263p->h263Options.partialPictureFreezeAndRelease.filled = true;
			h263p->h263Options.resizingPartPicFreezeAndRelease.filled = true;
			h263p->h263Options.fullPictureSnapshot.filled = true;
			h263p->h263Options.partialPictureSnapshot.filled = true;
			h263p->h263Options.videoSegmentTagging.filled = true;
			h263p->h263Options.progressiveRefinement.filled = true;
			////////Anex L end//////////
			h263p->h263Options.dynamicPictureResizingByFour.filled = true;
			h263p->h263Options.dynamicPictureResizingSixteenthPel.filled = true;
			h263p->h263Options.dynamicWarpingHalfPel.filled = true;
			h263p->h263Options.dynamicWarpingSixteenthPel.filled = true;

			h263p->h263Options.independentSegmentDecoding.filled = true;

			h263p->h263Options.slicesInOrder_NonRect.filled = true;
			h263p->h263Options.slicesInOrder_Rect.filled = true;
			h263p->h263Options.slicesNoOrder_NonRect.filled = true;
			h263p->h263Options.slicesNoOrder_Rect.filled = true;

			h263p->h263Options.alternateInterVLCMode.filled = true;

			h263p->h263Options.modifiedQuantizationMode.filled = true;
			h263p->h263Options.modifiedQuantizationMode.value = true; ///Anex T

			h263p->h263Options.reducedResolutionUpdate.filled = true;

			h263p->h263Options.transparencyParameters.filled = false;//o
			h263p->h263Options.separateVideoBackChannel.filled = true;//!0 non optional
			h263p->h263Options.refPictureSelection.filled = false;//o
			h263p->h263Options.customPictureClockFrequency.filled = false;//o
			h263p->h263Options.customPictureFormat.filled = false;//o
			h263p->h263Options.modeCombos.filled = false;//o
			///
			//////////////////////////
			h263p->h263Options.filled = true;
			//////////////////////////
			h263p->h263Options.h263Version3Options.filled = false;
			h263p->h263Options.videoBadMBsCap.filled = false;
			//
			h263p->filled = true;
			//
			vc[h263p_number]->choice = h263p;
			vc[h263p_number]->tag = VS_H245VideoCapability::e_h263VideoCapability;
			vc[h263p_number]->filled = true;

			VS_GatewayVideoMode m;
			m.CodecType = e_videoH263plus;
			m.PayloadType = SDP_PT_INVALID;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::video, e_videoH263plus);
			m.Mode = /*(h263p->sqcifMPI.value!=0)	* 0x01|*/
				(h263p->qcifMPI.value != 0) * 0x02 |
				(h263p->cifMPI.value != 0) * 0x04 |
				(h263p->cif4MPI.value != 0) * 0x08 |
				(h263p->cif16MPI.value != 0) * 0x10;
			m.Bitrate = h263p->maxBitRate.value * 100;
			m.CodecAnexInfo = 0;
			video_channel.rcv_modes_video.push_back(m);
			slides_channel.rcv_modes_video.push_back(m);

			//
			h263p = 0;
			//
		}
	}
	////////////////////////
	////////////////////////
	///H.263
	if (enabledCodecs.find("H263") != std::string::npos)
	{
		const unsigned int h263_number = GetVideoIndex(e_videoH263);
		if (h263_number < vc_number)
		{
			dprint1("\n\t h263_number = %d", h263_number);
			VS_H245H263VideoCapability * h263 = 0;
			h263 = new VS_H245H263VideoCapability;
			//
			h263->advancedPrediction.value = false;
			h263->arithmeticCoding.value = false;
			h263->bppMaxKb.value = 0;
			h263->cif16MPI.value = 0;
			h263->cif4MPI.value = 3;
			h263->cifMPI.value = 1;//(i==0)?1:0;
			h263->errorCompensation.value = false;
			h263->hrd_B.value = 0;
			h263->maxBitRate.value = m_H245Params.m_videoMaxBitrate * 10;
			h263->pbFrames.value = false;
			h263->qcifMPI.value = 1;///(i==1)?1:0;;
			h263->slowCif16MPI.value = 0;
			h263->slowCif4MPI.value = 0;
			h263->slowCifMPI.value = 0;
			h263->slowQcifMPI.value = 0;
			h263->slowSqcifMPI.value = 0;
			h263->sqcifMPI.value = 1;//(i==2)?1:0;;
			h263->temporalSpatialTradeOffCapability.value = false;
			h263->unrestrictedVector.value = false;
			//
			h263->advancedPrediction.filled = true;
			h263->arithmeticCoding.filled = true;
			h263->bppMaxKb.filled = true;
			h263->cif16MPI.filled = false;
			h263->cif4MPI.filled = true;
			h263->cifMPI.filled = true;
			h263->errorCompensation.filled = true;
			h263->hrd_B.filled = true;
			h263->maxBitRate.filled = true;
			h263->pbFrames.filled = true;
			h263->qcifMPI.filled = true;
			h263->slowCif16MPI.filled = false;
			h263->slowCif4MPI.filled = false;
			h263->slowCifMPI.filled = false;
			h263->slowQcifMPI.filled = false;
			h263->slowSqcifMPI.filled = false;
			h263->sqcifMPI.filled = false;
			h263->temporalSpatialTradeOffCapability.filled = true;
			h263->unrestrictedVector.filled = true;
			h263->enhancementLayerInfo.filled = false;
			h263->filled = true;
			//
			vc[h263_number]->choice = h263;
			vc[h263_number]->tag = VS_H245VideoCapability::e_h263VideoCapability;
			vc[h263_number]->filled = true;

			VS_GatewayVideoMode m;
			m.CodecType = e_videoH263;
			m.PayloadType = SDP_PT_H263;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::video, e_videoH263);
			m.Mode = (h263->sqcifMPI.value != 0) * 0x01 |
				(h263->qcifMPI.value != 0) * 0x02 |
				(h263->cifMPI.value != 0) * 0x04 |
				(h263->cif4MPI.value != 0) * 0x08 |
				(h263->cif16MPI.value != 0) * 0x10;

			m.Bitrate = h263->maxBitRate.value * 100;
			m.CodecAnexInfo = 0;
			video_channel.rcv_modes_video.push_back(m);
			slides_channel.rcv_modes_video.push_back(m);

			//
			h263 = 0;
			//
		}
	}
	////////////////////////
	///H.264
	if (enabledCodecs.find("H264") != std::string::npos)
	{
		const unsigned int h264_number = GetVideoIndex(e_videoH264);
		if (h264_number < vc_number)
		{
			unsigned int H264_profile = 64;//< Baseline

			// according to: Table 5/H.241 Level parameter values
			unsigned int H264_level = 29;
			switch (m_H245Params.m_reg_H264_level)
			{
			case 10: H264_level = 15; break;
			case 11: H264_level = 22; break;
			case 12: H264_level = 29; break;
			case 13: H264_level = 36; break;
			case 20: H264_level = 43; break;
			case 21: H264_level = 50; break;
			case 22: H264_level = 57; break;
			case 30: H264_level = 64; break;
			case 31: H264_level = 71; break;
			case 32: H264_level = 78; break;
			case 40: H264_level = 85; break;
			case 41: H264_level = 92; break;
			case 42: H264_level = 99; break;
			case 50: H264_level = 106; break;
			case 51: H264_level = 113; break;
			}

			unsigned int H264_customMBPS = m_H245Params.m_reg_H264_CustomMaxMBPS;		//< custom MBPS in 500 marcoblocks units (CustomMaxMBPS * 500)
			unsigned int H264_somepar = 80;
			unsigned int H264_customFS = 32;

			dprint1("\n\t h264_number = %d", h264_number);
			bool isFlagOk = true;
			unsigned int id[32] = { 0, 0, 8, 241, 0, 0, 1, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0 };

			unsigned int maxBitRate = m_H245Params.m_videoMaxBitrate * 10;
			VS_H245GenericCapability * result = 0;
			VS_H323_GenericCapabilityGenerator gen(id);
			if (!gen.SetMaxBitRate(maxBitRate))
			{
				dprint0("\n\t H.264 is not set properly - 2.");
				isFlagOk = false;
			}
			else
			{
				if (!gen.AddStandartIntParametr(true,
					VS_H245ParameterValue::e_booleanArray, H264_profile, H264_CODEC_PARAMETER_PROFILE))
				{
					dprint0("\n\t H.264 is not set properly - 2.");
					isFlagOk = false;
				}
				else if (!gen.AddStandartIntParametr(true,
					VS_H245ParameterValue::e_unsignedMin, H264_level, H264_CODEC_PARAMETER_LEVEL))
				{
					dprint0("\n\t H.264 is not set properly - 2.");
					isFlagOk = false;
				}
				else if (!gen.AddStandartIntParametr(true,
					VS_H245ParameterValue::e_unsignedMin, H264_customMBPS, H264_CODEC_PARAMETER_CUSTOMMBPS))
				{
					dprint0("\n\t H.264 is not set properly - 2.");
					isFlagOk = false;
				}
				else if (!gen.AddStandartIntParametr(true,
					VS_H245ParameterValue::e_unsignedMin, H264_customFS, H264_CODEC_PARAMETER_CUSTOMFS))
				{
					dprint0("\n\t H.264 is not set properly - 2.");
					isFlagOk = false;
				}
				//else if(!gen.AddStandartIntParametr( true ,
				//    VS_H245ParameterValue::e_unsignedMin , H264_somepar , 6 ))
				//{
				//    logprint0("\n\t H.264 is not set properly - 2.");
				//    isFlagOk = false;
				//}
				else  result = gen.Generate();
			}
			if (isFlagOk && result)
			{
				vc[h264_number]->choice = result;
				vc[h264_number]->tag = VS_H245VideoCapability::e_genericVideoCapability;
				vc[h264_number]->filled = true;
				//m_Logger->Printf("\n\t ---------------------------------------------");
				//m_Logger->Printf("\n\t ---------My VIDEO generic Capability---------");
				//m_Logger->Printf("\n\t ---------------------------------------------");
				//m_Logger->Printf("\n\t PARAMS!!!");
				//m_Logger->Printf("\n\t ---------------------------------------------");
			}

			VS_GatewayVideoMode m;
			m.CodecType = e_videoH264;
			m.PayloadType = SDP_PT_INVALID;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::video, e_videoH264);
			m.Mode = m_H245Params.m_reg_H264_level;
			m.Bitrate = maxBitRate * 100;
			m.CodecAnexInfo = 0;
			video_channel.rcv_modes_video.push_back(m);
			slides_channel.rcv_modes_video.push_back(m);
		}
	}
	/////
	/// H.261
	if (enabledCodecs.find("H261") != std::string::npos)
	{
		const unsigned int h261_number = GetVideoIndex(e_videoH261);
		if (h261_number < vc_number)
		{
			dprint1("\n\t h261_number = %d", h261_number);
			VS_H245H261VideoCapability * h261 = new VS_H245H261VideoCapability;
			if (!h261)
			{
				dprint0("\n\t There is not enought memory. H261");
				return false;
			}

			h261->cifMPI.value = 1;
			auto max_bw = (m_H245Params.m_videoMaxBitrate <= 1920) ? m_H245Params.m_videoMaxBitrate : 1920;
			h261->maxBitRate.value = max_bw * 10;
			h261->qcifMPI.value = 1;
			h261->stillImageTransmission.value = false;
			h261->temporalSpatialTradeOffCapability.value = false;
			///
			h261->cifMPI.filled = true;
			h261->maxBitRate.filled = true;
			h261->qcifMPI.filled = true;
			h261->stillImageTransmission.filled = true;
			h261->temporalSpatialTradeOffCapability.filled = true;
			///
			h261->filled = true;
			///
			vc[h261_number]->choice = h261;
			vc[h261_number]->tag = VS_H245VideoCapability::e_h261VideoCapability;
			vc[h261_number]->filled = true;

			VS_GatewayVideoMode m;
			m.CodecType = e_videoH261;
			m.PayloadType = SDP_PT_H261;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::video, e_videoH261);
			m.Mode = (h261->qcifMPI.value != 0) * 0x02 |
				(h261->cifMPI.value != 0) * 0x04;
			m.Bitrate = h261->maxBitRate.value * 100;
			m.CodecAnexInfo = 0;
			video_channel.rcv_modes_video.push_back(m);
			slides_channel.rcv_modes_video.push_back(m);

			h261 = 0;
		}
	}


	//////////
	// H224
	if (IsH224Enabled())
	{
		const unsigned int h224_number = GetDataIndex((int)VS_H323DataCodec::FECC);
		if (h224_number < dc_number)
		{
			dprint1("\n\t h224_number = %d", h224_number);
			VS_H245DataProtocolCapability* h224 = new VS_H245DataProtocolCapability;

			if (!h224)
			{
				dprint0("\n\t There is not enought memory. H224");
				return false;
			}

			h224->choice = new VS_AsnNull;
			h224->tag = VS_H245DataProtocolCapability::e_hdlcFrameTunnelling;
			h224->filled = true;

			dc[h224_number]->application.choice = h224;
			dc[h224_number]->application.tag = VS_H245DataApplicationCapability_Application::e_h224;
			dc[h224_number]->application.filled = true;
			dc[h224_number]->maxBitRate.value = 50;
			dc[h224_number]->maxBitRate.filled = true;
			dc[h224_number]->filled = true;

			VS_GatewayDataMode m;
			m.CodecType = VS_H323DataCodec::FECC;
			m.ExtendedCodec = true;
			m.PayloadType = SDP_PT_DYNAMIC_H224;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::application_fecc, static_cast<int>(VS_H323DataCodec::FECC));
			m.BitRate = dc[h224_number]->maxBitRate.value * 100;

			data_channel.rcv_modes_data.push_back(m);

			h224 = 0;
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////

	// fill syncFlag for h235 security capability
	for (auto& mode : audio_channel.rcv_modes_audio)
		mode.sec_cap.syncFlag = mode.PayloadType;
	for (auto& mode : video_channel.rcv_modes_video)
		mode.sec_cap.syncFlag = mode.PayloadType;
	for (auto& mode : slides_channel.rcv_modes_video)
		mode.sec_cap.syncFlag = mode.PayloadType;
	for (auto& mode : data_channel.rcv_modes_data)
		mode.sec_cap.syncFlag = mode.PayloadType;
	return true;
}

VS_GatewayVideoMode* VS_H323ParserInfo::GetRecvVideoMode(const int codec_type)
{
	for (auto& mode: video_channel.rcv_modes_video)
		if (mode.CodecType == codec_type)
			return &mode;
	return nullptr;
}

VS_GatewayAudioMode* VS_H323ParserInfo::GetRecvAudioMode(int codec_type)
{
	for (auto& mode: audio_channel.rcv_modes_audio)
		if (mode.CodecType == codec_type)
			return &mode;
	return nullptr;
}

VS_GatewayVideoMode* VS_H323ParserInfo::GetRecvSlidesVideoMode(int codec_type)
{
	for (auto& mode: slides_channel.rcv_modes_video)
		if (mode.CodecType == codec_type)
			return &mode;
	return nullptr;
}

VS_GatewayDataMode* VS_H323ParserInfo::GetRecvDataMode(VS_H323DataCodec codec_type)
{
	for (auto& mode : data_channel.rcv_modes_data)
		if (mode.CodecType == codec_type)
			return &mode;
	return nullptr;
}

void VS_H323ParserInfo::SetHangupMode(HangupMode mode)
{
	m_hangup_mode = mode;
}

HangupMode VS_H323ParserInfo::GetHangupMode() const
{
	return m_hangup_mode;
}

unsigned int VS_H323ParserInfo::GetAudioIndex( const int value )
{
	return GetCodecIndex( value );
}

unsigned int VS_H323ParserInfo::GetVideoIndex( const int value )
{
	return GetCodecIndex( value );
}

unsigned int VS_H323ParserInfo::GetDataIndex( const int value )
{
	return GetCodecIndex( value );
}

unsigned int VS_H323ParserInfo::GetCodecIndex( const int value )
{
	int index = value;
	int i = 0;
	if (index==0) return 10000;
	while(!(index & 0x01 ))
	{
		i++;
		index = index>>1;
	}
	return i;
}

int VS_H323ParserInfo::GetCodecID(const unsigned int index)
{
	return (1 << index);
}

const net::address& VS_H323ParserInfo::GetMyCsAddress() const
{
	if (m_my_external_cs_address.is_unspecified())
		return m_my_cs_address;

	return m_my_external_cs_address;
}

void VS_H323ParserInfo::SetMyExternalCsAddress(const net::address& address)
{
	m_my_external_cs_address = address;
}

const net::address& VS_H323ParserInfo::GetMyExternalCsAddress() const
{
	return m_my_external_cs_address;
}

const std::string &VS_H323ParserInfo::GetDialogID() const
{
	return m_DialogID;
}
void VS_H323ParserInfo::SetDialogID(std::string id)
{
	m_DialogID = std::move(id);
}

const std::string &VS_H323ParserInfo::GetConferenceID() const
{
	return m_ConferenceID;
}

void VS_H323ParserInfo::SetConferenceID(std::string confId)
{
	m_ConferenceID = std::move(confId);
}

 const std::string &VS_H323ParserInfo::GetCallIdentifier() const
{
	return m_CallIdentifier;
}

void VS_H323ParserInfo::SetCallIdentifier(std::string callId)
{
	m_CallIdentifier = std::move(callId);
}

bool VS_H323ParserInfo::IsKeepAliveEnabled() const
{
	return m_keep_alive_enabled;
}

void VS_H323ParserInfo::EnableKeepAlive()
{
	m_keep_alive_enabled = true;
	//m_keep_alive_tick = GetTickCount();
}

void VS_H323ParserInfo::SetPeerCsAddress(const net::address& address, net::port port)
{
	m_peer_cs_address = address;
	m_peer_cs_port = port;
}

const net::address& VS_H323ParserInfo::GetPeerCsAddress() const
{
	return m_peer_cs_address;
}

net::port VS_H323ParserInfo::GetPeerCsPort() const
{
	return m_peer_cs_port;
}

void VS_H323ParserInfo::SetOLCStartTick(std::chrono::steady_clock::time_point tick)
{
	m_start_tick = tick;
}

bool VS_H323ParserInfo::IsOLCTimeout(std::chrono::steady_clock::time_point now) const
{
	return m_start_tick != std::chrono::steady_clock::time_point() && now - m_start_tick >= WAIT_OLC_TIMEOUT;
};

void VS_H323ParserInfo::SetTCSWaitingTick(std::chrono::steady_clock::time_point tick)
{
	m_tcs_wait_tick = tick;
}

bool VS_H323ParserInfo::IsTCSSendNeeded(std::chrono::steady_clock::time_point now) const
{
	return m_tcs_wait_tick != std::chrono::steady_clock::time_point() && now - m_tcs_wait_tick >= std::chrono::milliseconds(TCS_WAIT_TIMEOUT);
}

bool VS_H323ParserInfo::GenNewMSDNums()
{
	m_H245Params.m_my_msd_num = rand_msd_distr(rand_generator);
	assert(m_H245Params.m_my_msd_num < 1 << 24);
	m_H245Params.m_my_msd_type = 50;

	m_H245Params.m_their_msd_num = 0;
	m_H245Params.m_their_msd_type = 0;

	m_H245Params.m_msd_type = 0;
	m_H245Params.m_msd_mode = 0;

	return ++m_msd_counter < MSD_COUNT_LIMIT;
}

void VS_H323ParserInfo::StartWaitingMSD()
{
	m_msd_wait_tick = clock().now();
}

bool VS_H323ParserInfo::IsMSDSendNeeded() const
{
	return m_msd_wait_tick != decltype(m_msd_wait_tick)()  && clock().now() - m_msd_wait_tick >= MSD_WAIT_TIMEOUT;
}

void VS_H323ParserInfo::StopWaitingMSD()
{
	m_msd_counter = 0;
	m_msd_wait_tick = decltype(m_msd_wait_tick)();
}

void VS_H323ParserInfo::StartMSDTimer()
{
	m_msd_timer_tick = clock().now();
}

bool VS_H323ParserInfo::IsMSDTimerExpired() const
{
	return m_msd_timer_tick != decltype(m_msd_timer_tick)() && clock().now() - m_msd_timer_tick >= MSDA_WAIT_TIMEOUT;
}

bool VS_H323ParserInfo::IsMSDTimerStarted() const
{
	return m_msd_timer_tick != decltype(m_msd_timer_tick)();
}

void VS_H323ParserInfo::StopMSDTimer()
{
	m_msd_timer_tick = decltype(m_msd_timer_tick)();
}

MSDState VS_H323ParserInfo::MsdState() const
{
	return m_msd_state;
}
void VS_H323ParserInfo::SetMsdState(MSDState state)
{
	m_msd_state = state;
}

void VS_H323ParserInfo::SetModesDone(bool isDone)
{
	m_set_modes_done = isDone;
}

bool VS_H323ParserInfo::IsSetModesDone() const
{
	return m_set_modes_done;
}

const std::shared_ptr<VS_SignalChannel> &VS_H323ParserInfo::GetH245Channel() const
{
	return m_h245_channel;
}

void VS_H323ParserInfo::SetH245Channel(std::shared_ptr<VS_SignalChannel> channel)
{
	m_h245_channel = std::move(channel);
}

VS_TearMessageQueue& VS_H323ParserInfo::GetH245InputQueue()
{
	return m_h245_input_queue;
}

void VS_H323ParserInfo::SetDisplayNameMy(std::string name)
{
	m_my_display_name = std::move(name);
}

const std::string &VS_H323ParserInfo::GetDisplayNameMy() const
{
	return m_my_display_name;
}

void VS_H323ParserInfo::SetDisplayNamePeer(std::string name)
{
	m_peer_display_name = std::move(name);
}

const std::string &VS_H323ParserInfo::GetDisplayNamePeer() const
{
	return m_peer_display_name;
}

void VS_H323ParserInfo::SetMyDialedDigit(std::string digit)
{
	m_my_dialed_digit = std::move(digit);
}

const std::string &VS_H323ParserInfo::GetMyDialedDigit() const
{
	return m_my_dialed_digit;
}

void VS_H323ParserInfo::SetSrcAlias(std::string alias)
{
	m_alias_from = alias;
}

const std::string &VS_H323ParserInfo::GetSrcAlias() const
{
	return m_alias_from;
}

void VS_H323ParserInfo::SetDstAlias(std::string alias)
{
	m_alias_to = std::move(alias);
}

const std::string &VS_H323ParserInfo::GetDstAlias() const
{
	return m_alias_to;
}

void VS_H323ParserInfo::SetSrcDigit(std::string digit)
{
	m_digit_from = std::move(digit);
}

const std::string &VS_H323ParserInfo::GetSrcDigit() const
{
	return m_digit_from;
}

void VS_H323ParserInfo::SetDstDigit(std::string digit)
{
	m_digit_to = std::move(digit);
}

const std::string &VS_H323ParserInfo::GetDstDigit() const
{
	return m_digit_to;
}

void VS_H323ParserInfo::SetH323UserType(const H323_User_Type user_type)
{
	m_user_type = user_type;
}

H323_User_Type VS_H323ParserInfo::GetH323UserType() const
{
	return m_user_type;
}

bool VS_H323ParserInfo::IsH323UserTypeAutoDetect() const
{
	//TODO: stub ???
	return true;
}

VS_MediaChannelInfo* VS_H323ParserInfo::GetMediaChannel(VS_H245LogicalChannelInfoDataType dataType)
{
	switch (dataType)
	{
	case e_audio:  return &audio_channel;;
	case e_video:  return &video_channel;
	case e_slides: return &slides_channel;
	case e_data:   return &data_channel;
	default:       return nullptr;
	}
}

bool VS_H323ParserInfo::CreateH245LogicalChannel(VS_H245LogicalChannelInfoDataType dataType, std::uint32_t forwardLogicalChannelNumber, bool isSender)
{
	if (m_LogicalChannelsMap.count(forwardLogicalChannelNumber) > 0)
		return true;

	auto& lc = m_LogicalChannelsMap[forwardLogicalChannelNumber];
	lc.m_forwardLogicalChannelNumber = forwardLogicalChannelNumber;
	lc.m_isSender = isSender;
	lc.m_dataType = dataType;

	if (lc.m_isSender)
	{
		switch (lc.m_dataType)
		{
		case e_audio:
			lc.m_sessionID = 1;
			break;
		case e_video:
			lc.m_sessionID = 2;
			break;
		case e_slides:
			switch (m_H245Params.m_msd_type)
			{
			case 1:
				// Master, we must set sessionID ourselves
				lc.m_sessionID = 32; // Value used by Polycom and Cisco
				break;
			case 2:
				// Slave, we must set sessionID to 0 and master will choose the real one and send it to us in OLCA
				lc.m_sessionID = 0;
				break;
			case 0:
			default:
				dprint2("Forced to select sessionID for slides before MSD completion\n");
				lc.m_sessionID = 0;
			}
			break;
		case e_data:
			lc.m_sessionID = 3;
			break;
		}
	}
	else
		lc.m_sessionID = 0;

	auto channel = GetMediaChannel(lc.m_dataType);
	return channel != nullptr;
}

bool VS_H323ParserInfo::GetH245LogicalChannel(std::uint32_t forwardLogicalChannelNumber, VS_H245LogicalChannelInfo &info) const
{
	auto lc_it = m_LogicalChannelsMap.find(forwardLogicalChannelNumber);
	if (lc_it == m_LogicalChannelsMap.cend())
		return false;
	info = lc_it->second;
	return true;
}

bool VS_H323ParserInfo::GetH245LogicalChannel(const VS_H245LogicalChannelInfoDataType dataType, bool isSender, VS_H245LogicalChannelInfo &info) const
{
	for (const auto& lc_kv: m_LogicalChannelsMap)
		if (lc_kv.second.m_isSender == isSender && lc_kv.second.m_dataType == dataType)
		{
			info = lc_kv.second;
			return true;
		}
	return false;
}

bool VS_H323ParserInfo::SetH245LogicalChannel(std::uint32_t forwardLogicalChannelNumber, const VS_H245LogicalChannelInfo &info)
{
	auto lc_it = m_LogicalChannelsMap.find(forwardLogicalChannelNumber);
	if (lc_it == m_LogicalChannelsMap.cend())
		return false;
	lc_it->second = info;
	return true;
}

bool VS_H323ParserInfo::CloseH245LogicalChannel(std::uint32_t forwardLogicalChannelNumber)
{
	return m_LogicalChannelsMap.erase(forwardLogicalChannelNumber) > 0;
}

const std::unordered_set<std::uint32_t> &VS_H323ParserInfo::GetPendingLCList() const
{
	return m_pending_lcs;
}

bool VS_H323ParserInfo::GetLCPending(std::uint32_t number) const
{
	return m_pending_lcs.count(number) > 0;
}

void VS_H323ParserInfo::SetLCPending(std::uint32_t number, bool isPending)
{
	if (isPending)
		m_pending_lcs.emplace(number);
	else
		m_pending_lcs.erase(number);
}

bool VS_H323ParserInfo::IsReady() const
{
	return IsReady(e_olc_audio) && IsReady(e_olc_video);
}

bool VS_H323ParserInfo::IsInDialog() const
{
	return m_is_in_dialog;
}

void VS_H323ParserInfo::SetInDialog(bool inDialog)
{
	m_is_in_dialog = inDialog;
	m_set_modes_done = false;
}

bool VS_H323ParserInfo::IsReady(VS_CapabilityExchangeStages stage) const
{
	const bool recv =     (m_H245Params.H245Flags[stage] & VS_H245Data::h245_rsp_recv) != 0;
	const bool send =     (m_H245Params.H245Flags[stage] & VS_H245Data::h245_rsp_send) != 0;
	const bool rejected = (m_H245Params.H245Flags[stage] & VS_H245Data::h245_rsp_rejected) != 0;

	return (recv && send) || (rejected && stage != e_olc_audio);
}

bool VS_H323ParserInfo::IsHangupStarted() const
{
	return m_isHangupStarted;
}

void VS_H323ParserInfo::SetHangupStarted(bool value)
{
	m_isHangupStarted = value;
}

bool VS_H323ParserInfo::IsGroupConf() const
{
	return m_IsGroupConf;
}

void VS_H323ParserInfo::SetGroupConf(bool isGroupConf)
{
	m_IsGroupConf = isGroupConf;
}

unsigned int VS_H323ParserInfo::GetCRV() const
{
	return m_crv;
}
void VS_H323ParserInfo::SetCRV(const unsigned int crv)
{
	m_crv = crv;
}

CallDirection VS_H323ParserInfo::GetCallDirection() const
{
	return m_direction;
}

void VS_H323ParserInfo::SetCallDirection(CallDirection direction)
{
	m_direction = direction;
}

VS_H245Data* VS_H323ParserInfo::GetH245Params()
{
	return &m_H245Params;
}

VS_H245VideoCapability** VS_H323ParserInfo::GetVideoCapability()
{
	return vc;
}

VS_H245AudioCapability** VS_H323ParserInfo::GetAudioCapability()
{
	return ac;
}

VS_H245DataApplicationCapability** VS_H323ParserInfo::GetDataCapability()
{
	return dc;
}

void VS_H323ParserInfo::SetPeerH245Version(unsigned version)
{
	m_peer_h245_version = version;
}

unsigned VS_H323ParserInfo::GetPeerH245Version() const
{
	return m_peer_h245_version;
}

void VS_H323ParserInfo::SetH239CapabilityPresent(bool value)
{
	m_h239_capability_present = value;
}

bool VS_H323ParserInfo::IsH239CapabilityPresent() const
{
	return m_h239_capability_present;
}

void VS_H323ParserInfo::SetH239Enabled(bool value)
{
	m_h239_enabled = value;
}

bool VS_H323ParserInfo::IsH239Enabled() const
{
	return m_h239_enabled;
}

VS_H239TokenInfo* VS_H323ParserInfo::GetH239PresentationToken()
{
	return &m_presentation_token;
}

bool VS_H323ParserInfo::GetSlideshowState() const
{
	return m_slideshow_active;
}

void VS_H323ParserInfo::SetSlideshowState(bool active)
{
	m_slideshow_active = active;
}

bool VS_H323ParserInfo::IsH224CapabilityPresent() const
{
	return m_h224_capability_present;
}

void VS_H323ParserInfo::SetH224CapabilityPresent(bool value)
{
	m_h224_capability_present = value;
}

void VS_H323ParserInfo::SetRecvAudioReady(bool value)
{
	m_is_recv_audio_ready = value;
}

void VS_H323ParserInfo::SetRecvVideoReady(bool value)
{
	m_is_recv_video_ready = value;
}

bool VS_H323ParserInfo::IsRecvAudioReady() const
{
	return m_is_recv_audio_ready;
}

bool VS_H323ParserInfo::IsRecvVideoReady() const
{
	return m_is_recv_video_ready;
}

unsigned VS_H323ParserInfo::GetACDefault() const
{
	return ac_default;
}

void VS_H323ParserInfo::SetACDefault(unsigned value)
{
	ac_default = value;
}

VS_H245VideoCapability* VS_H323ParserInfo::GetVCDefault()
{
	return &vc_default;
}

void VS_H323ParserInfo::SetVCDefault(VS_H245VideoCapability& cap)
{
	vc_default = cap;
	m_vc_default_set = true;
}

bool VS_H323ParserInfo::HasVCDefault() const
{
	return m_vc_default_set;
}

VS_H245VideoCapability* VS_H323ParserInfo::GetVSCDefault()
{
	return &vsc_default;
}

void VS_H323ParserInfo::SetVSCDefault(VS_H245VideoCapability& cap)
{
	vsc_default = cap;
	m_vsc_default_set = true;
}

bool VS_H323ParserInfo::HasVSCDefault() const
{
	return m_vsc_default_set;
}

VS_H245DataApplicationCapability* VS_H323ParserInfo::GetDCDefault()
{
	return &dc_default;
}

void VS_H323ParserInfo::SetDCDefault(VS_H245DataApplicationCapability& cap)
{
	dc_default = cap;
	m_dc_default_set = true;
}

bool VS_H323ParserInfo::HasDCDefault() const
{
	return m_dc_default_set;
}

void VS_H323ParserInfo::MarkAsGatekeeperCall()
{
	m_gk_registred_call = true;
}

bool VS_H323ParserInfo::IsGatekeeperCall() const
{
	return m_gk_registred_call;
}

void VS_H323ParserInfo::SetH264Level(int level)
{
	const unsigned int h264_number = GetVideoIndex(e_videoH264);
	if (h264_number < vc_number)
	{
		const unsigned int H264_profile = 64;//< Baseline

		unsigned int H264_level;
		switch (level)
		{
		case 10: H264_level = 15; break;
		case 11: H264_level = 22; break;
		case 12: H264_level = 29; break;
		case 13: H264_level = 36; break;
		case 20: H264_level = 43; break;
		case 21: H264_level = 50; break;
		case 22: H264_level = 57; break;
		case 30: H264_level = 64; break;
		case 31: H264_level = 71; break;
		case 32: H264_level = 78; break;
		case 40: H264_level = 85; break;
		case 41: H264_level = 92; break;
		case 42: H264_level = 99; break;
		case 50: H264_level = 106; break;
		case 51: H264_level = 113; break;
		default: H264_level = 29; break; // according to: Table 5/H.241 Level parameter values
		}

		bool isFlagOk = true;
		unsigned int id[32] =
		{
			0, 0, 8, 241, 0, 0, 1, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0
		};

		unsigned int maxBitRate = m_H245Params.m_videoMaxBitrate * 10;
		VS_H245GenericCapability * result = 0;
		VS_H323_GenericCapabilityGenerator gen(id);
		if (!gen.SetMaxBitRate(maxBitRate))
		{
			dprint0("\n\t H.264 is not set properly - 2.");
			isFlagOk = false;
		}
		else
		{
			if (!gen.AddStandartIntParametr(true,
				VS_H245ParameterValue::e_booleanArray, H264_profile, H264_CODEC_PARAMETER_PROFILE))
			{
				dprint0("\n\t H.264 is not set properly - 2.");
				isFlagOk = false;
			}
			else if (!gen.AddStandartIntParametr(true,
				VS_H245ParameterValue::e_unsignedMin, H264_level, H264_CODEC_PARAMETER_LEVEL))
			{
				dprint0("\n\t H.264 is not set properly - 2.");
				isFlagOk = false;
			}
			else  result = gen.Generate();
		}
		if (isFlagOk && result)
		{
			vc[h264_number]->choice = result;
		}
	}
}

const std::vector<int> &VS_H323ParserInfo::GetAudioCodecPrecedenceList(void) const
{
	return m_ac_precedence;
}

int VS_H323ParserInfo::GetACPayloadType(const int codecId)
{
	return get_ac_payload_type(codecId);
}

int VS_H323ParserInfo::GetACDynamicPayloadType(const int codecId)
{
	return get_ac_dynamic_payload_type(codecId);
}

bool VS_H323ParserInfo::FillACObjectId(const int codecId, VS_AsnObjectId *oid)
{
	return fill_ac_oid(codecId, oid);
}

bool VS_H323ParserInfo::IsMaster() const
{
	return m_H245Params.m_msd_type == 1;
}

void VS_H323ParserInfo::SetMSDCounterForTesting(int i) {
	m_msd_counter = i;
}

void VS_H323ParserInfo::SetBaseLCNumber(uint16_t base)
{
	m_base_LC_number = base;
}

void VS_H323ParserInfo::SetDTMF(const std::string &dtmf)
{
	m_dtmf = dtmf;
}

const std::string &VS_H323ParserInfo::GetDTMF(void) const
{
	return m_dtmf;
}

#undef DEBUG_CURRENT_MODULE

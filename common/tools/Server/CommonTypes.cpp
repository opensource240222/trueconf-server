#include "CommonTypes.h"
#include "std-generic/cpplib/VS_Container.h"
#include <boost/algorithm/string/predicate.hpp>

bool VS_MPEG4ESConfiguration::Serialize(VS_Container& cnt, string_view name) const
{
	VS_Container sub_cnt;
	if (!sub_cnt.AddValueI32("size_length", size_length)
	 || !sub_cnt.AddValueI32("index_length", index_length)
	 || !sub_cnt.AddValueI32("index_delta_length", index_delta_length)
//	 || !sub_cnt.AddValueI32("cts_delta_length", cts_delta_length)
//	 || !sub_cnt.AddValueI32("dts_delta_length", dts_delta_length)
//	 || !sub_cnt.AddValueI32("ras_indication", ras_indication)
	 || !sub_cnt.AddValueI32("constant_size", constant_size)
//	 || !sub_cnt.AddValueI32("constant_duration", constant_duration)
//	 || !sub_cnt.AddValueI32("max_displacement", max_displacement)
	 || (!config.empty() && !sub_cnt.AddValue("config", static_cast<const void*>(config.data()), config.size()))
	)
		return false;
	return cnt.AddValue(name, sub_cnt);
}

bool VS_MPEG4ESConfiguration::Deserialize(const VS_Container& cnt, const char* name)
{
	VS_Container sub_cnt;
	if (!(name ? cnt.GetValue(name, sub_cnt) : cnt.GetValue(sub_cnt)))
		return false;

	if (!sub_cnt.GetValueI32("size_length", size_length)
	 || !sub_cnt.GetValueI32("index_length", index_length)
	 || !sub_cnt.GetValueI32("index_delta_length", index_delta_length)
//	 || !sub_cnt.GetValueI32("cts_delta_length", cts_delta_length)
//	 || !sub_cnt.GetValueI32("dts_delta_length", dts_delta_length)
//	 || !sub_cnt.GetValueI32("ras_indication", ras_indication)
	 || !sub_cnt.GetValueI32("constant_size", constant_size)
//	 || !sub_cnt.GetValueI32("constant_duration", constant_duration)
//	 || !sub_cnt.GetValueI32("max_displacement", max_displacement)
	)
		return false;

	{
		size_t size(0);
		sub_cnt.GetValue("config", (void*)nullptr, size);
		config.resize(size);
		if (!sub_cnt.GetValue("config", (void*)config.data(), size))
			config.clear();
	}
	return true;
}

bool operator==(const VS_MPEG4ESConfiguration& lhs, const VS_MPEG4ESConfiguration& rhs)
{
	return lhs.size_length == rhs.size_length
	    && lhs.index_length == rhs.index_length
	    && lhs.index_delta_length == rhs.index_delta_length
//	    && lhs.cts_delta_length == rhs.cts_delta_length
//	    && lhs.dts_delta_length == rhs.dts_delta_length
//	    && lhs.ras_indication == rhs.ras_indication
	    && lhs.constant_size == rhs.constant_size
//	    && lhs.constant_duration == rhs.constant_duration
//	    && lhs.max_displacement == rhs.max_displacement
	    && lhs.config == rhs.config
	;
}
bool operator!=(const VS_MPEG4ESConfiguration& lhs, const VS_MPEG4ESConfiguration& rhs)
{
	return !(lhs == rhs);
}

bool VS_GatewayVideoMode::Serialize(VS_Container& cnt, string_view name) const
{
	VS_Container sub_cnt;
	if (!VS_GatewayMediaMode::Serialize(sub_cnt, "VS_GatewayMediaMode")) return false;
	if (!sub_cnt.AddValueI32("Mode", Mode)
	 || !sub_cnt.AddValueI32("Bitrate", Bitrate)
	 || !sub_cnt.AddValueI32("CodecType", CodecType)
	 || !sub_cnt.AddValueI32("CodecAnexInfo", CodecAnexInfo)
	 || !sub_cnt.AddValueI32("MaxFs", MaxFs)
	 || !sub_cnt.AddValueI32("MaxMbps", MaxMbps)
	 || (sizeOfSPS > 0 && !sub_cnt.AddValue("SPS", static_cast<const void*>(SequenceParameterSet), sizeOfSPS))
	 || (sizeOfPPS > 0 && !sub_cnt.AddValue("PPS", static_cast<const void*>(PictureParameterSet), sizeOfPPS))
	 || !sub_cnt.AddValue("IsFIRSupported", IsFIRSupported)
	 || !sub_cnt.AddValue("IsMixerCIFMode", IsMixerCIFMode)
	 || !sub_cnt.AddValueI32("PreferredWidth", preferred_width)
	 || !sub_cnt.AddValueI32("PreferredHeight", preferred_height)
	 || !sub_cnt.AddValueI32("GConfToTermVideoWidth", gconf_to_term_width)
	 || !sub_cnt.AddValueI32("GConfToTermVideoHeight", gconf_to_term_height)
	)
		return false;
	return cnt.AddValue(name, sub_cnt);
}

bool VS_GatewayVideoMode::Deserialize(const VS_Container& cnt, const char* name)
{
	VS_Container sub_cnt;
	if (!(name ? cnt.GetValue(name, sub_cnt) : cnt.GetValue(sub_cnt)))
		return false;

	if (!VS_GatewayMediaMode::Deserialize(sub_cnt, "VS_GatewayMediaMode")) return false;
	if (!sub_cnt.GetValueI32("Mode", Mode)
	 || !sub_cnt.GetValueI32("Bitrate", Bitrate)
	 || !sub_cnt.GetValueI32("CodecType", CodecType)
	 || !sub_cnt.GetValueI32("CodecAnexInfo", CodecAnexInfo)
	 || !sub_cnt.GetValueI32("MaxFs", MaxFs)
	 || !sub_cnt.GetValueI32("MaxMbps", MaxMbps)
	 || !sub_cnt.GetValue("IsFIRSupported", IsFIRSupported)
	 || !sub_cnt.GetValue("IsMixerCIFMode", IsMixerCIFMode)
	 || !sub_cnt.GetValueI32("PreferredWidth", preferred_width)
	 || !sub_cnt.GetValueI32("PreferredHeight", preferred_height)
	 || !sub_cnt.GetValueI32("GConfToTermVideoWidth", gconf_to_term_width)
	 || !sub_cnt.GetValueI32("GConfToTermVideoHeight", gconf_to_term_height)
	)
		return false;
	{
		size_t size(sizeof(SequenceParameterSet));
		sizeOfSPS = sub_cnt.GetValue("SPS", static_cast<void*>(SequenceParameterSet), size) ? size : 0;
	}
	{
		size_t size(sizeof(PictureParameterSet));
		sizeOfPPS = sub_cnt.GetValue("PPS", static_cast<void*>(PictureParameterSet), size) ? size : 0;
	}
	return true;
}

bool VS_GatewayMediaMode::Serialize(VS_Container& cnt, string_view name) const{
	VS_Container sub_cnt;
	if (!sub_cnt.AddValueI32("PayloadType", PayloadType)
		|| !sub_cnt.AddValue("InitializeNAT", InitializeNAT)
		|| !sub_cnt.AddValueI32("ClockRate", ClockRate)
		|| !sec_cap.Serialize(sub_cnt,"h235sec_caps"))
		return false;
	return cnt.AddValue(name, sub_cnt);
}

bool VS_GatewayMediaMode::Deserialize(const VS_Container& cnt, const char* name){
	VS_Container sub_cnt;
	if (!(name ? cnt.GetValue(name, sub_cnt) : cnt.GetValue(sub_cnt)))
		return false;

	if (!sub_cnt.GetValueI32("PayloadType", PayloadType)
		|| !sub_cnt.GetValue("InitializeNAT", InitializeNAT)
		|| !sub_cnt.GetValueI32("ClockRate", ClockRate)
		|| !sec_cap.Deserialize(sub_cnt, "h235sec_caps"))
		return false;
	return true;
}

bool VS_GatewayAudioMode::Serialize(VS_Container& cnt, string_view name) const
{
	VS_Container sub_cnt;
	if (!VS_GatewayMediaMode::Serialize(sub_cnt, "VS_GatewayMediaMode")) return false;
	if (!sub_cnt.AddValueI32("CodecType", CodecType)
	 || !mpeg4es.Serialize(sub_cnt, "mpeg4es")
	 || !sub_cnt.AddValue("SwapBytes", SwapBytes)
	)
		return false;
	return cnt.AddValue(name, sub_cnt);
}

bool VS_GatewayAudioMode::Deserialize(const VS_Container& cnt, const char* name)
{
	VS_Container sub_cnt;
	if (!(name ? cnt.GetValue(name, sub_cnt) : cnt.GetValue(sub_cnt)))
		return false;

	if (!VS_GatewayMediaMode::Deserialize(sub_cnt, "VS_GatewayMediaMode")) return false;
	if (!sub_cnt.GetValueI32("CodecType", CodecType)
	 || !mpeg4es.Deserialize(sub_cnt, "mpeg4es")
	 || !sub_cnt.GetValue("SwapBytes", SwapBytes)
	)
		return false;
	return true;
}

bool VS_GatewayDataMode::Serialize(VS_Container& cnt, string_view name) const
{
	VS_Container sub_cnt;
	if (!VS_GatewayMediaMode::Serialize(sub_cnt, "VS_GatewayMediaMode")) return false;
	if (!sub_cnt.AddValueI32("CodecType", CodecType)
	 || !sub_cnt.AddValueI32("BitRate", BitRate)
	 || !sub_cnt.AddValue("ExtendedCodec", ExtendedCodec)
	)
		return false;
	return cnt.AddValue(name, sub_cnt);
}

bool VS_GatewayDataMode::Deserialize(const VS_Container& cnt, const char* name)
{
	VS_Container sub_cnt;
	if (!(name ? cnt.GetValue(name, sub_cnt) : cnt.GetValue(sub_cnt)))
		return false;

	if (!VS_GatewayMediaMode::Deserialize(sub_cnt, "VS_GatewayMediaMode")) return false;
	if (!sub_cnt.GetValueI32("CodecType", CodecType)
	 || !sub_cnt.GetValueI32("BitRate", BitRate)
	 || !sub_cnt.GetValue("ExtendedCodec", ExtendedCodec)
	)
		return false;
	return true;
}

bool SlideInfo::Serialize(VS_Container& cnt) const
{
	if (!cnt.AddValueI32("SlideWidth", w)
	 || !cnt.AddValueI32("SlideHeight", h)
	 || !cnt.AddValueI32("SlideSize", size)
	 || !cnt.AddValueI32("SlideN", slide_n)
	 || !cnt.AddValueI32("SlideCount", slide_count)
	 || !cnt.AddValue("SlideType", img_type)
	 || !cnt.AddValue("SlideAbout", about)
	)
		return false;
	return true;
}

bool SlideInfo::Deserialize(const VS_Container& cnt)
{
	if (!cnt.GetValueI32("SlideWidth", w)
	 || !cnt.GetValueI32("SlideHeight", h)
	 || !cnt.GetValueI32("SlideSize", size)
	 || !cnt.GetValueI32("SlideN", slide_n)
	 || !cnt.GetValueI32("SlideCount", slide_count)
	 || !cnt.GetValue("SlideType", img_type)
	 || !cnt.GetValue("SlideAbout", about)
	)
		return false;
	return true;
}


namespace vs
{
	int GetCodecByEncodingName(string_view name, const std::uint32_t clockRate)
	{
		if (name.empty())
			return -1;

		if (name == "PCMU") { return e_rcvG711Ulaw64k; }
		else if (name == "PCMA") { return e_rcvG711Alaw64k; }
		else if (name == "G723") { return e_rcvG723; }
		else if (name == "G722") { return e_rcvG722_64k; }
		else if (name == "G728") { return e_rcvG728; }
		else if (name == "G729") { return e_rcvG729a; }
		else if (name == "SPEEX") { return e_rcvSPEEX_16kHz; }
		else if (name == "G7221"
			&& clockRate == 16000) {
			return e_rcvG722124;
		}
		else if (name == "OPUS") { return e_rcvOPUS; }
		else if (name == "H261") { return e_videoH261; }
		else if (name == "H263") { return e_videoH263; }
		else if (name == "H263-1998") { return e_videoH263plus; }
		else if (name == "H263-2000") { return e_videoH263plus2; }
		else if (name == "H264") { return e_videoH264; }
		else if ((name == "SIREN14" || name == "G7221") && clockRate == 32000) { return e_rcvSIREN14_24; }
		else if (name == "MPEG4-GENERIC") { return c_fakeMPEG4ES; } // actual codec is specified in "mode" parameter in a=fmtp
		else if (name == "MPA") { return e_rcvMPA; }
		else if (name == "X-H264UC") { return e_videoXH264UC; }
		else if (name == "H224") { return static_cast<int>(VS_H323DataCodec::FECC); }

		return -1;
	}

	bool GetCodecInfo(SDPMediaType mt, int codec, std::uint32_t &clockRate, eSDPPayloadType &PT, const char *&encodingName)
	{
		if (mt == SDPMediaType::audio)
		{
			switch (codec)
			{
			case e_rcvG711Ulaw64k: clockRate = 8000; encodingName = "PCMU";   PT = SDP_PT_G711U; break;
			case e_rcvG711Alaw64k: clockRate = 8000; encodingName = "PCMA";   PT = SDP_PT_G711A; break;
			case e_rcvG729a:       clockRate = 8000; encodingName = "G729";   PT = SDP_PT_G729A; break;
			case e_rcvG723:        clockRate = 8000; encodingName = "G723";   PT = SDP_PT_G723; break;
			case e_rcvG728:        clockRate = 8000; encodingName = "G728";   PT = SDP_PT_G728; break;
			case e_rcvG722124:     clockRate = 16000; encodingName = "G7221"; PT = SDP_PT_DYNAMIC_G722124; break;
			case e_rcvG722132:     clockRate = 16000; encodingName = "G7221"; PT = SDP_PT_DYNAMIC_G722132; break;
			case e_rcvG722_64k:    clockRate = 8000; encodingName = "G722";   PT = SDP_PT_G722_64k; break;
			case e_rcvSPEEX_8kHz:  clockRate = 8000; encodingName = "SPEEX";  PT = SDP_PT_DYNAMIC_SPEEX_8k; break;
			case e_rcvSPEEX_16kHz: clockRate = 16000; encodingName = "SPEEX"; PT = SDP_PT_DYNAMIC_SPEEX_16k; break;
			case e_rcvSPEEX_32kHz: clockRate = 32000; encodingName = "SPEEX"; PT = SDP_PT_DYNAMIC_SPEEX_32k; break;
			case e_rcvISAC_16kHz:  clockRate = 16000; encodingName = "ISAC";  PT = SDP_PT_DYNAMIC_ISAC_32k; break;
			case e_rcvSIREN14_24:  clockRate = 32000; encodingName = "G7221"; PT = SDP_PT_DYNAMIC_SIREN14_24k; break;
			case e_rcvSIREN14_32:  clockRate = 32000; encodingName = "G7221"; PT = SDP_PT_DYNAMIC_SIREN14_32k; break;
			case e_rcvSIREN14_48:  clockRate = 32000; encodingName = "G7221"; PT = SDP_PT_DYNAMIC_SIREN14_48k; break;
			case e_rcvOPUS:        clockRate = 16000; encodingName = "OPUS";  PT = SDP_PT_DYNAMIC_OPUS; break;
			case e_rcvMPA:         clockRate = 90000; encodingName = "MPA";   PT = SDP_PT_MPA; break;
			case e_rcvTelEvent:    clockRate = 8000; encodingName = "telephone-event"; PT = SDP_PT_DYNAMIC_TEL_EVENT; break;
			default: return false;
			};
		}
		else
			if (mt == SDPMediaType::video)
			{
				switch (codec)
				{
				case e_videoH261:      clockRate = 90000; encodingName = "H261";		PT = SDP_PT_H261; break;
				case e_videoH263:      clockRate = 90000; encodingName = "H263";		PT = SDP_PT_H263; break;
				case e_videoH263plus:  clockRate = 90000; encodingName = "H263-1998";   PT = SDP_PT_DYNAMIC_H263plus; break;
				case e_videoH263plus2: clockRate = 90000; encodingName = "H263-2000";   PT = SDP_PT_DYNAMIC_H263plus2; break;
				case e_videoH264:      clockRate = 90000; encodingName = "H264";		PT = SDP_PT_DYNAMIC_H264; break;
				case e_videoXH264UC:   clockRate = 90000; encodingName = "X-H264UC";	PT = SDP_PT_DYNAMIC_XH264UC; break;
				default: return false;
				};
			}
			else
				if (mt == SDPMediaType::application_fecc)
				{
					switch ((VS_H323DataCodec)codec)
					{
					case VS_H323DataCodec::FECC: clockRate = 4800; encodingName = "H224";		PT = SDP_PT_DYNAMIC_H224; break;
					default: return false;
					}
				}
				else return false;

				return true;
	}

	bool GetCodecInfoByName(string_view name, SDPMediaType &mt, int &codec,
		std::uint32_t &clockRate, eSDPPayloadType &PT, const char *&encodingName)
	{
		mt = SDPMediaType::invalid;


		if (boost::iequals("H261", name)) { mt = SDPMediaType::video; codec = e_videoH261; }
		else
		if (boost::iequals("H263", name)) { mt = SDPMediaType::video; codec = e_videoH263; }
		else
		if (boost::iequals("H263plus", name)) { mt = SDPMediaType::video; codec = e_videoH263plus; }
		else
		if (boost::iequals("H263plus2", name)) { mt = SDPMediaType::video; codec = e_videoH263plus2; }
		else
		if (boost::iequals("H264", name)) { mt = SDPMediaType::video; codec = e_videoH264; }
		else
		if (boost::iequals("XH264UC", name)) { mt = SDPMediaType::video; codec = e_videoXH264UC; }
		else
		if (boost::iequals("G711Ulaw64k", name)) { mt = SDPMediaType::audio; codec = e_rcvG711Ulaw64k; }
		else
		if (boost::iequals("G711Alaw64k", name)) { mt = SDPMediaType::audio; codec = e_rcvG711Alaw64k; }
		else
		if (boost::iequals("G729a", name)) { mt = SDPMediaType::audio; codec = e_rcvG729a; }
		else
		if (boost::iequals("G723", name)) { mt = SDPMediaType::audio; codec = e_rcvG723; }
		else
		if (boost::iequals("G728", name)) { mt = SDPMediaType::audio; codec = e_rcvG728; }
		else
		if (boost::iequals("G722124", name)) { mt = SDPMediaType::audio; codec = e_rcvG722124; }
		else
		if (boost::iequals("G722132", name)) { mt = SDPMediaType::audio; codec = e_rcvG722132; }
		else
		if (boost::iequals("G722_64k", name)) { mt = SDPMediaType::audio; codec = e_rcvG722_64k; }
		else
		if (boost::iequals("SPEEX_8kHz", name)) { mt = SDPMediaType::audio; codec = e_rcvSPEEX_8kHz; }
		else
		if (boost::iequals("SPEEX_16kHz", name)) { mt = SDPMediaType::audio; codec = e_rcvSPEEX_16kHz; }
		else
		if (boost::iequals("SPEEX_32kHz", name)) { mt = SDPMediaType::audio; codec = e_rcvSPEEX_32kHz; }
		else
		if (boost::iequals("ISAC_16kHz", name)) { mt = SDPMediaType::audio; codec = e_rcvISAC_16kHz; }
		else
		if (boost::iequals("SIREN14_24", name)) { mt = SDPMediaType::audio; codec = e_rcvSIREN14_24; }
		else
		if (boost::iequals("SIREN14_32", name)) { mt = SDPMediaType::audio; codec = e_rcvSIREN14_32; }
		else
		if (boost::iequals("SIREN14_48", name)) { mt = SDPMediaType::audio; codec = e_rcvSIREN14_48; }
		else
		if (boost::iequals("OPUS", name)) { mt = SDPMediaType::audio; codec = e_rcvOPUS; }
		else
		if (boost::iequals("MPA", name)) { mt = SDPMediaType::audio; codec = e_rcvMPA; }
		else
		if (boost::iequals("telephone-event", name)) { mt = SDPMediaType::audio; codec = e_rcvTelEvent; }
		else
		if (boost::iequals("H224", name)) { mt = SDPMediaType::application_fecc; codec = static_cast<int>(VS_H323DataCodec::FECC); }

		return GetCodecInfo(mt, codec, clockRate, PT, encodingName);
	}

	int GetCodecByStaticPayloadType(const SDPMediaType media_type, const eSDPPayloadType pt)
	{
		if (media_type == SDPMediaType::audio) {
			if (pt == SDP_PT_G711U)
				return e_rcvG711Ulaw64k;
			else if (pt == SDP_PT_G711A)
				return e_rcvG711Alaw64k;
			else if (pt == SDP_PT_G723)
				return e_rcvG723;
			else if (pt == SDP_PT_G728)
				return e_rcvG728;
			else if (pt == SDP_PT_G729A)
				return e_rcvG729a;
			else if (pt == SDP_PT_G722_64k)
				return e_rcvG722_64k;
			else if (pt == SDP_PT_MPA)
				return e_rcvMPA;
			return -1;
		}
		else if (media_type == SDPMediaType::video) {
			if (pt == SDP_PT_H261)
				return e_videoH261;
			else if (pt == SDP_PT_H263)
				return e_videoH263;
			//else if ( pt == SDP_PT_DYNAMIC_H263plus )
			//	return e_videoH263plus;
			//else if ( pt == SDP_PT_DYNAMIC_H263plus2 )
			//	return e_videoH263plus2;
			//else if ( pt == SDP_PT_DYNAMIC_H264 )
			//	return e_videoH264;
			else
				return -1;
		}
		else if (media_type == SDPMediaType::application_fecc) {
			if (pt == SDP_PT_DYNAMIC_H224)
				return (int)VS_H323DataCodec::FECC;
			else return -1;
		}

		return -1;
	}

	int GetCodecClockRateByCodecType(const SDPMediaType mt, int codec)
	{
		if (mt == SDPMediaType::audio)
		{
			switch (codec)
			{
			case e_rcvG711Ulaw64k:
				return 8000;
			case e_rcvG711Alaw64k:
				return 8000;
			case e_rcvG729a:
				return 8000;
			case e_rcvG723:
				return 8000;
			case e_rcvG728:
				return 8000;
			case e_rcvG722124:
				return 16000;
			case e_rcvG722132:
				return 16000;
			case e_rcvG722_64k:
				return 8000;
			case e_rcvSPEEX_8kHz:
				return 8000;
			case e_rcvSPEEX_16kHz:
				return 16000;
			case e_rcvSPEEX_32kHz:
				return 32000;
			case e_rcvISAC_16kHz:
				return 16000;
			case e_rcvSIREN14_24:
				return 32000;
			case e_rcvSIREN14_32:
				return 32000;
			case e_rcvSIREN14_48:
				return 32000;
			case e_rcvOPUS:
				return 16000;
			case e_rcvMPA:
				return 90000;
			case e_rcvTelEvent:
				return 8000;
			default:
				return vs::default_audio_clock_rate;
			};
		}
		else
			if (mt == SDPMediaType::video)
			{
				switch (codec)
				{
				case e_videoH261:
					return 90000;
				case e_videoH263:
					return 90000;
				case e_videoH263plus:
					return 90000;
				case e_videoH263plus2:
					return 90000;
				case e_videoH264:
					return 90000;
				case e_videoXH264UC:
					return 90000;
				default:
					return vs::default_video_clock_rate;
				};
			}
			else
				if (mt == SDPMediaType::application_fecc)
				{
					switch ((VS_H323DataCodec)codec)
					{
					case VS_H323DataCodec::FECC:
						return 4800;
					default:
						return vs::default_data_clock_rate;
					}
				}

		return vs::default_audio_clock_rate;
	}
}
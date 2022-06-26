#include "VS_SDPField_MediaStream.h"
#include "VS_SDPObjectFactory.h"
#include "VS_SDPCodec.h"
#include "VS_SDPCodecH264.h"
#include "VS_SDPField_Connection.h"
#include "VS_SDPField_Bandwidth.h"
#include "../tools/Server/CommonTypes.h"
#include "std-generic/cpplib/string_view.h"
#include "std/debuglog/VS_Debug.h"
#include <boost/algorithm/string/finder.hpp>
#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/split.hpp>
#include "std-generic/compat/memory.h"

#include <sstream>

static const std::string EMPTY_STR;

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SDPField_MediaStream::e1(
		"(?i)"
		" *(?:m) *= *"                       // header
		"([0-9A-Za-z_]+)"                    // media: token (actually rfc allows much more characters here: [-!#$%&'*+.0-9A-Za-z^_`{|}~]
		" +"
		"(?:(\\d+)(?:/(\\d+))?)"             // port
		" +"
		"([0-9A-Za-z_]+(?:/[0-9A-Za-z_]+)*)" // proto: token ["/" token]*
		"((?: +[0-9A-Za-z_*:@.]+)+)"                // fmt list: 1*(" " token)
		" *"
		"(?-i)"
	);

const boost::regex VS_SDPField_MediaStream::e2(	// <header>=<attribute>:<value>
				"(?i)"
				" *(\\w) *= *"				// header
				"(?:"
					"((?:\\w|\\-)+)"			// attribute
					"(?::(.*))?"			// value (if exist)
				"|"
					"(.*)"
				")"
				"(?-i)"
			);

const boost::regex VS_SDPField_MediaStream::e3("(?:[1-9] )(\\S*)(?: inline:)(.*)(?:\\|)(?:.*)");

VS_SDPField_MediaStream::VS_SDPField_MediaStream()
{
	VS_SDPField_MediaStream::Clean();
}

VS_SDPField_MediaStream::~VS_SDPField_MediaStream()
{
	VS_SDPField_MediaStream::Clean();
}

TSIPErrorCodes VS_SDPField_MediaStream::Decode(VS_SIPBuffer& _buffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	TSIPErrorCodes err = _buffer.GetNextBlockAllocConst(ptr, ptr_sz);
	if ( (TSIPErrorCodes::e_ok != err) || !ptr_sz || !ptr )
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	boost::cmatch m;
	bool regex_match_res;
	try
	{
		regex_match_res = boost::regex_match(ptr.get(), m, e1);
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPField_MediaStream::Decode case1 error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		_buffer.SkipHeader();

		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}
	std::string media_type = m[1];
	const std::string &thePort = m[2];
	const std::string &thePortRange = m[3];
	std::string theProto = m[4];
	std::string theFmt = m[5];

	std::transform(media_type.begin(), media_type.end(), media_type.begin(), toupper);
	std::transform(theProto.begin(), theProto.end(), theProto.begin(), toupper);

	bool media_supported = true;
	// media
	if      (media_type == "AUDIO")
		m_media_type = SDPMediaType::audio;
	else if (media_type == "VIDEO")
		m_media_type = SDPMediaType::video;
	else if (media_type == "APPLICATION")
		m_media_type = SDPMediaType::application;
	else if (media_type == "MESSAGE")
		m_media_type = SDPMediaType::message;
	else
		media_supported = false;

	// port
	m_port = atoi( thePort.c_str() );

	if (!thePortRange.empty())
	{
		m_port_range = atoi( thePortRange.c_str() );
	}else
	{
		m_port_range = 1;
	}

	// proto
	if      (theProto == "RTP/AVP")
		m_proto = SDP_RTPPROTO_RTP_AVP;
	else if (theProto == "RTP/SAVP")
		m_proto = SDP_RTPPROTO_RTP_SAVP;
	else if (theProto == "TCP/BFCP")
		m_proto = SDP_PROTO_TCP_BFCP;
	else if (theProto == "TCP/TLS/BFCP")
		m_proto = SDP_PROTO_TCP_TLS_BFCP;
	else if (theProto == "UDP/BFCP")
		m_proto = SDP_PROTO_UDP_BFCP;
	else if (theProto == "UDP/TLS/BFCP")
		m_proto = SDP_PROTO_UDP_TLS_BFCP;
	else if (theProto == "SIP")
		m_proto = SDP_PROTO_SIP;
	else
		media_supported = false;

	// fmt
	if (m_media_type == SDPMediaType::audio || m_media_type == SDPMediaType::video)
	{
		for (auto fmt_it = boost::make_find_iterator(theFmt, boost::algorithm::token_finder([](char x) { return x != ' '; }, boost::algorithm::token_compress_on)); !fmt_it.eof(); ++fmt_it)
		{
			int pt = atoi(std::string(fmt_it->begin(), fmt_it->end()).c_str());
			if (pt == 0 && (*fmt_it)[0] != '0') // conversion failed
			{
				media_supported = false;
				break;
			}
			m_his_pt_table.push_back(pt);
		}
	}

	// Validation
	if (m_media_type == SDPMediaType::audio || m_media_type == SDPMediaType::video)
	{
		if (m_proto == SDP_RTPPROTO_RTP_AVP || m_proto == SDP_RTPPROTO_RTP_SAVP)
			;
		else
			media_supported = false;
	}
	else if (m_media_type == SDPMediaType::application)
	{
		if (m_proto == SDP_PROTO_TCP_BFCP
		 || m_proto == SDP_PROTO_TCP_TLS_BFCP
		 || m_proto == SDP_PROTO_UDP_BFCP
		 || m_proto == SDP_PROTO_UDP_TLS_BFCP)
		{
			m_media_type = SDPMediaType::application_bfcp;
		}
		else if (m_proto == SDP_RTPPROTO_RTP_AVP)
		{
			m_media_type = SDPMediaType::application_fecc;
		}
		else
			media_supported = false;
	}
	else if (m_media_type == SDPMediaType::message)
	{
		size_t pos = 0;
		while (pos < theFmt.length() && theFmt[pos] == ' ') pos++;
		theFmt = theFmt.substr(pos, std::string::npos);
		m_message_url = theFmt;
	}
	else
		media_supported = false;

	if (!media_supported)
	{
		dstream3 << "[SIPParserLib::SDPError] MediaStream Field: buffer not match, dump |" << ptr.get() << "|";

		// skip all media description
		while (true)
		{
			ptr_sz = 0;

			_buffer.SkipHeader();

			err = _buffer.GetNextBlockAllocConst(ptr, ptr_sz);
			if ((TSIPErrorCodes::e_ok != err) || !ptr_sz || !ptr)
				break;

			if (ptr[0] == 'm' && ptr[1] == '=')
				break;
		}

		ptr = 0; ptr_sz = 0;

		SetValid(true);
		SetError(TSIPErrorCodes::e_ok);
		return TSIPErrorCodes::e_ok;
	}

	ptr_sz = 0;

	VS_SDPObjectFactory* factory = VS_SDPObjectFactory::Instance();
	assert(factory);

	_buffer.SkipHeader();

	std::map<int, VS_SIPBuffer> formats;
	bool stop = false;
	while ( !stop )
	{
		err = _buffer.GetNextBlockAllocConst(ptr, ptr_sz);
		if ( (TSIPErrorCodes::e_ok != err) || !ptr_sz || !ptr )
		{
			ptr_sz = 0;

			stop = true;
			continue;
		}

		boost::cmatch m;
		try
		{
			regex_match_res = boost::regex_match(ptr.get(), m, e2);
		}
		catch (const std::runtime_error &ex)
		{
			dstream1 << "VS_SDPField_MediaStream::Decode case2 error " << ex.what() << "\n";
			regex_match_res = false;
		}
		if (!regex_match_res)
		{
			SetValid(false);
			SetError(TSIPErrorCodes::e_match);
			return TSIPErrorCodes::e_match;
		}

		std::string header = m[1];
		std::string attribute = m[2];
		std::string value = m[3];
		std::string other = m[4];

		ptr_sz = 0;


		// LowerCase
		std::transform(header.begin(), header.end(), header.begin(), tolower);
		std::transform(attribute.begin(), attribute.end(), attribute.begin(), tolower);

		if (header == "m") {
			break;

		} else if (header == "c") {							// "a="
			other = "c=" + other + "\r\n";					// new line for Decode()
			VS_SIPBuffer tmp_buff(other);

			m_connection = vs::make_unique<VS_SDPField_Connection>();

			err = m_connection->Decode(tmp_buff);
			if ( err != TSIPErrorCodes::e_ok )
			{
				m_connection.reset();

				SetValid(false);
				SetError(err);
				return err;
			}

		} else if (header == "b") {
			other = "b=" + attribute + ":" + value + "\r\n";					// new line for Decode()
			VS_SIPBuffer tmp_buff(other);

			decltype(m_bandwidth) bandwidth(new VS_SDPField_Bandwidth);

			err = bandwidth->Decode(tmp_buff);
			if ( err != TSIPErrorCodes::e_ok )
			{
				_buffer.SkipHeader();
				continue;
			}
			m_bandwidth = std::move(bandwidth);

		} else if (header == "a") {							// "a="
			if (attribute == "rtpmap") {					// "a=rtpmap:"

				char* p_end;
				int pt = strtol(value.c_str(), &p_end, 10);
				if (p_end == value.c_str())
				{
					_buffer.SkipHeader();
					continue;
				}
				value.erase(0, p_end - value.c_str());

				// Decode in VS_SDPCodec
				VS_SDPRtpmapDecoder codec;
				VS_SIPBuffer tmp_buff(value + "\r\n");
				err = codec.Decode(tmp_buff);
				if ( err != TSIPErrorCodes::e_ok )
				{
					_buffer.SkipHeader();
					continue;
				}

				codec.SetMediaType(m_media_type);
				codec.SetPT(pt);

				if (codec.GetCodecType()==e_rcvSPEEX_16kHz && codec.GetClockRate()!=16000)		// we support only 16kHz for speex
				{
					_buffer.SkipHeader();
					continue;
				}

				// Create my exact codec
				auto create_codec_res = factory->CreateCodec(m_media_type, codec.GetCodecType());
				if ( (TSIPErrorCodes::e_ok != create_codec_res.error_code)
					 || !create_codec_res.p_field )
				{
					_buffer.SkipHeader();
					continue;
				}

				// Save info from base class (VS_SDPCodec)
				std::unique_ptr<VS_SDPCodec> u_p_field = std::move(create_codec_res.p_field);
				*u_p_field = static_cast<VS_SDPCodec &>(codec);

				if (codec.GetCodecType()==e_rcvG722124)		// temporary set None, then if a=fmtp: with bitrate exist, it will update the codec type
				{
					codec.SetCodecType(e_rcvNone);
				}

				m_remote_codecs.push_back(std::move(u_p_field));

			} else if (attribute == "fmtp") {	// "a=fmtp:"

				char* p_end;
				int pt = strtol(value.c_str(), &p_end, 10);
				if (p_end == value.c_str())
				{
					_buffer.SkipHeader();
					continue;
				}
				value.erase(0, p_end - value.c_str());

				m_raw_fmtp = value;
				VS_SIPBuffer tmp_buff(value + "\r\n");
				formats[pt] = tmp_buff;

			} else if (attribute == "content") {
				if      (value == "main")
					m_content = SDP_CONTENT_MAIN;
				else if (value == "slides")
					m_content = SDP_CONTENT_SLIDES;
				else
					m_content = SDP_CONTENT_UNKNOWN;
			} else if (attribute == "label") {
				m_label = value;
			} else if (attribute == "control") {
				m_control = value;
			} else if (attribute == "rtcp-fb") {
				m_is_fir_supported = m_is_fir_supported
					|| value.find("fir") != std::string::npos
					|| value.find("src") != std::string::npos; // from here: https://msdn.microsoft.com/en-us/library/hh659616(v=office.12).aspx
															   // 'src' declares 'Video Source Request' decribed here https://msdn.microsoft.com/en-us/library/hh659630(v=office.12).aspx
			} else if (attribute == "sendrecv")
				m_direction = SDP_MEDIACHANNELDIRECTION_SENDRECV;
			else if (attribute == "sendonly")
				m_direction = SDP_MEDIACHANNELDIRECTION_SEND;
			else if (attribute == "recvonly")
				m_direction = SDP_MEDIACHANNELDIRECTION_RECV;
			else if (attribute == "inactive")
				m_direction = SDP_MEDIACHANNELDIRECTION_INACTIVE;
			else if (attribute == "setup")
			{
				if      (value == "active")
					m_setup = SDP_SETUP_ACTIVE;
				else if (value == "passive")
					m_setup = SDP_SETUP_PASSIVE;
				else if (value == "actpass")
					m_setup = SDP_SETUP_ACTPASS;
				else if (value == "holdconn")
					m_setup = SDP_SETUP_HOLDCONN;
				else
				{
					_buffer.SkipHeader();
					continue;
				}
			}
			else if (attribute == "connection")
			{
				if      (value == "new")
					m_connection_attr = SDP_CONNECTION_NEW;
				else if (value == "existing")
					m_connection_attr = SDP_CONNECTION_EXISTING;
				else
				{
					_buffer.SkipHeader();
					continue;
				}
			}
			else if (attribute == "floorctrl")
			{
				bool valid = true;
				for (auto role_it = boost::make_find_iterator(value, boost::algorithm::token_finder([](char x) { return x != ' '; }, boost::algorithm::token_compress_on)); !role_it.eof(); ++role_it)
				{
					if      (*role_it == string_view("c-only"))
						m_bfcp_floor_ctrl |= SDP_FLOORCTRL_ROLE_C_ONLY;
					else if (*role_it == string_view("s-only"))
						m_bfcp_floor_ctrl |= SDP_FLOORCTRL_ROLE_S_ONLY;
					else if (*role_it == string_view("c-s"))
						m_bfcp_floor_ctrl |= SDP_FLOORCTRL_ROLE_C_S;
					else
					{
						valid = false;
						break;
					}
				}
				if (!valid)
				{
					m_bfcp_floor_ctrl = SDP_FLOORCTRL_ROLE_INVALID;
					_buffer.SkipHeader();
					continue;
				}
			}
			else if (attribute == "confid")
			{
				m_bfcp_conf_id = atoi(value.c_str());
				if (m_bfcp_conf_id == 0 && value[0] != '0') // conversion failed
				{
					_buffer.SkipHeader();
					continue;
				}
			}
			else if (attribute == "userid")
			{
				m_bfcp_user_id = atoi(value.c_str());
				if (m_bfcp_user_id == 0 && value[0] != '0') // conversion failed
				{
					_buffer.SkipHeader();
					continue;
				}
			}
			else if (attribute == "floorid")
			{
				m_bfcp_floor_id = atoi(value.c_str());
				if (m_bfcp_floor_id == 0 && value[0] != '0') // conversion failed
				{
					_buffer.SkipHeader();
					continue;
				}

				auto label_list_start = std::string::npos;
				if      ((label_list_start = value.find("mstrm:")) != std::string::npos)
					label_list_start += strlen("mstrm:");
				else if ((label_list_start = value.find("m-stream:")) != std::string::npos)
					label_list_start += strlen("m-stream:");
				else
				{
					_buffer.SkipHeader();
					continue;
				}
				auto range = boost::make_iterator_range(value.cbegin() + label_list_start, value.cend());
				boost::algorithm::split(m_bfcp_floor_labels, range, [](char x) { return x == ' '; });
			}
			else if (attribute == "ice-ufrag") {
				m_remote_ice_ufrag = value;
			} else if (attribute == "ice-pwd") {
				m_remote_ice_pwd = value;
			} else if (attribute == "crypto") {
				boost::cmatch m;
				try
				{
					if (boost::regex_match(value.c_str(), m, e3)) {
						m_remote_crypto_key = m[2];
					}
				}
				catch (const std::runtime_error &ex)
				{
					dstream1 << "VS_SDPCodec::Decode case3 error " << ex.what() << "\n";
				}
			} else if (attribute == "accept-types") {
				boost::iter_split(m_accept_types, value, boost::first_finder(" "));
			}
		} else {
			_buffer.SkipHeader();
			continue;
		}

		_buffer.SkipHeader();
	} // while()

// if no rtpmap-header for static pt - add codec manually
	for(std::size_t i=0; i < m_his_pt_table.size(); i++)
	{
		int pt = m_his_pt_table[i];

		// skip if dynamic payload
		if (pt >= 96)
			 continue;

		// skip if already at remote codec table (exist RTPMAP-header for this codec)
		if (std::any_of(m_remote_codecs.begin(), m_remote_codecs.end(), [pt](const std::unique_ptr<VS_SDPCodec>& x) { return x->GetPT() == pt; }))
			continue;

		const auto codec_type = vs::GetCodecByStaticPayloadType(m_media_type,static_cast<eSDPPayloadType>(pt));
		if (codec_type == -1)
			continue;

		auto field = factory->CreateCodec(m_media_type, codec_type);
		if (TSIPErrorCodes::e_ok != field.error_code)
			continue;

		field.p_field->SetPT(pt);
		field.p_field->SetCodecType(codec_type);

		m_remote_codecs.push_back(std::move(field.p_field));
	}

	// codec formats
	for (auto& c: m_remote_codecs)
	{
		auto fmt_it = formats.find(c->GetPT());
		if (fmt_it != formats.end())
			c->Decode(fmt_it->second);

		if (!c->IsValid())
		{
			c.reset();
		}
	}
	m_remote_codecs.erase(std::remove(m_remote_codecs.begin(), m_remote_codecs.end(), nullptr), m_remote_codecs.end());

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPField_MediaStream::Encode(VS_SIPBuffer &aBuffer) const
{
	if (!IsValid())
		return GetLastClassError();

	bool encode_attributes = true;
	const TSIPErrorCodes err = TSIPErrorCodes::e_null;

	// Media
	switch (m_media_type)
	{
	case SDPMediaType::audio:
		aBuffer.AddData("m=audio ");
		break;

	case SDPMediaType::video:
		aBuffer.AddData("m=video ");
		break;

	case SDPMediaType::application_bfcp:
		return EncodeBFCPStream(aBuffer);

	case SDPMediaType::application_fecc:
		aBuffer.AddData("m=application ");
		encode_attributes = true;
		break;

	case SDPMediaType::application:
		aBuffer.AddData("m=application ");
		encode_attributes = false;
		break;

	case SDPMediaType::message:
		aBuffer.AddData("m=message ");
		break;

	default:
		aBuffer.AddData("m=unknown ");
		encode_attributes = false;
		break;
	}

	// Port
	net::port port = m_local_port != 0 && m_port != 0 ? m_local_port : 0;
	if (port == 0)
		encode_attributes = false;

	char thePort[std::numeric_limits<net::port>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
	snprintf(thePort, sizeof thePort, "%u", port);

	aBuffer.AddData(thePort, strlen(thePort));

	// Port Range
	{
		if (m_port_range > 1)
		{
			char thePortRange[std::numeric_limits<net::port>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
			snprintf(thePortRange, sizeof thePortRange, "%u", m_port_range);

			aBuffer.AddData("/");
			aBuffer.AddData(thePortRange, strlen(thePortRange));
		}
	}

	// Proto
	{
		switch (m_proto)
		{
		case SDP_RTPPROTO_RTP_AVP:
			aBuffer.AddData(" RTP/AVP ");
			break;

		case SDP_RTPPROTO_RTP_SAVP:
			aBuffer.AddData(" RTP/SAVP ");
			break;

		case SDP_PROTO_SIP:
			aBuffer.AddData(" sip ");
			break;

		default:
			aBuffer.AddData(" unknown ");
			encode_attributes = false;
			break;
		}
	}

	if (m_media_type == SDPMediaType::message) {
		aBuffer.AddData(m_message_url);
		aBuffer.AddData("\r\n");
		aBuffer.AddData("a=accept-types:");
		for (const auto &t : m_accept_types) {
			aBuffer.AddData(t);
			aBuffer.AddData(" ");
		}
		aBuffer.AddData("\r\n");
		return TSIPErrorCodes::e_ok;
	}

	char pt_string[std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 + 1 /*0-terminator*/] = { 0 };
	for (auto it = m_local_codecs.begin(); it != m_local_codecs.end();)
	{
		snprintf(pt_string, sizeof pt_string, "%d", (*it)->GetPT());
		aBuffer.AddData(pt_string, strlen(pt_string));

		++it;
		if (it != m_local_codecs.end())
			aBuffer.AddData(" ");
	}
	if (m_local_codecs.empty())
		aBuffer.AddData("*");

	aBuffer.AddData("\r\n");				// New Line

	if (!encode_attributes)
	{
		return TSIPErrorCodes::e_ok;
	}

	if (m_connection)
	{
		TSIPErrorCodes err = m_connection->Encode(aBuffer);
		if (err != TSIPErrorCodes::e_ok)
			return err;
	}

	if (m_bandwidth)
	{
		TSIPErrorCodes err = m_bandwidth->Encode(aBuffer);
		if (err != TSIPErrorCodes::e_ok)
			return err;
	}

	if (m_media_type == SDPMediaType::video)
	{
		if (m_content == SDP_CONTENT_MAIN)
			aBuffer.AddData("a=content:main\r\n");
		else if (m_content == SDP_CONTENT_SLIDES)
			aBuffer.AddData("a=content:slides\r\n");
	}

	if (!m_label.empty())
	{
		aBuffer.AddData("a=label:");
		aBuffer.AddData(m_label);
		aBuffer.AddData("\r\n");
	}

	if (m_ice_enabled) {
		aBuffer.AddData("a=x-ssrc-range:");
		aBuffer.AddData(std::to_string(m_our_ssrc_range.first));
		aBuffer.AddData("-");
		aBuffer.AddData(std::to_string(m_our_ssrc_range.second));
		aBuffer.AddData("\r\n");

		if (m_media_type == SDPMediaType::audio) {
			aBuffer.AddData("a=x-source:main-audio\r\n");
			aBuffer.AddData("a=ice-lite\r\n");
		}
		else if (m_media_type == SDPMediaType::video) {
			aBuffer.AddData("a=rtcp-fb:* x-message app send:src,x-pli recv:src,x-pli\r\n");
			aBuffer.AddData("a=rtcp-rsize\r\n");
			aBuffer.AddData("a=x-source:main-video\r\n");
			aBuffer.AddData("a=ice-lite\r\n");
		}

                if (!m_our_ice_ufrag.empty())
                {
                  aBuffer.AddData("a=ice-ufrag:");
                  aBuffer.AddData(m_our_ice_ufrag);
                  aBuffer.AddData("\r\n");
                }
                if (!m_our_ice_pwd.empty())
                {
                  aBuffer.AddData("a=ice-pwd:");
                  aBuffer.AddData(m_our_ice_pwd);
                  aBuffer.AddData("\r\n");
                }

		std::string candidates;

		candidates += "a=candidate:1 1 UDP 2130706431 "; // 2130706431 defines candidate priority, hardcoded for now
		candidates += m_connection->GetHost();
		candidates += " " + std::to_string(port);
		candidates += " typ host\r\n";

		candidates += "a=candidate:1 2 UDP 2130705918 ";
		candidates += m_connection->GetHost();
		candidates += " " + std::to_string(port + 1);
		candidates += " typ host\r\n";

		aBuffer.AddData(candidates);
	}

	if (m_srtp_enabled && !m_our_crypto_key.empty()) {
		std::string crypto;

		crypto += "a=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:" + m_our_crypto_key + "|2^31\r\n";

		aBuffer.AddData(crypto);
	}

	for (auto it = m_local_codecs.begin(); it != m_local_codecs.end(); ++it)
	{
		const auto pt = (*it)->GetPT();
		if (m_media_type == SDPMediaType::audio &&
			m_no_rtpmap_audio_static_pt && pt < 35) {
			continue;
		}

		if (m_media_type == SDPMediaType::video &&
			m_no_rtpmap_video_static_pt && (pt == 31 || pt == 34)) {
			continue;
		}

		if (TSIPErrorCodes::e_ok != (*it)->Encode(aBuffer))
		{
			return err;
		}
	}

	switch (m_direction)
	{
	case SDP_MEDIACHANNELDIRECTION_INVALID:
	case SDP_MEDIACHANNELDIRECTION_SENDRECV:
		aBuffer.AddData("a=sendrecv\r\n"); break;
	case SDP_MEDIACHANNELDIRECTION_SEND:
		aBuffer.AddData("a=sendonly\r\n"); break;
	case SDP_MEDIACHANNELDIRECTION_RECV:
		aBuffer.AddData("a=recvonly\r\n"); break;
	case SDP_MEDIACHANNELDIRECTION_INACTIVE:
		aBuffer.AddData("a=inactive\r\n"); break;
	}

	if (m_media_type == SDPMediaType::video)
		aBuffer.AddData("a=rtcp-fb:* ccm fir\r\n");

	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPField_MediaStream::EncodeBFCPStream(VS_SIPBuffer &aBuffer) const //rfc4583
{
	if (m_media_type != SDPMediaType::application_bfcp)
	{
		return TSIPErrorCodes::e_UNKNOWN;
	}
	std::stringstream out;
	out << "m=application " << m_local_port << " ";
	switch (m_proto)
	{
	case SDP_PROTO_TCP_BFCP:
		out << "TCP/BFCP";
		break;
	case SDP_PROTO_TCP_TLS_BFCP:
		out << "TCP/TLS/BFCP";
		break;
	case SDP_PROTO_UDP_BFCP:
		out << "UDP/BFCP";
		break;
	case SDP_PROTO_UDP_TLS_BFCP:
		out << "UDP/TLS/BFCP";
		break;
	default:
		out << "unknown";
		break;
	}
	out << " *" << "\r\n";
/// floorctrl atr;
	if (m_bfcp_floor_ctrl != SDP_FLOORCTRL_ROLE_INVALID)
	{
		out << "a=floorctrl:";
		unsigned int role = m_bfcp_floor_ctrl;
		if (role & SDP_FLOORCTRL_ROLE_C_ONLY)
		{
			role &= ~SDP_FLOORCTRL_ROLE_C_ONLY;
			out << "c-only" << (role ? " " : "");
		}
		if (role & SDP_FLOORCTRL_ROLE_S_ONLY)
		{
			role &= ~SDP_FLOORCTRL_ROLE_S_ONLY;
			out << "s-only" << (role ? " " : "");
		}
		if (role & SDP_FLOORCTRL_ROLE_C_S)
		{
			role &= ~SDP_FLOORCTRL_ROLE_C_S;
			out << "c-s" << (role ? " " : "");
		}
		out << "\r\n";
	}
	if (m_bfcp_floor_id != 0 && !m_bfcp_floor_labels.empty())
		out << "a=floorid:" << m_bfcp_floor_id << " mstrm:" << m_bfcp_floor_labels[0] << "\r\n";
/// setup attr
	out << "a=setup:";
	switch (m_setup)
	{
	case SDP_SETUP_ACTIVE:
		out << "active";
		break;
	case SDP_SETUP_PASSIVE:
		out << "passive";
		break;
	case SDP_SETUP_ACTPASS:
		out << "actpass";
		break;
	case SDP_SETUP_HOLDCONN:
		out << "holdconn";
		break;
	default:
		return TSIPErrorCodes::e_UNKNOWN;
	}
	out << "\r\n";
	//connection atr
	out << "a=connection:";
	switch (m_connection_attr)
	{
	case SDP_CONNECTION_NEW:
		out << "new";
		break;
	case SDP_CONNECTION_EXISTING:
		out << "existing";
		break;
	default:
		return TSIPErrorCodes::e_UNKNOWN;
	}
	out << "\r\n";
	if (m_bfcp_floor_ctrl & (SDP_FLOORCTRL_ROLE_S_ONLY | SDP_FLOORCTRL_ROLE_C_S))
	{
		if (m_bfcp_conf_id != 0)
			out << "a=confid:" << m_bfcp_conf_id << "\r\n";
		if (m_bfcp_user_id != 0)
			out << "a=userid:" << m_bfcp_user_id << "\r\n";
	}
	aBuffer.AddData(out.str());
	return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_BaseField> VS_SDPField_MediaStream_Instance()
{
	return vs::make_unique<VS_SDPField_MediaStream>();
}

void VS_SDPField_MediaStream::Clean() noexcept
{
	VS_SIPError::Clean();

	m_media_type = SDPMediaType::invalid;
	m_direction = SDP_MEDIACHANNELDIRECTION_INVALID;
	m_local_port = DISCARD_PROTOCOL_PORT; // Port 0 has a special meaning in SDP (disabled media stream) so we use this as a default
	m_port = DISCARD_PROTOCOL_PORT;
	m_port_range = 0;
	m_proto = SDP_RTPPROTO_INVALID;
	m_connection.reset();
	m_bandwidth.reset();
	m_content = SDP_CONTENT_INVALID;
	m_label.clear();
	m_is_fir_supported = false;
	ClearLocalCodecs();
	ClearRemoteCodecs();

	m_control.clear();
	m_raw_fmtp.clear();

	m_setup = SDP_SETUP_INVALID;
	m_connection_attr = SDP_CONNECTION_INVALID;
	m_bfcp_floor_ctrl = SDP_FLOORCTRL_ROLE_INVALID;
	m_bfcp_conf_id = 0;
	m_bfcp_user_id = 0;
	m_bfcp_floor_id = 0;
	m_bfcp_floor_labels.clear();

	m_no_rtpmap_audio_static_pt = false;
	m_no_rtpmap_video_static_pt = false;
	m_ice_enabled = false;
	m_srtp_enabled = false;
}

SDPMediaType VS_SDPField_MediaStream::GetMediaType() const
{
	return m_media_type;
}

void VS_SDPField_MediaStream::SetMediaType(const SDPMediaType _media)
{
	m_media_type = _media;
}

net::port VS_SDPField_MediaStream::GetLocalPort() const
{
	return m_local_port;
}

void VS_SDPField_MediaStream::SetLocalPort(net::port port)
{
	m_local_port = port;
}

net::port VS_SDPField_MediaStream::GetPort() const
{
	return m_port;
}

void VS_SDPField_MediaStream::SetPort(net::port port)
{
	m_port = port;
}

net::port VS_SDPField_MediaStream::GetPortRange() const
{
	return m_port_range;
}

void VS_SDPField_MediaStream::SetPortRange(net::port port_range)
{
	m_port_range = port_range;
}

eSDP_RTPPROTO VS_SDPField_MediaStream::GetProto() const
{
	return m_proto;
}

void VS_SDPField_MediaStream::SetProto(eSDP_RTPPROTO proto)
{
	m_proto = proto;
}

// for H264 returns codec with the level set to the minimal value that is supported by both sides and additional params with common values for both sides
std::unique_ptr<VS_SDPCodec> VS_SDPField_MediaStream::GetBestCodec(bool use_remote_codec, bool use_remote_pt) const
{
	std::unique_ptr<VS_SDPCodec> result;

	if (m_remote_codecs.empty())
		return result;

	for (auto lc_it = m_local_codecs.begin(); lc_it != m_local_codecs.end(); ++lc_it)
	{
		for (auto rc_it = m_remote_codecs.begin(); rc_it != m_remote_codecs.end(); ++rc_it)
		{
			// TODO: ktrushnikov: do a full compare (with all params), example: operator=()
			if ((*rc_it)->GetCodecType() == (*lc_it)->GetCodecType())
			{
				const auto& best_codec = use_remote_codec ? rc_it->get() : lc_it->get();

				if ((*rc_it)->GetMediaType() == SDPMediaType::video
					&& (*rc_it)->GetCodecType() == e_videoH264)
				{
					const VS_SDPCodecH264 *localCodec = dynamic_cast<const VS_SDPCodecH264*>(lc_it->get());
					if (!localCodec)
						break;

					const VS_SDPCodecH264 *remoteCodec = dynamic_cast<const VS_SDPCodecH264*>(rc_it->get());
					if (!remoteCodec)
						break;
					int remoteLevel = remoteCodec->Level();

					// there can be multiple codec descriptions for H264 (with different levels)
					for (auto it3 = rc_it + 1; it3 != m_remote_codecs.end(); ++it3)
						if ((*it3)->GetCodecType() == e_videoH264)
						{
							const VS_SDPCodecH264 *h264Cur = dynamic_cast<const VS_SDPCodecH264*>(it3->get());
							if (h264Cur && h264Cur->Level() > remoteLevel)
							{
								remoteLevel = h264Cur->Level();
								remoteCodec = h264Cur;
							}
						}

					const VS_SDPCodecH264 *h264 = dynamic_cast<const VS_SDPCodecH264*>(best_codec);
					if (!h264)
						break;
					result = vs::make_unique<VS_SDPCodecH264>(*h264);
					if(!use_remote_codec)
					  VS_SDPCodecH264::GetCommonCodec(*localCodec, *remoteCodec, static_cast<VS_SDPCodecH264&>(*result));
				}
				else
					result = best_codec->Clone();

				result->SetPT(use_remote_pt ? (*rc_it)->GetPT() : (*lc_it)->GetPT());
				return result;
			}
		}
	}
	assert(result == nullptr);
	return result;
}

bool VS_SDPField_MediaStream::IsRecv() const
{
	if ( m_direction == SDP_MEDIACHANNELDIRECTION_INVALID || // default direction is sendrecv
		m_direction == SDP_MEDIACHANNELDIRECTION_RECV || m_direction == SDP_MEDIACHANNELDIRECTION_SENDRECV )
		return true;
	else
		return false;
}

bool VS_SDPField_MediaStream::IsSend() const
{
	if ( m_direction == SDP_MEDIACHANNELDIRECTION_INVALID || // default direction is sendrecv
		m_direction == SDP_MEDIACHANNELDIRECTION_SEND || m_direction == SDP_MEDIACHANNELDIRECTION_SENDRECV )
		return true;
	else
		return false;
}

bool VS_SDPField_MediaStream::IsEnabled() const
{
	return m_local_port != 0 && m_port != 0;
}

eSDP_MediaChannelDirection VS_SDPField_MediaStream::GetMediaDirection() const
{
	return m_direction;
}

void VS_SDPField_MediaStream::SetMediaDirection(eSDP_MediaChannelDirection direction)
{
	m_direction = direction;
}

VS_SDPField_Connection* VS_SDPField_MediaStream::GetConnection() const
{
	return m_connection.get();
}

bool VS_SDPField_MediaStream::SetConnection(const VS_SDPField_Connection* conn)
{
	if ( !conn )
		return false;

	if (m_connection)
		m_connection->Clean();
	else
		m_connection = vs::make_unique<VS_SDPField_Connection>();

	*(m_connection) = *(conn);
	return true;
}

const std::string &VS_SDPField_MediaStream::GetHost() const
{
	if ( !m_connection ) return EMPTY_STR;
	return m_connection->GetHost();
}

void VS_SDPField_MediaStream::SetHost(std::string host)
{
	if ( !m_connection )
		return ;

	m_connection->SetHost(std::move(host));
}

void VS_SDPField_MediaStream::SetBandwidth(const unsigned int _bandwidth)
{
	if ( !m_bandwidth )
	{
		m_bandwidth.reset(new VS_SDPField_Bandwidth);
		m_bandwidth->SetValid(true);
		m_bandwidth->SetError(TSIPErrorCodes::e_ok);
		m_bandwidth->SetType(SDP_BANDWIDTH_TIAS);
	}

	m_bandwidth->SetBandwidth(_bandwidth);
}

unsigned int VS_SDPField_MediaStream::GetBandwidth() const
{
	if ( m_bandwidth )
		return m_bandwidth->GetBandwidth();

	return 0;
}

eSDP_ContentType VS_SDPField_MediaStream::GetContent() const
{
	return m_content;
}

void VS_SDPField_MediaStream::SetContent(eSDP_ContentType content)
{
	m_content = content;
}

const std::string &VS_SDPField_MediaStream::GetLabel() const
{
	return m_label;
}

void VS_SDPField_MediaStream::SetLabel(std::string label)
{
	m_label = std::move(label);
}

eSDP_Setup VS_SDPField_MediaStream::GetSetup() const
{
	return m_setup;
}

void VS_SDPField_MediaStream::SetSetup(eSDP_Setup setup)
{
	m_setup = setup;
}

eSDP_Connection VS_SDPField_MediaStream::GetConnectionAttr() const
{
	return m_connection_attr;
}

void VS_SDPField_MediaStream::SetConnectionAttr(eSDP_Connection connection)
{
	m_connection_attr = connection;
}

std::uint16_t VS_SDPField_MediaStream::GetBFCPFloorCtrl() const
{
	return m_bfcp_floor_ctrl;
}

void VS_SDPField_MediaStream::SetBFCPFloorCtrl(std::uint16_t role)
{
	m_bfcp_floor_ctrl = role;
}
bfcp::ConferenceID VS_SDPField_MediaStream::GetBFCPConfID() const
{
	return m_bfcp_conf_id;
}

void VS_SDPField_MediaStream::SetBFCPConfID(bfcp::ConferenceID conf_id)
{
	m_bfcp_conf_id = conf_id;
}

bfcp::UserID VS_SDPField_MediaStream::GetBFCPUserID() const
{
	return m_bfcp_user_id;
}

void VS_SDPField_MediaStream::SetBFCPUserID(bfcp::UserID user_id)
{
	m_bfcp_user_id = user_id;
}

bfcp::FloorID VS_SDPField_MediaStream::GetBFCPFloorID() const
{
	return m_bfcp_floor_id;
}

void VS_SDPField_MediaStream::SetBFCPFloorID(bfcp::FloorID floor_id)
{
	m_bfcp_floor_id = floor_id;
}

std::vector<std::string>& VS_SDPField_MediaStream::BFCPFloorLabels()
{
	return m_bfcp_floor_labels;
}

const std::vector<std::shared_ptr<const VS_SDPCodec>>& VS_SDPField_MediaStream::GetLocalCodecs() const
{
	return m_local_codecs;
}

void VS_SDPField_MediaStream::ClearLocalCodecs()
{
	m_local_codecs.clear();
}

void VS_SDPField_MediaStream::AddLocalCodec(std::shared_ptr<const VS_SDPCodec> codec)
{
	m_local_codecs.push_back(std::move(codec));
}

void VS_SDPField_MediaStream::CopyLocalCodecsFrom(const VS_SIPGetInfoInterface& ctx)
{
	if (m_media_type == SDPMediaType::audio)
		for (const auto& it : ctx.GetLocalAudioCodecs())
			m_local_codecs.push_back(it);

	else if (m_media_type == SDPMediaType::video)
		for (const auto& it : ctx.GetLocalVideoCodecs())
			m_local_codecs.push_back(it);

	else if (m_media_type == SDPMediaType::application_fecc)
		for (const auto& it : ctx.GetLocalDataCodecs())
			m_local_codecs.push_back(it);

	m_no_rtpmap_audio_static_pt = ctx.NoRtpmapForAudioStaticPayload();
	m_no_rtpmap_video_static_pt = ctx.NoRtpmapForVideoStaticPayload();

	m_ice_enabled = ctx.IceEnabled();
	m_srtp_enabled = ctx.SrtpEnabled();

}

const std::vector<std::unique_ptr<VS_SDPCodec>>& VS_SDPField_MediaStream::GetRemoteCodecs() const
{
	return m_remote_codecs;
}

void VS_SDPField_MediaStream::ClearRemoteCodecs()
{
	m_remote_codecs.clear();
}

void VS_SDPField_MediaStream::CopyRemoteCodecsFrom(VS_SDPField_MediaStream* ms)
{
	for (const auto& c: ms->GetRemoteCodecs())
		m_remote_codecs.push_back(c->Clone());
}

bool VS_SDPField_MediaStream::GetFIRSupport() const
{
	return m_is_fir_supported;
}

void VS_SDPField_MediaStream::SetFIRSupport(bool value)
{
	m_is_fir_supported = value;
}

const std::string &VS_SDPField_MediaStream::GetControl() const
{
	return m_control;
}

const std::string &VS_SDPField_MediaStream::GetRawFmtp() const
{
	return m_raw_fmtp;
}

#undef DEBUG_CURRENT_MODULE

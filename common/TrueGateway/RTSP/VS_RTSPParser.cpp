#include "VS_RTSPParser.h"
#include "../../SIPParserLib/VS_RTSP_Const.h"
#include "../../SIPParserLib/VS_RTSP_MetaField.h"
#include "../../SIPParserLib/VS_SDPMetaField.h"
#include "../../SIPParserLib/VS_RTSP_Session.h"
#include "../../SIPParserLib/VS_RTSP_Public.h"
#include "../../SIPParserLib/VS_RTSP_Transport.h"
#include "../../SIPParserLib/VS_RTSP_StartLine.h"
#include "../../SIPParserLib/VS_SDPField_Bandwidth.h"
#include "../../SIPParserLib/VS_SDPField_MediaStream.h"
#include "../../SIPParserLib/VS_RTSP_UserAgent.h"
#include "../../SIPParserLib/VS_RTSP_Server.h"
#include "../../SIPParserLib/VS_SDPCodec.h"
#include "../../SIPParserLib/VS_SIPAuthBasic.h"
#include "../../SIPParserLib/VS_SIPAuthDigest.h"
#include "../../SIPParserLib/VS_SIPField_Auth.h"
#include "../../tools/Server/VS_ApplicationInfo.h"
#include "net/DNSUtils/VS_DNSTools.h"
#include "std/cpplib/base64.h"
#include "std/cpplib/curl_deleters.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/utf8.h"
#include "../../TrueGateway/CallConfig/VS_CallConfigCorrector.h"
#include "../../FakeClient/VS_ConferenceInfo.h"
#include "std/VS_TransceiverInfo.h"
#include "tools/Server/VS_MediaChannelInfo.h"

#include <boost/regex.hpp>

#include <cassert>
#include <sstream>
#include "std-generic/cpplib/ignore.h"
#include <curl/curl.h>

namespace
{
	constexpr std::chrono::seconds RTSP_REQUEST_TIMEOUT(30); // 30 sec

	// RFC1808
	const boost::regex URL_RE("^"
		"(?:([A-za-z0-9.+-]+):)?" // optional scheme
		"(?://([^/#]*))?" // optional net_loc
		"([^;?#]*)", // path
		boost::regex::optimize);

	constexpr char MAIN_DIALOG[] = "main_dialog";
}

VS_RTSPParser::VS_RTSPParser()
	: m_keep_alive_period(std::chrono::seconds(60))
	, m_state(state_null)
	, m_about_to_destroy(false)
	, m_useRemoteTransceiver(!ts::UseLocalTransceiver())
{
}


VS_RTSPParser::~VS_RTSPParser(void)
{
}

bool VS_RTSPParser::IsComplete(const void *data, std::size_t sz) const
{
	VS_RTSP_Response resp;
	TSIPErrorCodes err = resp.Decode(static_cast<const char *>(data), sz);
	if (err == TSIPErrorCodes::e_EndOfBuffer)
		return false; // message so far is valid, but incomplete
	return true;
}

int VS_RTSPParser::SetRecvBuf(const void *buf, std::size_t sz, const VS_ChannelID /*channelId*/, const net::address &/*remoteAddr*/,
	net::port /*remotePort*/, const net::address &/*localAddr*/, net::port /*localPort*/)
{
	bool clearBuffer = false;
	VS_SCOPE_EXIT { if (clearBuffer) m_input_buffer.clear(); };
	if ( !m_input_buffer.empty() || !IsComplete(buf, sz))
	{
		m_input_buffer.append(static_cast<const char *>(buf), sz);
	}
	if (!m_input_buffer.empty())
	{
		if (IsComplete(m_input_buffer.data(), m_input_buffer.size()))
		{
			buf = m_input_buffer.data();
			sz = m_input_buffer.size();
			clearBuffer = true;
		} else return true;
	}

	VS_RTSP_Response resp;
	if (TSIPErrorCodes::e_ok != resp.Decode(static_cast<const char *>(buf), sz)) return false;

	if (m_state == state_null) return false;

	if (resp.GetRTSPMetaField()->iSession)
		m_info.SetSessionsID(resp.GetRTSPMetaField()->iSession->GetSessionId());

	if (!resp.GetRTSPMetaField()->iStartLine) return false;

	const int ret_code = resp.GetRTSPMetaField()->iStartLine->getReturnCode();

	if (m_info.GetRemoteUserAgent().empty())
	{
		m_info.SetRemoteUserAgent(GetUAStringFromResponse(resp));
		auto &corrector = VS_CallConfigCorrector::GetInstance();

		corrector.CorrectCallConfig(m_config, VS_CallConfig::RTSP, const_cast<char *>(m_info.GetRemoteUserAgent().c_str()));
	}

	if (ret_code == 401 || ret_code == 407)
	{
		const auto auth_attempts = m_info.GetAuthAttempts();
		if (auth_attempts < 2 && EnableAuthorization(resp))
		{
			m_info.SetAuthAttempts(auth_attempts + 1);
			RetryCurrentAction(resp, buf, sz);
		}
		else
			Reject();
		return true;
	}
	else
		m_info.SetAuthAttempts(0);

	switch (ret_code)
	{
		case 200: ProcessNextAction(resp, buf, sz); break;
	default:
		if (m_state == state_playing)
			break; // Ignore all error responses to ping requests (OPTIONS or GET_PARAMETER).
		Reject();
	}

	return true;
}

void VS_RTSPParser::Reject()
{
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;
	confMethods->InviteReplay(string_view{ MAIN_DIALOG, sizeof(MAIN_DIALOG) - 1 }, e_call_rejected, false, {}, {});
	m_state = state_null;
	const char empty_buff[] = "";
	RetryCurrentAction(vs::ignore<VS_RTSP_Response>{}, empty_buff, sizeof(empty_buff));
}

void VS_RTSPParser::ProcessNextAction(const VS_RTSP_Response& resp, const void* buf, std::size_t sz)
{
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;
	switch (m_state)
	{
		case state_null:
			m_info.SetUserAgent( VS_TRUECONF_WS_DISPLAY_NAME );
			m_info.SetUrl( m_to );
			m_info.SetCSeq( 1 );
			m_info.SetAccept("application/sdp");
		break;

		case state_options:
			if (resp.GetRTSPMetaField()->iPublic)
				m_info.SetSupportedCommands(resp.GetRTSPMetaField()->iPublic->GetValue());
			break;

		case state_describe:
			if (resp.GetSDPPMetaField() == nullptr) { Reject(); return; };
			FillMediaChannelsBySDP(resp.GetSDPPMetaField());
			m_sdp.Decode(static_cast<const char *>(buf), sz);
			confMethods->SetMediaChannels(string_view{ MAIN_DIALOG, sizeof(MAIN_DIALOG) - 1 }, m_media_channels, {});
		break;

		case state_wait_for_media_channels:
			if (std::any_of(m_media_channels.begin(), m_media_channels.end(), [](const VS_MediaChannelInfo& x) { return x.our_rtp_address.port() == 0; }))
			{
				Reject();
				return;
			}
		break;

		case state_setup_audio:
		{
			UpdateKeepAlivePeriod(resp);

			auto audio_channel = AudioChannel();
			if (!audio_channel)
				break;
			if (resp.GetRTSPMetaField()->iTransport)
			{
				auto port = resp.GetRTSPMetaField()->iTransport->GetRemotePort();
				audio_channel->remote_rtp_address.port(port);
				audio_channel->remote_rtcp_address.port(port + 1);
			}
			confMethods->SetMediaChannels(string_view{ MAIN_DIALOG, sizeof(MAIN_DIALOG) - 1 }, m_media_channels, {});
		}
		break;

		case state_setup_video:
		{
			UpdateKeepAlivePeriod(resp);

			auto video_channel = VideoChannel();
			if (!video_channel)
				break;
			if (resp.GetRTSPMetaField()->iTransport)
			{
				auto port = resp.GetRTSPMetaField()->iTransport->GetRemotePort();
				video_channel->remote_rtp_address.port(port);
				video_channel->remote_rtcp_address.port(port + 1);
			}
			confMethods->SetMediaChannels(string_view{ MAIN_DIALOG, sizeof(MAIN_DIALOG) - 1 }, m_media_channels, {});
		}
		break;

		case state_playing:
			return;
	}

	if (m_state == state_teardown) m_state = state_null;
	else
	{
		++((int&)m_state);
	}

	RetryCurrentAction(resp, buf, sz);
}

bool VS_RTSPParser::EnableAuthorization(VS_RTSP_Response &rsp)
{
	if (m_config.Login.empty() || m_config.Password.empty())
		return false;

	if (!rsp.GetRTSPMetaField() || !rsp.GetRTSPMetaField()->iAuth || !rsp.GetRTSPMetaField()->iAuth->GetAuthInfo())
		return false;

	m_info.SetUser(m_config.Login);
	m_info.SetPassword(m_config.Password);

	const auto auth_info = rsp.GetRTSPMetaField()->iAuth->GetAuthInfo();
	std::shared_ptr<VS_SIPAuthScheme> scheme;
	if (std::dynamic_pointer_cast<VS_SIPAuthDigest>(auth_info))
		scheme = std::make_shared<VS_SIPAuthDigest>();
	else
		scheme = std::make_shared<VS_SIPAuthBasic>();
	scheme->AddInfo(*auth_info);
	scheme->login(m_info.GetUser());
	scheme->password(m_info.GetPassword());
	m_info.SetAuthScheme(scheme);
	return true;
}

void VS_RTSPParser::RetryCurrentAction(const VS_RTSP_Response &resp, const void* buf, std::size_t sz)
{
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;

	switch (m_state)
	{
		case state_options:
			if (!Send_Options())
			{
				Reject();
				return;
			};
			break;

		case state_describe:
			if (!Send_Describe())
			{
				Reject();
				return;
			};
			break;

		case state_setup_audio:
			if (!Send_SetupAudio(resp)) ProcessNextAction(resp, buf, sz);
			break;
		case state_setup_video:
			if (!Send_SetupVideo(resp)) ProcessNextAction(resp, buf, sz);
			break;

		case state_play:
			if (!Send_Play(resp))
			{
				Reject();
				return;
			};
			break;

		case state_playing:
			confMethods->InviteReplay(string_view{ MAIN_DIALOG, sizeof(MAIN_DIALOG) - 1 }, e_call_ok, false, {}, {});
			return;

		case state_teardown:
			if (!Send_Teardown())
			{
				ProcessNextAction(resp, buf, sz);
			};
			break;

		case state_null:
			m_start_logout_time = clock().now();
			confMethods->Logout(string_view{ MAIN_DIALOG, sizeof(MAIN_DIALOG) - 1 });
			break;
	}
}

void VS_RTSPParser::UpdateKeepAlivePeriod(const VS_RTSP_Response &rsp)
{
	if (rsp.GetRTSPMetaField() && rsp.GetRTSPMetaField()->iSession)
	{
		const auto timeout = rsp.GetRTSPMetaField()->iSession->GetTimeout();
		if (timeout > std::chrono::seconds(2))
			m_keep_alive_period = std::chrono::seconds(timeout - std::chrono::seconds(2));
		else if (timeout > std::chrono::seconds(0))
			m_keep_alive_period = std::chrono::seconds(timeout) * 9 / 10;
	}
}

bool VS_RTSPParser::Send_Teardown()
{
	if (m_info.GetSessionsID().empty()) return false;
	std::lock_guard<decltype(m_lock_request_for_send)> _(m_lock_request_for_send);
	return m_request_for_send.MakeTeardown( &m_info ) ;
}

bool VS_RTSPParser::Send_Describe()
{
	std::lock_guard<decltype(m_lock_request_for_send)> _(m_lock_request_for_send);
	return m_request_for_send.MakeDescribe( &m_info ) ;
}

bool VS_RTSPParser::Send_Options()
{
	std::lock_guard<decltype(m_lock_request_for_send)> _(m_lock_request_for_send);
	return m_request_for_send.MakeOptions( &m_info ) ;
}

bool VS_RTSPParser::Send_GetParameter()
{
	std::lock_guard<decltype(m_lock_request_for_send)> _(m_lock_request_for_send);
	return m_request_for_send.MakeGetParameter(&m_info);
}

bool VS_RTSPParser::Send_SetupAudio(const VS_RTSP_Response& resp)
{
	return Send_SetupMedia(resp, AudioChannel());
}

bool VS_RTSPParser::Send_SetupVideo(const VS_RTSP_Response& resp)
{
	return Send_SetupMedia(resp, VideoChannel());
}

bool VS_RTSPParser::Send_SetupMedia(const VS_RTSP_Response& resp, VS_MediaChannelInfo* channel)
{
	if (!channel) return false;

	if (channel->index >= m_sdp.GetSDPPMetaField()->iMediaStreams.size()) return false;
	auto ms = m_sdp.GetSDPPMetaField()->iMediaStreams[channel->index];
	if (!ms) return false;

	// from here: https://tools.ietf.org/html/rfc2326#page-58
	/*server SHOULD not allow a client to direct media streams to an
	  address that differs from the address commands are coming
      from

	  So we can be banned if destination media address not equal rtsp signaling address
	  or we must path through some authentication
	 */
	m_info.SetEndpoint(channel->our_rtp_address);
	m_info.UseRemoteTransceiver(m_useRemoteTransceiver);

	boost::smatch cu_match;
	boost::smatch u_match;
	if (boost::regex_search(ms->GetControl(), cu_match, URL_RE)
	 && boost::regex_search(m_info.GetUrl(), u_match, URL_RE))
	{
		assert(cu_match.size() >= 4);
		assert(u_match.size() >= 4);
		std::stringstream control_url;

		// Add scheme
		if (cu_match[1].length() > 0)
			control_url << cu_match[1];
		else if (u_match[1].length() > 0)
			control_url << u_match[1];
		else
			control_url << "rtsp";
		control_url << ':';

		// Add net_loc if present
		if (cu_match[2].length() > 0)
			control_url << "//" << cu_match[2];
		else if (u_match[2].length() > 0)
			control_url << "//" << u_match[2];

		// Prepend path from main url in case of relative control url
		if (cu_match[3].length() == 0 || *cu_match[3].first != '/')
		{
			// TODO: Resolve possible ".." in control url
			if (u_match[3].length() > 0)
				control_url << u_match[3];
			if (u_match[3].length() == 0 || *(u_match[3].second-1) != '/')
				control_url << '/';
		}

		// Add path and parameters/query/fragment from control url
		control_url << cu_match[3] << cu_match.suffix();
		m_info.SetControlUrl(control_url.str());
	}
	else
		// Can't parse url, will use main url as control url
		m_info.SetControlUrl(m_info.GetUrl());

	std::lock_guard<decltype(m_lock_request_for_send)> _(m_lock_request_for_send);
	return m_request_for_send.MakeSetup( &m_info );
}

bool VS_RTSPParser::Send_Play(const VS_RTSP_Response &resp)
{
	if (m_media_channels.empty()) return false;

	std::lock_guard<decltype(m_lock_request_for_send)> _(m_lock_request_for_send);
	return m_request_for_send.MakePlay( &m_info );
}


int VS_RTSPParser::GetBufForSend(void* buf, std::size_t &sz, const VS_ChannelID channelId, const net::address &remoteAddr, net::port remotePort, const net::address &localAddr, net::port localPort)
{
	std::lock_guard<decltype(m_lock_request_for_send)> lock(m_lock_request_for_send);
	if (!m_request_for_send.IsValid()) return false;
	size_t sz_tmp = sz;
	TSIPErrorCodes code = m_request_for_send.Encode((char *)buf, sz_tmp);
	sz = sz_tmp;
	if (code == TSIPErrorCodes::e_EndOfBuffer)
		return false;
	m_request_for_send.SetValid(false);
	m_last_message_time = clock().now();
	return code == TSIPErrorCodes::e_ok;
}

std::string VS_RTSPParser::NewDialogID(string_view sipTo, string_view dtmf, const VS_CallConfig &config, string_view myName)
{
	m_config = config;
	return std::string(MAIN_DIALOG, sizeof(MAIN_DIALOG) - 1);
}

bool VS_RTSPParser::InviteMethod(string_view dialogId, string_view fromId, string_view toId, const VS_ConferenceInfo &cfgInfo,
	string_view dnFromUTF8, bool newSession, bool forceCreate)
{
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return false;

	if (m_state != state_null) Reject();

	m_from = std::string(fromId);
	std::unique_ptr<CURL, CURL_deleter> curl(::curl_easy_init());
	assert(curl);

	int len = 0;
	std::unique_ptr<char[], curl_free_deleter> unescaped(::curl_easy_unescape(curl.get(), toId.data() + 1, toId.length() - 1, &len));

	if (len > 33 && unescaped[len - 33] == '/')
	{
		len = len - 33;
	}
	m_to = std::string(unescaped.get(), len);

	confMethods->UpdateDisplayName(string_view{ MAIN_DIALOG, sizeof(MAIN_DIALOG) - 1 }, m_to, false);

	m_start_request_time = clock().now();
	VS_RTSP_Response tmp;
	const char empty_buff[] = "";
	ProcessNextAction(tmp, empty_buff, sizeof(empty_buff));
	return true;
}

void VS_RTSPParser::Hangup(string_view /*dialoId*/)
{
	if (m_state != state_teardown && m_state != state_null)
	{
		m_start_request_time = clock().now();
		const char empty_buff[] = "";
		m_state = state_teardown;
		RetryCurrentAction(VS_RTSP_Response(), empty_buff, sizeof(empty_buff));
	}
}

acs::Response VS_RTSPParser::Protocol(const void */*buf*/, std::size_t /*sz*/)
{
	return acs::Response::not_my_connection ;
}

void VS_RTSPParser::Timeout()
{
	const auto time_elapsed = clock().now() - m_start_request_time;
	if (m_state != state_null && m_state != state_playing && time_elapsed > RTSP_REQUEST_TIMEOUT)
	{
		Reject();
	}

	if (m_state == state_playing && !m_request_for_send.IsValid() && clock().now() - m_last_message_time > m_keep_alive_period)
	{
		std::lock_guard<decltype(m_lock_request_for_send)> _{ m_lock_request_for_send };
		if (m_info.IsCommandSupported(REQUEST_GET_PARAMETER))
			Send_GetParameter();
		else
			Send_Options();
	}

	if (m_start_logout_time != decltype(m_start_logout_time)() && (clock().now() - m_start_logout_time > std::chrono::seconds(10)))
	{
		this->LoggedOutAsUser(string_view{ MAIN_DIALOG, sizeof(MAIN_DIALOG) - 1 });
		m_start_logout_time = {};
	}
}

void VS_RTSPParser::Shutdown()
{
}

VS_MediaChannelInfo* VS_RTSPParser::AudioChannel()
{
	auto audio_channel_it = std::find_if(m_media_channels.begin(), m_media_channels.end(), [](const VS_MediaChannelInfo& x) { return x.type == SDPMediaType::audio; });
	return audio_channel_it != m_media_channels.end() ? &*audio_channel_it : nullptr;
}

VS_MediaChannelInfo* VS_RTSPParser::VideoChannel()
{
	auto video_channel_it = std::find_if(m_media_channels.begin(), m_media_channels.end(), [](const VS_MediaChannelInfo& x) { return x.type == SDPMediaType::video; });
	return video_channel_it != m_media_channels.end() ? &*video_channel_it : nullptr;
}

bool VS_RTSPParser::SetMediaChannels(string_view /*dialogId*/, const std::vector<VS_MediaChannelInfo>& channels, const std::string& /*existingConfID*/, std::int32_t /*bandwRcv*/)
{
	for (const auto& channel: channels)
	{
		auto our_channel_it = std::find_if(m_media_channels.begin(), m_media_channels.end(), [&channel](const VS_MediaChannelInfo& x) { return x.index == channel.index; });
		if (our_channel_it == m_media_channels.end())
			continue;

		our_channel_it->our_rtp_address.address(m_my_media_ip);
		our_channel_it->our_rtp_address.port(channel.our_rtp_address.port());
		our_channel_it->our_rtcp_address.address(m_my_media_ip);
		our_channel_it->our_rtcp_address.port(channel.our_rtp_address.port());
	}
	if (m_state == state_wait_for_media_channels)
	{
		VS_RTSP_Response tmp;
		const char empty_buff[] = "";
		ProcessNextAction(tmp, empty_buff, sizeof(empty_buff));
	}
	return true;
}

bool VS_RTSPParser::FillMediaChannels(string_view /*dialogId*/, std::vector<VS_MediaChannelInfo>& channels)
{
	channels = m_media_channels;
	return true;
}

bool VS_RTSPParser::FillMediaChannelsBySDP(VS_SDPMetaField *sdp)
{
	if (sdp->iMediaStreams.empty()) return false;

	const auto remote_address = net::dns_tools::single_make_a_aaaa_lookup(m_config.HostName);
	if (remote_address.is_unspecified())
		return false;

	const auto sdp_bw = (sdp->iBandwidth)? sdp->iBandwidth->GetBandwidth(): 0;

	for (std::size_t i = 0; i < sdp->iMediaStreams.size(); ++i)
	{
		auto ms = sdp->iMediaStreams[i];
		if (!ms) continue;

		if (ms->GetRemoteCodecs().empty())
			continue;

		SDPMediaType media_type = ms->GetMediaType();
		if (media_type == SDPMediaType::audio)
		{
			if (AudioChannel())
				continue;

			auto audio_channel = m_media_channels.emplace(m_media_channels.end(), i);
			audio_channel->type = media_type;
			audio_channel->content = SDP_CONTENT_MAIN;
			audio_channel->direction = SDP_MEDIACHANNELDIRECTION_RECV;
			audio_channel->remote_rtp_address.address(remote_address);
			audio_channel->remote_rtp_address.port(0);
			audio_channel->remote_rtcp_address = audio_channel->remote_rtp_address;

			for (auto& codec: ms->GetRemoteCodecs())
			{
				VS_GatewayAudioMode tmpAudio;
				tmpAudio.PayloadType = codec->GetPT();
				tmpAudio.CodecType = codec->GetCodecType();
				tmpAudio.ClockRate = codec->GetClockRate();
				codec->FillRcvAudioMode(tmpAudio);
				audio_channel->rcv_modes_audio.push_back(tmpAudio);
			}
		}
		else if (media_type == SDPMediaType::video)
		{
			if (VideoChannel())
				continue;

			auto video_channel = m_media_channels.emplace(m_media_channels.end(), i);
			video_channel->type = media_type;
			video_channel->content = SDP_CONTENT_MAIN;
			video_channel->direction = SDP_MEDIACHANNELDIRECTION_RECV;
			video_channel->remote_rtp_address.address(remote_address);
			video_channel->remote_rtp_address.port(0);
			video_channel->remote_rtcp_address = video_channel->remote_rtp_address;

			for (auto& codec : ms->GetRemoteCodecs())
			{
				VS_GatewayVideoMode tmpVideo;
				tmpVideo.PayloadType = codec->GetPT();
				tmpVideo.CodecType = codec->GetCodecType();
				tmpVideo.ClockRate = codec->GetClockRate();
				codec->FillSndVideoMode(tmpVideo);

				if (tmpVideo.CodecType == e_videoH264)
				{
					const char *pos = strstr(ms->GetRawFmtp().c_str(), "sprop-parameter-sets=");
					if (pos != NULL)
					{
						int ipos = 0;
						pos += 21;
						const char* const sps = pos;
						for (; *pos != ',' && *pos != 0 && *pos != '\r'; ++pos)
							;
						const size_t sps_sz = pos - sps;

						pos++;
						const char* const pps = pos;
						for (; *pos != 0 && *pos != '\r' && *pos != ';'; ++pos)
							;
						const size_t pps_sz = pos - pps;

						size_t sz;
						//TODO: check return value
						base64_decode(sps, sps_sz, tmpVideo.SequenceParameterSet + 12, sz = VS_GatewayVideoMode::MAX_NAL_UNIT_SIZE - 12);
						tmpVideo.sizeOfSPS = sz;
						base64_decode(pps, pps_sz, tmpVideo.PictureParameterSet  + 12, sz = VS_GatewayVideoMode::MAX_NAL_UNIT_SIZE - 12);
						tmpVideo.sizeOfPPS = sz;

						tmpVideo.sizeOfSPS += 12; // Add size of RTP headers
						tmpVideo.sizeOfPPS += 12;
						memset(tmpVideo.SequenceParameterSet, 0, 12);			// write fake RTP header
						memset(tmpVideo.PictureParameterSet,  0, 12);
						// set RTP header ver = 2, padding = false, marker = fase
						tmpVideo.SequenceParameterSet[0] = tmpVideo.PictureParameterSet[0] = static_cast<char>(0x80);
						// set RTP Payload equal to video stream payload
						tmpVideo.SequenceParameterSet[1] = tmpVideo.PictureParameterSet[1] = tmpVideo.PayloadType;
					}
				}

				unsigned int ms_bw = ms->GetBandwidth();
				const auto bw = (sdp_bw && ms_bw) ? std::min(sdp_bw, ms_bw) : (ms_bw) ? ms_bw : sdp_bw;
				if (bw)
					tmpVideo.Bitrate = bw;
				else
					tmpVideo.Bitrate = 768 * 1024;

				tmpVideo.InitializeNAT = true;
				video_channel->rcv_modes_video.push_back(tmpVideo);
			}
		}
	}
	return !m_media_channels.empty();
}

// RTSP distinguish server and client side in the way different from SIP:
// there is no such thing as "User-Agent" field in server responses but "Server" field instead.
// It is aimed for the same purposes (though may have somewhat different value, like, i.e. in VLC).
std::string VS_RTSPParser::GetUAStringFromResponse(VS_RTSP_Response &rsp)
{
	auto *meta_field = rsp.GetRTSPMetaField();

	if (meta_field == nullptr)
	{
		return std::string();
	}

	if (meta_field->iServer != nullptr)
	{
		return meta_field->iServer->Get();
	}
	else if (meta_field->iUserAgent != nullptr)
	{
		return meta_field->iUserAgent->GetUAString();
	}

	return std::string();
}

void VS_RTSPParser::LoggedOutAsUser(string_view /*dialogId*/)
{
	m_start_logout_time = {};
	m_fireDialogFinished(string_view{ MAIN_DIALOG, sizeof(MAIN_DIALOG) - 1 });
}
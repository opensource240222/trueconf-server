#pragma once

#include <boost/asio/ip/udp.hpp>
#include "tools/Server/CommonTypes.h"

struct VS_MediaChannelInfo
{
	unsigned int index;
	SDPMediaType type;
	eSDP_ContentType content;
	eSDP_MediaChannelDirection direction;

	boost::asio::ip::udp::endpoint our_rtp_address;
	boost::asio::ip::udp::endpoint our_rtcp_address;
	boost::asio::ip::udp::endpoint remote_rtp_address;
	boost::asio::ip::udp::endpoint remote_rtcp_address;

	std::vector<VS_GatewayAudioMode> rcv_modes_audio;
	VS_GatewayAudioMode snd_mode_audio;
	std::vector<VS_GatewayVideoMode> rcv_modes_video;
	VS_GatewayVideoMode snd_mode_video;
	std::vector<VS_GatewayDataMode> rcv_modes_data;
	VS_GatewayDataMode snd_mode_data;

	std::string our_ice_pwd, remote_ice_pwd;
	std::string our_srtp_key, remote_srtp_key;
	std::pair<uint32_t, uint32_t> ssrc_range;

	explicit VS_MediaChannelInfo(unsigned int index_)
		: index(index_)
		, type(SDPMediaType::invalid)
		, content(SDP_CONTENT_INVALID)
		, direction(SDP_MEDIACHANNELDIRECTION_INACTIVE)
	{
	}

	bool Serialize(VS_Container& cnt, string_view name) const;
	bool Deserialize(const VS_Container& cnt, const char* name);

	bool IsRecv() const
	{
		return direction == SDP_MEDIACHANNELDIRECTION_RECV || direction == SDP_MEDIACHANNELDIRECTION_SENDRECV;
	}

	bool IsSend() const
	{
		return direction == SDP_MEDIACHANNELDIRECTION_SEND || direction == SDP_MEDIACHANNELDIRECTION_SENDRECV;
	}

	bool HasModes() const
	{
		switch (type)
		{
		case SDPMediaType::audio:
			return (IsRecv() && !rcv_modes_audio.empty())
				|| (IsSend() && snd_mode_audio.PayloadType != 0);
		case SDPMediaType::video:
			return (IsRecv() && !rcv_modes_video.empty())
				|| (IsSend() && snd_mode_video.PayloadType != 0);
		default:
			return false;
		}
	}

	bool operator==(const VS_MediaChannelInfo& other) const
	{
		return index == other.index
			&& type == other.type
			&& content == other.content
			&& direction == other.direction
			&& our_rtp_address == other.our_rtp_address
			&& our_rtcp_address == other.our_rtcp_address
			&& remote_rtp_address == other.remote_rtp_address
			&& remote_rtcp_address == other.remote_rtcp_address
			&& rcv_modes_audio == other.rcv_modes_audio
			&& snd_mode_audio == other.snd_mode_audio
			&& rcv_modes_video == other.rcv_modes_video
			&& snd_mode_video == other.snd_mode_video
			&& rcv_modes_data == other.rcv_modes_data
			&& snd_mode_data == other.snd_mode_data
			&& our_ice_pwd == other.our_ice_pwd
			&& remote_ice_pwd == other.remote_ice_pwd
			&& our_srtp_key == other.our_srtp_key
			&& remote_srtp_key == other.remote_srtp_key
			&& ssrc_range == other.ssrc_range;
	}

};

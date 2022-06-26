#include "VS_MediaChannelInfo.h"


bool Serialize_EndpointAddress(VS_Container& cnt, string_view name, const boost::asio::ip::udp::endpoint &value)
{
	VS_Container sub_cnt;
	boost::asio::ip::address addr = value.address();

	if (addr.is_v4()) {
		if (!sub_cnt.AddValue("ipv4", static_cast<int32_t>(addr.to_v4().to_ulong())))
			return false;
	}
	else if (addr.is_v6()) {
		auto ipv6Bytes = addr.to_v6().to_bytes();
		if (!sub_cnt.AddValue("ipv6", ipv6Bytes.data(), ipv6Bytes.size()))
			return false;
	}

	if (!sub_cnt.AddValue("port", static_cast<int32_t>(value.port())))
		return false;
	return cnt.AddValue(name, sub_cnt);
}

bool Deserialize_EndpointAddress(const VS_Container& cnt, const char* name, boost::asio::ip::udp::endpoint& value)
{
	VS_Container sub_cnt;
	if (!(name ? cnt.GetValue(name, sub_cnt) : cnt.GetValue(sub_cnt)))
		return false;

	int32_t ipv4_value(0);
	boost::asio::ip::address_v6::bytes_type ipv6_value = {};
	size_t ipv6_size = ipv6_value.size();
	if (sub_cnt.GetValue("ipv4", ipv4_value))
		value.address(boost::asio::ip::address_v4(static_cast<uint32_t>(ipv4_value)));
	else if (sub_cnt.GetValue("ipv6", ipv6_value.data(), ipv6_size))
		value.address(boost::asio::ip::address_v6(ipv6_value));

	int32_t port_value(0);
	if (!sub_cnt.GetValue("port", port_value))
		return false;
	value.port(port_value);
	return true;
}

bool VS_MediaChannelInfo::Serialize(VS_Container& cnt, string_view name) const
{
	VS_Container sub_cnt;
	if (!sub_cnt.AddValueI32("index", index)
		|| !sub_cnt.AddValueI32("type", type)
		|| !sub_cnt.AddValueI32("content", content)
		|| !sub_cnt.AddValueI32("direction", direction)
		|| !sub_cnt.AddValue("our_ice_pwd", our_ice_pwd)
		|| !sub_cnt.AddValue("remote_ice_pwd", remote_ice_pwd)
		|| !sub_cnt.AddValue("our_srtp_key", our_srtp_key)
		|| !sub_cnt.AddValue("remote_srtp_key", remote_srtp_key)
		|| !sub_cnt.AddValueI32("ssrc_range_min", ssrc_range.first)
		|| !sub_cnt.AddValueI32("ssrc_range_max", ssrc_range.second)
		|| !Serialize_EndpointAddress(sub_cnt, "our_rtp_address", our_rtp_address)
		|| !Serialize_EndpointAddress(sub_cnt, "our_rtcp_address", our_rtcp_address)
		|| !Serialize_EndpointAddress(sub_cnt, "remote_rtp_address", remote_rtp_address)
		|| !Serialize_EndpointAddress(sub_cnt, "remote_rtcp_address", remote_rtcp_address)
		|| !snd_mode_audio.Serialize(sub_cnt, "snd_mode_audio")
		|| !snd_mode_video.Serialize(sub_cnt, "snd_mode_video")
		|| !snd_mode_data.Serialize(sub_cnt, "snd_mode_data")
		)
		return false;
	for (const auto& rcv_mode_audio : rcv_modes_audio)
		if (!rcv_mode_audio.Serialize(sub_cnt, "rcv_mode_audio"))
			return false;
	for (const auto& rcv_mode_video : rcv_modes_video)
		if (!rcv_mode_video.Serialize(sub_cnt, "rcv_mode_video"))
			return false;
	for (const auto& rcv_mode_data : rcv_modes_data)
		if (!rcv_mode_data.Serialize(sub_cnt, "rcv_mode_data"))
			return false;
	return cnt.AddValue(name, sub_cnt);
}

bool VS_MediaChannelInfo::Deserialize(const VS_Container& cnt, const char* name)
{
	VS_Container sub_cnt;
	if (!(name ? cnt.GetValue(name, sub_cnt) : cnt.GetValue(sub_cnt)))
		return false;

	uint32_t ssrc_range_min, ssrc_range_max;
	if (!sub_cnt.GetValueI32("index", index)
		|| !sub_cnt.GetValueI32("type", type)
		|| !sub_cnt.GetValueI32("content", content)
		|| !sub_cnt.GetValueI32("direction", direction)
		|| !sub_cnt.GetValue("our_ice_pwd", our_ice_pwd)
		|| !sub_cnt.GetValue("remote_ice_pwd", remote_ice_pwd)
		|| !sub_cnt.GetValue("our_srtp_key", our_srtp_key)
		|| !sub_cnt.GetValue("remote_srtp_key", remote_srtp_key)
		|| !sub_cnt.GetValueI32("ssrc_range_min", ssrc_range_min)
		|| !sub_cnt.GetValueI32("ssrc_range_max", ssrc_range_max)
		|| !Deserialize_EndpointAddress(sub_cnt, "our_rtp_address", our_rtp_address)
		|| !Deserialize_EndpointAddress(sub_cnt, "our_rtcp_address", our_rtcp_address)
		|| !Deserialize_EndpointAddress(sub_cnt, "remote_rtp_address", remote_rtp_address)
		|| !Deserialize_EndpointAddress(sub_cnt, "remote_rtcp_address", remote_rtcp_address)
		|| !snd_mode_audio.Deserialize(sub_cnt, "snd_mode_audio")
		|| !snd_mode_video.Deserialize(sub_cnt, "snd_mode_video")
		|| !snd_mode_data.Deserialize(sub_cnt, "snd_mode_data")
		)
		return false;
	ssrc_range = { ssrc_range_min, ssrc_range_max };

	sub_cnt.Reset();
	while (sub_cnt.Next())
	{
		if (strcmp(sub_cnt.GetName(), "rcv_mode_audio") == 0)
		{
			VS_GatewayAudioMode mode;
			if (!mode.Deserialize(sub_cnt, nullptr))
				return false;
			rcv_modes_audio.push_back(mode);
		}
		else if (strcmp(sub_cnt.GetName(), "rcv_mode_video") == 0)
		{
			VS_GatewayVideoMode mode;
			if (!mode.Deserialize(sub_cnt, nullptr))
				return false;
			rcv_modes_video.push_back(mode);
		}
		else if (strcmp(sub_cnt.GetName(), "rcv_mode_data") == 0)
		{
			VS_GatewayDataMode mode;
			if (!mode.Deserialize(sub_cnt, nullptr))
				return false;
			rcv_modes_data.push_back(mode);
		}
	}
	return true;
}
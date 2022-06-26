#include <gtest/gtest.h>

#include "tools/Server/CommonTypes.h"
#include "tools/Server/VS_MediaChannelInfo.h"

TEST(MediaChannelInfo, Serializing) {
	VS_MediaChannelInfo info(1);
	info.content = eSDP_ContentType::SDP_CONTENT_MAIN;
	info.direction = eSDP_MediaChannelDirection::SDP_MEDIACHANNELDIRECTION_SENDRECV;
	info.our_ice_pwd = "out_ice_pwd";
	info.our_rtcp_address = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("1.2.3.4"), 1235);
	info.our_rtp_address = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("1.2.3.4"), 1234);
	info.our_srtp_key = "our_srtp_key";
	info.rcv_modes_audio.emplace_back();
	info.rcv_modes_data.emplace_back();
	info.rcv_modes_video.emplace_back();
	info.remote_ice_pwd = "remote_ice_pwd";
	info.remote_rtcp_address = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("4.3.2.1"), 5321);
	info.remote_rtp_address = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("4.3.2.1"), 5320);
	info.remote_srtp_key = "remote_srtp_key";
	info.snd_mode_audio = VS_GatewayAudioMode();
	info.snd_mode_video = VS_GatewayVideoMode();
	info.snd_mode_data = VS_GatewayDataMode();
	info.ssrc_range = std::make_pair(1, 100);
	info.type = SDPMediaType::video;

	VS_Container cnt;
	EXPECT_TRUE(info.Serialize(cnt, "test_info"));

	VS_MediaChannelInfo info1(1);
	EXPECT_TRUE(info1.Deserialize(cnt, "test_info"));

	info1.rcv_modes_data = info.rcv_modes_data;	// data is not present in Serialize/Deserialize functions
	EXPECT_EQ(info, info1);
}
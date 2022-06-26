#include "VS_RTPSessionTest.h"

#include "TransceiverCircuit/VS_RTPModuleParameters.h"
#include "TransceiverCircuit/VS_FFLSourceCollection.h"

#include <boost/make_shared.hpp>
#include "gmock/gmock-spec-builders.h"
#include "tools/Server/CommonTypes.h"
#include "tools/Server/VS_MediaChannelInfo.h"
#include "tests/common/ASIOEnvironment.h"
#include "std/cpplib/MakeShared.h"

void RTPSessionTest::SetUp()
{
	my_collection = vs::MakeShared<VS_FFLSourceCollection>(g_asio_environment->IOService(), nullptr);
}

void RTPSessionTest::TearDown()
{
}

VS_MediaChannelInfo MakeSRTPVideoChannel(int channel_id, int PayloadType) {
	VS_MediaChannelInfo video_channel(channel_id);
	video_channel.our_srtp_key = "/t5VTZA0FeqNQfhDvRxHhFRdkS+uHm9ngWv3P9sb";
	video_channel.remote_srtp_key = "zJ0pR2TwpBSVWNBiUtJq987yl78IBemS0oVxtmQP";
	video_channel.our_ice_pwd = "0q1YW88xT7p0pV0LpoCtDg";
	video_channel.remote_ice_pwd = "RFNRfWT4UWaO+Ka0ivn64AM/";
	video_channel.type = SDPMediaType::video;
	video_channel.direction = SDP_MEDIACHANNELDIRECTION_SENDRECV;
	video_channel.content = SDP_CONTENT_MAIN;

	VS_GatewayVideoMode m;
	m.CodecType = e_videoXH264UC;
	m.PayloadType = PayloadType;
	video_channel.rcv_modes_video.push_back(m);
	return video_channel;
}

VS_MediaChannelInfo MakeSlidesChannel(int channel_id, int PayloadType, const bool FIRSupported = false) {
	VS_MediaChannelInfo slides_channel(channel_id);
	slides_channel.type = SDPMediaType::video;
	slides_channel.direction = SDP_MEDIACHANNELDIRECTION_SENDRECV;
	slides_channel.content = SDP_CONTENT_SLIDES;

	VS_GatewayVideoMode m;
	m.CodecType = e_videoH264;
	m.PayloadType = PayloadType;
	m.IsFIRSupported = FIRSupported;
	slides_channel.rcv_modes_video.push_back(m);
	return slides_channel;
}

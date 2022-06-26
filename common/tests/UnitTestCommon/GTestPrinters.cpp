#include "GTestPrinters.h"
#include "tools/Server/VS_MediaChannelInfo.h"
#include "tools/Server/CommonTypes.h"
#include "std-generic/cpplib/string_view.h"

#include <gtest/gtest-printers.h>

void PrintTo(const VS_GatewayAudioMode& x, std::ostream* s)
{
	*s << "{acodec=" << x.CodecType << ", pt=" << x.PayloadType << "}";
}

void PrintTo(const VS_GatewayVideoMode& x, std::ostream* s)
{
	*s << "{vcodec=" << x.CodecType << ", pt=" << x.PayloadType << "}";
}

void PrintTo(const boost::asio::ip::udp::endpoint & x, std::ostream * s)
{
	*s << '{' << x << '}';
}

void PrintTo(const VS_MediaChannelInfo& x, std::ostream* s)
{
	*s << "{";
	if (x.type == SDPMediaType::audio)
		*s << "type=audio";
	else if (x.type == SDPMediaType::video)
		*s << "type=audio";
	else
		*s << "type=" << static_cast<int>(x.type);
	*s << ", our_rtp="; PrintTo(x.our_rtp_address, s);
	*s << ", our_rtcp="; PrintTo(x.our_rtcp_address, s);
	*s << ", remote_rtp="; PrintTo(x.remote_rtp_address, s);
	*s << ", remote_rtcp="; PrintTo(x.remote_rtcp_address, s);
	if (x.type == SDPMediaType::audio)
	{
		*s << ", rcv_modes=" << ::testing::PrintToString(x.rcv_modes_audio);
		*s << ", snd_mode="; PrintTo(x.snd_mode_audio, s);
	}
	else if (x.type == SDPMediaType::video)
	{
		*s << ", rcv_modes=" << ::testing::PrintToString(x.rcv_modes_video);
		*s << ", snd_mode="; PrintTo(x.snd_mode_video, s);
	}
	*s << "}";
}

void PrintTo(const basic_string_view<char, std::char_traits<char>>& x, std::ostream* s)
{
	*s << '"' << x << '"';
}

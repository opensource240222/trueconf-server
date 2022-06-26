#include "gtest/gtest.h"
#include "tools/SingleGatewayLib/VS_RTPMediaChannels.h"

TEST(RTPChannel, BuildRTPChannelTest) //TODO:FIXME(this test on demand KT)
{
	boost::asio::io_service ios;
	boost::asio::io_service::strand strand{ ios };
	VS_RTPMediaChannels channels(ios);
	auto &&ch = channels.Get(1, true);
	ch->SetSSRCRange({1, 100});
}

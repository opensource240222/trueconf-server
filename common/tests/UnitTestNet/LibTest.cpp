#include "net/Lib.h"

#include <gtest/gtest.h>

namespace net_test {

TEST(IsRecoverableUDPReadError, OptimizationCorrectness)
{
	EXPECT_EQ(make_error_code(boost::asio::error::host_unreachable   ).category(), boost::system::system_category());
	EXPECT_EQ(make_error_code(boost::asio::error::connection_aborted ).category(), boost::system::system_category());
	EXPECT_EQ(make_error_code(boost::asio::error::connection_refused ).category(), boost::system::system_category());
	EXPECT_EQ(make_error_code(boost::asio::error::connection_reset   ).category(), boost::system::system_category());
	EXPECT_EQ(make_error_code(boost::asio::error::message_size       ).category(), boost::system::system_category());
	EXPECT_EQ(make_error_code(boost::asio::error::network_reset      ).category(), boost::system::system_category());
	EXPECT_EQ(make_error_code(boost::asio::error::network_unreachable).category(), boost::system::system_category());
}

}

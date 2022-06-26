#include "net/Connect.h"
#include "tests/common/ASIOEnvironment.h"
#include "tests/common/TestHelpers.h"
#include "tests/mocks/asio_protocol_mock.h"
#include "tests/mocks/asio_resolver_mock.h"
#include "tests/mocks/asio_socket_mock.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace net_test {

TEST(Connect, HostName_Success)
{
	using ::testing::_;
	using ::testing::AllOf;
	using ::testing::Invoke;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Property;
	using ::testing::StrEq;
	using ::testing::Sequence;

	using protocol = test::asio::tcp_mock;
	using resolver = protocol::resolver;
	using socket = protocol::socket;

	const protocol::endpoint eps[] = {
		{ boost::asio::ip::address_v4(0x12344321), 4242 },
		{ boost::asio::ip::address_v4(0x87655678), 1313 },
	};

	resolver::on_new([&](resolver* r) {
		EXPECT_CALL(*r, async_resolve_mocked_fwd(AllOf(
				Property(&resolver::query::host_name, StrEq("server-name.tld")),
				Property(&resolver::query::service_name, StrEq("my_srv"))
			)))
			.Times(1)
			.WillOnce(Invoke([&, r](const resolver::query& q) {
				r->complete_resolve(
#if BOOST_VERSION < 106600
					resolver::iterator::create(std::begin(eps), std::end(eps), q.host_name(), q.service_name())
#else
					resolver::results_type::create(std::begin(eps), std::end(eps), q.host_name(), q.service_name())
#endif
				);
			}));
	});

	socket::impl::on_new([&](socket::impl* s) {
		Sequence seq;
		EXPECT_CALL(*s, async_connect(eps[0]))
			.Times(1).InSequence(seq)
			.WillOnce(InvokeWithoutArgs([s]() {
				s->complete_connect(boost::asio::error::host_unreachable);
			}));
		EXPECT_CALL(*s, async_connect(eps[1]))
			.Times(1).InSequence(seq)
			.WillOnce(InvokeWithoutArgs([s]() {
				s->complete_connect();
			}));
	});

	vs::event done(false);
	net::Connect<protocol>(g_asio_environment->IOService(), "server-name.tld", "my_srv", [&](const boost::system::error_code& ec, socket&&) {
		EXPECT_FALSE(ec) << ec.message();
		done.set();
	});
	EXPECT_TRUE(test::WaitFor("Call to callback", done));
}

}

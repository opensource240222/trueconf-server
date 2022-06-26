#include "HttpHandler.h"
#include "tests/common/TestHelpers.h"
#include "std-generic/cpplib/string_view.h"
#include "std/cpplib/latch.h"
#include "std/cpplib/VS_Replace.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/md5.h"

//#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <memory>
#include "std-generic/cpplib/ThreadUtils.h"

std::string HttpHandlerTest::connect_send_read(string_view in)
{
	boost::asio::ip::tcp::socket s(g_asio_environment->IOService());
	boost::system::error_code ec;
	s.connect({ ADDR_FOR_TEST, PORT_FOR_TEST }, ec);
	if (ec)
		return {};
	auto bytes_transfered = boost::asio::write(s, boost::asio::buffer(in.begin(), in.length()), ec);
	if (bytes_transfered != in.length())
		return {};
	char reply[4096] = { 0 };
	auto reply_length = s.read_some(boost::asio::buffer(reply), ec);
	if (ec)
		return {};
	return std::string(reply, reply_length);
};

std::string HttpHandlerTest::get_str_to_hash(string_view sv, const int N)
{
	auto pos = sv.find_first_of('/');
	if (pos == decltype(sv)::npos)
		return {};
	sv.remove_prefix(pos);	pos = 0;
	for (int i = 2; i <= N; ++i)
	{
		pos = sv.find_first_of('/', ++pos);
		if (pos == decltype(sv)::npos)
			return {};
	}
	sv.remove_suffix(sv.length() - pos);
	return (std::string)sv;
}

TEST_F(HttpHandlerTest, RouterMonitor)
{
	using ::testing::AtLeast;

	std::string in_transport_router("GET /rm/tr?auth=8455DF5E77E05BC2562E1D833A07C60A HTTP/1.1\r\n\r\n");
	string_view in_acs("GET /acs/tr\r\n\r\n");
	string_view in_stream_router("GET /rm/sr\r\n\r\n");

	std::vector<string_view> invalid_queries;
	invalid_queries.reserve(6);
	invalid_queries.emplace_back("GET /rm/tr? HTTP/1.1\r\n\r\n");
	invalid_queries.emplace_back("GET /rm/tr?auth= HTTP/1.1\r\n\r\n");
	invalid_queries.emplace_back("GET /rm/tr?auth=qwerywqrowqeriquweriquweyrw HTTP/1.1\r\n\r\n");
	invalid_queries.emplace_back("GET /rm/tr?auth=**sign HTTP/1.1\r\n\r\n");
	invalid_queries.emplace_back("GET /rm/tr?auth=*ts*sign HTTP/1.1\r\n\r\n");
	invalid_queries.emplace_back("GET /rm/tr?auth=rand*qwer*sign HTTP/1.1\r\n\r\n");

	auto endpoint = std::make_shared<transport_test::EndpointMock>("4AE6D6ACEF52A8F6D7D60BCD935F9CD4");
	EXPECT_CALL(*endpoint, GetId()).Times(AtLeast(1));
	router_transport->TEST_AddEndpoint(endpoint);

	char sess_id[] = { "123456789" };
	string_view random("ABCDEF12345");
	auto curr_time = std::chrono::system_clock::now().time_since_epoch();
	auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(curr_time + std::chrono::seconds(900)).count();

	std::string secret;
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	key.SetString(sess_id, SESSIONID_TAG_NAME);
	key.GetString(secret, SESSIONID_TAG_NAME);

	// make auth format: [RAND]*[TIMESTAMP]*[SIGN]
	// [SIGN] = md5(RAND + TIMESTAMP + SECRET)
	// SECRET - value CFG\Session Id
	MD5 md5;
	md5.Update(random);
	md5.Update(std::to_string(timestamp));
	md5.Update(secret);
	md5.Final();
	char sign[33];
	md5.GetString(sign);

	std::string auth(random);
	auth.append("*");
	auth.append(std::to_string(timestamp));
	auth.append("*");
	auth.append(sign);

	//check correct GET query
	VS_ReplaceAll(in_transport_router, "8455DF5E77E05BC2562E1D833A07C60A", auth);
	auto reply = connect_send_read(in_transport_router);
	ASSERT_TRUE(reply.find("200 OK")!=std::string::npos);

	//check case-sensetive param name
	VS_ReplaceAll(in_transport_router, "auth", "AUTH");
	reply = connect_send_read(in_transport_router);
	ASSERT_TRUE(reply.find("200 OK") != std::string::npos);

	md5.Reset();
	md5.Update(random);
	timestamp = std::chrono::duration_cast<std::chrono::seconds>(curr_time - std::chrono::seconds(1800)).count();
	md5.Update(std::to_string(timestamp));
	md5.Update(secret);
	md5.Final();
	md5.GetString(sign);

	std::string invalid_auth(random);
	invalid_auth.append("*");
	invalid_auth.append(std::to_string(timestamp));
	invalid_auth.append("*");
	invalid_auth.append(sign);

	//check incorrect timestamp
	VS_ReplaceAll(in_transport_router, auth, invalid_auth);
	reply = connect_send_read(in_transport_router);
	ASSERT_TRUE(reply.find("400 Bad request") != std::string::npos);

	for (auto query : invalid_queries)
	{
		reply = connect_send_read(query);
		ASSERT_TRUE(reply.find("400 Bad request") != std::string::npos);
	}
}

TEST_F(HttpHandlerTest, UsersStatuses)
{
	// test invalid http request
	auto reply = connect_send_read("GET /s4/\r\n\r\n");
	ASSERT_TRUE(reply.find("HTTP/1.1 400 Bad request\r\n") != string_view::npos) << "malformed request, server should answer 400 Bad Request";

	// test with old timestamp
	reply = connect_send_read(raw_s4);
	ASSERT_TRUE(reply.find("HTTP/1.1 400 Bad request\r\n") != string_view::npos) << "old invalid timestamp was not checked by server";

	// replace timestamp with now + 15mins
	std::string new_timestamp(raw_s4);
	auto t = (std::chrono::system_clock::now() + std::chrono::minutes(15)).time_since_epoch();
	VS_ReplaceAll(new_timestamp, "1525893763", std::to_string(std::chrono::duration_cast<std::chrono::seconds>(t).count()));
	reply = connect_send_read(new_timestamp);
	ASSERT_TRUE(reply.find("HTTP/1.1 400 Bad request\r\n") != string_view::npos) << "invalid sign was treated as valid";

	// recalc sign
	std::string new_sign(new_timestamp);
	auto to_hash = get_str_to_hash(new_sign, 3);
	ASSERT_TRUE(!to_hash.empty());
	char recalc1_hash[33] = { 0 };
	VS_ConvertToMD5(to_hash, recalc1_hash);
	VS_ReplaceAll(new_sign, "8455DF5E77E05BC2562E1D833A07C60A", recalc1_hash);
	reply = connect_send_read(new_sign);
	ASSERT_TRUE(reply.find("vp_status({\"vasya\":1})") != string_view::npos) << "should return same user:status as in FakeUsersStatuses";

	// add status security to registry and try with old status_security value (should fail)
	std::string new_status_security("blablabla");
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	key.SetString(new_status_security.c_str(), STATUS_KEY_TAG_NAME);
	reply = connect_send_read(new_sign);
	ASSERT_TRUE(reply.find("HTTP/1.1 400 Bad request\r\n") != string_view::npos) << "invalid sign with new status_security was treated as valid";

	// recalc sign with new_status_security
	decltype(recalc1_hash) recalc2_hash = { 0 };
	to_hash += new_status_security;
	VS_ConvertToMD5(to_hash, recalc2_hash);
	VS_ReplaceAll(new_sign, recalc1_hash, recalc2_hash);
	reply = connect_send_read(new_sign);
	ASSERT_TRUE(reply.find("HTTP/1.1 200 Ok\r\n") != string_view::npos) << "new_status_security failed, so server did not check it from registry";

	// test custom callback
	std::string new_callback(new_sign);
	VS_ReplaceAll(new_callback, "callback=vp_status", "callback=ktrushnikov");
	reply = connect_send_read(new_callback);
	ASSERT_TRUE(reply.find("ktrushnikov({\"vasya\":1})") != string_view::npos) << "custom name of callback was not returned (or incorrect statuses)";

	std::string s2 = raw_s2;
	VS_ReplaceAll(s2, "1527702299", std::to_string(std::chrono::duration_cast<std::chrono::seconds>(t).count()));
	to_hash = get_str_to_hash(s2, 4);
	ASSERT_TRUE(!to_hash.empty());
	to_hash += new_status_security;
	VS_ConvertToMD5(to_hash, recalc1_hash);
	VS_ReplaceAll(s2, "E087F840D74D9619A8005A327509187B", recalc1_hash);
	reply = connect_send_read(s2);
	ASSERT_TRUE(reply.find("vp_status({\"#guest:e257cf92\":-127})") != string_view::npos) << "/s2/ request failed";
}

TEST_F(HttpHandlerTest, TorrentAnnounce)
{
	auto reply = connect_send_read(raw_torrent_announce);
	ASSERT_STREQ(reply.c_str(), raw_torrent_announce) << "torrent announce was not processed";
}

TEST_F(HttpHandlerTest, AcsTestAttemptStr)
{
	auto t1 = std::chrono::steady_clock::now();
	auto reply = connect_send_read(raw_AcsTestAttemptStr);
	auto t2 = std::chrono::steady_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count();
	ASSERT_TRUE(diff <= 2) << "acs should just close connection, so our blocking read will fail, but unblock";
}

TEST_F(HttpHandlerTest, FromWebConfig)
{
	string_view sv(raw_from_web_config);
	auto pos = sv.find(http::handlers::HeadersEnd);
	if (pos != string_view::npos)
		sv.remove_prefix(pos + http::handlers::HeadersEnd.length());
	std::string content = (std::string) sv;
	EXPECT_CALL(web_config_slot, Slot_SendConfiguratorCommand(::testing::StrEq(content), content.length())).Times(1);
	auto reply = connect_send_read(raw_from_web_config);
	ASSERT_TRUE(reply.find("HTTP/1.1 200 Ok\r\n") != string_view::npos) << "called WebConfigSlot::Slot_SendConfiguratorCommand as VS_PushDataSignalSlot";
}

void HttpHandlerTest::prepare_ddos_request(std::string& s2)
{
	s2 = raw_s2;
	auto t = (std::chrono::system_clock::now() + std::chrono::minutes(15)).time_since_epoch();
	VS_ReplaceAll(s2, "1527702299", std::to_string(std::chrono::duration_cast<std::chrono::seconds>(t).count()));
	auto to_hash = get_str_to_hash(s2, 4);
	ASSERT_TRUE(!to_hash.empty());
	std::string status_security;
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	if (key.GetString(status_security, STATUS_KEY_TAG_NAME))
		to_hash += status_security;
	char recalc1_hash[33] = { 0 };
	VS_ConvertToMD5(to_hash, recalc1_hash);
	VS_ReplaceAll(s2, "E087F840D74D9619A8005A327509187B", recalc1_hash);
}

bool connect_send(string_view in, uint32_t N)
{
	std::vector<boost::asio::ip::tcp::socket> v;
	vs::latch write_completion(N);
	for (uint32_t i = 0; i < N; ++i)
	{
		v.emplace_back(g_asio_environment->IOService());
		boost::system::error_code ec;
		v[i].connect({ ADDR_FOR_TEST, PORT_FOR_TEST }, ec);
		if (ec)
			return false;
		boost::asio::async_write(v[i], boost::asio::buffer(in.begin(), in.length()), [&](const boost::system::error_code& /*ec*/, size_t /*bytes_transferred*/) {
			write_completion.count_down(1);
		});
	}
	write_completion.wait();
	return true;
};


TEST_F(HttpHandlerTest, DDos_less_than_limit)
{
	auto N = http::MAX_HTTP_CONCURRENT_PROCESSING_REQUEST;
	std::string s2;
	prepare_ddos_request(s2);
	fake_users_statuses->start_processing.reset();
	ASSERT_TRUE(connect_send(s2, N));
	fake_users_statuses->start_processing.set();
	ASSERT_TRUE(test::WaitFor("Request handling", [&]() {
		return fake_users_statuses->processed_requests == N;
	}, 10, 1000)) << "We should be able to process all requests, because we have not reached the limit of concurrent not finished requests.";
}

TEST_F(HttpHandlerTest, DDos_more_than_limit)
{
	auto N = 2 * http::MAX_HTTP_CONCURRENT_PROCESSING_REQUEST;
	std::string s2;
	prepare_ddos_request(s2);
	fake_users_statuses->start_processing.reset();
	ASSERT_TRUE(connect_send(s2, N));
	vs::SleepFor(N * std::chrono::milliseconds(5)); // some time to let the ACS handle incoming connections
	fake_users_statuses->start_processing.set();
	vs::SleepFor(N * std::chrono::milliseconds(5)); // some time to allow HttpHandler to process requests it should have dropped
	EXPECT_EQ(fake_users_statuses->processed_requests, http::MAX_HTTP_CONCURRENT_PROCESSING_REQUEST);
	// If the check above will start to fail, replace it with these two:
	// EXPECT_GE(fake_users_statuses->processed_requests, http::MAX_HTTP_CONCURRENT_PROCESSING_REQUEST) << "we should be able to process at least this amount of requests";
	// EXPECT_LT(fake_users_statuses->processed_requests, N) << "we should not process all rquests, because of limit";
}
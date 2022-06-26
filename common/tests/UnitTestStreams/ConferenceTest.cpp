#include "BufferMock.h"
#include "ConferencesConditionsMock.h"
#include "RouterInternalMock.h"
#include "tests/common/ASIOEnvironment.h"
#include "tests/common/GTestMatchers.h"
#include "tests/common/TestHelpers.h"
#include "tests/fakes/asio_socket_fake.h"
#include "streams_v2/Router/Conference.h"
#include "streams_v2/Router/Conference_impl.h"
#include "std/cpplib/event.h"
#include "std/cpplib/MakeShared.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

#include "std-generic/compat/memory.h"

namespace streams_test {

static const auto max_silence = std::chrono::hours(24); // Very large to ensure RouterConference::Timer() will not interfere with debugging.

static const char conf_name[] = "Test Conference";
static const char part_name_1[] = "Participant1";
static const char part_name_2[] = "ParticipantTwo";
static const char part_name_3[] = "OtherParticipant";
static const auto conference_type = VS_Conference_Type::CT_MULTISTREAM;

static const unsigned char serialized_handshake_reply[] = {
	'_', 'V', 'S', '_', 'S', 'T', 'R', 'E', 'A', 'M', 'S', '_', 0, 0, 0, 0,
	0x41, // version=1, head_cksum
	0xf9, // head_cksum, body_cksum,
	0x0e, // body_cksum, body_length
	0x00, // body_length
	0x01,
	0xff,
};

static const stream::FrameHeader frame_1_header { 15/*length*/, 1234/*tick_count*/, stream::Track::audio, 0xf2/*cksum*/ };
static const unsigned char frame_1_body[] = {
	'F', 'a', 'k', 'e', ' ', 'F', 'r', 'a', 'm', 'e', ' ', 'D', 'a', 't', 'a',
};

static const stream::FrameHeader frame_2_header { 16/*length*/, 5678/*tick_count*/, stream::Track::video, 0xac/*cksum*/ };
static const unsigned char frame_2_body[] = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
};

using Conference = stream::RouterConference<test::asio::tcp_socket_fake>;
using FrameReceiverMock = ::testing::MockFunction<void(const char*, const char*, const stream::FrameHeader*, const void*)>;

struct ConferenceTest : public ::testing::Test
{
	ConferenceTest()
		: strand(g_asio_environment->IOService())
		, total_read_bytes(0)
		, total_write_bytes(0)
		, done(false)
	{
	}

	void StartConference()
	{
		using ::testing::_;
		using ::testing::AnyNumber;
		using ::testing::Invoke;
		using ::testing::Return;
		using ::testing::StrEq;

		EXPECT_CALL(router, GetCCS())
			.Times(AnyNumber())
			.WillRepeatedly(Return(&ccs));
		EXPECT_CALL(router, NotifyRead(_))
			.Times(AnyNumber())
			.WillRepeatedly(Invoke([this](size_t bytes) { total_read_bytes += bytes; }));
		EXPECT_CALL(router, NotifyWrite(_))
			.Times(AnyNumber())
			.WillRepeatedly(Invoke([this](size_t bytes) { total_write_bytes += bytes; }));
		EXPECT_CALL(router, Timer(_)).Times(AnyNumber());
		EXPECT_CALL(ccs, CreateConference(StrEq(conf_name))).Times(1);
		conf = vs::MakeShared<Conference>(strand, &router, conf_name, conference_type, nullptr, false, max_silence);
		conf->Start();
	}

	void ConnectToFrameSink()
	{
		fr_conn = conf->ConnectToFrameSink([this] (const char* conf_name, const char* part_name, const stream::FrameHeader* header, const void* data) { frame_receiver.Call(conf_name, part_name, header, data); });
	}

	::testing::AssertionResult StopConference()
	{
		using ::testing::_;
		using ::testing::InvokeWithoutArgs;
		using ::testing::StrEq;

		EXPECT_CALL(ccs, RemoveConference(StrEq(conf_name), _)).Times(1)
			.WillOnce(InvokeWithoutArgs([&]() { done.set(); }));
		conf->Stop();
		return test::WaitFor("Conference stop", done);
	}

	bool AddParticipant(string_view name)
	{
		using ::testing::Eq;
		using ::testing::StrEq;

		EXPECT_CALL(ccs, AddParticipant(StrEq(conf_name), Eq(name))).Times(1);
		return conf->AddParticipant(name, nullptr, false, max_silence);
	}

	bool AddParticipant(string_view name, BufferMock*& buffer, ::testing::Sequence& seq)
	{
		using ::testing::_;
		using ::testing::Eq;
		using ::testing::Return;
		using ::testing::StrEq;

		buffer = new BufferMock;
		EXPECT_CALL(*buffer, Init(Eq(conf_name), Eq(name)))
			.Times(1).InSequence(seq)
			.WillOnce(Return(true));
		EXPECT_CALL(*buffer, SetParticipantStatisticsInterface(_))
			.Times(1).InSequence(seq);
		EXPECT_CALL(ccs, AddParticipant(StrEq(conf_name), Eq(name))).Times(1);
		return conf->AddParticipant(name, buffer, false, max_silence);
	}

	boost::asio::io_service::strand strand;
	RouterInternalMock router;
	ConferencesConditionsMock ccs;
	FrameReceiverMock frame_receiver;
	boost::signals2::scoped_connection fr_conn;
	std::shared_ptr<Conference> conf;
	size_t total_read_bytes;
	size_t total_write_bytes;
	vs::event done;
};

TEST_F(ConferenceTest, AddParticipant)
{
	StartConference();
	EXPECT_EQ(0u, conf->GetParticipantCount());

	EXPECT_TRUE(AddParticipant(part_name_1));
	EXPECT_EQ(1u, conf->GetParticipantCount());

	EXPECT_TRUE(StopConference());
}

TEST_F(ConferenceTest, AddParticipant_Existing)
{
	using ::testing::StrEq;

	StartConference();
	EXPECT_EQ(0u, conf->GetParticipantCount());

	EXPECT_TRUE(AddParticipant(part_name_1));
	EXPECT_EQ(1u, conf->GetParticipantCount());

	EXPECT_CALL(ccs, AddParticipant(StrEq(conf_name), StrEq(part_name_1))).Times(0);
	EXPECT_FALSE(conf->AddParticipant(part_name_1, nullptr, false, max_silence));
	EXPECT_EQ(1u, conf->GetParticipantCount());

	EXPECT_TRUE(StopConference());
}

TEST_F(ConferenceTest, AddParticipant_SelfConnected)
{
	using ::testing::Eq;
	using ::testing::StrEq;

	StartConference();
	EXPECT_EQ(0u, conf->GetParticipantCount());

	EXPECT_CALL(ccs, AddParticipant(StrEq(conf_name), StrEq(part_name_1))).Times(1);
	stream::Track tracks[] = { stream::Track::audio };
	EXPECT_TRUE(conf->AddParticipant(part_name_1, part_name_1, nullptr, false, max_silence, tracks, sizeof(tracks)/sizeof(tracks[0])));
	EXPECT_EQ(1u, conf->GetParticipantCount());

	EXPECT_TRUE(StopConference());
}

TEST_F(ConferenceTest, RemoveParticipant)
{
	using ::testing::_;
	using ::testing::StrEq;

	StartConference();
	EXPECT_EQ(0u, conf->GetParticipantCount());

	EXPECT_TRUE(AddParticipant(part_name_1));
	EXPECT_EQ(1u, conf->GetParticipantCount());

	EXPECT_CALL(ccs, RemoveParticipant(StrEq(conf_name), StrEq(part_name_1), _)).Times(1);
	conf->RemoveParticipant(part_name_1);
	EXPECT_EQ(0u, conf->GetParticipantCount());

	EXPECT_TRUE(StopConference());
}

TEST_F(ConferenceTest, HandshakeReply_SenderConnection)
{
	using ::testing::_;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	StartConference();

	BufferMock* buffer = nullptr;
	Sequence seq;
	EXPECT_TRUE(AddParticipant(part_name_1, buffer, seq));

	test::asio::tcp_socket_fake socket(g_asio_environment->IOService());
	auto socket_fake = socket.impl_.get();
	socket_fake->get_state().is_open() = true;

	EXPECT_CALL(*buffer, GetFrame(_,_,_,_))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
			return stream::Buffer::Status::non_fatal;
		}));
	uint8_t mtracks[32] = { 0x02 }; // audio
	conf->SetParticipantConnection(part_name_1, std::move(socket), stream::ClientType::sender, mtracks);

	EXPECT_TRUE(test::WaitFor("Last GetFrame() call", done));
	EXPECT_THAT(socket_fake->get_state().write_data(), ElementsAreArray(serialized_handshake_reply));
	EXPECT_EQ(total_write_bytes, sizeof(serialized_handshake_reply));

	EXPECT_TRUE(StopConference());
}

TEST_F(ConferenceTest, HandshakeReply_ReceiverConnection)
{
	using ::testing::_;
	using ::testing::ElementsAreArray;

	StartConference();

	EXPECT_TRUE(AddParticipant(part_name_1));

	test::asio::tcp_socket_fake socket(g_asio_environment->IOService());
	auto socket_fake = socket.impl_.get();
	socket_fake->get_state().is_open() = true;

	uint8_t mtracks[32] = { 0x02 }; // audio
	conf->SetParticipantConnection(part_name_1, std::move(socket), stream::ClientType::receiver, mtracks);

	EXPECT_THAT(socket_fake->get_state().write_data(), ElementsAreArray(serialized_handshake_reply));
	// Can't check if NotifyWrite was called correctly because there is no easy way to wait for that call
	// EXPECT_EQ(total_write_bytes, sizeof(serialized_handshake_reply));

	EXPECT_TRUE(StopConference());
}

TEST_F(ConferenceTest, ReadFrame)
{
	using ::testing::AllOf;
	using ::testing::ElementsAreArray;
	using ::testing::Field;
	using ::testing::InvokeWithoutArgs;
	using ::testing::StrEq;

	StartConference();
	ConnectToFrameSink();

	EXPECT_TRUE(AddParticipant(part_name_1));

	test::asio::tcp_socket_fake socket(g_asio_environment->IOService());
	auto socket_fake = socket.impl_.get();
	socket_fake->get_state().is_open() = true;
	socket_fake->get_state()
		.add_read_data(frame_1_header)
		.add_read_data(frame_1_body, sizeof(frame_1_body));

	EXPECT_CALL(frame_receiver, Call(
			StrEq(conf_name),
			StrEq(part_name_1),
			AllOf(
				Field(&stream::FrameHeader::length, frame_1_header.length),
				Field(&stream::FrameHeader::tick_count, frame_1_header.tick_count),
				Field(&stream::FrameHeader::track, frame_1_header.track),
				Field(&stream::FrameHeader::cksum, frame_1_header.cksum)
			),
			AsArray<unsigned char>(sizeof(frame_1_body), ElementsAreArray(frame_1_body))
		))
		.Times(1)
		.WillOnce(InvokeWithoutArgs([&]() { done.set(); }));
	uint8_t mtracks[32] = { 0x02 }; // audio
	conf->SetParticipantConnection(part_name_1, std::move(socket), stream::ClientType::receiver, mtracks);

	EXPECT_TRUE(test::WaitFor("FrameReceived signal", done));
	EXPECT_EQ(total_read_bytes, sizeof(frame_1_header) + sizeof(frame_1_body));

	EXPECT_TRUE(StopConference());
}

TEST_F(ConferenceTest, ReadFrame_Invalid)
{
	using ::testing::_;

	StartConference();
	ConnectToFrameSink();

	EXPECT_TRUE(AddParticipant(part_name_1));

	test::asio::tcp_socket_fake socket(g_asio_environment->IOService());
	auto socket_fake = socket.impl_.get();
	socket_fake->get_state().is_open() = true;
	auto frame_header_bad = frame_1_header;
	frame_header_bad.cksum = 0;
	socket_fake->get_state()
		.add_read_data(frame_header_bad)
		.add_read_data(frame_1_body, sizeof(frame_1_body));

	EXPECT_CALL(frame_receiver, Call(_,_,_,_)).Times(0);
	uint8_t mtracks[32] = { 0x02 }; // audio
	conf->SetParticipantConnection(part_name_1, std::move(socket), stream::ClientType::receiver, mtracks);

	EXPECT_TRUE(test::WaitFor("socket.close()", [&]() {
		return socket_fake->get_state().is_open() == false;
	}));
	EXPECT_EQ(total_read_bytes, sizeof(frame_1_header) + sizeof(frame_1_body));

	EXPECT_TRUE(StopConference());
}

TEST_F(ConferenceTest, ReadFrame_DifferentTrack)
{
	using ::testing::_;

	StartConference();
	ConnectToFrameSink();

	EXPECT_TRUE(AddParticipant(part_name_1));

	test::asio::tcp_socket_fake socket(g_asio_environment->IOService());
	auto socket_fake = socket.impl_.get();
	socket_fake->get_state().is_open() = true;
	socket_fake->get_state()
		.add_read_data(frame_1_header)
		.add_read_data(frame_1_body, sizeof(frame_1_body));

	EXPECT_CALL(frame_receiver, Call(_,_,_,_)).Times(0);
	uint8_t mtracks[32] = { 0x04 }; // video
	conf->SetParticipantConnection(part_name_1, std::move(socket), stream::ClientType::receiver, mtracks);

	EXPECT_TRUE(test::WaitFor("socket.close()", [&]() {
		return socket_fake->get_state().is_open() == false;
	}));
	EXPECT_GE(total_read_bytes, sizeof(frame_1_header));

	EXPECT_TRUE(StopConference());
}

TEST_F(ConferenceTest, RouteFrame)
{
	using ::testing::_;
	using ::testing::AnyNumber;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	StartConference();

	EXPECT_TRUE(AddParticipant(part_name_1));

	BufferMock* buffer = nullptr;
	Sequence seq;
	EXPECT_TRUE(AddParticipant(part_name_2, buffer, seq));

	stream::Track tracks[] = { stream::Track::audio, stream::Track::video };
	EXPECT_TRUE(conf->ConnectParticipantSender(part_name_1, part_name_2, tracks, sizeof(tracks)/sizeof(tracks[0])));

	test::asio::tcp_socket_fake socket(g_asio_environment->IOService());
	auto socket_fake = socket.impl_.get();
	socket_fake->get_state().is_open() = true;
	socket_fake->get_state()
		.add_read_data(frame_1_header)
		.add_read_data(frame_1_body, sizeof(frame_1_body))
		.add_read_data(frame_2_header)
		.add_read_data(frame_2_body, sizeof(frame_2_body));

	EXPECT_CALL(*buffer, GetFrame(_,_,_,_)).Times(AnyNumber());
	EXPECT_CALL(*buffer, PutFrame_mocked(_, stream::Track::audio, _, _))
		.With(ArgsAsArray<char, 2, 3>(ElementsAreArray(frame_1_body)))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			return stream::Buffer::Status::success;
		}));
	EXPECT_CALL(*buffer, PutFrame_mocked(_, stream::Track::video, _, _))
		.With(ArgsAsArray<char, 2, 3>(ElementsAreArray(frame_2_body)))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
			return stream::Buffer::Status::success;
		}));
	uint8_t mtracks[32] = { 0x06 }; // audio, video
	conf->SetParticipantConnection(part_name_1, std::move(socket), stream::ClientType::receiver, mtracks);

	EXPECT_TRUE(test::WaitFor("PutFrame", done));
	EXPECT_EQ(total_read_bytes, sizeof(frame_1_header) + sizeof(frame_1_body) + sizeof(frame_2_header) + sizeof(frame_2_body));

	EXPECT_TRUE(StopConference());
}

TEST_F(ConferenceTest, RouteFrame_OnlyWantedTracks)
{
	using ::testing::_;
	using ::testing::AnyNumber;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	StartConference();

	EXPECT_TRUE(AddParticipant(part_name_1));

	BufferMock* buffer = nullptr;
	Sequence seq;
	EXPECT_TRUE(AddParticipant(part_name_2, buffer, seq));

	stream::Track tracks[] = { stream::Track::video };
	EXPECT_TRUE(conf->ConnectParticipantSender(part_name_1, part_name_2, tracks, sizeof(tracks)/sizeof(tracks[0])));

	test::asio::tcp_socket_fake socket(g_asio_environment->IOService());
	auto socket_fake = socket.impl_.get();
	socket_fake->get_state().is_open() = true;
	socket_fake->get_state()
		.add_read_data(frame_1_header)
		.add_read_data(frame_1_body, sizeof(frame_1_body))
		.add_read_data(frame_2_header)
		.add_read_data(frame_2_body, sizeof(frame_2_body));

	EXPECT_CALL(*buffer, GetFrame(_,_,_,_)).Times(AnyNumber());
	EXPECT_CALL(*buffer, PutFrame_mocked(_, stream::Track::audio, _, _)).Times(0);
	EXPECT_CALL(*buffer, PutFrame_mocked(_, stream::Track::video, _, _))
		.With(ArgsAsArray<char, 2, 3>(ElementsAreArray(frame_2_body)))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
			return stream::Buffer::Status::success;
		}));
	uint8_t mtracks[32] = { 0x06 }; // audio, video
	conf->SetParticipantConnection(part_name_1, std::move(socket), stream::ClientType::receiver, mtracks);

	EXPECT_TRUE(test::WaitFor("PutFrame", done));
	EXPECT_EQ(total_read_bytes, sizeof(frame_1_header) + sizeof(frame_1_body) + sizeof(frame_2_header) + sizeof(frame_2_body));

	EXPECT_TRUE(StopConference());
}

TEST_F(ConferenceTest, RouteFrame_DisconnectedReceiver)
{
	using ::testing::_;
	using ::testing::AnyNumber;
	using ::testing::Field;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	StartConference();
	ConnectToFrameSink();

	EXPECT_TRUE(AddParticipant(part_name_1));

	BufferMock* buffer = nullptr;
	Sequence seq;
	EXPECT_TRUE(AddParticipant(part_name_2, buffer, seq));

	stream::Track tracks[] = { stream::Track::audio };
	EXPECT_TRUE(conf->ConnectParticipantSender(part_name_1, part_name_2, tracks, sizeof(tracks)/sizeof(tracks[0])));
	EXPECT_TRUE(conf->DisconnectParticipantReceiver(part_name_2, part_name_1));

	test::asio::tcp_socket_fake socket(g_asio_environment->IOService());
	auto socket_fake = socket.impl_.get();
	socket_fake->get_state().is_open() = true;
	socket_fake->get_state()
		.add_read_data(frame_1_header)
		.add_read_data(frame_1_body, sizeof(frame_1_body));

	EXPECT_CALL(frame_receiver, Call(_, _, Field(&stream::FrameHeader::track, stream::Track::audio), _))
		.Times(1)
		.WillOnce(InvokeWithoutArgs([&]() { done.set(); }));

	EXPECT_CALL(*buffer, GetFrame(_,_,_,_)).Times(AnyNumber());
	EXPECT_CALL(*buffer, PutFrame_mocked(_, stream::Track::audio, _, _)).Times(0);
	uint8_t mtracks[32] = { 0x06 }; // audio, video
	conf->SetParticipantConnection(part_name_1, std::move(socket), stream::ClientType::receiver, mtracks);

	EXPECT_TRUE(test::WaitFor("Last frame receive", done));
	EXPECT_EQ(total_read_bytes, sizeof(frame_1_header) + sizeof(frame_1_body));

	EXPECT_TRUE(StopConference());
}

TEST_F(ConferenceTest, RouteFrame_MultipleReceivers)
{
	using ::testing::_;
	using ::testing::AnyNumber;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	StartConference();

	EXPECT_TRUE(AddParticipant(part_name_1));

	BufferMock* buffer_1 = nullptr;
	Sequence seq_1;
	EXPECT_TRUE(AddParticipant(part_name_2, buffer_1, seq_1));

	BufferMock* buffer_2 = nullptr;
	Sequence seq_2;
	EXPECT_TRUE(AddParticipant(part_name_3, buffer_2, seq_2));

	stream::Track tracks[] = { stream::Track::audio };
	EXPECT_TRUE(conf->ConnectParticipantSender(part_name_1, part_name_2, tracks, sizeof(tracks)/sizeof(tracks[0])));
	EXPECT_TRUE(conf->ConnectParticipantSender(part_name_1, part_name_3, tracks, sizeof(tracks)/sizeof(tracks[0])));

	test::asio::tcp_socket_fake socket(g_asio_environment->IOService());
	auto socket_fake = socket.impl_.get();
	socket_fake->get_state().is_open() = true;
	socket_fake->get_state()
		.add_read_data(frame_1_header)
		.add_read_data(frame_1_body, sizeof(frame_1_body));

	EXPECT_CALL(*buffer_1, GetFrame(_,_,_,_)).Times(AnyNumber());
	EXPECT_CALL(*buffer_1, PutFrame_mocked(_, stream::Track::audio, _, _))
		.With(ArgsAsArray<char, 2, 3>(ElementsAreArray(frame_1_body)))
		.Times(1).InSequence(seq_1)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
			return stream::Buffer::Status::success;
		}));
	vs::event done_2(false);
	EXPECT_CALL(*buffer_2, GetFrame(_,_,_,_)).Times(AnyNumber());
	EXPECT_CALL(*buffer_2, PutFrame_mocked(_, stream::Track::audio, _, _))
		.With(ArgsAsArray<char, 2, 3>(ElementsAreArray(frame_1_body)))
		.Times(1).InSequence(seq_2)
		.WillOnce(InvokeWithoutArgs([&]() {
			done_2.set();
			return stream::Buffer::Status::success;
		}));
	uint8_t mtracks[32] = { 0x06 }; // audio, video
	conf->SetParticipantConnection(part_name_1, std::move(socket), stream::ClientType::receiver, mtracks);

	EXPECT_TRUE(test::WaitFor("PutFrame (buffer_1)", done));
	EXPECT_TRUE(test::WaitFor("PutFrame (buffer_2)", done_2));
	EXPECT_EQ(total_read_bytes, sizeof(frame_1_header) + sizeof(frame_1_body));

	EXPECT_TRUE(StopConference());
}

TEST_F(ConferenceTest, RouteFrame_FatalPutFrame)
{
	using ::testing::_;
	using ::testing::AnyNumber;
	using ::testing::ElementsAreArray;
	using ::testing::Field;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;
	using ::testing::StrEq;

	StartConference();
	ConnectToFrameSink();

	EXPECT_TRUE(AddParticipant(part_name_1));

	BufferMock* buffer = nullptr;
	Sequence seq;
	EXPECT_TRUE(AddParticipant(part_name_2, buffer, seq));

	stream::Track tracks[] = { stream::Track::audio, stream::Track::video };
	EXPECT_TRUE(conf->ConnectParticipantSender(part_name_1, part_name_2, tracks, sizeof(tracks)/sizeof(tracks[0])));

	test::asio::tcp_socket_fake socket(g_asio_environment->IOService());
	auto socket_fake = socket.impl_.get();
	socket_fake->get_state().is_open() = true;
	socket_fake->get_state()
		.add_read_data(frame_1_header)
		.add_read_data(frame_1_body, sizeof(frame_1_body))
		.add_read_data(frame_2_header)
		.add_read_data(frame_2_body, sizeof(frame_2_body));

	EXPECT_CALL(frame_receiver, Call(_, _, Field(&stream::FrameHeader::track, stream::Track::audio), _))
		.Times(1);
	EXPECT_CALL(frame_receiver, Call(_, _, Field(&stream::FrameHeader::track, stream::Track::video), _))
		.Times(1)
		.WillOnce(InvokeWithoutArgs([&]() { done.set(); }));

	EXPECT_CALL(*buffer, GetFrame(_,_,_,_)).Times(AnyNumber());
	EXPECT_CALL(*buffer, PutFrame_mocked(_, stream::Track::audio, _, _))
		.With(ArgsAsArray<char, 2, 3>(ElementsAreArray(frame_1_body)))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			return stream::Buffer::Status::fatal;
		}));
	EXPECT_CALL(*buffer, PutFrame_mocked(_, stream::Track::video, _, _)).Times(0);
	EXPECT_CALL(ccs, RemoveParticipant(StrEq(conf_name), StrEq(part_name_2), _)).Times(1);
	uint8_t mtracks[32] = { 0x06 }; // audio, video
	conf->SetParticipantConnection(part_name_1, std::move(socket), stream::ClientType::receiver, mtracks);
	// Can't use socket_fake anymore because it may be deleted

	EXPECT_TRUE(test::WaitFor("Last frame receive", done));
	EXPECT_EQ(1u, conf->GetParticipantCount());
	EXPECT_EQ(total_read_bytes, sizeof(frame_1_header) + sizeof(frame_1_body) + sizeof(frame_2_header) + sizeof(frame_2_body));

	EXPECT_TRUE(StopConference());
}

TEST_F(ConferenceTest, WriteFrame)
{
	using ::testing::_;
	using ::testing::ElementsAreArray;
	using ::testing::Invoke;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	StartConference();

	BufferMock* buffer = nullptr;
	Sequence seq;
	EXPECT_TRUE(AddParticipant(part_name_1, buffer, seq));

	test::asio::tcp_socket_fake socket(g_asio_environment->IOService());
	auto socket_fake = socket.impl_.get();
	socket_fake->get_state().is_open() = true;

	EXPECT_CALL(*buffer, GetFrame(_,_,_,_))
		.Times(1).InSequence(seq)
		.WillOnce(Invoke([&](uint32_t& tick_count, stream::Track& track, std::unique_ptr<char[]>& buffer, size_t& size) {
			tick_count = 1234;
			track = stream::Track::audio;
			size = sizeof(frame_1_body);
			buffer = vs::make_unique_default_init<char[]>(size);
			std::memcpy(buffer.get(), frame_1_body, size);
			return stream::Buffer::Status::success;
		}));
	EXPECT_CALL(*buffer, GetFrame(_,_,_,_))
		.Times(1).InSequence(seq)
		.WillOnce(Invoke([&](uint32_t& tick_count, stream::Track& track, std::unique_ptr<char[]>& buffer, size_t& size) {
			tick_count = 5678;
			track = stream::Track::video;
			size = sizeof(frame_2_body);
			buffer = vs::make_unique_default_init<char[]>(size);
			std::memcpy(buffer.get(), frame_2_body, size);
			return stream::Buffer::Status::success;
		}));
	EXPECT_CALL(*buffer, GetFrame(_,_,_,_))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
			return stream::Buffer::Status::non_fatal;
		}));
	uint8_t mtracks[32] = { 0x06 }; // audio, video
	conf->SetParticipantConnection(part_name_1, std::move(socket), stream::ClientType::sender, mtracks);

	EXPECT_TRUE(test::WaitFor("Last GetFrame() call", done));
	{
		auto s = socket_fake->get_state();
		s.drop_write_data(sizeof(serialized_handshake_reply));
		EXPECT_THAT(s.get_write_data(sizeof(frame_1_header)), ElementsAreArray(reinterpret_cast<const uint8_t*>(&frame_1_header), sizeof(frame_1_header)));
		EXPECT_THAT(s.get_write_data(sizeof(frame_1_body)), ElementsAreArray(frame_1_body));
		EXPECT_THAT(s.get_write_data(sizeof(frame_2_header)), ElementsAreArray(reinterpret_cast<const uint8_t*>(&frame_2_header), sizeof(frame_2_header)));
		EXPECT_THAT(s.get_write_data(sizeof(frame_2_body)), ElementsAreArray(frame_2_body));
	}
	EXPECT_EQ(total_write_bytes, sizeof(serialized_handshake_reply) + sizeof(frame_1_header) + sizeof(frame_1_body) + sizeof(frame_2_header) + sizeof(frame_2_body));

	EXPECT_TRUE(StopConference());
}

TEST_F(ConferenceTest, WriteFrame_FatalGetFrame)
{
	using ::testing::_;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;
	using ::testing::StrEq;

	StartConference();

	BufferMock* buffer = nullptr;
	Sequence seq;
	EXPECT_TRUE(AddParticipant(part_name_1, buffer, seq));

	test::asio::tcp_socket_fake socket(g_asio_environment->IOService());
	auto socket_fake = socket.impl_.get();
	socket_fake->get_state().is_open() = true;

	EXPECT_CALL(*buffer, GetFrame(_,_,_,_))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			return stream::Buffer::Status::fatal;
		}));
	EXPECT_CALL(*buffer, GetFrame(_,_,_,_))
		.Times(0).InSequence(seq);
	EXPECT_CALL(ccs, RemoveParticipant(StrEq(conf_name), StrEq(part_name_1), _))
		.Times(1)
		.WillOnce(InvokeWithoutArgs([&]() { done.set(); }));
	uint8_t mtracks[32] = { 0x06 }; // audio, video
	conf->SetParticipantConnection(part_name_1, std::move(socket), stream::ClientType::sender, mtracks);
	// Can't use socket_fake anymore because it may be deleted

	EXPECT_TRUE(test::WaitFor("RemoveParticipant", done));

	EXPECT_TRUE(StopConference());
}

}

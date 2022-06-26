#include "Data.h"
#include "transport/Message.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace transport_test {

static const transport::MessageFixedPart c_msg_header = {
	13, 0, 0, 1,
	0,
	sizeof(transport::MessageFixedPart),
	123,
	456789,
	0x89abcdef,
};

TEST(TransportMessage, HeaderChecksum)
{
	EXPECT_EQ(126, GetMessageHeaderChecksum(c_msg_header));
}

TEST(TransportMessage, HeaderChecksum_Ignore_notify)
{
	auto header = c_msg_header;
	header.notify = 0;
	const auto cksum_0 = GetMessageHeaderChecksum(header);
	header.notify = 1;
	const auto cksum_not_0 = GetMessageHeaderChecksum(header);
	EXPECT_EQ(cksum_0, cksum_not_0);
}

TEST(TransportMessage, HeaderChecksum_Ignore_head_cksum)
{
	auto header = c_msg_header;
	header.head_cksum = 0;
	const auto cksum_0 = GetMessageHeaderChecksum(header);
	header.head_cksum = 0xab;
	const auto cksum_not_0 = GetMessageHeaderChecksum(header);
	EXPECT_EQ(cksum_0, cksum_not_0);
}

TEST(TransportMessage, HeaderChecksum_Ignore_ms_life_count)
{
	auto header = c_msg_header;
	header.ms_life_count = 0;
	const auto cksum_0 = GetMessageHeaderChecksum(header);
	header.ms_life_count = 0x02468ace;
	const auto cksum_not_0 = GetMessageHeaderChecksum(header);
	EXPECT_EQ(cksum_0, cksum_not_0);
}

TEST(TransportMessage, Create)
{
	using ::testing::StrEq;

	transport::Message msg(true, 123, 789, "SrcCID", "SrcService", "AddString", "DstCID", "DstService", "SrcUser", "DstUser", "SrcServer", "DstServer", serialized_message_body, sizeof(serialized_message_body));
	EXPECT_TRUE(msg.IsValid());

	EXPECT_TRUE(msg.IsRequest());
	EXPECT_FALSE(msg.IsReply());
	EXPECT_EQ(123, msg.SeqNum());
	EXPECT_EQ(789, msg.TimeLimit());
	EXPECT_THAT(msg.SrcCID(), StrEq("SrcCID"));
	EXPECT_THAT(msg.SrcService(), StrEq("SrcService"));
	EXPECT_THAT(msg.SrcUser(), StrEq("SrcUser"));
	EXPECT_THAT(msg.SrcServer(), StrEq("SrcServer"));
	EXPECT_THAT(msg.DstCID(), StrEq("DstCID"));
	EXPECT_THAT(msg.DstService(), StrEq("DstService"));
	EXPECT_THAT(msg.DstUser(), StrEq("DstUser"));
	EXPECT_THAT(msg.DstServer(), StrEq("DstServer"));
	EXPECT_THAT(msg.AddString(), StrEq("AddString"));
	ASSERT_EQ(msg.BodySize(), sizeof(serialized_message_body));
	for (size_t i = 0; i < msg.BodySize(); ++i)
		EXPECT_EQ(msg.Body()[i], serialized_message_body[i]) << "index=" << i;
}

TEST(TransportMessage, Make)
{
	using ::testing::StrEq;

	transport::Message msg = transport::Message::Make()
		.Request()
		.SeqNumber(123)
		.TimeLimit(789)
		.SrcCID("SrcCID")
		.SrcService("SrcService")
		.AddString("AddString")
		.DstCID("DstCID")
		.DstService("DstService")
		.SrcUser("SrcUser")
		.DstUser("DstUser")
		.SrcServer("SrcServer")
		.DstServer("DstServer")
		.Body(serialized_message_body, sizeof(serialized_message_body));
	EXPECT_TRUE(msg.IsValid());

	EXPECT_TRUE(msg.IsRequest());
	EXPECT_FALSE(msg.IsReply());
	EXPECT_EQ(123, msg.SeqNum());
	EXPECT_EQ(789, msg.TimeLimit());
	EXPECT_THAT(msg.SrcCID(), StrEq("SrcCID"));
	EXPECT_THAT(msg.SrcService(), StrEq("SrcService"));
	EXPECT_THAT(msg.SrcUser(), StrEq("SrcUser"));
	EXPECT_THAT(msg.SrcServer(), StrEq("SrcServer"));
	EXPECT_THAT(msg.DstCID(), StrEq("DstCID"));
	EXPECT_THAT(msg.DstService(), StrEq("DstService"));
	EXPECT_THAT(msg.DstUser(), StrEq("DstUser"));
	EXPECT_THAT(msg.DstServer(), StrEq("DstServer"));
	EXPECT_THAT(msg.AddString(), StrEq("AddString"));

	ASSERT_EQ(msg.BodySize(), sizeof(serialized_message_body));
	for (size_t i = 0; i < msg.BodySize(); ++i)
		EXPECT_EQ(msg.Body()[i], serialized_message_body[i]) << "index=" << i;
}

TEST(TransportMessage, MakeReply)
{
	using ::testing::StrEq;

	transport::Message msg_req(true, 123, 789, "SrcCID", "SrcService", "AddString", "DstCID", "DstService", "SrcUser", "DstUser", "SrcServer", "DstServer", serialized_message_body, sizeof(serialized_message_body));
	EXPECT_TRUE(msg_req.IsValid());

	const char body_2[] = "BODY";
	transport::Message msg = transport::Message::Make()
		.ReplyTo(msg_req)
		.TimeLimit(456)
		.Body(body_2, sizeof(body_2));
	EXPECT_TRUE(msg.IsValid());

	EXPECT_FALSE(msg.IsRequest());
	EXPECT_TRUE(msg.IsReply());
	EXPECT_EQ(123, msg.SeqNum());
	EXPECT_EQ(456, msg.TimeLimit());
	EXPECT_THAT(msg.SrcCID(), StrEq("DstCID"));
	EXPECT_THAT(msg.SrcService(), StrEq("DstService"));
	EXPECT_THAT(msg.SrcUser(), StrEq("DstUser"));
	EXPECT_THAT(msg.SrcServer(), StrEq(""));
	EXPECT_THAT(msg.DstCID(), StrEq("SrcCID"));
	EXPECT_THAT(msg.DstService(), StrEq("SrcService"));
	EXPECT_THAT(msg.DstUser(), StrEq("SrcUser"));
	EXPECT_THAT(msg.DstServer(), StrEq("SrcServer"));
	EXPECT_THAT(msg.AddString(), StrEq(""));

	ASSERT_EQ(msg.BodySize(), sizeof(body_2));
	for (size_t i = 0; i < msg.BodySize(); ++i)
		EXPECT_EQ(msg.Body()[i], body_2[i]) << "index=" << i;
}

TEST(TransportMessage, Serialize)
{
	transport::Message msg(true, 123, 789, "SrcCID", "SrcService", "AddString", "DstCID", "DstService", "SrcUser", "DstUser", "SrcServer", "DstServer", serialized_message_body, sizeof(serialized_message_body));
	EXPECT_TRUE(msg.IsValid());

	ASSERT_EQ(msg.Size(), sizeof(serialized_message));
	for (size_t i = 0; i < msg.Size(); ++i)
		EXPECT_EQ(msg.Data()[i], serialized_message[i]) << "index=" << i;
}

TEST(TransportMessage, Deserialize)
{
	using ::testing::StrEq;

	transport::Message msg(serialized_message, sizeof(serialized_message));
	EXPECT_TRUE(msg.IsValid());

	EXPECT_TRUE(msg.IsRequest());
	EXPECT_FALSE(msg.IsReply());
	EXPECT_EQ(123, msg.SeqNum());
	EXPECT_EQ(789, msg.TimeLimit());
	EXPECT_THAT(msg.SrcCID(), StrEq("SrcCID"));
	EXPECT_THAT(msg.SrcService(), StrEq("SrcService"));
	EXPECT_THAT(msg.SrcUser(), StrEq("SrcUser"));
	EXPECT_THAT(msg.SrcServer(), StrEq("SrcServer"));
	EXPECT_THAT(msg.DstCID(), StrEq("DstCID"));
	EXPECT_THAT(msg.DstService(), StrEq("DstService"));
	EXPECT_THAT(msg.DstUser(), StrEq("DstUser"));
	EXPECT_THAT(msg.DstServer(), StrEq("DstServer"));
	EXPECT_THAT(msg.AddString(), StrEq("AddString"));
	ASSERT_EQ(msg.BodySize(), sizeof(serialized_message_body));
	for (size_t i = 0; i < msg.BodySize(); ++i)
		EXPECT_EQ(msg.Body()[i], serialized_message_body[i]) << "index=" << i;
}

TEST(TransportMessage, SetField)
{
	using ::testing::StrEq;

	transport::Message msg(true, 123, 789, "SrcCID", "SrcService", "AddString", "DstCID", "DstService", "SrcUser", "DstUser", "SrcServer", "DstServer", serialized_message_body, sizeof(serialized_message_body));
	EXPECT_TRUE(msg.IsValid());

	EXPECT_TRUE(msg.SetAddString("A new, longer value"));
	EXPECT_TRUE(msg.IsValid());

	// Test all fields because Message was regenerated.
	EXPECT_TRUE(msg.IsRequest());
	EXPECT_FALSE(msg.IsReply());
	EXPECT_EQ(123, msg.SeqNum());
	EXPECT_EQ(789, msg.TimeLimit());
	EXPECT_THAT(msg.SrcCID(), StrEq("SrcCID"));
	EXPECT_THAT(msg.SrcService(), StrEq("SrcService"));
	EXPECT_THAT(msg.SrcUser(), StrEq("SrcUser"));
	EXPECT_THAT(msg.SrcServer(), StrEq("SrcServer"));
	EXPECT_THAT(msg.DstCID(), StrEq("DstCID"));
	EXPECT_THAT(msg.DstService(), StrEq("DstService"));
	EXPECT_THAT(msg.DstUser(), StrEq("DstUser"));
	EXPECT_THAT(msg.DstServer(), StrEq("DstServer"));
	EXPECT_THAT(msg.AddString(), StrEq("A new, longer value"));
	ASSERT_EQ(msg.BodySize(), sizeof(serialized_message_body));
	for (size_t i = 0; i < msg.BodySize(); ++i)
		EXPECT_EQ(msg.Body()[i], serialized_message_body[i]) << "index=" << i;
}

TEST(TransportMessage, SetBody)
{
	transport::Message msg(true, 123, 789, "SrcCID", "SrcService", "AddString", "DstCID", "DstService", "SrcUser", "DstUser", "SrcServer", "DstServer", serialized_message_body, sizeof(serialized_message_body));
	EXPECT_TRUE(msg.IsValid());

	const char body_2[] = "BODY";
	EXPECT_TRUE(msg.SetBody(body_2, sizeof(body_2)));

	EXPECT_TRUE(msg.IsValid());
	ASSERT_EQ(msg.BodySize(), sizeof(body_2));
	for (size_t i = 0; i < msg.BodySize(); ++i)
		EXPECT_EQ(msg.Body()[i], body_2[i]) << "index=" << i;
}

}

#include "../SIPParserLib/VS_BFCPAttribute.h"
#include "../SIPParserLib/VS_BFCPMessage.h"
#include "../SIPParserLib/VS_BFCPSession.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace {

static const uint8_t bfcp_FloorStatus_Polycom[] = {
	0x20, 0x08, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x05, 0x04, 0x00, 0x01, 0x1f, 0x10, 0x00, 0x00, 0x25, 0x0c, 0x00, 0x00, 0x0b, 0x04, 0x03, 0x00, 0x23, 0x04, 0x00, 0x01,
};

TEST(BFCPDecode, Simple)
{
	bfcp::DecodeStatus status;
	size_t size = sizeof(bfcp_FloorStatus_Polycom);
	auto msg = bfcp::Message::DecodeAny(bfcp_FloorStatus_Polycom, size, status);
	EXPECT_EQ(bfcp::DecodeStatus::Success, status.type);
	EXPECT_EQ(sizeof(bfcp_FloorStatus_Polycom), size);
	EXPECT_TRUE(msg);
}

TEST(BFCPDecode, Fragment)
{
	bfcp::DecodeStatus status;
	size_t size = sizeof(bfcp_FloorStatus_Polycom) / 2;
	auto msg = bfcp::Message::DecodeAny(bfcp_FloorStatus_Polycom, size, status);
	EXPECT_EQ(bfcp::DecodeStatus::PartialData, status.type);
	EXPECT_EQ(sizeof(bfcp_FloorStatus_Polycom), size);
	EXPECT_FALSE(msg);
}

TEST(BFCPDecode, Garbage)
{
	const uint8_t raw[] = { 0xde, 0xad, 0xc0, 0xde, 0x0f, 0x5a, 0x6e, };
	bfcp::DecodeStatus status;
	size_t size = sizeof(raw);
	auto msg = bfcp::Message::DecodeAny(raw, size, status);
	EXPECT_NE(bfcp::DecodeStatus::Success, status.type);
	EXPECT_NE(bfcp::DecodeStatus::PartialData, status.type);
	EXPECT_FALSE(msg);
}

TEST(BFCPDecode, UnknownPrimitive)
{
	const uint8_t raw[] = { // Primitive 0x42
		0x20, 0x42, 0x00, 0x01, 0x11, 0x11, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33, 0xde, 0xad, 0xc0, 0xde,
	};
	bfcp::DecodeStatus status;
	size_t size = sizeof(raw);
	auto msg = bfcp::Message::DecodeAny(raw, size, status);
	EXPECT_EQ(bfcp::DecodeStatus::UnknownPrimitive, status.type);
	EXPECT_EQ(0x42, status.primitive_type);
	EXPECT_EQ(sizeof(raw), size);
	EXPECT_FALSE(msg);
}

TEST(BFCPDecode, UnknownAttribute)
{
	const uint8_t raw[] = { // FloorRequest with extra non-mandatory attribute 0x42
		0x20, 0x01, 0x00, 0x02, 0x11, 0x11, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33, 0x05, 0x04, 0x44, 0x44, 0x84, 0x04, 0x00, 0x00,
	};
	bfcp::DecodeStatus status;
	size_t size = sizeof(raw);
	auto msg = bfcp::Message::DecodeAny(raw, size, status);
	EXPECT_EQ(bfcp::DecodeStatus::Success, status.type);
	EXPECT_EQ(sizeof(raw), size);
	EXPECT_TRUE(msg);
}

TEST(BFCPDecode, UnknownMandatoryAttribute)
{
	const uint8_t raw[] = { // FloorRequest with extra mandatory attribute 0x42
		0x20, 0x01, 0x00, 0x02, 0x11, 0x11, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33, 0x05, 0x04, 0x44, 0x44, 0x85, 0x04, 0x00, 0x00,
	};
	bfcp::DecodeStatus status;
	size_t size = sizeof(raw);
	auto msg = bfcp::Message::DecodeAny(raw, size, status);
	EXPECT_EQ(bfcp::DecodeStatus::UnknownMandatoryAttribute, status.type);
	EXPECT_EQ(0x42, status.attribute_type);
	EXPECT_EQ(sizeof(raw), size);
	EXPECT_FALSE(msg);
}

TEST(BFCPEncode, Simple)
{
	using ::testing::ElementsAreArray;
	auto msg = std::make_unique<bfcp::Message_FloorStatus>(1, 1, 0, 1,
		std::make_unique<bfcp::Attribute_FLOOR_ID>(1),
		std::make_unique<bfcp::Attribute_FLOOR_REQUEST_INFORMATION>(0,
			std::make_unique<bfcp::Attribute_OVERALL_REQUEST_STATUS>(0,
				std::make_unique<bfcp::Attribute_REQUEST_STATUS>(bfcp::RequestStatus::Granted, 0),
				std::make_unique<bfcp::Attribute_FLOOR_REQUEST_STATUS>(1)
			)
		)
	);
	size_t size = sizeof(bfcp_FloorStatus_Polycom);
	std::vector<uint8_t> raw(size);
	EXPECT_TRUE(msg->Encode(raw.data(), size));
	ASSERT_EQ(sizeof(bfcp_FloorStatus_Polycom), size);
	EXPECT_THAT(raw, ElementsAreArray(bfcp_FloorStatus_Polycom));
}

TEST(BFCPEncode, SmallBuffer)
{
	auto msg = std::make_unique<bfcp::Message_Hello>(1, 1, 2, 3);

	size_t size = 4;
	std::vector<uint8_t> raw(size);
	EXPECT_FALSE(msg->Encode(raw.data(), size));
	EXPECT_EQ(12, size);
}

class BFCPSessionTestBase : public ::testing::Test
{
protected:
	BFCPSessionTestBase() = default;

public:
	void FeedMessage(const bfcp::Message* msg)
	{
		size_t size = 0;
		msg->Encode(nullptr, size);
		std::vector<uint8_t> raw(size);
		ASSERT_TRUE(msg->Encode(raw.data(), size));
		EXPECT_TRUE(session_base->SetRecvData(raw.data(), size));
	}

	std::unique_ptr<bfcp::Message> GetSendMessage()
	{
		size_t size = 0;
		EXPECT_FALSE(session_base->GetSendData(nullptr, size));
		std::vector<uint8_t> raw(size);
		EXPECT_TRUE(session_base->GetSendData(raw.data(), size));
		bfcp::DecodeStatus status;
		auto msg = bfcp::Message::DecodeAny(raw.data(), size, status);
		EXPECT_EQ(bfcp::DecodeStatus::Success, status.type);
		EXPECT_TRUE(msg);
		return msg;
	}

	std::shared_ptr<bfcp::SessionBase> session_base;
};

class BFCPClientSessionTest : public BFCPSessionTestBase
{
public:
	BFCPClientSessionTest()
		: session(std::make_unique<bfcp::ClientSession>(our_conf_id, our_user_id, false))
	{
		session_base = session;
		session->ConnectToFloorRequestStatus([this](bfcp::FloorID floor_id, bfcp::RequestStatus status) {
			FloorRequestStatus_list.emplace_back(floor_id, status);
		});
		session->ConnectToFloorStatus([this](bfcp::FloorID floor_id, bfcp::RequestStatus status, bfcp::UserID user_id) {
			FloorStatus_list.emplace_back(floor_id, status, user_id);
		});
	}

public:
	static const bfcp::ConferenceID our_conf_id;
	static const bfcp::ConferenceID our_user_id;

	std::shared_ptr<bfcp::ClientSession> session;
	std::vector<std::tuple<bfcp::FloorID, bfcp::RequestStatus>> FloorRequestStatus_list;
	std::vector<std::tuple<bfcp::FloorID, bfcp::RequestStatus, bfcp::UserID>> FloorStatus_list;
};

const bfcp::ConferenceID BFCPClientSessionTest::our_conf_id = 1;
const bfcp::ConferenceID BFCPClientSessionTest::our_user_id = 2;

TEST_F(BFCPClientSessionTest, RequestFloor_Request)
{
	const bfcp::FloorID floor_id = 42;
	session->RequestFloor(floor_id);
	auto msg_ = GetSendMessage();
	auto msg = dynamic_cast<bfcp::Message_FloorRequest*>(msg_.get());
	ASSERT_NE(nullptr, msg);
	EXPECT_EQ(our_conf_id, msg->conference_id);
	EXPECT_EQ(our_user_id, msg->user_id);
	ASSERT_EQ(1, msg->attrs_FLOOR_ID.size());
	EXPECT_EQ(bfcp::AttributeType::FLOOR_ID, msg->attrs_FLOOR_ID[0]->type);
	EXPECT_EQ(floor_id, msg->attrs_FLOOR_ID[0]->value);
}

TEST_F(BFCPClientSessionTest, RequestFloor_Response)
{
	const bfcp::FloorID floor_id = 42;
	const bfcp::FloorRequestID floor_request_id = 55;
	session->RequestFloor(floor_id);
	FeedMessage(std::make_unique<bfcp::Message_FloorRequestStatus>(1, our_conf_id, 0, our_user_id,
		std::make_unique<bfcp::Attribute_FLOOR_REQUEST_INFORMATION>(floor_request_id,
		std::make_unique<bfcp::Attribute_FLOOR_REQUEST_STATUS>(floor_id),
			std::make_unique<bfcp::Attribute_OVERALL_REQUEST_STATUS>(floor_request_id,
				std::make_unique<bfcp::Attribute_REQUEST_STATUS>(bfcp::RequestStatus::Granted, 0)
			)
		)
	).get());

	ASSERT_EQ(1u, FloorRequestStatus_list.size());
	EXPECT_EQ(floor_id, std::get<0>(FloorRequestStatus_list[0]));
	EXPECT_EQ(bfcp::RequestStatus::Granted, std::get<1>(FloorRequestStatus_list[0]));
}

TEST_F(BFCPClientSessionTest, ReleaseFloor_Released)
{
	const bfcp::FloorID floor_id = 42;
	EXPECT_FALSE(session->ReleaseFloor(floor_id));
	size_t size = 123;
	EXPECT_TRUE(session->GetSendData(nullptr, size));
	EXPECT_EQ(0, size);
}

TEST_F(BFCPClientSessionTest, ReleaseFloor_Request)
{
	const bfcp::FloorID floor_id = 42;
	const bfcp::FloorRequestID floor_request_id = 55;
	session->RequestFloor(floor_id);
	FeedMessage(std::make_unique<bfcp::Message_FloorRequestStatus>(1, our_conf_id, 0, our_user_id,
		std::make_unique<bfcp::Attribute_FLOOR_REQUEST_INFORMATION>(floor_request_id,
		std::make_unique<bfcp::Attribute_FLOOR_REQUEST_STATUS>(floor_id),
			std::make_unique<bfcp::Attribute_OVERALL_REQUEST_STATUS>(floor_request_id,
				std::make_unique<bfcp::Attribute_REQUEST_STATUS>(bfcp::RequestStatus::Granted, 0)
			)
		)
	).get());
	GetSendMessage();
	EXPECT_TRUE(session->ReleaseFloor(floor_id));
	auto msg_ = GetSendMessage();
	auto msg = dynamic_cast<bfcp::Message_FloorRelease*>(msg_.get());
	ASSERT_NE(nullptr, msg);
	EXPECT_EQ(our_conf_id, msg->conference_id);
	EXPECT_EQ(our_user_id, msg->user_id);
	ASSERT_NE(nullptr, msg->attr_FLOOR_REQUEST_ID);
	EXPECT_EQ(bfcp::AttributeType::FLOOR_REQUEST_ID, msg->attr_FLOOR_REQUEST_ID->type);
	EXPECT_EQ(floor_request_id, msg->attr_FLOOR_REQUEST_ID->value);
}

TEST_F(BFCPClientSessionTest, ReleaseFloor_Response)
{
	const bfcp::FloorID floor_id = 42;
	const bfcp::FloorRequestID floor_request_id = 55;
	session->RequestFloor(floor_id);
	FeedMessage(std::make_unique<bfcp::Message_FloorRequestStatus>(1, our_conf_id, 0, our_user_id,
		std::make_unique<bfcp::Attribute_FLOOR_REQUEST_INFORMATION>(floor_request_id,
		std::make_unique<bfcp::Attribute_FLOOR_REQUEST_STATUS>(floor_id),
			std::make_unique<bfcp::Attribute_OVERALL_REQUEST_STATUS>(floor_request_id,
				std::make_unique<bfcp::Attribute_REQUEST_STATUS>(bfcp::RequestStatus::Granted, 0)
			)
		)
	).get());
	FloorRequestStatus_list.clear();
	EXPECT_TRUE(session->ReleaseFloor(floor_id));
	FeedMessage(std::make_unique<bfcp::Message_FloorRequestStatus>(1, our_conf_id, 0, our_user_id,
		std::make_unique<bfcp::Attribute_FLOOR_REQUEST_INFORMATION>(floor_request_id,
		std::make_unique<bfcp::Attribute_FLOOR_REQUEST_STATUS>(floor_id),
			std::make_unique<bfcp::Attribute_OVERALL_REQUEST_STATUS>(floor_request_id,
				std::make_unique<bfcp::Attribute_REQUEST_STATUS>(bfcp::RequestStatus::Released, 0)
			)
		)
	).get());
	ASSERT_EQ(1u, FloorRequestStatus_list.size());
	EXPECT_EQ(floor_id, std::get<0>(FloorRequestStatus_list[0]));
	EXPECT_EQ(bfcp::RequestStatus::Released, std::get<1>(FloorRequestStatus_list[0]));
}

TEST_F(BFCPClientSessionTest, SubscribeToFloorStatus)
{
	const bfcp::FloorID floor_id = 42;
	session->SubscribeToFloorStatus({ floor_id });
	auto msg_ = GetSendMessage();
	auto msg = dynamic_cast<bfcp::Message_FloorQuery*>(msg_.get());
	ASSERT_NE(nullptr, msg);
	EXPECT_EQ(our_conf_id, msg->conference_id);
	EXPECT_EQ(our_user_id, msg->user_id);
	ASSERT_EQ(1, msg->attrs_FLOOR_ID.size());
	EXPECT_EQ(bfcp::AttributeType::FLOOR_ID, msg->attrs_FLOOR_ID[0]->type);
	EXPECT_EQ(floor_id, msg->attrs_FLOOR_ID[0]->value);
}

TEST_F(BFCPClientSessionTest, SubscribeToFloorStatus_Repeat)
{
	const bfcp::FloorID floor_id = 42;
	session->SubscribeToFloorStatus({ floor_id });
	GetSendMessage();
	session->SubscribeToFloorStatus({ floor_id });
	size_t size = 123;
	EXPECT_TRUE(session->GetSendData(nullptr, size));
	EXPECT_EQ(0, size);
}

TEST_F(BFCPClientSessionTest, UnsubscribeToFloorStatus)
{
	const bfcp::FloorID floor_id = 42;
	session->SubscribeToFloorStatus({ floor_id });
	GetSendMessage();
	session->UnsubscribeToFloorStatus({ floor_id });
	auto msg_ = GetSendMessage();
	auto msg = dynamic_cast<bfcp::Message_FloorQuery*>(msg_.get());
	ASSERT_NE(nullptr, msg);
	EXPECT_EQ(our_conf_id, msg->conference_id);
	EXPECT_EQ(our_user_id, msg->user_id);
	ASSERT_EQ(0, msg->attrs_FLOOR_ID.size());
}

TEST_F(BFCPClientSessionTest, UnsubscribeToFloorStatus_NotSubscribed)
{
	const bfcp::FloorID floor_id = 42;
	session->UnsubscribeToFloorStatus({ floor_id });
	size_t size = 123;
	EXPECT_TRUE(session->GetSendData(nullptr, size));
	EXPECT_EQ(0, size);
}

TEST_F(BFCPClientSessionTest, FloorStatusHandling)
{
	const bfcp::FloorID floor_id = 42;
	const bfcp::FloorID user_id = 13;
	FeedMessage(std::make_unique<bfcp::Message_FloorStatus>(1, our_conf_id, 0, our_user_id,
		std::make_unique<bfcp::Attribute_FLOOR_ID>(floor_id),
		std::make_unique<bfcp::Attribute_FLOOR_REQUEST_INFORMATION>(123,
			std::make_unique<bfcp::Attribute_OVERALL_REQUEST_STATUS>(123,
				std::make_unique<bfcp::Attribute_REQUEST_STATUS>(bfcp::RequestStatus::Granted, 0)
			),
			std::make_unique<bfcp::Attribute_BENEFICIARY_INFORMATION>(user_id)
		)
	).get());
	ASSERT_EQ(1u, FloorStatus_list.size());
	EXPECT_EQ(floor_id, std::get<0>(FloorStatus_list[0]));
	EXPECT_EQ(bfcp::RequestStatus::Granted, std::get<1>(FloorStatus_list[0]));
	EXPECT_EQ(user_id, std::get<2>(FloorStatus_list[0]));
}

class BFCPServerSessionTest : public BFCPSessionTestBase
{
public:
	BFCPServerSessionTest()
		: session(std::make_unique<bfcp::ServerSession>(our_conf_id, false))
	{
		session_base = session;
		session->ConnectToFloorStatusChange([this](bfcp::FloorID floor_id, bfcp::RequestStatus status, bfcp::UserID user_id) {
			FloorStatusChange_list.emplace_back(floor_id, status, user_id);
		});
	}

public:
	static const bfcp::ConferenceID our_conf_id;

	std::shared_ptr<bfcp::ServerSession> session;
	std::vector<std::tuple<bfcp::FloorID, bfcp::RequestStatus, bfcp::UserID>> FloorStatusChange_list;
};

const bfcp::ConferenceID BFCPServerSessionTest::our_conf_id = 1;

TEST_F(BFCPServerSessionTest, RequestFloor_Callback)
{
	const bfcp::FloorID floor_id = 42;
	const bfcp::UserID user_id = 123;
	EXPECT_EQ(bfcp::RequestStatus::Granted, session->RequestFloor(floor_id, user_id));
	ASSERT_EQ(1u, FloorStatusChange_list.size());
	EXPECT_EQ(floor_id, std::get<0>(FloorStatusChange_list[0]));
	EXPECT_EQ(bfcp::RequestStatus::Granted, std::get<1>(FloorStatusChange_list[0]));
	EXPECT_EQ(user_id, std::get<2>(FloorStatusChange_list[0]));
}

TEST_F(BFCPServerSessionTest, RequestFloor_Net)
{
	const bfcp::FloorID floor_id = 42;
	const bfcp::UserID user_id = 123;
	const bfcp::UserID remote_user_id = 321;
	FeedMessage(std::make_unique<bfcp::Message_FloorQuery>(1, our_conf_id, 1, remote_user_id,
		std::make_unique<bfcp::Attribute_FLOOR_ID>(floor_id)
	).get());
	EXPECT_EQ(bfcp::RequestStatus::Granted, session->RequestFloor(floor_id, user_id));
	auto msg_ = GetSendMessage();
	auto msg = dynamic_cast<bfcp::Message_FloorStatus*>(msg_.get());
	ASSERT_NE(nullptr, msg);
	EXPECT_EQ(our_conf_id, msg->conference_id);
	EXPECT_EQ(remote_user_id, msg->user_id);
	ASSERT_NE(nullptr, msg->attr_FLOOR_ID);
	EXPECT_EQ(floor_id, msg->attr_FLOOR_ID->value);
	ASSERT_EQ(1u, msg->attrs_FLOOR_REQUEST_INFORMATION.size());
	ASSERT_EQ(1u, msg->attrs_FLOOR_REQUEST_INFORMATION[0]->attrs_FLOOR_REQUEST_STATUS.size());
	EXPECT_EQ(floor_id, msg->attrs_FLOOR_REQUEST_INFORMATION[0]->attrs_FLOOR_REQUEST_STATUS[0]->floor_id);
	ASSERT_NE(nullptr, msg->attrs_FLOOR_REQUEST_INFORMATION[0]->attr_OVERALL_REQUEST_STATUS);
	ASSERT_NE(nullptr, msg->attrs_FLOOR_REQUEST_INFORMATION[0]->attr_OVERALL_REQUEST_STATUS->attr_REQUEST_STATUS);
	EXPECT_EQ(bfcp::RequestStatus::Granted, msg->attrs_FLOOR_REQUEST_INFORMATION[0]->attr_OVERALL_REQUEST_STATUS->attr_REQUEST_STATUS->request_status);
	ASSERT_NE(nullptr, msg->attrs_FLOOR_REQUEST_INFORMATION[0]->attr_BENEFICIARY_INFORMATION);
	ASSERT_EQ(user_id, msg->attrs_FLOOR_REQUEST_INFORMATION[0]->attr_BENEFICIARY_INFORMATION->beneficiary_id);
}

TEST_F(BFCPServerSessionTest, ReleaseFloor_Released)
{
	const bfcp::FloorID floor_id = 42;
	const bfcp::UserID user_id = 123;
	const bfcp::UserID remote_user_id = 321;
	FeedMessage(std::make_unique<bfcp::Message_FloorQuery>(1, our_conf_id, 1, remote_user_id,
		std::make_unique<bfcp::Attribute_FLOOR_ID>(floor_id)
	).get());
	EXPECT_EQ(bfcp::RequestStatus::Released, session->ReleaseFloor(floor_id, user_id));
	ASSERT_EQ(0u, FloorStatusChange_list.size());
	size_t size = 123;
	EXPECT_TRUE(session->GetSendData(nullptr, size));
	EXPECT_EQ(0, size);
}

TEST_F(BFCPServerSessionTest, ReleaseFloor_NotOwned)
{
	const bfcp::FloorID floor_id = 42;
	const bfcp::UserID user_id = 123;
	const bfcp::UserID remote_user_id = 321;
	const bfcp::UserID user_id_2 = 876;
	FeedMessage(std::make_unique<bfcp::Message_FloorQuery>(1, our_conf_id, 1, remote_user_id,
		std::make_unique<bfcp::Attribute_FLOOR_ID>(floor_id)
	).get());
	EXPECT_EQ(bfcp::RequestStatus::Granted, session->RequestFloor(floor_id, user_id));
	FloorStatusChange_list.clear();
	GetSendMessage();
	EXPECT_EQ(bfcp::RequestStatus::Cancelled, session->ReleaseFloor(floor_id, user_id_2));
	ASSERT_EQ(0u, FloorStatusChange_list.size());
	size_t size = 123;
	EXPECT_TRUE(session->GetSendData(nullptr, size));
	EXPECT_EQ(0, size);
}

TEST_F(BFCPServerSessionTest, ReleaseFloor_Callback)
{
	const bfcp::FloorID floor_id = 42;
	const bfcp::UserID user_id = 123;
	EXPECT_EQ(bfcp::RequestStatus::Granted, session->RequestFloor(floor_id, user_id));
	FloorStatusChange_list.clear();
	EXPECT_EQ(bfcp::RequestStatus::Released, session->ReleaseFloor(floor_id, user_id));
	ASSERT_EQ(1u, FloorStatusChange_list.size());
	EXPECT_EQ(floor_id, std::get<0>(FloorStatusChange_list[0]));
	EXPECT_EQ(bfcp::RequestStatus::Released, std::get<1>(FloorStatusChange_list[0]));
	EXPECT_EQ(user_id, std::get<2>(FloorStatusChange_list[0]));
}

TEST_F(BFCPServerSessionTest, ReleaseFloor_Net)
{
	const bfcp::FloorID floor_id = 42;
	const bfcp::UserID user_id = 123;
	const bfcp::UserID remote_user_id = 321;
	FeedMessage(std::make_unique<bfcp::Message_FloorQuery>(1, our_conf_id, 1, remote_user_id,
		std::make_unique<bfcp::Attribute_FLOOR_ID>(floor_id)
		).get());
	EXPECT_EQ(bfcp::RequestStatus::Granted, session->RequestFloor(floor_id, user_id));
	GetSendMessage();
	EXPECT_EQ(bfcp::RequestStatus::Released, session->ReleaseFloor(floor_id, user_id));
	auto msg_ = GetSendMessage();
	auto msg = dynamic_cast<bfcp::Message_FloorStatus*>(msg_.get());
	ASSERT_NE(nullptr, msg);
	EXPECT_EQ(our_conf_id, msg->conference_id);
	EXPECT_EQ(remote_user_id, msg->user_id);
	ASSERT_NE(nullptr, msg->attr_FLOOR_ID);
	EXPECT_EQ(floor_id, msg->attr_FLOOR_ID->value);
	ASSERT_EQ(1u, msg->attrs_FLOOR_REQUEST_INFORMATION.size());
	ASSERT_EQ(1u, msg->attrs_FLOOR_REQUEST_INFORMATION[0]->attrs_FLOOR_REQUEST_STATUS.size());
	EXPECT_EQ(floor_id, msg->attrs_FLOOR_REQUEST_INFORMATION[0]->attrs_FLOOR_REQUEST_STATUS[0]->floor_id);
	ASSERT_NE(nullptr, msg->attrs_FLOOR_REQUEST_INFORMATION[0]->attr_OVERALL_REQUEST_STATUS);
	ASSERT_NE(nullptr, msg->attrs_FLOOR_REQUEST_INFORMATION[0]->attr_OVERALL_REQUEST_STATUS->attr_REQUEST_STATUS);
	EXPECT_EQ(bfcp::RequestStatus::Released, msg->attrs_FLOOR_REQUEST_INFORMATION[0]->attr_OVERALL_REQUEST_STATUS->attr_REQUEST_STATUS->request_status);
	ASSERT_NE(nullptr, msg->attrs_FLOOR_REQUEST_INFORMATION[0]->attr_BENEFICIARY_INFORMATION);
	ASSERT_EQ(user_id, msg->attrs_FLOOR_REQUEST_INFORMATION[0]->attr_BENEFICIARY_INFORMATION->beneficiary_id);
}

TEST_F(BFCPServerSessionTest, Bug48910_infinite_loop)
{
	const char http_scanner[] =
		"GET / HTTP/1.1\r\n"
		"Host: ns.km21121-02.keymachine.de\r\n"
		"User-Agent: Mozilla/5.0 (compatible; Nimbostratus-Bot/v1.3.2; http://cloudsystemnetworks.com)\r\n\r\n";
	ASSERT_TRUE(session_base->SetRecvData(http_scanner, sizeof(http_scanner)));
}

}

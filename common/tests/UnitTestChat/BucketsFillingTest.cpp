#include "chatlib/chain/BucketsOfMessages.h"
#include "chatlib/msg/attr.h"
#include "chatlib/msg/chat_messages_construct.h"
#include "chatlib/storage/make_chat_storage.h"
#include "chatutils/ExternalComponentsStub.h"
#include "chatutils/GlobalConfigStub.h"
#include "chatutils/ResolverStub.h"

#include <gtest/gtest.h>

#include <memory>
#include <random>

class BucketsFillingTest :
	public ::testing::TestWithParam<std::tuple<int32_t, int32_t>>	// first - count of messages, second - bucket capacity
{
public:
	std::random_device rnd_dev;
};

TEST_P(BucketsFillingTest, SimplePutMessages)
{
	auto chat_storage = chat::make_chat_storage("sqlite3:db=file::memory:?cache=shared");
	int32_t capacity = std::get<1>(GetParam());
	auto cfg = std::make_shared<GlobalConfigStub>(std::make_shared<ResolverStub>());
	cfg->SetChatStorage(chat_storage);
	cfg->SetBucketCapacity(capacity);

	chat::BucketsOfMessages bucket1;
	bucket1.Init(cfg);
	chat::BucketsOfMessages bucket2;
	bucket2.Init(cfg);

	auto msg1 = std::string("msg1");
	chat::ChatID chat_id = "chatid";
	chat::CallID from = "from_id";
	chat::CallID from_instance = "from_id/1";
	vs::CallIDType sender_type = vs::CallIDType::client;
	int32_t all_msg_count = std::get<0>(GetParam());
	std::uniform_int_distribution<uint16_t> distr(0, 1);
	for (int32_t i = 0; i < all_msg_count; ++i)
	{
		uint16_t choice = distr(rnd_dev);
		chat::BucketsOfMessages& outgoing_bucket = choice == 1
			? bucket1
			: bucket2;
		chat::BucketsOfMessages& incomming_bucket = choice == 1
			? bucket2
			: bucket1;

		auto msg_ptr(
			chat::msg::ContentMessage{}
			.Text(msg1)
			.Seal(chat_id, from, from_instance, sender_type, {}, {}));
		ASSERT_TRUE(outgoing_bucket.PutMessage(msg_ptr, true));

		int32_t bucket_num = 0;
		ASSERT_TRUE(msg_ptr->GetParamI64(chat::attr::BUCKET_paramName, bucket_num));

		int32_t calculated_bucket = i / capacity;
		EXPECT_EQ(bucket_num, calculated_bucket);
		msg_ptr->OnMsgIsStored(chat::cb::ProcessingResult::ok);

		ASSERT_TRUE(incomming_bucket.PutMessage(msg_ptr, false));

		uint32_t cur_bucket = (i + 1) / capacity;
		EXPECT_EQ(incomming_bucket.GetCurrentBucket(chat_id), cur_bucket);
	}
}

INSTANTIATE_TEST_CASE_P(PutMessagesInBucket,
	BucketsFillingTest,
	::testing::Values(std::make_tuple(10, 5),
		std::make_tuple(137, 16),
		std::make_tuple(1000, 16)));
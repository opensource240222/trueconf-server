#include "std/cpplib/BlockQueue.h"
#include "tests/common/GTestMatchers.h"

#include <gtest/gtest.h>

#include "std-generic/compat/iterator.h"

namespace block_queue_test {

static const char block_1[] = "First block";
static const char block_2[] = "Second block";
static const char block_3[] = "Block #3";

using ::testing::ElementsAreArray;

TEST(BlockQueue, Push_Pop)
{
	vs::BlockQueue q;
	EXPECT_TRUE(q.Empty());
	q.PushBack(block_1, vs::size(block_1));

	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Front().Data(), AsArray<char>(q.Front().Size(), ElementsAreArray(block_1)));
	q.PopFront();

	EXPECT_TRUE(q.Empty());
}

TEST(BlockQueue, Push_3_Pop_3)
{
	vs::BlockQueue q;
	EXPECT_TRUE(q.Empty());

	q.PushBack(block_1, vs::size(block_1));
	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Back().Data(), AsArray<char>(q.Back().Size(), ElementsAreArray(block_1)));

	q.PushBack(block_2, vs::size(block_2));
	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Back().Data(), AsArray<char>(q.Back().Size(), ElementsAreArray(block_2)));

	q.PushBack(block_3, vs::size(block_3));
	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Back().Data(), AsArray<char>(q.Back().Size(), ElementsAreArray(block_3)));

	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Front().Data(), AsArray<char>(q.Front().Size(), ElementsAreArray(block_1)));
	q.PopFront();

	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Front().Data(), AsArray<char>(q.Front().Size(), ElementsAreArray(block_2)));
	q.PopFront();

	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Front().Data(), AsArray<char>(q.Front().Size(), ElementsAreArray(block_3)));
	q.PopFront();

	EXPECT_TRUE(q.Empty());
}

TEST(BlockQueue, Push_3_Pop_3_SmallChunks)
{
	constexpr size_t chunk_size = 20;
	static_assert(chunk_size < vs::size(block_1) + vs::size(block_2), "Chunk size is too big");
	static_assert(chunk_size < vs::size(block_2) + vs::size(block_3), "Chunk size is too big");

	vs::BlockQueue q(chunk_size);
	EXPECT_TRUE(q.Empty());

	q.PushBack(block_1, vs::size(block_1));
	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Back().Data(), AsArray<char>(q.Back().Size(), ElementsAreArray(block_1)));

	q.PushBack(block_2, vs::size(block_2));
	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Back().Data(), AsArray<char>(q.Back().Size(), ElementsAreArray(block_2)));

	q.PushBack(block_3, vs::size(block_3));
	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Back().Data(), AsArray<char>(q.Back().Size(), ElementsAreArray(block_3)));

	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Front().Data(), AsArray<char>(q.Front().Size(), ElementsAreArray(block_1)));
	q.PopFront();

	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Front().Data(), AsArray<char>(q.Front().Size(), ElementsAreArray(block_2)));
	q.PopFront();

	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Front().Data(), AsArray<char>(q.Front().Size(), ElementsAreArray(block_3)));
	q.PopFront();

	EXPECT_TRUE(q.Empty());
}

TEST(BlockQueue, Push_Pop_Mixed)
{
	vs::BlockQueue q;
	EXPECT_TRUE(q.Empty());

	q.PushBack(block_1, vs::size(block_1));
	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Back().Data(), AsArray<char>(q.Back().Size(), ElementsAreArray(block_1)));

	q.PushBack(block_2, vs::size(block_2));
	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Back().Data(), AsArray<char>(q.Back().Size(), ElementsAreArray(block_2)));

	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Front().Data(), AsArray<char>(q.Front().Size(), ElementsAreArray(block_1)));
	q.PopFront();

	q.PushBack(block_3, vs::size(block_3));
	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Back().Data(), AsArray<char>(q.Back().Size(), ElementsAreArray(block_3)));

	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Front().Data(), AsArray<char>(q.Front().Size(), ElementsAreArray(block_2)));
	q.PopFront();

	ASSERT_FALSE(q.Empty());
	EXPECT_THAT(q.Front().Data(), AsArray<char>(q.Front().Size(), ElementsAreArray(block_3)));
	q.PopFront();

	EXPECT_TRUE(q.Empty());
}

TEST(BlockQueue, ResizeBack)
{
	vs::BlockQueue q;
	q.PushBack(block_1, vs::size(block_1));
	q.PushBack(block_2, vs::size(block_2));

	ASSERT_FALSE(q.Empty());
	q.ResizeBack(vs::size(block_2) + 13);

	ASSERT_FALSE(q.Empty());
	ASSERT_EQ(q.Back().Size(), vs::size(block_2) + 13);
	EXPECT_THAT(q.Back().Data(), AsArray<char>(vs::size(block_2), ElementsAreArray(block_2)));
}

TEST(BlockQueue, ResizeBack_SmallChunks)
{
	constexpr size_t chunk_size = 40;
	static_assert(chunk_size >= vs::size(block_1) + vs::size(block_2) + 6, "Chunk size is too small");

	vs::BlockQueue q(chunk_size);
	q.PushBack(block_1, vs::size(block_1));
	q.PushBack(block_2, vs::size(block_2));

	ASSERT_FALSE(q.Empty());
	q.ResizeBack(vs::size(block_2) + 13);

	ASSERT_FALSE(q.Empty());
	ASSERT_EQ(q.Back().Size(), vs::size(block_2) + 13);
	EXPECT_THAT(q.Back().Data(), AsArray<char>(vs::size(block_2), ElementsAreArray(block_2)));
}

TEST(BlockQueue, Clear)
{
	vs::BlockQueue q;
	EXPECT_TRUE(q.Empty());
	q.PushBack(block_1, vs::size(block_1));
	q.PushBack(block_2, vs::size(block_2));
	q.PushBack(block_3, vs::size(block_3));

	EXPECT_FALSE(q.Empty());
	q.Clear();
	EXPECT_TRUE(q.Empty());
}

TEST(BlockQueue, Size)
{
	vs::BlockQueue q;
	EXPECT_EQ(q.Size(), 0);
	q.PushBack(block_1, vs::size(block_1));
	EXPECT_EQ(q.Size(), 1);
	q.PushBack(block_2, vs::size(block_2));
	EXPECT_EQ(q.Size(), 2);
	q.PopFront();
	EXPECT_EQ(q.Size(), 1);
	q.PushBack(block_3, vs::size(block_3));
	EXPECT_EQ(q.Size(), 2);
	q.PopFront();
	EXPECT_EQ(q.Size(), 1);
	q.Clear();
	EXPECT_EQ(q.Size(), 0);
	q.Clear();
}

TEST(BlockQueue, Iterators)
{
	vs::BlockQueue q;
	EXPECT_EQ(q.begin(), q.end());
	q.PushBack(block_1, vs::size(block_1));
	q.PushBack(block_2, vs::size(block_2));
	q.PushBack(block_3, vs::size(block_3));

	vs::BlockQueue::iterator it = q.begin();

	ASSERT_NE(it, q.end());
	EXPECT_THAT(it->Data(), AsArray<char>(it->Size(), ElementsAreArray(block_1)));
	++it;

	ASSERT_NE(it, q.end());
	EXPECT_THAT(it->Data(), AsArray<char>(it->Size(), ElementsAreArray(block_2)));
	++it;

	ASSERT_NE(it, q.end());
	EXPECT_THAT(it->Data(), AsArray<char>(it->Size(), ElementsAreArray(block_3)));
	++it;

	EXPECT_EQ(it, q.end());

	q.Clear();
	EXPECT_EQ(q.begin(), q.end());
}

TEST(BlockQueue, ConstIterators)
{
	vs::BlockQueue q;
	const vs::BlockQueue& qr = q;
	EXPECT_EQ(qr.begin(), qr.end());
	q.PushBack(block_1, vs::size(block_1));
	q.PushBack(block_2, vs::size(block_2));
	q.PushBack(block_3, vs::size(block_3));

	vs::BlockQueue::const_iterator it = qr.begin();

	ASSERT_NE(it, qr.end());
	EXPECT_THAT(it->Data(), AsArray<char>(it->Size(), ElementsAreArray(block_1)));
	++it;

	ASSERT_NE(it, qr.end());
	EXPECT_THAT(it->Data(), AsArray<char>(it->Size(), ElementsAreArray(block_2)));
	++it;

	ASSERT_NE(it, qr.end());
	EXPECT_THAT(it->Data(), AsArray<char>(it->Size(), ElementsAreArray(block_3)));
	++it;

	EXPECT_EQ(it, qr.end());

	q.Clear();
	EXPECT_EQ(qr.begin(), qr.end());
}

}

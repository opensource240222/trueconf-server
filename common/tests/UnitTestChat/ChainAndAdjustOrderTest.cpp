#include "globvars.h"

#include "chatlib/chain/adjust_msg_order.h"
#include "chatlib/chain/ChainContainer.h"
#include "chatlib/chain/ChainOfMessages.h"
#include "chatlib/msg/attr.h"
#include "chatlib/msg/chat_messages_construct.h"
#include "chatlib/utils/chat_utils.h"
#include "chatlib/storage/make_chat_storage.h"
#include "chatutils/GlobalConfigStub.h"

#include "std-generic/compat/algorithm.h"
#include "std-generic/compat/memory.h"
#include "std-generic/cpplib/macro_utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <random>
#include <fstream>
#include <sstream>

using ::testing::ContainerEq;
using ::testing::get;
using ::testing::Pointwise;
using ::testing::SizeIs;
namespace
{
struct PutMessageToChainParam
{
	PutMessageToChainParam(uint32_t chain_count, uint32_t msg_count)
		: count_of_chains(chain_count)
		, count_of_messages(msg_count)
	{}
	uint32_t count_of_chains; //particioants
	uint32_t count_of_messages;
};
class PutMessageToChain :
	public ::testing::TestWithParam < PutMessageToChainParam>
{
public:
	std::random_device rnd_dev;
};
class ChainReadWrite
{
	std::fstream file_;
	std::stringstream ss_;
	bool write_immediately_ = false;
	std::string file_name_;
protected:
	virtual ~ChainReadWrite()
	{
		if (write_immediately_)
			return;
		if (!file_name_.empty() && ss_.rdbuf()->in_avail() > 0)
			file_.open(file_name_, std::fstream::out);
		if (file_.is_open() && ss_.rdbuf()->in_avail() > 0)
			file_ << ss_.rdbuf();
	}
ChainReadWrite(
	const std::string & file_name,
	std::ios_base::openmode mode,
	const bool write_immediately = false)
	: file_(file_name,mode)
	, write_immediately_(write_immediately)
{}
ChainReadWrite(const std::string &file_name) : file_name_(file_name)
{}
virtual void LogItem(
	const bool isOutgoing,
	const chat::CallID& call_id,
	const chat::ChatMessageID&id,
	const chat::ChatMessageID&prev,
	chat::ChatMessageTimestamp stamp)
{
	std::string dir = isOutgoing ? "out" : "in";
	std::string prev_id = prev.empty() ? "<none>" : prev;
	if (write_immediately_)
	{
		file_
			<< dir << " "
			<< call_id << " "
			<< id << " "
			<< prev_id << " "
			<< chat::timestamp_to_uint(stamp) << std::endl;
		file_.flush();
	}
	else
		ss_
		<< dir << " "
		<< call_id << " "
		<< id << " "
		<< prev_id << " "
		<< chat::timestamp_to_uint(stamp) << std::endl;
}

virtual bool GetNextItem(
	bool &isOutgoing,
	chat::CallID&call_id,
	chat::ChatMessageID&id,
	chat::ChatMessageID &prev,
	chat::ChatMessageTimestamp&stamp)
{
	if (file_.eof())
		return false;
	std::string dir;
	std::string prev_id;
	call_id.clear();
	id.clear();
	prev.clear();
	uint64_t tp_in_uint(0);
	file_ >> dir >> call_id >> id >> prev_id >> tp_in_uint;
	stamp = chat::uint_to_timestamp(tp_in_uint);
	isOutgoing = dir == "out" ? true : false;
	prev = prev_id == "<none>" ? "" : prev_id;
	return !call_id.empty();
}
public:
bool IsOpen() const
{
	return file_.is_open();
}
void Done()
{
	file_name_.clear();
	file_.close();
}

};
class ChainLogWriter : public ChainReadWrite
{
public:
	ChainLogWriter(const std::string &fname, const bool write_immediately)
		: ChainReadWrite(
			fname,
			std::fstream::out,
			write_immediately)
	{}
	ChainLogWriter(const std::string &fname) :ChainReadWrite(fname)
	{}
	virtual void LogItem(
		bool isOutgoing,
		const chat::CallID& call_id,
		const chat::ChatMessageID&id,
		const chat::ChatMessageID&prev,
		chat::ChatMessageTimestamp stamp) override
	{
		ChainReadWrite::LogItem(isOutgoing, call_id, id, prev, stamp);
	}
};
class ChainLogReader : public ChainReadWrite
{
public:
	ChainLogReader(const std::string &fname)
		: ChainReadWrite(fname,std::ios_base::in)
	{}
	virtual bool GetNextItem(
		bool &isOutgoing,
		chat::CallID&call_id,
		chat::ChatMessageID&id,
		chat::ChatMessageID &prev,
		chat::ChatMessageTimestamp&stamp) override
	{
		return ChainReadWrite::GetNextItem(
			isOutgoing,
			call_id,
			id,
			prev,
			stamp);
	}
};
class Participant
{
	std::shared_ptr<GlobalConfigStub> cfg_;
	chat::CallID from_;
	chat::CallID from_instance_;
	vs::CallIDType sender_type_;
	chat::ChatID chat_id_;
	chat::ChainOfMessages chain_;
	std::vector< chat::msg::ChatMessagePtr> incomming_; // must be shuffeled
	std::mt19937 g_;
	uint32_t count_msg_;
	uint32_t index_;
	ChainLogWriter &chain_log_;

public:
	Participant(
		std::random_device&rnd,
		const std::shared_ptr<GlobalConfigStub> &cfg,
		chat::ChatIDRef id,
		chat::CallIDRef from,
		chat::CallIDRef from_instance,
		vs::CallIDType call_id_type,
		const uint32_t &count_msg,
		ChainLogWriter&log)
		: cfg_(cfg)
		, from_(from)
		, from_instance_(from_instance)
		, sender_type_(call_id_type)
		, chat_id_(id)
		, g_(rnd())
		, count_msg_(count_msg)
		, index_(0)
		, chain_log_(log)
	{
		chain_.Init(cfg);
	}
	template<typename OutputIt>
	void  GetMsgIdxInOrder(OutputIt out)
	{
		chain_.GetMsgIdxInOrder(chat_id_, out);
	}
	template<class OutputIt >
	OutputIt CopyFromContainer(OutputIt d_first) const
	{
		return chain_.CopyFromContainer(chat_id_, d_first);
	}
	const chat::chain::ChainContainer &GetChain() const
	{
		return chain_.GetChain(chat_id_);
	}
	void PutIncommingMessage(chat::msg::ChatMessagePtr&&msg) // incomming
	{
		incomming_.push_back(std::move(msg));
		Tick();
	}
	chat::msg::ChatMessagePtr GetMessage() //outgoing
	{
		if (index_ < count_msg_)
		{
			//generate next message
			std::string msg;
			msg += "From call id ";
			msg += from_;
			msg += " msg # ";
			msg += std::to_string(index_);
			auto m = chat::msg::ContentMessage{}.Text(msg).Seal(chat_id_, from_, from_instance_, sender_type_, {}, {});
			++index_;
			chain_.PutMessage(m, true);

			chat::ChatMessageID msg_id, prev;
			chat::ChatMessageTimestamp stamp;
			m->GetParam(chat::attr::MESSAGE_ID_paramName, msg_id);
			m->GetParam(chat::attr::PREVIOUS_MESSAGE_ID_paramName, prev);
			m->GetParam(chat::attr::TIMESTAMP_paramName, stamp);

			chain_log_.LogItem(true, from_, msg_id, prev, stamp);
			return m;
		}
		else
			return {};
	}
	bool IsFinished() const
	{
		return incomming_.empty() && index_ >= count_msg_;
	}
	void Tick()
	{
		if (incomming_.empty())
			return;
		// shiffle cached messages
		std::shuffle(incomming_.begin(), incomming_.end(), g_);
		std::uniform_int_distribution<uint16_t> dist(0, 1);
		uint32_t val = dist(g_);
		// emulate lost message (skip or not)
		if (1 == val)
		{
			auto m = std::move(incomming_.front());
			chat::ChatMessageID msg_id, prev;
			chat::ChatMessageTimestamp stamp;
			m->GetParam(chat::attr::MESSAGE_ID_paramName, msg_id);
			m->GetParam(chat::attr::PREVIOUS_MESSAGE_ID_paramName, prev);
			m->GetParam(chat::attr::TIMESTAMP_paramName, stamp);
			chain_log_.LogItem(false, from_, msg_id, prev, stamp);
			chain_.PutMessage(m, false);
			incomming_.erase(incomming_.begin());
		}
	}
};
class ChainAndAdjust : public ::testing::TestWithParam<size_t> // size of elements
{
protected:
	std::random_device rnd_dev;
};
class TestChainFromFile : public ::testing::TestWithParam<std::string>
{};
struct AdjustMessageOrderTestParam
{
	AdjustMessageOrderTestParam(
		size_t num_of_elements,
		size_t gap,
		size_t init_sz)
		: num_of_elements(num_of_elements)
		, gap(gap)
		, init_sz(init_sz)
	{}
	size_t num_of_elements;
	size_t gap;
	size_t init_sz;
};
class AdjustMessageOrderTest :
	public ::testing::TestWithParam<AdjustMessageOrderTestParam>
{
	size_t gap_;
	size_t elements_count_;
	size_t old_gap_;
	size_t vector_sz_;
	vs::map<
		chat::ChatMessageID,
		chat::storage_utils::order_info> result_data_;
	decltype(result_data_.end()) last_left_ = result_data_.end();
	decltype(result_data_.end()) last_right_ = result_data_.end();
protected:
	std::random_device rd_;
	std::vector<chat::storage_utils::order_info> source_;
	void SetUp() override
	{
		elements_count_ = GetParam().num_of_elements;
		gap_ = GetParam().gap;
		vector_sz_ = GetParam().init_sz;
		old_gap_ = chat::storage_utils::get_gap();
		chat::storage_utils::set_gap(gap_);
		for (auto i = 0u; i < elements_count_; ++i)
			source_.emplace_back(std::to_string(i), 0);
		std::sort(
			source_.begin(),
			source_.end(),
			[](const chat::storage_utils::order_info &x1,
				const chat::storage_utils::order_info &x2)
					{return x1.id < x2.id; });
	}
	void TearDown() override
	{
		chat::storage_utils::set_gap(old_gap_);
	}
	bool CheckOrder() const
	{
		if (result_data_.size() != source_.size())
			return false;
		if (source_.size() == 1)
			return true;
		auto iter = std::make_pair(source_.begin(), source_.begin() + 1);
		for (; iter.second != source_.end(); ++iter.first, ++iter.second)
		{
			auto first = result_data_.find(iter.first->id);
			auto second = result_data_.find(iter.second->id);
			EXPECT_NE(first, result_data_.end());
			EXPECT_NE(second, result_data_.end());
			if (first->second.order >= second->second.order)
				return false;
		}
		return true;
	}
void AddResult(const chat::storage_utils::adjust_result &adj_res)
{
	for (auto i = adj_res.first; i != adj_res.last; ++i)
		result_data_[i->id] = *i;
	ASSERT_TRUE(std::is_sorted(
		result_data_.begin(),
		result_data_.end(),
		[&](
			const std::pair<
				chat::ChatMessageID,
				chat::storage_utils::order_info> &lhs,
			const std::pair<
				chat::ChatMessageID,
				chat::storage_utils::order_info> &rhs)
		{
			return lhs.second.order < rhs.second.order
				&& lhs.second.id < rhs.second.id;
		}));
	last_left_ = result_data_.end();
	last_right_ = result_data_.end();
}
std::vector<chat::storage_utils::order_info>
GetStartData(const chat::ChatMessageID& id/*id for insert*/)
{
	std::vector<chat::storage_utils::order_info> res;
	if (0 == vector_sz_)
	{
		for (const auto &i : result_data_)
			res.push_back(i.second);
	}
	else if (result_data_.empty())
		return res;
	else
	{
		auto current = std::lower_bound(
			result_data_.begin(),
			result_data_.end(),
			id,
			[](
				const decltype(result_data_)::value_type &info,
				const chat::ChatMessageID &id)
			{return info.first < id; });
		if (current == result_data_.begin())
		{
			last_right_ = result_data_.begin();
			return res;
		}
		--current;
		if (1 == vector_sz_)
		{
			res.push_back(current->second);
			last_left_ = last_right_ = current;
			++last_right_;
		}
		else
		{
			int64_t shift_left = vector_sz_ / 2;
			int64_t shift_right = vector_sz_ - shift_left;
			auto left_iter = current;
			auto right_iter = current;
			if (std::distance(result_data_.begin(), left_iter) < shift_left)
				left_iter = result_data_.begin();
			else if (shift_left > 0)
				std::advance(left_iter, std::negate<int64_t>()(shift_left));
			if (std::distance(right_iter, result_data_.end()) < shift_right)
				right_iter = result_data_.end();
			else
				std::advance(right_iter, shift_right);
			std::transform(
				left_iter,
				right_iter,
				std::back_inserter(res),
				[](const decltype(result_data_)::value_type &val)
				{return val.second; });
			last_left_ = left_iter;
			last_right_ = right_iter;
		}
	}
	EXPECT_TRUE(std::is_sorted(res.begin(), res.end(),
		[](
			const chat::storage_utils::order_info&first,
			const chat::storage_utils::order_info &second)
		{return std::tie(first.order, first.id)
			< std::tie(second.order, second.id);}));
	return res;
}
std::vector<chat::storage_utils::order_info> GetMore(
	chat::storage_utils::next_or_prec get_where)
{
	using chat::storage_utils::next_or_prec;
	using chat::storage_utils::order_info;
	std::vector<chat::storage_utils::order_info> res;
	if (0 == vector_sz_)
		return res;
	if (next_or_prec::next == get_where)
	{
		if (last_right_ != result_data_.end())
		{
			auto new_right = last_right_;
			if (static_cast<size_t>(std::distance(
					last_right_,
					result_data_.end())) < vector_sz_)
				new_right = result_data_.end();
			else
				std::advance(new_right, vector_sz_);
			std::transform(
				last_right_,
				new_right,
				std::back_inserter(res),
				[](const std::pair<chat::ChatMessageID, order_info>&val)
				{return val.second; });
			last_right_ = new_right;
		}
	}
	else if(next_or_prec::preceding == get_where)
	{
		if (last_left_ != result_data_.begin()
			&& last_left_ != result_data_.end())
		{
			auto new_left = last_left_;
			if (static_cast<size_t>(std::distance(
					result_data_.begin(),
					last_left_)) < vector_sz_)
				new_left = result_data_.begin();
			else
				std::advance(new_left, std::negate<int64_t>()(vector_sz_));
			std::transform(
				new_left,
				last_left_,
				std::back_inserter(res),
				[](const std::pair<chat::ChatMessageID, order_info>&val)
				{return val.second; });
			last_left_ = new_left;
		}
	}
	else
	{
		EXPECT_TRUE(false) << "process all values from enum next_or_prec!";
	}
	return res;
}
};
struct msg_descr
{
	VS_FORWARDING_CTOR3(msg_descr, msg_id, parent_id, timestamp) {}
	chat::ChatMessageID msg_id;
	chat::ChatMessageID parent_id;
	chat::ChatMessageTimestamp timestamp;
	bool operator ==(const msg_descr&other)
	{
		return other.msg_id == msg_id
			&& other.parent_id == parent_id
			&& other.timestamp == timestamp;

	}
};
bool eq_chain_items(const msg_descr &lhs, const chat::storage_utils::order_info &rhs)
{
	return lhs.msg_id == rhs.id;
}
MATCHER(ChainEq, "")
{
	return eq_chain_items(get<0>(arg), get<1>(arg));
}
}

TEST_P(PutMessageToChain, ShuffledMessagesTest)
{
	std::string file_name = ".\\ShuffledMessagesTest"
		+ std::to_string(GetParam().count_of_chains)
		+ "_"
		+ std::to_string(GetParam().count_of_messages)
		+ ".in";
	ChainLogWriter chain_log(file_name);

	auto cfg = std::make_shared<GlobalConfigStub>(nullptr);
	cfg->SetBucketCapacity(c_default_bucket_capacity);
	cfg->SetMaxChainLen(c_default_chain_len);
	auto storage = chat::make_chat_storage("sqlite3:db=file::memory:?cache=shared");
	cfg->SetChatStorage(storage);

	auto part_count = GetParam().count_of_chains;
	std::vector<chat::ChatMessageID> order_outgoing_msg;
	auto count_of_msgs = GetParam().count_of_messages;
	order_outgoing_msg.reserve(count_of_msgs);
	std::vector<Participant> parts;
	chat::ChatID id = "#Chat Id#";
	for (auto i = 0u; i < part_count; ++i)
	{
		chat::CallID from = "User#" + std::to_string(i);
		parts.emplace_back(
				rnd_dev,
				cfg,
				id,
				from,
				from,
				vs::CallIDType::client,
				count_of_msgs,
				chain_log);
	}
	auto isFinish = [&parts]()->bool
	{
		for (const auto &i : parts)
		{
			if (!i.IsFinished())
				return false;
		}
		return true;
	};
	std::uniform_int_distribution<size_t> dist(0, parts.size() - 1);
	while (!isFinish())
	{
		auto outgoing_index = dist(rnd_dev);
		auto &part = parts[outgoing_index];
		auto msg = part.GetMessage();
		if (nullptr != msg)
		{
			chat::ChatMessageID msg_id;
			ASSERT_TRUE(msg->GetParam(
				chat::attr::MESSAGE_ID_paramName, msg_id));
			order_outgoing_msg.push_back(msg_id);
		}
		for (auto &i : parts)
		{
			if (nullptr == msg)
				i.Tick();
			else if (&i != &part)
			{
				auto copy = vs::make_unique<chat::msg::ChatMessage>(
					msg->GetContainer());
				i.PutIncommingMessage(std::move(copy));
			}
		}
	}
	//test that all chains are equeal
	std::vector<chat::ChatMessageID> for_cmp;
	parts[0].GetMsgIdxInOrder(std::back_inserter(for_cmp));
	ASSERT_EQ(for_cmp.size(), order_outgoing_msg.size());
	for (auto &i : parts)
	{
		std::vector<chat::ChatMessageID> ordered_chain;
		i.GetMsgIdxInOrder(
			std::back_inserter(ordered_chain));
		ASSERT_EQ(for_cmp, ordered_chain);
	}
	for (auto &i : parts)
	{
		std::vector<chat::chain::detail::Data> chain;
		i.CopyFromContainer(std::back_inserter(chain));

		auto check_by_timestamp = [](
			const chat::chain::detail::Data&first,
			const chat::chain::detail::Data&second) -> bool
		{
			return first.timestamp > second.timestamp;
		};
		vs::set<chat::ChatMessageID> previous_idx;
		auto check_by_parent = [&previous_idx]
		(const chat::chain::detail::Data&x) -> bool
		{
			auto& id = x.id;
			auto& parent = x.parent;
			EXPECT_TRUE(previous_idx.find(id) == previous_idx.end());
			EXPECT_NE(x.id, x.parent);
			if (!parent.empty())
			{
				if (previous_idx.end() == previous_idx.find(x.parent))
					return true;
			}
			previous_idx.insert(x.id);
			return false;
		};
		auto bad_element = std::adjacent_find(
			chain.begin(),
			chain.end(),
			check_by_timestamp);
		ASSERT_TRUE(bad_element == chain.end());
		bad_element = std::find_if(
			chain.begin(),
			chain.end(),
			check_by_parent);
		ASSERT_TRUE(bad_element == chain.end());
		// check corresponding order_num and order in container
		const auto& container = i.GetChain();
		ASSERT_TRUE(container.integrity_check());
	}
	chain_log.Done();
}

INSTANTIATE_TEST_CASE_P(
	CheckChainConstruction,
	PutMessageToChain,
	::testing::Values( PutMessageToChainParam(2, 10)
		, PutMessageToChainParam(2, 100)
		, PutMessageToChainParam(3, 10)
		//, PutMessageToChainParam(6, 100)
		//, PutMessageToChainParam(7, 50)
		//, PutMessageToChainParam(8, 100)
		//, PutMessageToChainParam(2, 1243)
		//, PutMessageToChainParam(3, 1243)
	));

TEST_P(PutMessageToChain, FillChainsAndCompareResultSimple)
{
	std::vector<std::tuple<
		std::shared_ptr<GlobalConfigStub>,
		chat::CallID,
		chat::ChainOfMessages>> chains;
	uint32_t count_of_chains = GetParam().count_of_chains;
	chat::ChatID chat_id = "TEST CHAT";
	auto storage = chat::make_chat_storage("sqlite3:db=file::memory:?cache=shared");
	auto cfg = std::make_shared<GlobalConfigStub>(nullptr);
	cfg->SetBucketCapacity(c_default_bucket_capacity);
	cfg->SetMaxChainLen(c_default_chain_len);
	cfg->SetChatStorage(storage);
	cfg->SetTailLength(c_default_tail_len);
	for (uint32_t i = 0; i < count_of_chains; i++)
	{
		chat::ChainOfMessages chain;
		chain.Init(cfg);
		chains.emplace_back(
			cfg,
			chat::CallID("From_" + std::to_string(i)),
			std::move(chain));
	}
	std::uniform_int_distribution<uint32_t> distr(0, count_of_chains - 1);

	uint32_t msg_count = GetParam().count_of_messages;

	chat::ChatMessageID previous_id;
	for (uint32_t i = 0; i < msg_count; ++i)
	{
		uint32_t index_from = distr(rnd_dev);
		std::string msg = "msg # ";
		msg += std::to_string(i);
		auto chat_msg = chat::msg::ContentMessage{}.Text(msg).Seal(
			chat_id,
			std::get<1>(chains[index_from]),
			std::get<1>(chains[index_from]),
			vs::CallIDType::client,
			{},
			{});

		{
			auto put_res = std::get<2>(chains[index_from]).PutMessage(
				chat_msg,
				true);
			ASSERT_NE(put_res.range.first, put_res.range.last);
		}

		ASSERT_TRUE(chat_msg->GetParam(
			chat::attr::PREVIOUS_MESSAGE_ID_paramName,
			previous_id));

		for (uint32_t j = 0; j < count_of_chains; ++j)
		{
			if (j == index_from)
				continue;
			auto put_res = std::get<2>(chains[j]).PutMessage(chat_msg, false);
			ASSERT_NE(put_res.range.first, put_res.range.last);
		}
	}

	/**
	for all messages get msg id in order and compare
	*/
	std::vector<chat::ChatMessageID> order_first;
	order_first.reserve(msg_count);
	std::get<2>(chains[0]).GetMsgIdxInOrder(
		chat_id,
		std::back_inserter(order_first));
	for (uint32_t i = 1; i < count_of_chains; ++i)
	{
		std::vector<chat::ChatMessageID> order_second;
		order_second.reserve(msg_count);
		std::get<2>(chains[i]).GetMsgIdxInOrder(
			chat_id,
			std::back_inserter(order_second));
		ASSERT_EQ(order_first, order_second);
		ASSERT_TRUE(std::get<2>(chains[i]).GetChain(chat_id).integrity_check());
	}
}
TEST(ChainPredefined_simple, ChainContainerTest)
{
	/**
	1 - 2 - 3 - 4 - 5 - 6;
	1: 1
	2: 1 - 2
	4: 1 - 4 - 2
	6: 1 - 6 - 4 - 2
	3: 1 - 6 - 2 - 3 - 4
	5: 1 - 2 - 3 - 4 - 5 - 6
	*/
	using namespace chat;
	std::vector<msg_descr> orig{
		{ "1","",  uint_to_timestamp(1) },
		{ "2","1", uint_to_timestamp(4) },
		{ "3","2", uint_to_timestamp(5) },
		{ "4","3", uint_to_timestamp(3) },
		{ "5","4", uint_to_timestamp(6) },
		{ "6","5", uint_to_timestamp(2) }
	};
	std::vector<uint32_t> test_order{ 0,1,3,5,2,4 };
	chain::ChainContainer chain;
	std::vector<storage_utils::order_info> result_order;
	for (auto i : test_order)
	{
		auto where_it = chain.insert(
			orig[i].msg_id,
			orig[i].parent_id,
			orig[i].timestamp);
		bool is_first = where_it == chain.begin();
		auto prev_it = where_it;
		auto prev = std::make_pair(is_first, is_first
			? chain.end()
			: --prev_it);
		uint64_t order_after_index(-1);
		if (!prev.first)
		{
			auto after = std::find_if(
				result_order.begin(), result_order.end(),
				[&](const storage_utils::order_info &inf)
				{return inf.id == prev.second->id; });
			ASSERT_NE(after, result_order.end());
			order_after_index = after->order;
		}
		for (auto it = where_it; it != chain.end(); ++it)
		{
			storage_utils::order_info val(it->id, UINT64_MAX);
			result_order.erase(
				std::remove_if(
					result_order.begin(),
					result_order.end(),
					[&](const storage_utils::order_info &inf)
					{return inf.id == it->id; }),
				result_order.end());
			auto adj_res = adjust_msg_order(
				result_order,
				order_after_index,
				val,
				[](storage_utils::next_or_prec)
				{return std::vector<storage_utils::order_info>(); });
			order_after_index = result_order[adj_res.where_inserted].order;
			++prev.second;
			prev.first = false;
		}
	}
	EXPECT_THAT(orig, Pointwise(ChainEq(), result_order));
}
void DoChainAndAdjust_ShuffleOrder(
	const std::vector<msg_descr> &orig,
	const std::vector<size_t> &test_order)
{
	chat::chain::ChainContainer chain;
	uint64_t insert_after_index = std::numeric_limits<uint64_t>::max();
	std::vector<chat::storage_utils::order_info> result_order;
	vs::map<chat::ChatMessageID, uint64_t> updated_order;
	for (auto i : test_order)
	{
		auto where_it = chain.insert(
			orig[i].msg_id,
			orig[i].parent_id,
			orig[i].timestamp);
		if (where_it == chain.begin())
			insert_after_index = std::numeric_limits<uint64_t>::max();
		else
		{
			auto prev_it = where_it;
			--prev_it;
			auto prev_order_it = updated_order.find(prev_it->id);
			ASSERT_NE(prev_order_it, updated_order.end());
			insert_after_index = prev_order_it->second;
		}
		for (auto it = where_it; it != chain.end(); ++it)
		{
			if (insert_after_index != std::numeric_limits<uint64_t>::max())
			{
				auto next_it = std::upper_bound(
					result_order.begin(), result_order.end(),
					insert_after_index,
					[](uint64_t val, const chat::storage_utils::order_info& inf)
				{return val < inf.order; });
				if (next_it != result_order.end()
					&& next_it->id == it->id)
				{
					insert_after_index = next_it->order;
					continue;
				}
			}
			chat::storage_utils::order_info val(it->id, UINT64_MAX);
			{
				auto cur_val_it = updated_order.find(it->id);
				if (cur_val_it != updated_order.end())
				{
					auto val_for_erase = std::lower_bound(
						result_order.begin(), result_order.end(),
						cur_val_it->second,
						[](const chat::storage_utils::order_info &inf, const uint64_t val)
					{return inf.order < val; });
					ASSERT_EQ(val_for_erase->id, it->id);
					result_order.erase(val_for_erase);
				}
			}
			auto adj_res = adjust_msg_order(
				result_order,
				insert_after_index,
				val,
				[](chat::storage_utils::next_or_prec)
			{return std::vector<chat::storage_utils::order_info>(); });
			for (auto it1 = adj_res.first; it1 != adj_res.last; ++it1)
				updated_order[it1->id] = it1->order;
			ASSERT_GT(result_order.size(), adj_res.where_inserted);
			ASSERT_EQ(result_order[adj_res.where_inserted].id, it->id);
			insert_after_index = result_order[adj_res.where_inserted].order;
		}
	}
	EXPECT_THAT(orig, Pointwise(ChainEq(), result_order));
	for (const auto &i : result_order)
	{
		auto upd_iter = updated_order.find(i.id);
		ASSERT_NE(upd_iter, updated_order.end());
		ASSERT_EQ(upd_iter->second, i.order);
		updated_order.erase(upd_iter);
	}
	ASSERT_EQ(updated_order.size(), 0u);
}
TEST_P(ChainAndAdjust, ShuffleOrder)
{
	using namespace chat;

	std::vector<msg_descr> orig;
	orig.reserve(GetParam());
	std::vector<size_t> test_order;
	test_order.reserve(GetParam());
	// fill orig vector
	std::uniform_int_distribution<uint64_t> distr(0, GetParam());
	std::string prev_id = ""; // first is empty string
	for (size_t i = 0; i < GetParam(); ++i)
	{
		orig.emplace_back(
			std::to_string(i),
			prev_id,
			uint_to_timestamp(distr(rnd_dev)));
		test_order.push_back(i);
		prev_id = orig.back().msg_id;
	}
	// shuffle indexes for insert to chain
	std::mt19937 g(rnd_dev());
	std::shuffle(test_order.begin(), test_order.end(), g);
	DoChainAndAdjust_ShuffleOrder(orig, test_order);
	if (HasFailure())
	{
		std::string file_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
		file_name += ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name();
		file_name += ".log";
		std::replace(file_name.begin(), file_name.end(), '/', '_');
		// 1. original msg_id, parent_id, timestamp
		// 2. test_order
		std::ofstream out(file_name);
		out << "original\n";
		for (const auto &i : orig)
			out <<
			i.msg_id << ' ' <<
			(i.parent_id.empty() ? "empty" : i.parent_id) << ' ' <<
			chat::timestamp_to_uint(i.timestamp) << '\n';
		out << '\n';
		out << "test_order\n";
		for (const auto &i : test_order)
			out << i << '\n';
	}

}
INSTANTIATE_TEST_CASE_P(
	ChainInserAdjustOrder,
	ChainAndAdjust,
	::testing::Values(
		0x10,
		0x20,
		0x50,
		0x100/*,
		0x200,
		0x500
		*/

	));

TEST_P(TestChainFromFile, SortDebugFromFile)
{
	std::string case_name = ::testing::UnitTest::GetInstance()->current_test_case()->name();
	if (case_name.find("ShuffledMessagesTest") != std::string::npos)
	{
		ChainLogReader reader(GetParam());
		if (!reader.IsOpen())
			return;
		bool isOut(false);
		chat::CallID c_id;
		chat::ChatMessageID id;
		chat::ChatMessageID prev;
		chat::ChatMessageTimestamp stamp;
		vs::map<chat::CallID, chat::chain::ChainContainer> chains;
		uint32_t counter(0);
		while (reader.GetNextItem(isOut, c_id, id, prev, stamp))
		{
			auto it = chains.find(c_id);
			if (it == chains.end())
				it = chains.emplace(c_id, chat::chain::ChainContainer()).first;
			if (isOut)
				it->second.push_back(id, stamp);
			else
				it->second.insert(id, prev, stamp);
			++counter;
		}
		if (chains.empty())
			return;
		std::vector<std::tuple<
			chat::ChatMessageID,
			chat::ChatMessageID,
			chat::ChatMessageTimestamp>> first_idx;

		auto first = chains.begin();
		for (auto &i : first->second)
			first_idx.emplace_back(
				i.id,
				i.parent,
				i.timestamp);

		for (auto i = ++first; i != chains.end(); i++)
		{
			std::vector<std::tuple<
				chat::ChatMessageID,
				chat::ChatMessageID,
				chat::ChatMessageTimestamp>> idx_for_cmp;
			for (auto &j : i->second)
				idx_for_cmp.emplace_back(
					j.id,
					j.parent,
					j.timestamp);

			ASSERT_EQ(idx_for_cmp.size(), first_idx.size());
			for (size_t i = 0; i < first_idx.size(); i++)
				ASSERT_EQ(idx_for_cmp[i], first_idx[i])
				<< "index = " << i << '\n';
		}
		for (auto &i : chains)
		{
			std::vector<chat::chain::detail::Data> chain;
			std::copy(i.second.begin(), i.second.end(), std::back_inserter(chain));
			auto check_by_timestamp = [](
				const chat::chain::detail::Data&first,
				const chat::chain::detail::Data&second) -> bool
			{
				auto f_t = first.timestamp;
				auto s_t = second.timestamp;
				if (!first.parent.empty() && second.parent.empty())
					return true;
				if ((first.parent.empty() && second.parent.empty())
					|| (!first.parent.empty()
						&& !second.parent.empty()))
					return f_t > s_t;
				return false;
			};

			vs::set<chat::ChatMessageID> previous_idx;
			auto check_by_parent = [&previous_idx]
			(const chat::chain::detail::Data&x) ->bool
			{
				auto& id = x.id;
				auto& parent = x.parent;

				bool val = previous_idx.find(id) == previous_idx.end();

				EXPECT_TRUE(val);
				EXPECT_NE(x.id, x.parent);;

				if (!parent.empty())
				{
					if (previous_idx.end() == previous_idx.find(x.parent))
						return true;
				}
				previous_idx.insert(x.id);
				return false;
			};
			auto bad_element = std::adjacent_find(
				chain.begin(),
				chain.end(),
				check_by_timestamp);
			ASSERT_TRUE(bad_element == chain.end())
				<< "chain for  " << i.first
				<< " index # " << std::distance(chain.begin(), bad_element) << '\n';
			bad_element = std::find_if(
				chain.begin(),
				chain.end(),
				check_by_parent);
			ASSERT_TRUE(bad_element == chain.end())
				<< "chain for  " << i.first
				<< " index # " << std::distance(chain.begin(), bad_element) << '\n';
		}
	}
	else if(case_name.find("ShuffleOrder") != std::string::npos)
	{
		std::ifstream in(GetParam());
		if (!in.is_open())
			return;
		std::string first_line;
		std::getline(in, first_line);
		std::vector<msg_descr> orig;
		std::vector<size_t> test_order;
		std::vector<chat::storage_utils::order_info> result_order;

		ASSERT_EQ(first_line, "original") << "file format is wrong!";
		for(std::string line; line != "test_order" && !in.eof();)
		{
			std::getline(in, line);
			std::istringstream iss(line);
			std::vector<std::string> words{ std::istream_iterator<std::string>{iss},
						std::istream_iterator<std::string>{} };
			if (words.size() != 3)
				continue;

			if (words[1] == "empty")
				words[1].clear();
			orig.emplace_back(
				words[0], words[1], chat::uint_to_timestamp(::atoll(words[2].c_str())));
		}
		for(std::string line; !in.eof();)
		{
			in >> line;
			if (line.empty())
				break;
			test_order.emplace_back(::atoi(line.c_str()));
		}
		DoChainAndAdjust_ShuffleOrder(orig, test_order);
	}
}
//INSTANTIATE_TEST_CASE_P(
//	ShuffledMessagesTest,
//	TestChainFromFile,
//	::testing::Values(std::string(".\\ShuffledMessagesTest2_10.in")
//		, std::string(".\\ShuffledMessagesTest2_100.in")
//		, std::string(".\\ShuffledMessagesTest3_10.in")
//	));
//INSTANTIATE_TEST_CASE_P(
//	ShuffleOrder,
//	TestChainFromFile,
//	::testing::Values(std::string("./ShuffleOrder_0ChainInserAdjustOrder_CHAT_ChainAndAdjust.log")
//		, std::string("./ShuffleOrder_1ChainInserAdjustOrder_CHAT_ChainAndAdjust.log")
//		, std::string("./ ShuffleOrder_2ChainInserAdjustOrder_CHAT_ChainAndAdjust.log")
//		, std::string("./ShuffleOrder_3ChainInserAdjustOrder_CHAT_ChainAndAdjust.log")
//	));

TEST_P(AdjustMessageOrderTest, AdjustOrderConsistently)
{
	using namespace chat::storage_utils;
	for (const auto &i : source_)
	{
		auto v = GetStartData(i.id);
		auto insert_after = v.empty() ? -1 : v.back().order;
		auto adj_res = adjust_msg_order(
			v,
			insert_after,
			i,
			[this](chat::storage_utils::next_or_prec get_where)
			{return GetMore(get_where); });
		AddResult(adj_res);
	}
	ASSERT_TRUE(CheckOrder());
}
TEST_P(AdjustMessageOrderTest, AdjustOrderShaffle)
{
	using namespace chat::storage_utils;
	auto shaffled_order = source_;
	std::shuffle(
		shaffled_order.begin(),
		shaffled_order.end(),
		std::mt19937(rd_()));
	for (const auto &i : shaffled_order)
	{
		auto v = GetStartData(i.id);
		uint64_t insert_after(0);
		auto lower = std::lower_bound(
			v.begin(), v.end(),
			i.id,
			[](const order_info&info, const chat::ChatMessageID& val)
			{return info.id < val; });
		if (lower == v.begin())
			insert_after = -1;
		else
			insert_after = (--lower)->order;
		auto adj_res = adjust_msg_order(
			v,
			insert_after,
			i,
			[this](chat::storage_utils::next_or_prec get_where)
			{return GetMore(get_where); });
		AddResult(adj_res);
	}
	ASSERT_TRUE(CheckOrder());
}
INSTANTIATE_TEST_CASE_P(AdjustOrderTest,
	AdjustMessageOrderTest,
	::testing::Values( AdjustMessageOrderTestParam(10, 4, 0)
		, AdjustMessageOrderTestParam(10, 0xff, 0)
		, AdjustMessageOrderTestParam(10, 1000000, 0)

		, AdjustMessageOrderTestParam(10, 4, 1)
		, AdjustMessageOrderTestParam(10, 0xff, 1)
		, AdjustMessageOrderTestParam(10, 1000000, 1)

		, AdjustMessageOrderTestParam(10, 4, 2)
		, AdjustMessageOrderTestParam(10, 0xff, 2)
		, AdjustMessageOrderTestParam(10, 1000000, 2)

		, AdjustMessageOrderTestParam(10, 4, 10)
		, AdjustMessageOrderTestParam(10, 0xff, 10)
		, AdjustMessageOrderTestParam(10, 1000000, 10)

		, AdjustMessageOrderTestParam(10, 4, 100)
		, AdjustMessageOrderTestParam(10, 0xff, 100)
		, AdjustMessageOrderTestParam(10, 1000000, 100)

		, AdjustMessageOrderTestParam(100, 4, 0)
		, AdjustMessageOrderTestParam(100, 0xff, 0)
		, AdjustMessageOrderTestParam(100, 1000000, 0)

		, AdjustMessageOrderTestParam(100, 4, 1)
		, AdjustMessageOrderTestParam(100, 0xff, 1)
		, AdjustMessageOrderTestParam(100, 1000000, 1)

		, AdjustMessageOrderTestParam(100, 4, 10)
		, AdjustMessageOrderTestParam(100, 0xff, 10)
		, AdjustMessageOrderTestParam(100, 1000000, 10)

		, AdjustMessageOrderTestParam(100, 4, 100)
		, AdjustMessageOrderTestParam(100, 0xff, 100)
		, AdjustMessageOrderTestParam(100, 1000000, 100)

		//, AdjustMessageOrderTestParam(1000, 4,0)
		//, AdjustMessageOrderTestParam(1000, 0xff,0)
		//, AdjustMessageOrderTestParam(1000, 1000000,0)

		//, AdjustMessageOrderTestParam(1000, 4, 1)
		//, AdjustMessageOrderTestParam(1000, 0xff, 1)
		//, AdjustMessageOrderTestParam(1000, 1000000, 1)

		//, AdjustMessageOrderTestParam(1000, 4, 10)
		//, AdjustMessageOrderTestParam(1000, 0xff, 10)
		//, AdjustMessageOrderTestParam(1000, 1000000, 10)

		//, AdjustMessageOrderTestParam(1000, 4, 100)
		//, AdjustMessageOrderTestParam(1000, 0xff, 100)
		//, AdjustMessageOrderTestParam(1000, 1000000, 100)

		//, AdjustMessageOrderTestParam(10000, 0xff,0)
		//, AdjustMessageOrderTestParam(10000, 0xfff,0)
		//, AdjustMessageOrderTestParam(10000, 0xffff,0)
		//, AdjustMessageOrderTestParam(10000, 1000000,0)

		//, AdjustMessageOrderTestParam(10000, 0xff, 1)
		//, AdjustMessageOrderTestParam(10000, 0xfff, 1)
		//, AdjustMessageOrderTestParam(10000, 0xffff, 1)
		//, AdjustMessageOrderTestParam(10000, 1000000, 1)

		//, AdjustMessageOrderTestParam(10000, 0xff, 100)
		//, AdjustMessageOrderTestParam(10000, 0xfff, 100)
		//, AdjustMessageOrderTestParam(10000, 1000000, 100)
	));
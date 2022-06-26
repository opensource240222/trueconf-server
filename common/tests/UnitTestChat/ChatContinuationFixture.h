#pragma once

#include "DBBackEndFixture.h"
#include "tests/UnitTestChat/SetLayerHelperSimple.h"
#include "tests/UnitTestChat/TestHelpers.h"

#include "chatutils/helpers.h"
#include "std-generic/cpplib/macro_utils.h"
#include "std/cpplib/ASIOThreadPool.h"

#include <boost/optional.hpp>
#include "std-generic/cpplib/string_view.h"

namespace chat_test
{
class ChatContinuationFixture : public DBBackEndFixture
{
	/**
		init chat and parts from dump;
	*/
	class ArrivedMsgCatchLayer : public SetLayerHelperSimple
	{
		std::function<void(chat::msg::ChatMessagePtr &&, chat::CallIDRef)> msg_arrived_;
	public:
		ArrivedMsgCatchLayer(const std::function<void(chat::msg::ChatMessagePtr&&,chat::CallIDRef)> &f)
			: msg_arrived_(f)
		{}
		void OnChatMessageArrived(
			chat::msg::ChatMessagePtr&& m,
			chat::CallIDRef sender) override final
		{
			msg_arrived_(std::move(m),sender);
		}
	};
	class ProxyAboveLayer : public chat::LayerInterface
	{
	private:
		void ForwardBelowMessage(chat::msg::ChatMessagePtr&&,
			std::vector<chat::ParticipantDescr>&&) override
		{
			//skip;
		}
		void ShutDown() override
		{}
	public:
		void SendMsgAbove(chat::msg::ChatMessagePtr &&msg, chat::CallIDRef from)
		{
			ForwardAboveMessage(std::move(msg),from);
		}
	};
	struct LostMsgsTestParams
	{
		VS_FORWARDING_CTOR6(LostMsgsTestParams,
			result, storage, msgs_for_insert, msg_receiver, msg_sink,cfg){}
		VS_FORWARDING_CTOR1(LostMsgsTestParams, result) {}
		::testing::AssertionResult result;
		chat::ChatStoragePtr storage;
		std::vector<chat::msg::ChatMessagePtr> msgs_for_insert;
		std::shared_ptr<ProxyAboveLayer> msg_receiver;
		std::shared_ptr<ArrivedMsgCatchLayer> msg_sink;
		chat::GlobalConfigPtr cfg;
	};
protected:
	static const uint32_t s_chain_len = 10;
	static const uint32_t s_bucket_capacity = 5;
	static const uint32_t s_max_chain_len = 10;
	static const uint32_t s_tail_length = 0x10;
public:
	ChatContinuationFixture();
	~ChatContinuationFixture();
protected:
	::testing::AssertionResult
		InitBS(bool empty_db, bool try_pg_db_init);
	::testing::AssertionResult
		InitPart(string_view user, bool empty_db);
	::testing::AssertionResult InitPartsDBFilled(bool try_pg_db_init = true);
	::testing::AssertionResult InitPartsFillDBExcept(std::vector<chat::CallID>&& call_ids, bool try_pg_db_init = true);
	std::vector<chat::chain_item>
		GetChainFromStorage(chat::ChatIDRef chat_id, const chat::ChatStoragePtr &storage);
	LostMsgsTestParams
		PrepareLostMessagesTestParams(chat::ChatIDRef chat_id,
			const std::function<void(chat::msg::ChatMessagePtr&&, chat::CallIDRef)> &msg_catch_func);
	::testing::AssertionResult
		OrderConsistencyCheck(chat::ChatIDRef chat_id, const chat::ChatStoragePtr& storage);
	::testing::AssertionResult
		FillChatWithPGBackEnd(string_view storage_config);
	::testing::AssertionResult ResurrectDB(chat::CallIDRef call_id, chat::CallID ep_name, SQLiteDBParam destination, bool make_empty, SQLiteDBParam source = SQLiteDBParam());
	size_t RecvMsgsCount(
		chat::MessageType msgType = chat::MessageType::undefined,
		const std::vector<chat::CallIDRef> &Ids = {}) const;
	size_t EpCount() const;
	void ClearHistory(bool forAll);
	void RemoveMessage(bool forAll);

	boost::optional<ChatUtilsEnvironment> env_;
	vs::ASIOThreadPool atp_;
	part_collection all_parts_, only_users_, only_bs_;
	chat::CallID instance_with_pg_backend_;
};
}

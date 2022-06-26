#include "ChatTestFixture.h"

#include "chatlib/msg/parse_message.h"
#include "chatlib/storage/ChatStorage.h"

#include <gtest/gtest.h>

#include <fstream>

TEST_F(ChatTestFixture, StorageSimpleCase)
{
	InitParticipantsWithMainStorage();
	RegisterAllInstancesInFakeRotuer();
	SendMessageToChatCase();
	std::vector<std::vector<std::string>> txt_msgs;
	for (const auto& i : participants_ctx_)
	{
		//orders in the chain and in the storage must be equal;
		auto storage = i.second->global_cfg->GetChatStorage();
		auto chain_from_db = storage->GetLastMessages(current_chat_id_,
			std::numeric_limits<int32_t>::max());
		// get all text message
		std::vector<std::string> part_msgs;
		for (const auto &id : chain_from_db)
		{
			auto msg = storage->GetMessage(id.msg_id, {});
			auto parsed_msg = chat::msg::detail::parse_text_msg(msg);
			if (parsed_msg.success)
				part_msgs.push_back(std::move(parsed_msg.msg.content.text));
		}
		txt_msgs.push_back(std::move(part_msgs));
		ASSERT_FALSE(txt_msgs.back().empty());
	}
	auto iter = std::adjacent_find(
		txt_msgs.begin(),
		txt_msgs.end(),
		std::not_equal_to<std::vector<std::string>>());
	ASSERT_EQ(iter, txt_msgs.end());
}
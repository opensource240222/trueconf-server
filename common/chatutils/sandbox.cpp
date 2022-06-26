#include "sandbox.h"

#include "ExternalComponentsStub.h"
#include "GlobalConfigStub.h"
#include "helpers.h"
#include "ResolverStub.h"
#include "TransportChannelRouter.h"

#include "chatlib/chain/ChainContainer.h"
#include "chatlib/chain/ChainOfMessages.h"
#include "chatlib/msg/attr.h"
#include "chatlib/utils/chat_utils.h"

#include "std/cpplib/ASIOThreadPool.h"
#include "std-generic/compat/memory.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

#include <memory>
#include <random>

std::random_device rd_;
std::mt19937 g_(rd_());

void insert_in_chain_bench()
{
	auto now = std::chrono::steady_clock::now();
	uint32_t msg_count = 1000000;
	chat::chain::ChainContainer chain1;
	{
		for (auto i = 0u; i < msg_count; ++i)
		{
			auto msg_id = chat::GenerateUUID();
			auto timestamp = chat::ChatMessageTimestamp(
				std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch()));
			chain1.push_back(msg_id, timestamp);
		}
	}
	auto diff = std::chrono::steady_clock::now() - now;
	std::cout << "Generation of " << msg_count
		<< " messages take " << std::chrono::duration_cast<std::chrono::seconds>(diff).count()
		<< "sec\n";
	now = std::chrono::steady_clock::now();
	{
		chat::chain::ChainContainer chain2;
		for (const auto &i : chain1)
			chain2.insert(i.id, i.parent, i.timestamp);
		std::cout << "Copy of " << msg_count
			<< " messages (correct order) take " << std::chrono::duration_cast<std::chrono::seconds>(diff).count()
			<< "sec\n";
	}
	std::vector<chat::chain::detail::Data> chain_vec;
	std::copy(chain1.begin(), chain1.end(), std::back_inserter(chain_vec));
	std::shuffle(chain_vec.begin(), chain_vec.end(),g_);
	now = std::chrono::steady_clock::now();
	{
		chat::chain::ChainContainer chain2;
		for (const auto &i : chain1)
			chain2.insert(i.id, i.parent, i.timestamp);
		std::cout << "Copy of " << msg_count
			<< " messages (random order) take " << std::chrono::duration_cast<std::chrono::seconds>(diff).count()
			<< "sec\n";
	}
}

void send_new_message_to_old_chat(const ChatUtilsEnvironment &env)
{
	/**
	use generated db ()

	send message to chat 2dccc9e22c6ad662ca5f7113880c6c577e11b328

	*/
	const uint32_t max_chain_len = 100;
	const uint32_t bucket_capacity = 5;
	const uint32_t tail_length = 0x10;
	const std::string test_chat_id = "2dccc9e22c6ad662ca5f7113880c6c577e11b328";
	auto parts = GenerateChatParts(3, 3, 1, max_chain_len,
									bucket_capacity,
									tail_length, false, env);

	auto & part = parts["user1@trueconf.com"][0];
	std::string data("Message to old chat from "
		+ part->account_->GetCallID()
		+ "; ep "
		+ part->account_->GetExactCallID());
	part->app_layer_->SendGroup(
		test_chat_id,
		chat::msg::ContentMessage{}.Text(data),
		[test_chat_id](
			chat::cb::ProcessingResult res,
			chat::ChatMessageIDRef msg_id,
			const chat::cb::MsgIdAndOrderInChain& updated_order)
		{
			auto order_in_chain = std::find_if(
				updated_order.begin(), updated_order.end(),
				[&](const auto& item) { return item.first == msg_id; });
			assert(order_in_chain != updated_order.end());
			std::cout << "res = " << static_cast<uint32_t>(res)
				<< "\n chat id =  " << test_chat_id
				<< "\n msg_id = " << msg_id
				<< "\n order_in_chain = " << order_in_chain->second.integral << '.' << order_in_chain->second.fractional;
		});
}
void chat_sandbox()
{
	vs::ASIOThreadPool atp(1);
	atp.Start();
	ChatUtilsEnvironment env{
		std::make_shared<TransportChannelRouter>(atp.get_io_service()),
		std::make_shared<ResolverStub>(),
		std::make_shared<steady_clock_wrapper>(),
		atp.get_io_service()
	};
	send_new_message_to_old_chat(env);
}

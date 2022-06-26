#include "helpers.h"
#include "ResolverStub.h"
#include "sandbox.h"
#include "TransportChannelRouter.h"

#include "tests/common/Utils.h"

#include "chatlib/utils/chat_utils.h"
#include "std/cpplib/ASIOThreadPool.h"
#include "std/cpplib/ThreadUtils.h"
#include "std-generic/cpplib/scope_exit.h"

#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <iostream>
#include <random>

namespace po = boost::program_options;
int main(int argc, char* argv[])
{
	vs::ASIOThreadPool atp(1);
	atp.Start();
	VS_SCOPE_EXIT{
		atp.Stop();
	};
	test::InitRegistry();

	chat::SetUUIDGeneratorFunc([]()
	{
		return boost::uuids::to_string(boost::uuids::random_generator()());
	});
	size_t users(0);
	size_t bs_count(0);
	size_t eps_count(0);
	size_t msgs_count(0);
	uint32_t max_chain_len(0);
	uint32_t bucket_capacity(0);
	uint32_t tail_length(0);

	po::options_description desc("Allowed options", 1000);
	desc.add_options()
		("help", "produce help message")
		("genchat", "generate chat with db for each endpoint")
		("users,u", po::value<size_t>(&users)->default_value(3),
			"count of users")
		("ep", po::value<size_t>(&eps_count)->default_value(3),
		"count of endpoints for each user")
		("bs", po::value<size_t>(&bs_count)->default_value(1),
			"count of bs")
		("msg,m", po::value<size_t>(&msgs_count)->default_value(10),
			"count of messages from each user")
		("chain_len,l", po::value<uint32_t>(&max_chain_len)->default_value(100),
			"default len of messages chain")
		("bucket_capacity", po::value<uint32_t>(&bucket_capacity)->default_value(5),
			"bucket capacity")
		("tail_len", po::value<uint32_t>(&tail_length)->default_value(0x10),
			"length of tail")
		("clean","delete old data bases if exist")
		;
	auto usage = [&]()
	{
		std::cout << desc;
	};
	po::variables_map vm;
	try
	{
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
	}
	catch (std::exception&)
	{
		usage();
		return -1;
	}
	if (vm.count("help") > 0)
	{
		usage();
		return 1;
	}
	if (vm.count("genchat") > 0)
	{
		ChatUtilsEnvironment env(
			std::make_shared<TransportChannelRouter>(atp.get_io_service()),
			std::make_shared<ResolverStub>(),
			std::make_shared<steady_clock_wrapper>(),
			atp.get_io_service());
		auto chat_parts = GenerateChatParts(
			users,
			eps_count,
			bs_count,
			max_chain_len,
			bucket_capacity,
			tail_length,
			vm.count("clean")>0,
			env);
		GenerateChat(msgs_count, chat_parts);
		for (const auto& part : chat_parts)
		{
			for (const auto& ep : part.second)
			{
				ep->app_layer_->ShutDown();
			}
		}
		atp.Stop();
		return 0;
	}
	chat_sandbox();
	usage();
	return 0;
}
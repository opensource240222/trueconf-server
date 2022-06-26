#pragma once

#include "std/cpplib/event.h"
#include "std/cpplib/function.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

#include <atomic>

namespace test { namespace asio {

template <class Protocol>
class basic_resolver_mock
{
public:
	// Asio types
	using protocol_type = Protocol;
	using endpoint_type = typename Protocol::endpoint;
	using iterator = typename Protocol::resolver::iterator;
#if BOOST_VERSION >= 106600
	using results_type = typename Protocol::resolver::results_type;
#endif
	using query = typename Protocol::resolver::query;

public:
	// GMock interface
	using resolve_handler = vs::function<void (const boost::system::error_code& error, iterator iterator)>;

	template <class Callback>
	static void on_new(Callback&& cb)
	{
		new_callback() = std::forward<Callback>(cb);
	}

	void wait_resolve()
	{
		EXPECT_TRUE(resolve_event_.wait_for(std::chrono::seconds(1))) << "async_resolve() wasn't called in time.";
	}
	void complete_resolve(iterator it, const boost::system::error_code& ec = {})
	{
		EXPECT_TRUE(resolve_requested_.exchange(false, std::memory_order_acq_rel)) << "Test attempted to complete non-existent resolve request.";
		auto handler = std::move(resolve_handler_);
		ios_.post([handler, ec, it]() { handler(ec, it); });
	}

	// Asio interface
	// All throwing overloads are not implemented because we don't want crashes.
	// All synchronous calls are not implemented because we don't use them.

	explicit basic_resolver_mock(boost::asio::io_service& io_service)
		: ios_(io_service)
		, resolve_requested_(false)
		, resolve_event_(false)
	{
		using ::testing::_;
		using ::testing::InvokeWithoutArgs;

		ON_CALL(*this, cancel()).WillByDefault(InvokeWithoutArgs([this]() {
			if (resolve_requested_.load(std::memory_order_acquire))
				complete_resolve(iterator(), boost::asio::error::operation_aborted);
		}));
		ON_CALL(*this, async_resolve_mocked_fwd(_)).WillByDefault(InvokeWithoutArgs(&resolve_event_, &vs::event::set));
		ON_CALL(*this, async_resolve_mocked_rev(_)).WillByDefault(InvokeWithoutArgs(&resolve_event_, &vs::event::set));

		auto cb = std::move(new_callback());
		if (cb)
			cb(this);
	}

	MOCK_METHOD1_T(async_resolve_mocked_fwd, void(const query& q));
	template <typename ResolveHandler>
	void async_resolve(const query& q, ResolveHandler&& handler)
	{
		init_resolve(std::forward<ResolveHandler>(handler));
		async_resolve_mocked_fwd(q);
	}

	MOCK_METHOD1_T(async_resolve_mocked_rev, void(const endpoint_type& e));
	template <typename ResolveHandler>
	void async_resolve(const endpoint_type& e, ResolveHandler&& handler)
	{
		init_resolve(std::forward<ResolveHandler>(handler));
		async_resolve_mocked_rev(e);
	}

	MOCK_METHOD0_T(cancel, void());

	boost::asio::io_service& get_io_service()
	{
		return ios_;
	}

private:
	template <class ResolveHandler>
	void init_resolve(ResolveHandler&& handler)
	{
		EXPECT_FALSE(resolve_requested_.exchange(true, std::memory_order_acq_rel)) << "New asynchronous resolve is started before previous one was completed.";
		resolve_handler_ = std::forward<ResolveHandler>(handler);
	}

public:
	boost::asio::io_service& ios_;

private:
	std::atomic<bool> resolve_requested_;
	resolve_handler resolve_handler_;
	vs::event resolve_event_;

	// TODO: C++17: inline variable
	static vs::function<void (basic_resolver_mock*)>& new_callback()
	{
		static vs::function<void (basic_resolver_mock*)> value;
		return value;
	}
};

using tcp_resolver_mock = basic_resolver_mock<boost::asio::ip::tcp>;
using udp_resolver_mock = basic_resolver_mock<boost::asio::ip::udp>;

#if !(defined(_MSC_VER) && _MSC_VER < 1900)
static_assert(!std::is_copy_constructible<tcp_resolver_mock>::value, "!");
static_assert(!std::is_copy_assignable   <tcp_resolver_mock>::value, "!");
static_assert(!std::is_move_constructible<tcp_resolver_mock>::value, "!");
static_assert(!std::is_move_assignable   <tcp_resolver_mock>::value, "!");

static_assert(!std::is_copy_constructible<udp_resolver_mock>::value, "!");
static_assert(!std::is_copy_assignable   <udp_resolver_mock>::value, "!");
static_assert(!std::is_move_constructible<udp_resolver_mock>::value, "!");
static_assert(!std::is_move_assignable   <udp_resolver_mock>::value, "!");
#endif

}}

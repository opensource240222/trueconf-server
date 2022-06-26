#pragma once

#include "std/cpplib/event.h"
#include "std/cpplib/function.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

#include "std-generic/compat/memory.h"
#include <atomic>
#include <cstdint>
#include <queue>
#include <vector>

namespace test { namespace asio {

using connect_handler = vs::function<void (const boost::system::error_code&)>;
using read_handler    = vs::function<void (const boost::system::error_code&, std::size_t)>;
using write_handler   = vs::function<void (const boost::system::error_code&, std::size_t)>;
using mutable_buffers = std::vector<boost::asio::mutable_buffer>;
using const_buffers   = std::vector<boost::asio::const_buffer>;

template <class Protocol>
class basic_socket_mock : public boost::asio::socket_base
{
public:
	// Asio types
	using protocol_type = Protocol;
	using endpoint_type = typename Protocol::endpoint;
#if defined(_WIN32)
	using native_handle_type = uintptr_t;
#else
	using native_handle_type = int;
#endif

public:
	// GMock interface
	class impl
	{
	public:
		template <class Callback>
		static void on_new(Callback&& cb)
		{
			impl::new_callbacks().emplace(std::forward<Callback>(cb));
		}

		explicit impl(boost::asio::io_service& ios)
			: ios_(ios)
			, is_open_(false)
			, connect_requested_(false)
			, connect_event_(false)
			, read_requested_(false)
			, read_event_(false)
			, write_requested_(false)
			, write_event_(false)
		{
			using ::testing::_;
			using ::testing::InvokeWithoutArgs;

			ON_CALL(*this, open(_,_)).WillByDefault(InvokeWithoutArgs([this]() {
				is_open_ = true;
			}));
			ON_CALL(*this, cancel(_)).WillByDefault(InvokeWithoutArgs([this]() {
				if (connect_requested_.load(std::memory_order_acquire))
					complete_connect(boost::asio::error::operation_aborted);
				if (read_requested_.load(std::memory_order_acquire))
					complete_read(0, boost::asio::error::operation_aborted);
				if (write_requested_.load(std::memory_order_acquire))
					complete_write(0, boost::asio::error::operation_aborted);
			}));
			ON_CALL(*this, close(_)).WillByDefault(InvokeWithoutArgs([this]() {
				is_open_ = false;
				if (connect_requested_.load(std::memory_order_acquire))
					complete_connect(boost::asio::error::operation_aborted);
				if (read_requested_.load(std::memory_order_acquire))
					complete_read(0, boost::asio::error::operation_aborted);
				if (write_requested_.load(std::memory_order_acquire))
					complete_write(0, boost::asio::error::operation_aborted);
			}));
			ON_CALL(*this, async_connect(_)).WillByDefault(InvokeWithoutArgs(&connect_event_, &vs::event::set));
			ON_CALL(*this, async_receive(_)).WillByDefault(InvokeWithoutArgs(&read_event_, &vs::event::set));
			ON_CALL(*this, async_receive_from(_,_)).WillByDefault(InvokeWithoutArgs(&read_event_, &vs::event::set));
			ON_CALL(*this, async_send(_)).WillByDefault(InvokeWithoutArgs(&write_event_, &vs::event::set));
			ON_CALL(*this, async_send_to(_,_)).WillByDefault(InvokeWithoutArgs(&write_event_, &vs::event::set));

			auto& cbs = new_callbacks();
			if (!cbs.empty())
			{
				cbs.front()(this);
				cbs.pop();
			}
		}

		MOCK_METHOD1_T(async_connect, void(const endpoint_type& peer_endpoint));
		MOCK_METHOD1_T(async_receive, void(socket_base::message_flags flags));
		MOCK_METHOD2_T(async_receive_from, void(endpoint_type& sender_endpoint, socket_base::message_flags flags));
		MOCK_METHOD1_T(async_send, void(socket_base::message_flags flags));
		MOCK_METHOD2_T(async_send_to, void(const endpoint_type& destination, socket_base::message_flags flags));
		MOCK_CONST_METHOD1_T(at_mark, bool(boost::system::error_code& ec));
		MOCK_CONST_METHOD1_T(available, std::size_t(boost::system::error_code& ec));
		MOCK_METHOD2_T(bind, void(const endpoint_type& endpoint, boost::system::error_code& ec));
		MOCK_METHOD1_T(cancel, void(boost::system::error_code& ec));
		MOCK_METHOD1_T(close, void(boost::system::error_code& ec));
		MOCK_CONST_METHOD1_T(local_endpoint, endpoint_type(boost::system::error_code& ec));
		MOCK_CONST_METHOD0_T(non_blocking, bool());
		MOCK_METHOD2_T(non_blocking, void(bool mode, boost::system::error_code& ec));
		MOCK_METHOD2_T(open, void(const protocol_type& protocol, boost::system::error_code& ec));
		MOCK_CONST_METHOD1_T(remote_endpoint, endpoint_type(boost::system::error_code& ec));
		MOCK_METHOD2_T(shutdown, void(shutdown_type what, boost::system::error_code& ec));

		void wait_connect()
		{
			EXPECT_TRUE(connect_event_.wait_for(std::chrono::seconds(1))) << "async_connect() wasn't called in time.";
		}
		void wait_read()
		{
			EXPECT_TRUE(read_event_.wait_for(std::chrono::seconds(1))) << "async_receive() wasn't called in time.";
		}
		void wait_write()
		{
			EXPECT_TRUE(write_event_.wait_for(std::chrono::seconds(1))) << "async_send() wasn't called in time.";
		}

		void complete_connect(const boost::system::error_code& ec = {})
		{
			EXPECT_TRUE(connect_requested_.exchange(false, std::memory_order_acq_rel)) << "Test attempted to complete non-existent connect request.";
			auto handler = std::move(connect_handler_);
			ios_.post([handler, ec]() { handler(ec); });
		}
		void complete_read(std::size_t bytes_transferred, const boost::system::error_code& ec = {})
		{
			EXPECT_TRUE(read_requested_.exchange(false, std::memory_order_acq_rel)) << "Test attempted to complete non-existent read request.";
			auto handler = std::move(read_handler_);
			ios_.post([handler, ec, bytes_transferred]() { handler(ec, bytes_transferred); });
		}
		void complete_write(std::size_t bytes_transferred, const boost::system::error_code& ec = {})
		{
			EXPECT_TRUE(write_requested_.exchange(false, std::memory_order_acq_rel)) << "Test attempted to complete non-existent write request.";
			auto handler = std::move(write_handler_);
			ios_.post([handler, ec, bytes_transferred]() { handler(ec, bytes_transferred); });
		}

	private:
		template <class ConnectHandler>
		void init_connect(ConnectHandler&& handler)
		{
			EXPECT_FALSE(connect_requested_.exchange(true, std::memory_order_acq_rel)) << "New asynchronous connect is started before previous one was completed.";
			connect_handler_ = std::forward<ConnectHandler>(handler);
		}
		template <class MutableBufferSequence, class ReadHandler>
		void init_read(const MutableBufferSequence& buffers, ReadHandler&& handler)
		{
			EXPECT_FALSE(read_requested_.exchange(true, std::memory_order_acq_rel)) << "New asynchronous read is started before previous one was completed.";
			read_handler_ = std::forward<ReadHandler>(handler);
			read_buffers_.clear();
			for (const auto& x : buffers)
				read_buffers_.emplace_back(x);
		}
		template <class ConstBufferSequence, class WriteHandler>
		void init_write(const ConstBufferSequence& buffers, WriteHandler&& handler)
		{
			EXPECT_FALSE(write_requested_.exchange(true, std::memory_order_acq_rel)) << "New asynchronous write is started before previous one was completed.";
			write_handler_ = std::forward<WriteHandler>(handler);
			write_buffers_.clear();
			for (const auto& x : buffers)
				write_buffers_.emplace_back(x);
		}

	public:
		boost::asio::io_service& ios_;
		bool is_open_;
		mutable_buffers read_buffers_;
		const_buffers write_buffers_;

	private:
		std::atomic<bool> connect_requested_;
		connect_handler connect_handler_;
		vs::event connect_event_;

		std::atomic<bool> read_requested_;
		read_handler read_handler_;
		vs::event read_event_;

		std::atomic<bool> write_requested_;
		write_handler write_handler_;
		vs::event write_event_;

		// TODO: C++17: inline variable
		static std::queue<vs::function<void (impl*)>>& new_callbacks()
		{
			static std::queue<vs::function<void (impl*)>> value;
			return value;
		}

		friend class basic_socket_mock;
	};
	std::unique_ptr<impl> impl_;

public:
	// Asio interface
	// All throwing overloads are not implemented because we don't want crashes.
	// All synchronous calls are not implemented because we don't use them.

	explicit basic_socket_mock(boost::asio::io_service& io_service)
		: impl_(vs::make_unique<impl>(io_service))
	{
	}
	/* Not implemented: those ctors call open() and bind(), but throwing calls are not implemented.
	basic_socket_mock(boost::asio::io_service& io_service, const protocol_type& protocol);
	basic_socket_mock(boost::asio::io_service& io_service, const endpoint_type& endpoint);
	*/
	/* Not implemented: we don't support native handles
	basic_socket_mock(boost::asio::io_service& io_service, const protocol_type& protocol, const native_handle_type& native_socket);
	*/
	basic_socket_mock(basic_socket_mock&& other)
		: impl_(std::move(other.impl_))
	{
	}
	/* Not implemented: ???
	template <typename Protocol1>
	basic_socket_mock(basic_socket_mock<Protocol1>&& other);
	*/

	basic_socket_mock& operator=(basic_socket_mock&& other)
	{
		impl_ = std::move(other.impl_);
		return *this;
	}
	/* Not implemented: ???
	template <typename Protocol1>
	basic_socket_mock& operator=(basic_socket_mock<Protocol1>&& other);
	*/

	/* Not implemented: we don't support native handles
	boost::system::error_code assign(const protocol_type& protocol, const native_handle_type& native_socket, boost::system::error_code& ec);
	*/

	template <typename ConnectHandler>
	void async_connect(const endpoint_type& peer_endpoint, ConnectHandler&& handler)
	{
		impl_->init_connect(std::forward<ConnectHandler>(handler));
		impl_->async_connect(peer_endpoint);
	}

	// TCP-only
	template <typename MutableBufferSequence, typename ReadHandler>
	void async_read_some(const MutableBufferSequence& buffers, ReadHandler&& handler)
	{
		impl_->init_read(buffers, std::forward<ReadHandler>(handler));
		impl_->async_receive(0);
	}

	template <typename MutableBufferSequence, typename ReadHandler>
	void async_receive(const MutableBufferSequence& buffers, ReadHandler&& handler)
	{
		impl_->init_read(buffers, std::forward<ReadHandler>(handler));
		impl_->async_receive(0);
	}

	template <typename MutableBufferSequence, typename ReadHandler>
	void async_receive(const MutableBufferSequence& buffers, socket_base::message_flags flags, ReadHandler&& handler)
	{
		impl_->init_read(buffers, std::forward<ReadHandler>(handler));
		impl_->async_receive(flags);
	}

	// UDP-only
	template <typename MutableBufferSequence, typename ReadHandler>
	void async_receive_from(const MutableBufferSequence& buffers, endpoint_type& sender_endpoint, ReadHandler&& handler)
	{
		impl_->init_read(buffers, std::forward<ReadHandler>(handler));
		impl_->async_receive_from(sender_endpoint, 0);
	}

	// UDP-only
	template <typename MutableBufferSequence, typename ReadHandler>
	void async_receive_from(const MutableBufferSequence& buffers, endpoint_type& sender_endpoint, socket_base::message_flags flags, ReadHandler&& handler)
	{
		impl_->init_read(buffers, std::forward<ReadHandler>(handler));
		impl_->async_receive_from(sender_endpoint, flags);
	}

	template <typename ConstBufferSequence, typename WriteHandler>
	void async_send(const ConstBufferSequence& buffers, WriteHandler&& handler)
	{
		impl_->init_write(buffers, std::forward<WriteHandler>(handler));
		impl_->async_send(0);
	}

	template <typename ConstBufferSequence, typename WriteHandler>
	void async_send(const ConstBufferSequence& buffers, socket_base::message_flags flags, WriteHandler&& handler)
	{
		impl_->init_write(buffers, std::forward<WriteHandler>(handler));
		impl_->async_send(flags);
	}

	// UDP-only
	template <typename ConstBufferSequence, typename WriteHandler>
	void async_send_to(const ConstBufferSequence& buffers, const endpoint_type& destination, WriteHandler&& handler)
	{
		impl_->init_write(buffers, std::forward<WriteHandler>(handler));
		impl_->async_send_to(destination, 0);
	}

	// UDP-only
	template <typename ConstBufferSequence, typename WriteHandler>
	void async_send_to(const ConstBufferSequence& buffers, const endpoint_type& destination, socket_base::message_flags flags, WriteHandler&& handler)
	{
		impl_->init_write(buffers, std::forward<WriteHandler>(handler));
		impl_->async_send_to(destination, flags);
	}

	// TCP-only
	template <typename ConstBufferSequence, typename WriteHandler>
	void async_write_some(const ConstBufferSequence& buffers, WriteHandler&& handler)
	{
		impl_->init_write(buffers, std::forward<WriteHandler>(handler));
		impl_->async_send(0);
	}

	bool at_mark(boost::system::error_code& ec) const
	{
		return impl_->at_mark(ec);
	}

	std::size_t available(boost::system::error_code& ec) const
	{
		return impl_->available(ec);
	}

	boost::system::error_code bind(const endpoint_type& endpoint, boost::system::error_code& ec)
	{
		impl_->bind(endpoint, ec);
		return ec;
	}

	boost::system::error_code cancel(boost::system::error_code& ec)
	{
		impl_->cancel(ec);
		return ec;
	}

	boost::system::error_code close(boost::system::error_code& ec)
	{
		impl_->close(ec);
		return ec;
	}

	boost::asio::io_service& get_io_service()
	{
		return impl_->ios_;
	}

	/* Not implemented: TODO
	template <typename GettableSocketOption>
	boost::system::error_code get_option(GettableSocketOption& option, boost::system::error_code& ec) const;
	*/

	/* Not implemented: TODO
	template <typename IoControlCommand>
	boost::system::error_code io_control(IoControlCommand & command, boost::system::error_code& ec);
	*/

	bool is_open() const
	{
		return impl_->is_open_;
	}

	endpoint_type local_endpoint(boost::system::error_code& ec) const
	{
		return impl_->local_endpoint(ec);
	}

	/* Not implemented: we don't have lowest_layer_type
	lowest_layer_type& lowest_layer();
	const lowest_layer_type& lowest_layer() const;
	*/

	native_handle_type native_handle()
	{
		// We don't support native handles but have to provide this function, so return invalid handle value to make code compile.
		return static_cast<native_handle_type>(-1);
	}

	/* Not implemented: we don't support native handles
	bool native_non_blocking() const;
	boost::system::error_code native_non_blocking(bool mode, boost::system::error_code& ec);
	*/

	bool non_blocking() const
	{
		return impl_->non_blocking();
	}

	boost::system::error_code non_blocking(bool mode, boost::system::error_code& ec)
	{
		impl_->non_blocking(mode, ec);
		return ec;
	}

	boost::system::error_code open(const protocol_type& protocol, boost::system::error_code& ec)
	{
		impl_->open(protocol, ec);
		return ec;
	}

	endpoint_type remote_endpoint(boost::system::error_code& ec) const
	{
		return impl_->remote_endpoint(ec);
	}

	/* Not implemented: TODO
	template <typename SettableSocketOption>
	boost::system::error_code set_option(const SettableSocketOption& option, boost::system::error_code& ec);
	*/

	boost::system::error_code shutdown(shutdown_type what, boost::system::error_code& ec)
	{
		impl_->shutdown_type(what, ec);
		return ec;
	}
};

using tcp_socket_mock = basic_socket_mock<boost::asio::ip::tcp>;
using udp_socket_mock = basic_socket_mock<boost::asio::ip::udp>;

#if !(defined(_MSC_VER) && _MSC_VER < 1900)
static_assert(!std::is_copy_constructible<tcp_socket_mock>::value, "!");
static_assert(!std::is_copy_assignable   <tcp_socket_mock>::value, "!");
static_assert( std::is_move_constructible<tcp_socket_mock>::value, "!");
static_assert( std::is_move_assignable   <tcp_socket_mock>::value, "!");

static_assert(!std::is_copy_constructible<udp_socket_mock>::value, "!");
static_assert(!std::is_copy_assignable   <udp_socket_mock>::value, "!");
static_assert( std::is_move_constructible<udp_socket_mock>::value, "!");
static_assert( std::is_move_assignable   <udp_socket_mock>::value, "!");
#endif

}}
